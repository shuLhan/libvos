//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_FTP_DAEMON_HH
#define _LIBVOS_FTP_DAEMON_HH 1

#include <signal.h>
#include "List.hh"
#include "FTPD_client.hh"
#include "FTPD_user.hh"

namespace vos {

#define	FTPD_DEF_ADDRESS	"0.0.0.0"
#define	FTPD_DEF_PORT		21
#define	FTPD_DEF_PATH		"./"


enum _FTP_reply_msg_idx {
	CODE_150 = 0
,	CODE_200
,	CODE_211
,	CODE_213
,	CODE_215
,	CODE_220
,	CODE_221
,	CODE_226
,	CODE_227
,	CODE_230
,	CODE_250
,	CODE_257
,	CODE_331
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
,	NODE_IS_DIR
,	NODE_IS_NOT_DIR
,	TYPE_ALWAYS_BIN
,	MODE_ALWAYS_STREAM
,	STRU_ALWAYS_FILE
,	N_ADD_REPLY_MSG
};
extern const char* _FTP_add_reply_msg[N_ADD_REPLY_MSG];

enum _FTP_auth_mode {
	AUTH_NOLOGIN = 0
,	AUTH_LOGIN
,	N_FTP_MODE
};

extern const char* _FTP_month[12];

/**
 * @class		: FTPD
 * @attr		:
 *	- _running	: flag for checking if server still running.
 *	- _auth_mode	: authentication mode, login or without login.
 *	- _maxfd	: maximum file descriptor, used by 'select()'.
 *	- _path		: the real path to directory that the server serve to
 *                        the networks.
 *	- _dir		: Dir object, contain cache of all files in 'path'.
 *	- _fd_all	: all file descriptor in the server, used by
 *                        'select()'.
 *	- _fd_read	: the change descriptor, file descriptor that has the
 *                        change flag on after 'select()'.
 *	- _clients	: list of all server client.
 *	- _users	: list of all server account.
 * @desc		:
 * A simple FTP server module for serving a file system to the network.
 */
class FTPD : public SockServer {
public:
	FTPD();
	~FTPD();

	int init(const char* address = FTPD_DEF_ADDRESS
		, const uint16_t port = FTPD_DEF_PORT
		, const char* path = FTPD_DEF_PATH
		, const int auth_mode = AUTH_NOLOGIN);

	int user_add(const char* name, const char* pass);
	int user_is_exist(const char* name, const char* pass = NULL);

	int add_command(const int code, const char* cmd_name
			, void (*callback)(FTPD*, FTPD_client*));
	int set_path(const char* path);
	void set_default_callback();
	int set_callback(const int code, void (*callback)(FTPD*, FTPD_client*));
	int run();
	void client_process();
	int client_get_command(Socket* c, FTPD_cmd* ftp_cmd);
	void client_add(FTPD_client* c);
	void client_del(FTPD_client *c);
	int client_get_path(FTPD_client* c, int check_parm = 1);
	int client_get_parent_path(FTPD_client* c);

	int		_running;
	int		_auth_mode;
	int		_maxfd;
	Buffer		_path;
	Dir		_dir;
	fd_set		_fd_all;
	fd_set		_fd_read;
	List		_clients;
	List		_users;
	List		_cmds;

	static void on_cmd_USER(FTPD* s, FTPD_client* c);
	static void on_cmd_PASS(FTPD* s, FTPD_client* c);
	static void on_cmd_SYST(FTPD* s, FTPD_client* c);
	static void on_cmd_TYPE(FTPD* s, FTPD_client* c);
	static void on_cmd_MODE(FTPD* s, FTPD_client* c);
	static void on_cmd_STRU(FTPD* s, FTPD_client* c);
	static void on_cmd_FEAT(FTPD* s, FTPD_client* c);
	static void on_cmd_SIZE(FTPD* s, FTPD_client* c);
	static void on_cmd_MDTM(FTPD* s, FTPD_client* c);
	
	static void on_cmd_CWD(FTPD* s, FTPD_client* c);
	static void on_cmd_CDUP(FTPD* s, FTPD_client* c);
	static void on_cmd_PWD(FTPD* s, FTPD_client* c);

	static uint16_t GET_PASV_PORT();
	static void on_cmd_PASV(FTPD* s, FTPD_client* c);
	static void on_cmd_LIST(FTPD* s, FTPD_client* c);
	static void on_cmd_NLST(FTPD* s, FTPD_client* c);
	static void on_cmd_RETR(FTPD* s, FTPD_client* c);
	static void on_cmd_STOR(FTPD* s, FTPD_client* c);

	static void on_cmd_DELE(FTPD* s, FTPD_client* c);
	static void on_cmd_RNFR(FTPD* s, FTPD_client* c);
	static void on_cmd_RNTO(FTPD* s, FTPD_client* c);
	static void on_cmd_RMD(FTPD* s, FTPD_client* c);
	static void on_cmd_MKD(FTPD* s, FTPD_client* c);

	static void on_cmd_QUIT(FTPD* s, FTPD_client* c);
	static void on_cmd_unknown(FTPD_client* c);
private:
	FTPD(const FTPD&);
	void operator=(const FTPD&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
