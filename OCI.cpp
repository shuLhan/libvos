/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 *	- ranggaws@gmail.com
 */

#include "OCI.hpp"

namespace vos {

const char *_oci_errmsg[] = {
	"\0"
,	"[vos::OCI_____] Error: OCI success with info\n"
,	"[vos::OCI_____] Error: OCI need data\n"
,	"[vos::OCI_____] Error: OCI no data\n"
,	"[vos::OCI_____] Error: NULL handle, unable to extract detailed "\
	" diagnostic information\n"
,	"[vos::OCI_____] Error: OCI invalid handle\n"
,	"[vos::OCI_____] Error: OCI still executing\n"
,	"[vos::OCI_____] Error: OCI continue\n"
};

int	OCI::_spool_min		= 0;
int	OCI::_spool_max		= 8;
int	OCI::_spool_inc		= 1;
int	OCI::_stmt_cache_size	= 10;
int	OCI::_spool_name_len	= 0;
char*	OCI::_spool_name	= NULL;
Buffer	OCI::_spool_conn_name;

/**
 * @method	: OCI::OCI
 * @desc	: OCI object constructor.
 */
OCI::OCI() :
	_stat(0)
,	_env_mode(OCI_DEF_ENV_MODE)
,	_table_changes_n(0)
,	_row_changes_n(0)
,	_v(NULL)
,	_value_i(0)
,	_value_sz(Buffer::DFLT_SIZE)
,	_lock()
,	_env(NULL)
,	_err(NULL)
,	_server(NULL)
,	_session(NULL)
,	_spool(NULL)
,	_service(NULL)
,	_auth(NULL)
,	_stmt(NULL)
,	_cursor(NULL)
,	_cursor_bind(NULL)
,	_subscr(NULL)
,	_table_changes(NULL)
,	_row_changes(NULL)
{}

/**
 * @method	: OCI::~OCI
 * @desc	: OCI object destructor.
 */
OCI::~OCI()
{
	lock();

	cursor_release();
	stmt_release();

	logout();
	disconnect();

	if (_subscr) {
		stmt_unsubscribe();
	}
	if (_err) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[OCI] free error handle\n");
		}
		OCIHandleFree(_err, OCI_HTYPE_ERROR);
		_err = 0;
	}
	if (_env) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[OCI] free environment handle\n");
		}
		OCIHandleFree(_env, OCI_HTYPE_ENV);
		_env = 0;
	}

	if (_v) {
		free(_v);
		_v = 0;
	}

	unlock();

	pthread_mutex_destroy(&_lock);
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
	_v = (OCIValue**) calloc(_value_sz, sizeof(OCIValue *));
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
int OCI::check(void* handle, int type)
{
	char*	errmsg	= 0;
	sb4	errcode	= 0;
	int	s	= 0;

	switch (_stat) {
	case OCI_SUCCESS:
		return 0;
	case OCI_SUCCESS_WITH_INFO:
		errmsg = (char*)_oci_errmsg[E_OCI_SUCC_WITH_INFO];
		break;
	case OCI_NEED_DATA:
		errmsg = (char*)_oci_errmsg[E_OCI_NEED_DATA];
		break;
	case OCI_NO_DATA:
		errmsg = (char*)_oci_errmsg[E_OCI_NO_DATA];
		break;
	case OCI_ERROR:
		s = -1;
		if (handle) {
			errmsg = (char*)calloc(2048, sizeof(errmsg));
			OCIErrorGet(handle, 1, 0, &errcode, (text *) errmsg
					, 2048, type);
		} else {
			errmsg = (char*)_oci_errmsg[E_OCI_NULL_HNDL];
		}
		break;
	case OCI_INVALID_HANDLE:
		errmsg	= (char *)_oci_errmsg[E_OCI_INVLD_HNDL];
		s	= -1;
		break;
	case OCI_STILL_EXECUTING:
		errmsg	= (char *)_oci_errmsg[E_OCI_STILL_EXEC];
		s	= -1;
		break;
	case OCI_CONTINUE:
		errmsg	= (char *)_oci_errmsg[E_OCI_CONT];
		s	= -1;
		break;
	}

	fprintf(stderr, "[vos::OCI_____] check: status '%d' code '%d'\n"
		"  ERROR: %s\n", _stat, errcode, errmsg);

	return s;
}

/**
 * @method	: OCI::create_env
 * @desc	: create OCI Environment handle.
 */
void OCI::create_env()
{
	if (LIBVOS_DEBUG) {
		fprintf(stderr, "[OCI] create environment handle\n");
	}

	_stat = OCIEnvCreate(&_env, _env_mode, 0, 0, 0, 0, 0, 0);
	check_env();
}

/**
 * @method	: OCI::create_err
 * @desc	: create OCI Error handle.
 */
void OCI::create_err()
{
	if (LIBVOS_DEBUG) {
		fprintf(stderr, "[OCI] create error handle\n");
	}
	_stat = OCIHandleAlloc(_env, (void **) &_err, OCI_HTYPE_ERROR, 0, 0);
	check_env();
}

/**
 * @method		: OCI::connect
 * @param		:
 *	> hostname	: hostname or IP address of Oracle database.
 *	> service_name	: SID or service name of Oracle database.
 *	> port		: Oracle database port.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	create a connection to Oracle database identified by 'service_name' at
 *	'hostname':'port'. In case of success connection status '_cs' will be
 *	set to connected.
 */
int OCI::connect(const char* hostname, const char* service_name, int port)
{
	register int	s;
	Buffer		conn;

	s = conn.aprint("//%s:%d/%s", hostname, port, service_name);
	if (s < 0) {
		return -1;
	}

	s = create_session_pool(conn._v, conn._i);

	return s;
}

/**
 * @method		: OCI::create_session
 * @param		:
 *	> conn		: connection string, i.e: "//ip.ad.dr.es/SID".
 *	> conn_len	: length of connection string 'conn'.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 */
int OCI::create_session(const char *conn, int conn_len)
{
	if (!conn) {
		return -1;
	}

	register int s = 0;

	_stat = OCIHandleAlloc(_env, (void**) &_server, OCI_HTYPE_SERVER, 0, 0);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	if (!conn_len) {
		conn_len = (int) strlen(conn);
	}

	_stat = OCIServerAttach(_server, _err, (text *) conn, conn_len
				, OCI_DEFAULT);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	_stat = OCIHandleAlloc(_env, (void**) &_service, OCI_HTYPE_SVCCTX, 0, 0);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	_stat = OCIAttrSet(_service, OCI_HTYPE_SVCCTX, (void *) _server, 0
				, OCI_ATTR_SERVER, _err);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	_stat = OCIHandleAlloc(_env, (void **) &_session, OCI_HTYPE_SESSION
				, 0, 0);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	return 0;
}

/**
 * @method		: OCI::create_session_pool
 * @param		:
 *	> conn		: Oracle connection string.
 *	> conn_len	: length of 'conn' string (default to zero).
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	create a connection to Oracle database, identified by connection
 *	string 'conn' with format: '//hostname:port/service-name'.
 */
int OCI::create_session_pool(const char* conn, int conn_len)
{
	if (!conn) {
		return -1;
	}

	register int s;

	if (!_env) {
		s = init();
		if (s < 0) {
			return -1;
		}
	}

	_stat = OCIHandleAlloc(_env, (void**) &_spool, OCI_HTYPE_SPOOL, 0, 0);
	s = check_env();
	if (s < 0) {
		return s;
	}

	_stat = OCIAttrSet(_spool, OCI_HTYPE_SPOOL, &OCI::_stmt_cache_size, 0
				, OCI_ATTR_SPOOL_STMTCACHESIZE, _err);
	s = check_err();
	if (s < 0) {
		return s;
	}

	_spool_name = (char*) calloc(128, sizeof(_spool_name));
	if (!_spool_name) {
		return -1;
	}

	if (0 == conn_len) {
		conn_len = (int) strlen(conn);
	}

	_stat = OCISessionPoolCreate(_env, _err, _spool
				, (OraText**) &_spool_name
				, (ub4 *) &_spool_name_len
				, (const OraText *) conn, conn_len
				, OCI::_spool_min, OCI::_spool_max
				, OCI::_spool_inc
				, (OraText *) "", 0
				, (OraText *) "", 0
				, OCI_SPC_STMTCACHE);
	s = check_err();
	if (s < 0) {
		return s;
	}

	_spool_conn_name.copy_raw(conn);

	if (LIBVOS_DEBUG) {
		printf(	"[vos::OCI_____] create_session_pool:\n"
			"  connected to      : %s\n"
			"  session pool name : %s\n"
			, _spool_conn_name._v, _spool_name);
	}

	return 0;
}

/**
 * @method		: OCI::login
 * @param		:
 *	> username	: name of user on Oracle database.
 *	> password	: identification for 'username'.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	login to Oracle database as user 'username' identified by 'password'.
 *	In case of success, connection status will be set to logged-in.
 */
int OCI::login(const char* username, const char* password, const char* conn)
{
	if (!username || !password) {
		return -1;
	}

	register int s;

	if (!_env) {
		s = init();
		if (s < 0) {
			return -1;
		}
	}

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] login: create authentication handle\n");
	}

	_stat = OCIHandleAlloc(_env, (void **) &_auth, OCI_HTYPE_AUTHINFO, 0,
				NULL);
	s = check_env();
	if (s < 0) {
		return -1;
	}

	_stat = OCIAttrSet(_auth, OCI_HTYPE_AUTHINFO, (void *) username
			, (unsigned int) strlen(username), OCI_ATTR_USERNAME
			, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	_stat = OCIAttrSet(_auth, OCI_HTYPE_AUTHINFO, (void *) password
			, (unsigned int) strlen(password), OCI_ATTR_PASSWORD
			, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] login: get session\n");
	}

	_stat = OCISessionGet(_env, _err, &_service, _auth, (OraText *) conn
				, (unsigned int) strlen(conn), NULL, 0, NULL
				, NULL, NULL, OCI_DEFAULT);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	/* database lock */
	pthread_mutex_init(&_lock, NULL);

	return 0;
}

int OCI::login_new_session(const char* username, const char* password
				, const char* conn)
{
	register int s;

	s = init();
	if (s != 0) {
		return -1;
	}

	s = create_session(conn);
	if (s != 0) {
		return -1;
	}

	/* set user name */
	_stat = OCIAttrSet(_session, OCI_HTYPE_SESSION, (text *) username
				, (unsigned int) strlen(username)
				, OCI_ATTR_USERNAME, _err);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	/* set user password */
	_stat = OCIAttrSet(_session, OCI_HTYPE_SESSION, (text *) password
				, (unsigned int) strlen(password)
				, OCI_ATTR_PASSWORD, _err);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	/* start new session */
	_stat = OCISessionBegin(_service, _err, _session, OCI_CRED_RDBMS
				, OCI_DEFAULT);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	_stat = OCIAttrSet(_service, OCI_HTYPE_SVCCTX, _session, 0
				, OCI_ATTR_SESSION, _err);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	return 0;
}

/**
 * @method	: OCI::get_new_session
 * @param	:
 *	> db	: pointer to OCI object.
 *	> user	: user for database connection.
 *	> pass	: user identification for database connection.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 */
int OCI::get_new_session(OCI* db, const char* user, const char* pswd)
{
	int s;

	if (!db) {
		printf("[OCI_____] OCI* is null\n");
		return -1;
	}

	if (!db->_v) {
		db->_v = (OCIValue**) calloc(_value_sz, sizeof(OCIValue *));
		if (!db->_v) {
			return -1;
		}
	}

	db->_env = _env;
	db->_err = _err;

	_stat = OCIHandleAlloc(_env, (void **) &db->_auth, OCI_HTYPE_AUTHINFO
				, 0, NULL);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	_stat = OCIAttrSet(db->_auth, OCI_HTYPE_AUTHINFO, (void *) user
			, (unsigned int) strlen(user), OCI_ATTR_USERNAME, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	_stat = OCIAttrSet(db->_auth, OCI_HTYPE_AUTHINFO, (void *) pswd
			, (unsigned int) strlen(pswd), OCI_ATTR_PASSWORD, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] get_new_session\n");
	}

	_stat = OCISessionGet(_env, _err, &db->_service, db->_auth
			, (OraText *) _spool_name, _spool_name_len
			, NULL, 0, NULL, NULL, NULL
			, OCI_SESSGET_SPOOL);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	return 0;
}

/**
 * @method	: OCI::stmt_describe
 * @param	:
 *	> stmt	: Oracle SQL query (DDL or DML).
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *	describe statement 'stmt' to get number of column in result set,
 *	including type, length, and width of column.
 */
int OCI::stmt_describe(const char* stmt)
{
	register int	s		= 0;
	int		n_cols		= 0;
	int		dtype		= 0;
	int		col_name_len	= 0;
	int		col_width	= 0;
	int		char_semantics	= 0;
	char*		col_name	= 0;
	OCIParam*	parm		= 0;

	if (! stmt) {
		return 0;
	}

	_stat = OCIHandleAlloc(_env, (void **) &_stmt, OCI_HTYPE_STMT, 0, 0);
	s = check_env();
	if (s < 0) {
		return s;
	}

	_stat = OCIStmtPrepare(_stmt, _err, (OraText *) stmt
				, (unsigned int) strlen(stmt)
				, OCI_NTV_SYNTAX, OCI_DEFAULT);
	s = check_err();
	if (s < 0) {
		return s;
	}

	_stat = OCIStmtExecute(_service, _stmt, _err, 0, 0, 0, 0, OCI_DEFAULT);
	s = check_err();
	if (s) {
		return s;
	}

	_stat = OCIAttrGet(_stmt, OCI_HTYPE_STMT, &n_cols, 0,
			OCI_ATTR_PARAM_COUNT, _err);
	s = check_err();
	if (s < 0) {
		return s;
	}

	for (int i = 1; i <= n_cols; i++) {
		_stat = OCIParamGet(_stmt, OCI_HTYPE_STMT, _err, (void **) &parm,
					i);
		s = check_err();
		if (s < 0) {
			return s;
		}

		_stat = OCIAttrGet(parm, OCI_DTYPE_PARAM, &dtype, 0,
				OCI_ATTR_DATA_TYPE, _err);
		s = check_err();
		if (s < 0) {
			return s;
		}

		_stat = OCIAttrGet(parm, OCI_DTYPE_PARAM, &col_name,
				(ub4 *) &col_name_len, OCI_ATTR_NAME, _err);
		s = check_err();
		if (s < 0) {
			return s;
		}

		_stat = OCIAttrGet(parm, OCI_DTYPE_PARAM, &char_semantics, 0,
				OCI_ATTR_CHAR_USED, _err);
		s = check_err();
		if (s < 0) {
			return s;
		}

		if (char_semantics) {
			_stat = OCIAttrGet(parm, OCI_DTYPE_PARAM, &col_width, 0,
					OCI_ATTR_CHAR_SIZE, _err);
		} else {
			_stat = OCIAttrGet(parm, OCI_DTYPE_PARAM, &col_width, 0,
					OCI_ATTR_DATA_SIZE, _err);
		}
		s = check_err();
		if (s < 0) {
			return s;
		}

		if (LIBVOS_DEBUG) {
			printf(	"[vos::OCI_____] stmt_describe:\n"
				"  data type     : %d\n"
				"  column name   : %s\n"
				"  column length : %d\n"
				"  column width  : %d\n\n"
				, dtype, col_name, col_name_len, col_width);
		}
	}

	return 0;
}

/**
 * @method	: OCI::stmt_prepare
 * @param	:
 *	> stmt	: Oracle SQL query (DDL or DML).
 * @return	:
 *	> 0	: success.
 *	> -1	: fail.
 * @desc	: prepare SQL query 'stmt' for execution.
 */
int OCI::stmt_prepare(const char* stmt)
{
	if (!stmt) {
		return -1;
	}

	register int s;

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] stmt_prepare:\n%s\n", stmt);
	}

	_stat = OCIStmtPrepare2(_service, &_stmt, _err, (const OraText *) stmt
				, (unsigned int) strlen(stmt), 0, 0
				, OCI_NTV_SYNTAX, OCI_DEFAULT);
	s = check_err();
	if (s < 0) {
		unlock();
	}

	return s;
}

/**
 * @method	: OCI::stmt_prepare_r
 * @param	:
 *	> stmt	: Oracle SQL query (DDL or DML).
 * @return	:
 *	> 0	: success.
 *	> -1	: fail.
 * @desc	: prepare SQL query 'stmt' for execution.
 *
 * This is thread-safe version of stmt_prepare. If you use this method always
 * call stmt_release() after end of statement.
 */
int OCI::stmt_prepare_r(const char* stmt)
{
	if (!stmt) {
		return -1;
	}

	lock();

	return stmt_prepare(stmt);
}

int OCI::stmt_subscribe(void* callback)
{
	register int	s		= 0;
	unsigned int	type		= OCI_SUBSCR_NAMESPACE_DBCHANGE;
	int		get_rowid	= TRUE;
	int		timeout		= 0;

	_stat = OCIHandleAlloc(_env, (void **) &_subscr
				, OCI_HTYPE_SUBSCRIPTION, 0, 0);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	_stat = OCIAttrSet(_subscr, OCI_HTYPE_SUBSCRIPTION, (void *) &type
				, sizeof(ub4), OCI_ATTR_SUBSCR_NAMESPACE
				, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	/* Associate a notification callback */
	_stat = OCIAttrSet(_subscr, OCI_HTYPE_SUBSCRIPTION, callback, 0
				, OCI_ATTR_SUBSCR_CALLBACK, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	/* Allow extraction of rowid information */
	_stat = OCIAttrSet(_subscr, OCI_HTYPE_SUBSCRIPTION
				, (void *) &get_rowid, sizeof(ub4)
				, OCI_ATTR_CHNF_ROWIDS, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	/* Set a timeout value */
	_stat = OCIAttrSet(_subscr, OCI_HTYPE_SUBSCRIPTION,(void *) &timeout
				, 0, OCI_ATTR_SUBSCR_TIMEOUT, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	/* Create a new registration in the DBCHANGE namespace */
	_stat = OCISubscriptionRegister(_service, &_subscr, 1, _err
					, OCI_DEFAULT);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	/* Associate the statement with the subscription handle */
	s = OCIAttrSet(_stmt, OCI_HTYPE_STMT, _subscr, 0
			, OCI_ATTR_CHNF_REGHANDLE, _err);
	s = check_err();
	if (s < 0) {
		return -1;
	}

	return 0;
}

void OCI::stmt_unsubscribe()
{
	if (_subscr) {
		_stat = OCISubscriptionUnRegister(_service, _subscr, _err
						, OCI_DEFAULT);
		check_err();
	}
}

/**
 * @method	: OCI::stmt_execute
 * @param	:
 *	> stmt	: Oracle SQL query (DDL or DML).
 * @return	:
 *	> 0	: success.
 *	< <0	: fail.
 * @desc	: execute SQL query in 'stmt'.
 */
int OCI::stmt_execute(const char *stmt)
{
	register int	s		= 0;
	register int	i		= 0;
	int		stmt_type	= 0;

	if (stmt) {
		s = stmt_prepare(stmt);
		if (s < 0) {
			return s;
		}
	}

	_stat = OCIAttrGet(_stmt, OCI_HTYPE_STMT, &stmt_type, 0
				, OCI_ATTR_STMT_TYPE, _err);
	s = check_err();
	if (s < 0) {
		goto out;
	}

	if (stmt_type == OCI_STMT_SELECT) {
		i = 0;
	} else {
		i = 1;
	}

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] stmt_execute:\n%s\n", stmt? stmt:"-");
	}

	_stat = OCIStmtExecute(_service, _stmt, _err, i, 0, 0, 0
				, OCI_DEFAULT | OCI_COMMIT_ON_SUCCESS);
	s = check_err();
	if (s < 0) {
		goto out;
	}

	if (stmt_type != OCI_STMT_SELECT) {
		for (i = 1; i <= _value_i; i++) {
			if (_v[i]) {
				_v[i]->_i = Buffer::TRIM(_v[i]->_v, 0);
			}
		}
	}
out:
	if (stmt) {
		stmt_release();
	}
	return s;
}

int OCI::stmt_execute_r(const char* stmt)
{
	int s = 0;

	lock();
	s = stmt_execute(stmt);
	unlock();

	return s;
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
	register int s;

	for (int idx = 1; idx <= _value_i; idx++) {
		if (_v[idx]) {
			memset(_v[idx]->_v, '\0', _v[idx]->_l);
		}
	}

	_stat = OCIStmtFetch(_stmt, _err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
	if (_stat == OCI_NO_DATA) {
		return 1;
	}

	s = check_err();
	if (s < 0) {
		return s;
	}

	for (int idx = 1; idx <= _value_i; idx++) {
		if (_v[idx]) {
			_v[idx]->_i = Buffer::TRIM(_v[idx]->_v, 0);
		}
	}

	return 0;
}

/**
 * @method	: OCI::stmt_release
 * @desc	: release OCI Statement object.
 */
void OCI::stmt_release()
{
	if (LIBVOS_DEBUG) {
		fprintf(stderr, "[OCI] release statement\n");
	}

	if (_stmt) {
		_stat = OCIStmtRelease(_stmt, _err, NULL, 0, OCI_DEFAULT);
		check_err();
		_stmt = 0;
	}
	release_buffer();
}

/**
 * @method	: OCI::stmt_release_r
 * @desc	: release OCI Statement object.
 * This is thread-safe version of stmt_release(), use this version only if
 * you use stmt_prepare_r() at beginning of statement.
 */
void OCI::stmt_release_r()
{
	stmt_release();
	unlock();
}

/**
 * @method	: OCI::logout
 * @desc	: release connection back to session pool.
 */
void OCI::session_release()
{
	if (_service) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[OCI] free session\n");
		}
		OCISessionRelease(_service, _err, NULL, 0, OCI_DEFAULT);
		_service = 0;
	}
	if (_auth) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[OCI] free auth\n");
		}
		OCIHandleFree(_auth, OCI_HTYPE_AUTHINFO);
		_auth = 0;
	}
	_err = NULL;
	_env = NULL;
}

/**
 * @method	: OCI::logout
 * @desc	: logout from Oracle database.
 */
void OCI::logout()
{
	if (_service) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[OCI] free session\n");
		}
		OCISessionRelease(_service, _err, NULL, 0, OCI_DEFAULT);
		_service = 0;
	}
	if (_auth) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[OCI] free auth\n");
		}
		OCIHandleFree(_auth, OCI_HTYPE_AUTHINFO);
		_auth = 0;
	}
}

/**
 * @method	: OCI::disconnect
 * @desc	: close database connection from server.
 */
void OCI::disconnect()
{
	if (_spool) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[OCI] free session pool handle\n");
		}

		_stat = OCISessionPoolDestroy(_spool, _err, OCI_DEFAULT);
		check_err();
		_spool = 0;

		if (_spool_name) {
			_spool_name = 0;
		}
	}
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
		_value_sz += Buffer::DFLT_SIZE;
		_v = (OCIValue**) realloc(_v, _value_sz * sizeof(OCIValue*));
		if (!_v) {
			return;
		}

		memset(_v + _value_i + 1, 0,
			Buffer::DFLT_SIZE * sizeof(OCIValue*));
	}

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] stmt_new_value: new buffer at '%d'\n"
			, pos);
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
		fprintf(stderr
			, "[vos::OCI_____] stmt_new_value: unknown type '%d'!\n"
			, type);
		return;
	}

	if (pos > _value_i) {
		_value_i = pos;
	}
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

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] stmt_bind: type '%d' at '%d'\n", type
			, pos);
	}

	_stat = OCIBindByPos(_stmt, &_v[pos]->_bind, _err, pos, _v[pos]->_v
				, _v[pos]->_l - 1, SQLT_CHR, 0, 0, 0, 0, 0
				, OCI_DEFAULT);
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

	if (LIBVOS_DEBUG) {
		printf("[vos::OCI_____] stmt_define: type '%d' at '%d'\n"
			, type, pos);
	}

	_stat = OCIDefineByPos(_stmt, &_v[pos]->_define, _err, pos
				, _v[pos]->_v, _v[pos]->_l - 1
				, SQLT_CHR, 0, 0, 0
				, OCI_DEFAULT);
	check_err();
}

/**
 * @method	: OCI::stmt_bind_cursor
 * @param	:
 *	> pos	: position of value to bind to in statement.
 */
int OCI::stmt_bind_cursor(const int pos)
{
	int s = 0;

	_stat = OCIHandleAlloc(_env, (void **) &_cursor, OCI_HTYPE_STMT, 0
				, NULL);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	_stat = OCIBindByPos(_stmt, &_cursor_bind, _err, pos, &_cursor, 0
				, SQLT_RSET, 0, 0, NULL, 0, 0, OCI_DEFAULT);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	return 0;
}

int OCI::cursor_define(const int pos, const int type)
{
	int s = 0;

	stmt_new_value(pos, type);

	_stat = OCIDefineByPos(_cursor, &_v[pos]->_define, _err, pos
				, _v[pos]->_v, _v[pos]->_l - 1, SQLT_CHR
				, 0, 0, 0, OCI_DEFAULT);
	s = check_err();

	return s;
}

int OCI::cursor_fetch()
{
	int s = 0;

	for (int idx = 1; idx <= _value_i; idx++) {
		if (_v[idx]) {
			memset(_v[idx]->_v, '\0', _v[idx]->_l);
		}
	}

	_stat = OCIStmtFetch(_cursor, _err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
	if (_stat == OCI_NO_DATA) {
		return 1;
	}

	s = check_err();
	if (s < 0) {
		return s;
	}

	for (int idx = 1; idx <= _value_i; idx++) {
		if (_v[idx]) {
			_v[idx]->_i = Buffer::TRIM(_v[idx]->_v, 0);
		}
	}

	return 0;
}

void OCI::cursor_release()
{
	if (_cursor) {
		OCIHandleFree(_cursor, OCI_HTYPE_STMT);
		_cursor = 0;
	}
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
const char* OCI::get_value(const int pos)
{
	if (pos > _value_i || !_v[pos]) {
		return "\0";
	}
	if (_v[pos]->_p != pos) {
		return "\0";
	}
	return _v[pos]->v();
}

/**
 * @method	: OCI::get_value_number
 * @param	:
 *	> pos	: position of value in statement.
 * @return	:
 *	< number: in long integer format.
 * @desc	: get a value of result set at column 'pos' as number.
 */
long OCI::get_value_number(const int pos)
{
	if (pos > _value_i || ! _v[pos]) {
		return 0;
	}
	if (_v[pos]->_p != pos) {
		return 0;
	}
	return strtol(_v[pos]->_v, 0, 0); 
}

int OCI::get_notification_type(void* descriptor, unsigned int* type)
{
	register int s;

	_stat = OCIAttrGet(descriptor, OCI_DTYPE_CHDES, type, 0
				, OCI_ATTR_CHDES_NFYTYPE, _err);
	s = check_err();

	return s;
}

/**
 * @method		: OCI::get_table_changes
 * @param		:
 *	> descriptor	: descriptor from notification callback.
 * @return		:
 *	> n		: number of table changes in notification.
 * @desc		: get table changes from notification descriptor.
 */
int OCI::get_table_changes(void* descriptor)
{
	register int s;

	_table_changes		= NULL;
	_table_changes_n	= 0;

	_stat = OCIAttrGet(descriptor, OCI_DTYPE_CHDES, &_table_changes, 0
				, OCI_ATTR_CHDES_TABLE_CHANGES, _err);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	_stat = OCICollSize(_env, _err, _table_changes, &_table_changes_n);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	return _table_changes_n;
}

int OCI::get_table_descriptor(int index, void **table_desc)
{
	register int	s;
	boolean		exist		= 0;
	void**		table_desc_p	= NULL;
	void*		elemen_index	= NULL;

	_stat = OCICollGetElem(_env, _err, _table_changes, index
				, &exist, (void **) &table_desc_p
				, &elemen_index);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	(*table_desc) = *table_desc_p;

	return 0;
}

/**
 * @method		: OCI::get_table_operation
 * @param		:
 *	> table_desc	: pointer to table descriptor.
 *	> operation	: return value, database operation 'table_desc'.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	get type of operation of change on table descriptor 'table_desc'.
 */
int OCI::get_table_operation(void* table_desc , unsigned int* operation)
{
	register int s;

	_stat = OCIAttrGet(table_desc, OCI_DTYPE_TABLE_CHDES, operation, 0
				, OCI_ATTR_CHDES_TABLE_OPFLAGS, _err);
	s = check_err();

	return s;
}

int OCI::get_table_name(void* table_desc, char** table_name)
{
	register int s;

	_stat = OCIAttrGet(table_desc, OCI_DTYPE_TABLE_CHDES, table_name
				, 0, OCI_ATTR_CHDES_TABLE_NAME, _err);
	s = check_err();

	return s;
}

/**
 * @method		: OCI::get_row_changes
 * @param		:
 *	> table_desc	: pointer to table descriptor.
 * @return		:
 *	< >=0		: number of row changes on table 'table_desc'.
 *	< -1		: fail.
 * @desc		: get row changes.
 */
int OCI::get_row_changes(void* table_desc)
{
	register int s;

	_stat = OCIAttrGet(table_desc, OCI_DTYPE_TABLE_CHDES, &_row_changes
				, 0, OCI_ATTR_CHDES_TABLE_ROW_CHANGES, _err);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	_stat = OCICollSize(_env, _err, _row_changes, &_row_changes_n);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	return _row_changes_n;
}

/**
 * @method		: OCI::get_row_descriptor
 * @param		:
 *	> index		: index of row change.
 *	> row_desc	: return value, pointer to row descriptor.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: get row descriptor from row changes.
 */
int OCI::get_row_change_descriptor(int index, void** row_desc)
{
	register int	s;
	boolean		exist		= 0;
	void**		row_desc_p	= NULL;
	void*		element_idx	= NULL;

	/* get row descriptor change */
	_stat = OCICollGetElem(_env, _err, _row_changes, index, &exist
				, (void **) &row_desc_p, &element_idx);
	s = check_err();
	if (s != 0) {
		return -1;
	}

	(*row_desc) = *row_desc_p;

	return 0;
}

int OCI::get_row_change_op(void* rowd, unsigned int* op)
{
	register int s;

	_stat = OCIAttrGet(rowd, OCI_DTYPE_ROW_CHDES, (void *) op, 0
				, OCI_ATTR_CHDES_ROW_OPFLAGS, _err);
	s = check_err();

	return s;
}

int OCI::get_row_change_id(void* rowd, char** rowid, unsigned int* rowid_len)
{
	register int s;

	_stat = OCIAttrGet(rowd, OCI_DTYPE_ROW_CHDES, (void *) rowid
				, rowid_len, OCI_ATTR_CHDES_ROW_ROWID, _err);
	s = check_err();

	return s;
}

void OCI::release_buffer()
{
	if (!_v) {
		return;
	}
	for (int i = 0; i <= _value_i; i++) {
		if (!_v[i]) {
			continue;
		}
		if (LIBVOS_DEBUG) {
			printf("[vos::OCI_____] release_buffer: free at %d\n"
				, i);
		}
		_v[i]->reset();
		delete _v[i];
		_v[i] = 0;
	}
	_value_i = 0;
}

void OCI::lock()
{
	int s;

	do { s = pthread_mutex_trylock(&_lock); } while (s != 0);
}

void OCI::unlock()
{
	pthread_mutex_unlock(&_lock);
}

} /* namespace::vos */
