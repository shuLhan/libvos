/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_OCI_HPP
#define	_LIBVOS_OCI_HPP	1

#include "OCIValue.hpp"

namespace vos {

#define	OCI_DEF_PORT		1521
#define	OCI_DEF_ENV_MODE	(OCI_THREADED | OCI_OBJECT | OCI_EVENTS)

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

/**
 * @class			: OCI
 * @attr			:
 *	- PORT			: static, default Oracle database server.
 *	- DFLT_SIZE		: static, default buffer size for '_v'.
 *	- _stat			: status of OCI call.
 *	- _env_mode		: OCI environment mode.
 *	- _table_changes_n	: number of table changed after receiving
 *				notification.
 *	- _row_changes_n	: number of row changed after receiving
 *				notification.
 *	- _v			: a return value or result set after executing
 *                                SQL query.
 *	- _value_i		: current size of '_v' array.
 *	- _value_sz		: maximum size of '_v' array.
 *	- _env			: OCI Environment handle.
 *	- _err			: OCI Error handle.
 *	- _server		: OCI Server handle.
 *	- _session		: OCI Session handle.
 *	- _spool		: OCI Session Pool handle.
 *	- _service		: OCI service context handle.
 *	- _spool_name		: name of session pool.
 *	- _auth			: OCI Authority handle.
 *	- _stmt			: OCI Statement handle.
 *	- _subscr		: OCI Subscription handle, for database change
 *				notification.
 *	- _table_changes	: OCI collection for table changes.
 *	- _row_changes		: OCI collection for row changes.
 *
 *	- _spool_min		: static, minimum session pool.
 *	- _spool_max		: static, maximum session pool.
 *	- _spool_inc		: static, incremental value for session pool.
 *	- _stmt_cache_size	: static, maximum query to be cached by
 *                                session pool.
 *	- _spool_name_len	: length of session pool name.
 *
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

	int connect(const char *hostname, const char *service_name,
			int port = OCI_DEF_PORT);

	int create_session(const char *conn, int conn_len = 0);
	int create_session_pool(const char *conn, int conn_len = 0);

	int login(const char *username, const char *password,
			const char *conn = NULL);
	int login_new_session(const char *username, const char *password,
				const char *conn = NULL);
	int stmt_describe(const char *stmt);
	int stmt_prepare(const char *stmt);

	int stmt_subscribe(void *callback);
	void stmt_unsubscribe();

	int stmt_execute(const char *stmt = 0);
	int  stmt_fetch();
	void stmt_release();
	void logout();
	void disconnect();

	void stmt_new_value(const int pos, const int type);
	void stmt_bind(const int pos, const int type = OCI_T_VARCHAR);
	void stmt_define(const int pos, const int type = OCI_T_VARCHAR);

	int stmt_bind_cursor(const int pos);
	int cursor_define(const int pos, const int type = OCI_T_VARCHAR);
	int cursor_fetch();
	void cursor_release();

	void release_buffer();

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
	long get_value_number(const int pos);

	int get_notification_type(void *descriptor, unsigned int *type);

	int get_table_changes(void *descriptor);
	int get_table_descriptor(int index, void **table_desc);
	int get_table_operation(void *table_desc, unsigned int *operation);
	int get_table_name(void *table_desc, char **table_name);

	int get_row_changes(void *table_d);
	int get_row_change_descriptor(int index, void **rowd);
	int get_row_change_op(void *rowd, unsigned int *op);
	int get_row_change_id(void *rowd, char **rowid,
				unsigned int *rowid_len);

	int		_stat;
	int		_env_mode;
	int		_table_changes_n;
	int		_row_changes_n;
	OCIValue	**_v;

	int		_value_i;
	int		_value_sz;

	OCIEnv		*_env;
	OCIError	*_err;
	OCIServer	*_server;
	OCISession	*_session;
	OCISPool	*_spool;
	OCISvcCtx	*_service;
	OCIAuthInfo	*_auth;
	OCIStmt		*_stmt;
	OCIStmt		*_cursor;
	OCIBind		*_cursor_bind;
	OCISubscription	*_subscr;
	OCIColl		*_table_changes;
	OCIColl		*_row_changes;

private:
	OCI(const OCI&);
	void operator=(const OCI&);

	int check(void *handle, int type);

	inline int check_env() {
		return check(_env, OCI_HTYPE_ENV);
	}
	inline int check_err() {
		return check(_err, OCI_HTYPE_ERROR);
	}

	static int	_spool_min;
	static int	_spool_max;
	static int	_spool_inc;
	static int	_stmt_cache_size;
	static int	_spool_name_len;
	static char	*_spool_name;
	static Buffer	_spool_conn_name;
};

} /* namespace::vos */

#endif
