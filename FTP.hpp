/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FTP_H
#define	_LIBVOS_FTP_H	1

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include "File.hpp"

namespace vos {

#define	FTP_DFLT_BUFFER_SIZE	8192
#define	FTP_DFLT_EOL		_file_eol[FILE_EOL_NIX]
#define	FTP_DFLT_PORT		21
#define	FTP_TIMEOUT		2
#define	FTP_UTIMEOUT_INC	200000
#define	FTP_UTIMEOUT_MAX	1000000

enum __ftp_cmd_idx {
	FTP_CMD_USER	= 0,
	FTP_CMD_PASS,
	FTP_CMD_ACCT,
	FTP_CMD_CWD,
	FTP_CMD_CDUP,
	FTP_CMD_PASV,
	FTP_CMD_TYPE,
	FTP_CMD_RETR,
	FTP_CMD_STOR,
	FTP_CMD_LIST,
	FTP_CMD_NLST,
	FTP_CMD_DELE,
	FTP_CMD_RNFR,
	FTP_CMD_RNTO,
	FTP_CMD_RMD,
	FTP_CMD_MKD,
	FTP_CMD_PWD,
	FTP_CMD_QUIT,
	N_FTP_CMD
};

enum _ftp_stat {
	FTP_STT_DISCONNECT	= FILE_OPEN_NO,
	FTP_STT_CONNECTED	= FILE_OPEN_R | FILE_OPEN_W,
	FTP_STT_LOGGED_IN	= FILE_OPEN_W << 1,
	FTP_STT_LOGGED_OUT	= FILE_OPEN_W << 2
};

enum _ftp_mode {
	FTP_MODE_NORMAL		= 0,
	FTP_MODE_PASV		= 1
};

class FTP : public File {
public:
	FTP(const char *host = 0, const int port = FTP_DFLT_PORT);
	~FTP();

	void connect(const char *host, const int port = FTP_DFLT_PORT,
			const int mode = FTP_MODE_NORMAL);
	int login(const char *username, const char *password);
	void logout();
	void disconnect();

	void send();
	int recv(const int timeout = FTP_TIMEOUT);
	void dump_send();
	void dump_receive();

	int send_cmd(const int cmd, const char *parm = 0);
	int get_reply(const int timeout = FTP_TIMEOUT);

	int parsing_pasv_reply(Buffer *addr, int *port);
	int do_pasv(const int cmd, const char *in, const char *out = 0);

	int do_cdup() {
		return send_cmd(FTP_CMD_CDUP);
	};
	int do_cd(const char *path = "/") {
		return send_cmd(FTP_CMD_CWD, path);
	};
	int do_delete(const char *path) {
		return send_cmd(FTP_CMD_DELE, path);
	};
	int do_mkdir(const char *path) {
		return send_cmd(FTP_CMD_MKD, path);
	};
	int do_pwd() {
		return send_cmd(FTP_CMD_PWD);
	};
	int do_rmdir(const char *path) {
		return send_cmd(FTP_CMD_RMD, path);
	};
	int do_type() {
		return send_cmd(FTP_CMD_TYPE);
	};
	int do_get(const char *in, const char *out = 0) {
		return do_pasv(FTP_CMD_RETR, in, out);
	}
	int do_list(const char *path = ".", const char *out = 0) {
		return do_pasv(FTP_CMD_LIST, path, out);
	}
	int do_list_simple(const char *path, const char *out = 0) {
		return do_pasv(FTP_CMD_NLST, path, out);
	}

	int do_put(const char *path);
	int do_rename(const char *from, const char *to);
private:
	int		_port;
	int		_reply;
	int		_mode;
	struct timeval	_tout;

	DISALLOW_COPY_AND_ASSIGN(FTP);
};

} /* namespace::vos */

#endif
