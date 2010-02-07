/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_OCI_HPP
#define	_LIBVOS_OCI_HPP	1

#include "OCIValue.hpp"

namespace vos {

enum _oci_errcode {
	E_OCI_OK			= 0,
	E_OCI_SUCC_WITH_INFO,
	E_OCI_NEED_DATA,
	E_OCI_NO_DATA,
	E_OCI_NULL_HNDL,		/* 4 */
	E_OCI_INVLD_HNDL,
	E_OCI_STILL_EXEC,
	E_OCI_CONT
};

enum _oci_conn_status {
	OCI_STT_DISCONNECT	= 0,
	OCI_STT_CONNECTED,
	OCI_STT_LOGGED_IN
};

/**
 * @class			: OCI
 * @attr			:
 *	- PORT			: static, default Oracle database server.
 *	- DFLT_SIZE		: static, default buffer size for '_v'.
 *	- _s			: status of OCI call.
 *	- _cs			: OCI client connection status.
 *	- _v			: a return value or result set after executing
 *                                SQL query.
 *	- _spool_min		: static, minimum session pool.
 *	- _spool_max		: static, maximum session pool.
 *	- _spool_inc		: static, incremental value for session pool.
 *	- _stmt_cache_size	: static, maximum query to be cached by
 *                                session pool.
 *	- _spool_name_len	: length of session pool name.
 *	- _value_i		: current size of '_v' array.
 *	- _value_sz		: maximum size of '_v' array.
 *	- _env			: OCI Environment handle.
 *	- _err			: OCI Error handle.
 *	- _spool		: OCI Session Pool handle.
 *	- _session		: OCI service context handle.
 *	- _spool_name		: name of session pool.
 *	- _auth			: OCI Authority handle.
 *	- _stmt			: OCI Statement handle.
 * @desc			:
 *	module to talking to Oracle database server using Oracle Client
 *	Interface library.
 */
class OCI {
public:
	OCI();
	~OCI();
	int init();
	void create_env();
	void create_err();
	void connect(const char *hostname, const char *service_name,
			int port = PORT);
	void login(const char *username, const char *password);
	void stmt_describe(const char *stmt);
	void stmt_prepare(const char *stmt);
	void stmt_execute(const char *stmt = 0);
	int  stmt_fetch();
	void stmt_release();
	void logout();
	void disconnect();

	void stmt_new_value(const int pos, const int type);
	void stmt_bind(const int pos, const int type = OCI_T_VARCHAR);
	void stmt_define(const int pos, const int type = OCI_T_VARCHAR);

	/* bind specific handle */
	inline void stmt_bind_number(const int pos) {
		stmt_bind(pos, OCI_T_NUMBER);
	}
	inline void stmt_bind_date(const int pos) {
		stmt_bind(pos, OCI_T_DATE);
	}
	inline void stmt_bind_raw(const int pos) {
		stmt_bind(pos, OCI_T_RAW);
	}
	inline void stmt_bind_rowid(const int pos) {
		stmt_bind(pos, OCI_T_ROWID);
	}
	inline void stmt_bind_char(const int pos) {
		stmt_bind(pos, OCI_T_CHAR);
	}
	inline void stmt_bind_binary_float(const int pos) {
		stmt_bind(pos, OCI_T_BINARY_FLOAT);
	}
	inline void stmt_bind_binary_double(const int pos) {
		stmt_bind(pos, OCI_T_BINARY_DOUBLE);
	}
	inline void stmt_bind_timestamp(const int pos) {
		stmt_bind(pos, OCI_T_TIMESTAMP);
	}
	inline void stmt_bind_timestamp_with_tz(const int pos) {
		stmt_bind(pos, OCI_T_TIMESTAMP_WITH_TZ);
	}
	inline void stmt_bind_timestamp_with_ltz(const int pos) {
		stmt_bind(pos, OCI_T_TIMESTAMP_WITH_LTZ);
	}
	inline void stmt_bind_interval_y_to_m(const int pos) {
		stmt_bind(pos, OCI_T_INTERVAL_Y_TO_M);
	}
	inline void stmt_bind_interval_d_to_s(const int pos) {
		stmt_bind(pos, OCI_T_INTERVAL_D_TO_S);
	}
	inline void stmt_bind_urowid(const int pos) {
		stmt_bind(pos, OCI_T_UROWID);
	}

	/* define specific handle */
	inline void stmt_define_number(const int pos) {
		stmt_define(pos, OCI_T_NUMBER);
	}
	inline void stmt_define_date(const int pos) {
		stmt_define(pos, OCI_T_DATE);
	}
	inline void stmt_define_raw(const int pos) {
		stmt_define(pos, OCI_T_RAW);
	}
	inline void stmt_define_rowid(const int pos) {
		stmt_define(pos, OCI_T_ROWID);
	}
	inline void stmt_define_char(const int pos) {
		stmt_define(pos, OCI_T_CHAR);
	}
	inline void stmt_define_binary_float(const int pos) {
		stmt_define(pos, OCI_T_BINARY_FLOAT);
	}
	inline void stmt_define_binary_double(const int pos) {
		stmt_define(pos, OCI_T_BINARY_DOUBLE);
	}
	inline void stmt_define_timestamp(const int pos) {
		stmt_define(pos, OCI_T_TIMESTAMP);
	}
	inline void stmt_define_timestamp_with_tz(const int pos) {
		stmt_define(pos, OCI_T_TIMESTAMP_WITH_TZ);
	}
	inline void stmt_define_timestamp_with_ltz(const int pos) {
		stmt_define(pos, OCI_T_TIMESTAMP_WITH_LTZ);
	}
	inline void stmt_define_interval_y_to_m(const int pos) {
		stmt_define(pos, OCI_T_INTERVAL_Y_TO_M);
	}
	inline void stmt_define_interval_d_to_s(const int pos) {
		stmt_define(pos, OCI_T_INTERVAL_D_TO_S);
	}
	inline void stmt_define_urowid(const int pos) {
		stmt_define(pos, OCI_T_UROWID);
	}

	char * get_value(const int pos);
	int get_value_number(const int pos);

	static unsigned int PORT;
	static unsigned int DFLT_SIZE;

	int		_s;
	int		_cs;
	OCIValue	**_v;
private:
	OCI(const OCI&);
	void operator=(const OCI&);

	int check(void *handle, int type);
	inline void check_env() { check(_env, OCI_HTYPE_ENV); }
	inline void check_err() { check(_err, OCI_HTYPE_ERROR); }

	static int	_spool_min;
	static int	_spool_max;
	static int	_spool_inc;
	static int	_stmt_cache_size;

	int		_spool_name_len;
	int		_value_i;
	int		_value_sz;
	OCIEnv		*_env;
	OCIError	*_err;
	OCISPool	*_spool;
	OCISvcCtx	*_session;
	char		*_spool_name;
	OCIAuthInfo	*_auth;
	OCIStmt		*_stmt;
};

} /* namespace::vos */

#endif
