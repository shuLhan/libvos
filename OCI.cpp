/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "OCI.hpp"

namespace vos {

const char *_oci_errmsg[] = {
	"\0",
	"[OCI] Error: OCI success with info\n",
	"[OCI] Error: OCI need data\n",
	"[OCI] Error: OCI no data\n",
	"[OCI] Error: NULL handle, Unable to extract detailed diagnostic" \
		" information\n",
	"[OCI] Error: OCI invalid handle\n",
	"[OCI] Error: OCI still executing\n",
	"[OCI] Error: OCI continue\n"
};

unsigned int OCI::PORT		= 1521;
unsigned int OCI::DFLT_SIZE	= 8;

int OCI::_spool_min		= 0;
int OCI::_spool_max		= 8;
int OCI::_spool_inc		= 1;
int OCI::_stmt_cache_size	= 10;

/**
 * @method	: OCI::OCI
 * @desc	: OCI object constructor.
 */
OCI::OCI() :
	_s(0),
	_cs(OCI_STT_DISCONNECT),
	_v(NULL),
	_spool_name_len(0),
	_value_i(0),
	_value_sz(DFLT_SIZE),
	_env(NULL),
	_err(NULL),
	_spool(NULL),
	_session(NULL),
	_spool_name(NULL),
	_auth(NULL),
	_stmt(NULL)
{}

/**
 * @method	: OCI::~OCI
 * @desc	: OCI object destructor.
 */
OCI::~OCI()
{
/*
	if (_cs == OCI_STT_LOGGED_IN)
		logout();
	if (_cs == OCI_STT_CONNECTED)
		disconnect();
*/
}

/**
 * @method	: OCI::init
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: initialize OCI object.
 */
int OCI::init()
{
	_v = reinterpret_cast<OCIValue **> (calloc(_value_sz, sizeof(_v)));
	if (!_v) {
		return -1;
	}

	create_env();
	create_err();

	return 0;
}

/**
 * @method		: OCI::check
 * @param		:
 *	> handle	: pointer to OCIError object.
 *	> type		: type of error.
 * @return		:
 *	< 0		: success.
 *	< !0		: fail.
 * @desc		:
 *	function to check for error after calling one of the OCI functions.
 */
int OCI::check(void *handle, int type)
{
	char	*errmsg = 0;
	sb4	errcode = 0;

	switch (_s) {
	case OCI_SUCCESS:
		return 0;
	case OCI_SUCCESS_WITH_INFO:
		errmsg = const_cast<char *>
			(_oci_errmsg[E_OCI_SUCC_WITH_INFO]);
		return 0;
	case OCI_NEED_DATA:
		errmsg = const_cast<char *>
			(_oci_errmsg[E_OCI_NEED_DATA]);
		break;
	case OCI_NO_DATA:
		errmsg = const_cast<char *>
			(_oci_errmsg[E_OCI_NO_DATA]);
		break;
	case OCI_ERROR:
		if (handle) {
			errmsg = reinterpret_cast<char *> (calloc(2048,
							sizeof(errmsg)));
			OCIErrorGet(handle, 1, 0, &errcode, (text *) errmsg,
					2048, type);
		} else {
			errmsg = const_cast<char *>
				(_oci_errmsg[E_OCI_NULL_HNDL]);
		}
		break;
	case OCI_INVALID_HANDLE:
		errmsg = const_cast<char *>
			(_oci_errmsg[E_OCI_INVLD_HNDL]);
		break;
	case OCI_STILL_EXECUTING:
		errmsg = const_cast<char *>
			(_oci_errmsg[E_OCI_STILL_EXEC]);
		break;
	case OCI_CONTINUE:
		errmsg = const_cast<char *>
			(_oci_errmsg[E_OCI_CONT]);
		break;
	}

	return E_OCI;
}

/**
 * @method	: OCI::create_env
 * @desc	: create OCI Environment handle.
 */
void OCI::create_env()
{
	_s = OCIEnvCreate(&_env, OCI_THREADED, 0, 0, 0, 0, 0, 0);
	check_env();
}

/**
 * @method	: OCI::create_err
 * @desc	: create OCI Error handle.
 */
void OCI::create_err()
{
	_s = OCIHandleAlloc(_env, (void **) &_err, OCI_HTYPE_ERROR, 0, 0);
	check_env();
}

/**
 * @method		: OCI::connect
 * @param		:
 *	> hostname	: hostname or IP address of Oracle database.
 *	> service_name	: SID or service name of Oracle database.
 *	> port		: Oracle database port.
 * @desc		:
 *	create a connection to Oracle database identified by 'service_name' at
 *	'hostname':'port'. In case of success connection status '_cs' will be
 *	set to connected.
 */
void OCI::connect(const char *hostname, const char *service_name, int port)
{
	Buffer *conn = new Buffer();

	conn->aprint("//%s:%d/%s", hostname, port, service_name);

	_s = OCIHandleAlloc(_env, (void **) &_spool, OCI_HTYPE_SPOOL, 0, 0);
	check_env();

	_s = OCIAttrSet(_spool, OCI_HTYPE_SPOOL,
				&OCI::_stmt_cache_size, 0,
				OCI_ATTR_SPOOL_STMTCACHESIZE, _err);
	check_err();

	_spool_name = (char *) calloc(128, sizeof(_spool_name));

	_s = OCISessionPoolCreate(_env, _err, _spool, (OraText **) &_spool_name,
				(ub4 *) &_spool_name_len,
				(const OraText *) conn->_v, conn->_i,
				OCI::_spool_min, OCI::_spool_max,
				OCI::_spool_inc,
				(OraText *) "", 0,
				(OraText *) "", 0, OCI_SPC_STMTCACHE);
	check_err();

	if (LIBVOS_DEBUG)
		printf("[OCI] session pool name : %s\n", _spool_name);

	delete conn;

	_cs = OCI_STT_CONNECTED;
}

/**
 * @method		: OCI::login
 * @param		:
 *	> username	: name of user on Oracle database.
 *	> password	: identification for 'username'.
 * @desc		:
 *	login to Oracle database as user 'username' identified by 'password'.
 *	In case of success, connection status will be set to logged-in.
 */
void OCI::login(const char *username, const char *password)
{
	_s = OCIHandleAlloc(_env, (void **) &_auth, OCI_HTYPE_AUTHINFO, 0, 0);
	check_env();

	_s = OCIAttrSet(_auth, OCI_HTYPE_AUTHINFO, (void *) username,
			strlen(username), OCI_ATTR_USERNAME, _err);
	check_err();

	_s = OCIAttrSet(_auth, OCI_HTYPE_AUTHINFO, (void *) password,
			strlen(password), OCI_ATTR_PASSWORD, _err);
	check_err();

	_s = OCISessionGet(_env, _err, &_session, _auth, (OraText *) _spool_name,
				_spool_name_len, 0, 0, 0, 0, 0,
				OCI_SESSGET_SPOOL);
	check_err();

	_cs = OCI_STT_LOGGED_IN;
}

/**
 * @method	: OCI::stmt_describe
 * @param	:
 *	> stmt	: Oracle SQL query (DDL or DML).
 * @desc	:
 *	describe statement 'stmt' to get number of column in result set,
 *	including type, length, and width of column.
 */
void OCI::stmt_describe(const char *stmt)
{
	int		n_cols		= 0;
	int		dtype		= 0;
	int		col_name_len	= 0;
	int		col_width	= 0;
	int		char_semantics	= 0;
	char		*col_name	= 0;
	OCIParam	*parm		= 0;

	if (! stmt)
		return;

	_s = OCIHandleAlloc(_env, (void **) &_stmt, OCI_HTYPE_STMT, 0, 0);
	check_env();

	_s = OCIStmtPrepare(_stmt, _err, (OraText *) stmt, strlen(stmt),
				OCI_NTV_SYNTAX, OCI_DEFAULT);
	check_err();

	_s = OCIStmtExecute(_session, _stmt, _err, 0, 0, 0, 0, OCI_DEFAULT);
	check_err();

	_s = OCIAttrGet(_stmt, OCI_HTYPE_STMT, &n_cols, 0,
			OCI_ATTR_PARAM_COUNT, _err);
	check_err();

	for (int i = 1; i <= n_cols; i++) {
		_s = OCIParamGet(_stmt, OCI_HTYPE_STMT, _err, (void **) &parm,
					i);
		check_err();

		_s = OCIAttrGet(parm, OCI_DTYPE_PARAM, &dtype, 0,
				OCI_ATTR_DATA_TYPE, _err);
		check_err();

		_s = OCIAttrGet(parm, OCI_DTYPE_PARAM, &col_name,
				(ub4 *) &col_name_len, OCI_ATTR_NAME, _err);
		check_err();

		_s = OCIAttrGet(parm, OCI_DTYPE_PARAM, &char_semantics, 0,
				OCI_ATTR_CHAR_USED, _err);
		check_err();

		if (char_semantics) {
			_s = OCIAttrGet(parm, OCI_DTYPE_PARAM, &col_width, 0,
					OCI_ATTR_CHAR_SIZE, _err);
		} else {
			_s = OCIAttrGet(parm, OCI_DTYPE_PARAM, &col_width, 0,
					OCI_ATTR_DATA_SIZE, _err);
		}
		check_err();

		printf(" data type     : %d\n", dtype);
		printf(" column name   : %s\n", col_name);
		printf(" column length : %d\n", col_name_len);
		printf(" column width  : %d\n\n", col_width);
	}
}

/**
 * @method	: OCI::stmt_prepare
 * @param	:
 *	> stmt	: Oracle SQL query (DDL or DML).
 * @desc	: prepare SQL query 'stmt' for execution.
 */
void OCI::stmt_prepare(const char *stmt)
{
	_s = OCIStmtPrepare2(_session, &_stmt, _err, (const OraText *) stmt,
				strlen(stmt), 0, 0, OCI_NTV_SYNTAX,
				OCI_DEFAULT);
	check_err();
}

/**
 * @method	: OCI::stmt_execute
 * @param	:
 *	> stmt	: Oracle SQL query (DDL or DML).
 * @desec	: execute SQL query in 'stmt'.
 */
void OCI::stmt_execute(const char *stmt)
{
	int i = 0;
	int stmt_type = 0;

	/* prepare statement */
	if (stmt)
		stmt_prepare(stmt);

	_s = OCIAttrGet(_stmt, OCI_HTYPE_STMT, &stmt_type, 0,
			OCI_ATTR_STMT_TYPE, _err);
	check_err();

	if (stmt_type == OCI_STMT_SELECT)
		i = 0;
	else
		i = 1;

	_s = OCIStmtExecute(_session, _stmt, _err, i, 0, 0, 0,
				OCI_COMMIT_ON_SUCCESS);
	check_err();

	if (stmt_type != OCI_STMT_SELECT) {
		for (i = 1; i <= _value_i; i++)
			_v[i]->_i = Buffer::TRIM(_v[i]->_v, 0);
	}
}

/**
 * @method	: OCI::stmt_fetch
 * @return	:
 *	< 0	: success.
 *	< !0	: fail.
 * @desc	: get the next value from result set.
 */
int OCI::stmt_fetch()
{
	_s = OCIStmtFetch(_stmt, _err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
	if (_s == OCI_NO_DATA)
		return 1;

	check_err();

	for (int idx = 1; idx <= _value_i; idx++) {
		if (_v[idx])
			_v[idx]->_i = Buffer::TRIM(_v[idx]->_v, 0);
	}

	return 0;
}

/**
 * @method	: OCI::stmt_release
 * @desc	: release OCI Statement object.
 */
void OCI::stmt_release()
{
	_s = OCIStmtRelease(_stmt, _err, 0, 0, OCI_DEFAULT);

	check_err();
	_stmt		= 0;
	_value_i	= 0;
}

/**
 * @method	: OCI::logout
 * @desc	: logout from Oracle database.
 */
void OCI::logout()
{
	OCISessionRelease(_session, _err, 0, 0, OCI_DEFAULT);
	OCIHandleFree(_auth, OCI_HTYPE_AUTHINFO);
	_auth	= 0;
	_cs	= OCI_STT_CONNECTED;
}

/**
 * @method	: OCI::disconnect
 * @desc	: close database connection from server.
 */
void OCI::disconnect()
{
	_s = OCISessionPoolDestroy(_spool, _err, OCI_DEFAULT);
	check_err();

	_spool = 0;
	if (_spool_name) {
		free(_spool_name);
		_spool_name = 0;
	}

	_cs = OCI_STT_DISCONNECT;
}

/**
 * @method	: OCI::stmt_new_value
 * @param	:
 *	> pos	: position of value in array.
 *	> type	: type of value to create.
 * @desc	:
 *	initialize value of result set, at index 'pos' in array of '_v'.
 */
void OCI::stmt_new_value(const int pos, const int type)
{
	if (pos >= _value_sz) {
		_value_sz += DFLT_SIZE;
		_v = (OCIValue **) realloc(_v, _value_sz * sizeof(_v));
	}

	switch (type) {
	case OCI_T_VARCHAR:
		_v[pos] = OCIValue::INIT(pos);
		break;
	case OCI_T_NUMBER:
		_v[pos] = OCIValue::NUMBER(pos);
		break;
	case OCI_T_DATE:
		_v[pos] = OCIValue::DATE(pos);
		break;
	case OCI_T_RAW:
		_v[pos] = OCIValue::RAW(pos);
		break;
	case OCI_T_ROWID:
		_v[pos] = OCIValue::ROWID(pos);
		break;
	case OCI_T_CHAR:
		_v[pos] = OCIValue::CHAR(pos);
		break;
	case OCI_T_BINARY_FLOAT:
		_v[pos] = OCIValue::BINARY_FLOAT(pos);
		break;
	case OCI_T_BINARY_DOUBLE:
		_v[pos] = OCIValue::BINARY_DOUBLE(pos);
		break;
	case OCI_T_TIMESTAMP:
		_v[pos] = OCIValue::TIMESTAMP(pos);
		break;
	case OCI_T_TIMESTAMP_WITH_TZ:
		_v[pos] = OCIValue::TIMESTAMP_WITH_TZ(pos);
		break;
	case OCI_T_TIMESTAMP_WITH_LTZ:
		_v[pos] = OCIValue::TIMESTAMP_WITH_LTZ(pos);
		break;
	case OCI_T_INTERVAL_Y_TO_M:
		_v[pos] = OCIValue::INTERVAL_Y_TO_M(pos);
		break;
	case OCI_T_INTERVAL_D_TO_S:
		_v[pos] = OCIValue::INTERVAL_D_TO_S(pos);
		break;
	case OCI_T_UROWID:
		_v[pos] = OCIValue::UROWID(pos);
		break;
	default:
		fprintf(stderr, "[LIBVOS] OCI error: unknow type %d!\n",
				type);
		return;
	}

	if (pos > _value_i)
		_value_i = pos;
	_v[pos]->_p = pos;
}

/**
 * @method	: OCI::stmt_bind
 * @param	:
 *	> pos	: position of value to bind to.
 *	> type	: type of value.
 * @desc	:
 *	bind variable '_v[pos]' to statement object to query the result later,
 *	after executing the statement.
 */
void OCI::stmt_bind(const int pos, const int type)
{
	stmt_new_value(pos, type);

	_s = OCIBindByPos(_stmt, &_v[pos]->_bind, _err, pos, _v[pos]->_v,
				_v[pos]->_l - 1, SQLT_CHR, 0, 0, 0, 0, 0,
				OCI_DEFAULT);
	check_err();
}

/**
 * @method	: OCI::stmt_define
 * @param	:
 *	> pos	: position of value in statement.
 *	> type	: type of value.
 * @desc	:
 *	define a variable '_v[pos]' to statement object to query the result
 *	later, after executing the statement.
 */
void OCI::stmt_define(const int pos, const int type)
{
	stmt_new_value(pos, type);

	_s = OCIDefineByPos(_stmt, &_v[pos]->_define, _err, pos,
				_v[pos]->_v, _v[pos]->_l - 1,
				SQLT_CHR, 0, 0, 0,
				OCI_DEFAULT);
	check_err();
}

/**
 * @method	: OCI::get_value
 * @param	:
 *	> pos	: position of value in statement.
 * @return	:
 *	< v	: success.
 *	< 0	: fail.
 * @desc	: get a value of result set at column 'pos' as string.
 */
char * OCI::get_value(const int pos)
{
	if (pos > _value_i || !_v[pos])
		return 0;

	if (_v[pos]->_p != pos)
		return 0;

	return _v[pos]->_v;
}

/**
 * @method	: OCI::get_value_number
 * @param	:
 *	> pos	: position of value in statement.
 * @return	:
 *	< v	: success.
 *	< 0	: fail.
 * @desc	: get a value of result set at column 'pos' as number.
 */
int OCI::get_value_number(const int pos)
{
	if (pos > _value_i || ! _v[pos])
		return 0;

	if (_v[pos]->_p != pos)
		return 0;

	return strtol(_v[pos]->_v, 0, 0); 
}

} /* namespace::vos */
