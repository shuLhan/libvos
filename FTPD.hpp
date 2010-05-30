/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FTP_DAEMON_HPP
#define	_LIBVOS_FTP_DAEMON_HPP	1

#include <signal.h>
#include "Dir.hpp"
#include "FTPClient.hpp"
#include "FTPUser.hpp"

namespace vos {

#define	FTPD_DEF_ADDRESS	"0.0.0.0"
#define	FTPD_DEF_PORT		21
#define	FTPD_DEF_PATH		NULL
#define	FTPD_DEF_PASV_PORT	5000

enum _FTP_reply_code {
	CODE_150 = 0
,	CODE_200
,	CODE_215
,	CODE_220
,	CODE_221
,	CODE_226
,	CODE_227
,	CODE_230
,	CODE_250
,	CODE_257
,	CODE_331
,	CODE_332
,	CODE_350
,	CODE_421
,	CODE_425
,	CODE_450
,	CODE_451
,	CODE_501
,	CODE_502
,	CODE_503
,	CODE_530
,	CODE_550
,	CODE_553
,	N_REPLY_CODE
};
extern const char* _FTP_reply_msg[N_REPLY_CODE];

enum _FTP_add_reply_msg {
	NODE_NOT_FOUND = 0
,	TYPE_ALWAYS_BIN
,	MODE_ALWAYS_STREAM
,	STRU_ALWAYS_FILE
,	N_ADD_REPLY_MSG
};
extern const char* _FTP_add_reply_msg[N_ADD_REPLY_MSG];

enum _FTP_ {
	AUTH_NOLOGIN = 0
,	AUTH_LOGIN
,	N_FTP_MODE
};

class FTPD : public Socket {
public:
	FTPD();
	~FTPD();

	int init(const char* address = FTPD_DEF_ADDRESS
		, const int port = FTPD_DEF_PORT
		, const char* path = FTPD_DEF_PATH
		, const int auth_mode = AUTH_NOLOGIN);
	int add_user(const char* name, const char* pass);
	int set_path(const char* path);
	void set_default_callback();
	int set_callback(const int type, void (*callback)(FTPD*, FTPClient*));
	int run();
	void client_process();
	void client_add(FTPClient* c);
	void client_del(FTPClient *c);

	int		_running;
	int		_auth_mode;
	int		_fds_max;
	int		_pasv_port_next;
	Buffer		_path;
	Dir		_dir;
	fd_set		_fds_all;
	fd_set		_fds_read;
	FTPClient*	_all_client;
	FTPUser*	_users;

	void (*_fcb[N_FTP_CMD])(FTPD*, FTPClient*);

	static int get_path_node(FTPD* server, FTPClient* c
				, Buffer* cmd_path
				, DirNode** node, Buffer* rpath
				, Buffer* dirname);

	static void on_accept(FTPD* server, FTPClient* client);
	static void on_cmd_USER(FTPD* server, FTPClient* client);
	static void on_cmd_PASS(FTPD* server, FTPClient* client);
	static void on_cmd_SYST(FTPD* server, FTPClient* client);
	static void on_cmd_TYPE(FTPD* server, FTPClient* client);
	static void on_cmd_MODE(FTPD* server, FTPClient* client);
	static void on_cmd_STRU(FTPD* server, FTPClient* client);
	
	static void on_cmd_CWD(FTPD* server, FTPClient* client);
	static void on_cmd_CDUP(FTPD* server, FTPClient* client);
	static void on_cmd_PWD(FTPD* server, FTPClient* client);

	static void on_cmd_PASV(FTPD* server, FTPClient* client);
	static void on_cmd_LIST(FTPD* server, FTPClient* client);
	static void on_cmd_NLST(FTPD* server, FTPClient* client);
	static void on_cmd_RETR(FTPD* server, FTPClient* client);
	static void on_cmd_STOR(FTPD* server, FTPClient* client);

	static void on_cmd_DELE(FTPD* server, FTPClient* client);
	static void on_cmd_RNFR(FTPD* server, FTPClient* client);
	static void on_cmd_RNTO(FTPD* server, FTPClient* client);
	static void on_cmd_RMD(FTPD* server, FTPClient* client);
	static void on_cmd_MKD(FTPD* server, FTPClient* client);

	static void on_cmd_QUIT(FTPD* server, FTPClient* client);
	static void on_cmd_unknown(FTPClient* client);
private:
	FTPD(const FTPD&);
	void operator=(const FTPD&);
};

} /* namespace::vos */

#endif
