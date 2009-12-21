/**
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

class OCI {
public:
	int		_s;
	int		_cs;
	OCIValue	**_v;

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
	void stmt_bind_number(const int pos) {
		stmt_bind(pos, OCI_T_NUMBER);
	}
	void stmt_bind_date(const int pos) {
		stmt_bind(pos, OCI_T_DATE);
	}
	void stmt_bind_raw(const int pos) {
		stmt_bind(pos, OCI_T_RAW);
	}
	void stmt_bind_rowid(const int pos) {
		stmt_bind(pos, OCI_T_ROWID);
	}
	void stmt_bind_char(const int pos) {
		stmt_bind(pos, OCI_T_CHAR);
	}
	void stmt_bind_binary_float(const int pos) {
		stmt_bind(pos, OCI_T_BINARY_FLOAT);
	}
	void stmt_bind_binary_double(const int pos) {
		stmt_bind(pos, OCI_T_BINARY_DOUBLE);
	}
	void stmt_bind_timestamp(const int pos) {
		stmt_bind(pos, OCI_T_TIMESTAMP);
	}
	void stmt_bind_timestamp_with_tz(const int pos) {
		stmt_bind(pos, OCI_T_TIMESTAMP_WITH_TZ);
	}
	void stmt_bind_timestamp_with_ltz(const int pos) {
		stmt_bind(pos, OCI_T_TIMESTAMP_WITH_LTZ);
	}
	void stmt_bind_interval_y_to_m(const int pos) {
		stmt_bind(pos, OCI_T_INTERVAL_Y_TO_M);
	}
	void stmt_bind_interval_d_to_s(const int pos) {
		stmt_bind(pos, OCI_T_INTERVAL_D_TO_S);
	}
	void stmt_bind_urowid(const int pos) {
		stmt_bind(pos, OCI_T_UROWID);
	}

	/* define specific handle */
	void stmt_define_number(const int pos) {
		stmt_define(pos, OCI_T_NUMBER);
	}
	void stmt_define_date(const int pos) {
		stmt_define(pos, OCI_T_DATE);
	}
	void stmt_define_raw(const int pos) {
		stmt_define(pos, OCI_T_RAW);
	}
	void stmt_define_rowid(const int pos) {
		stmt_define(pos, OCI_T_ROWID);
	}
	void stmt_define_char(const int pos) {
		stmt_define(pos, OCI_T_CHAR);
	}
	void stmt_define_binary_float(const int pos) {
		stmt_define(pos, OCI_T_BINARY_FLOAT);
	}
	void stmt_define_binary_double(const int pos) {
		stmt_define(pos, OCI_T_BINARY_DOUBLE);
	}
	void stmt_define_timestamp(const int pos) {
		stmt_define(pos, OCI_T_TIMESTAMP);
	}
	void stmt_define_timestamp_with_tz(const int pos) {
		stmt_define(pos, OCI_T_TIMESTAMP_WITH_TZ);
	}
	void stmt_define_timestamp_with_ltz(const int pos) {
		stmt_define(pos, OCI_T_TIMESTAMP_WITH_LTZ);
	}
	void stmt_define_interval_y_to_m(const int pos) {
		stmt_define(pos, OCI_T_INTERVAL_Y_TO_M);
	}
	void stmt_define_interval_d_to_s(const int pos) {
		stmt_define(pos, OCI_T_INTERVAL_D_TO_S);
	}
	void stmt_define_urowid(const int pos) {
		stmt_define(pos, OCI_T_UROWID);
	}

	static unsigned int PORT;
	static unsigned int DFLT_SIZE;

	char * get_value(const int pos);
	int get_value_number(const int pos);

private:
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

	int check(void *handle, int type);
	void check_env() { check(_env, OCI_HTYPE_ENV); }
	void check_err() { check(_err, OCI_HTYPE_ERROR); }
	DISALLOW_COPY_AND_ASSIGN(OCI);
};

} /* namespace::vos */

#endif
