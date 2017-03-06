//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_FTP_HH
#define _LIBVOS_FTP_HH 1

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "FTP_cmd.hh"

namespace vos {

enum _ftp_mode {
	FTP_MODE_NORMAL	= 0
,	FTP_MODE_PASV
};

/**
 * @class		: FTP
 * @attr		:
 *	- _reply	: status of reply code.
 *	- _mode		: mode of current FTP object, is it in normal or
 *	                  passive connection.
 *	- _fd_all	: all open descriptor in this object.
 *	- _fd_read	: temporary read descriptor for select().
 *	- _timeout	: temporary variable for storing timeout value when
 *			doing select().
 *	- PORT		: static, default port number to be used.
 *	- TIMEOUT	: static, default time out value, in seconds.
 * @desc		: module to talk to server using FTP protocol.
 */
class FTP : public Socket {
public:
	FTP();
	~FTP();

	int connect(const char* host, const uint16_t port = PORT
			, const int mode = FTP_MODE_NORMAL);
	int login(const char* username, const char* password);
	void logout();
	void disconnect();

	int recv(const unsigned int to_sec = TIMEOUT
		, const unsigned int to_usec = 0);

	int send_cmd(const char* cmd, const char* parm);
	int get_reply(const unsigned int timeout);

	int parsing_pasv_reply(Buffer* addr, uint16_t* port);
	int do_pasv(const char* cmd, const char* parm, const char* out);

	int do_put(const char *path);
	int do_rename(const char *from, const char *to);

	inline int do_cdup() {
		return send_cmd(_FTP_cmd[FTP_CMD_CDUP], NULL);
	}
	inline int do_cd(const char *path = "/") {
		return send_cmd(_FTP_cmd[FTP_CMD_CWD], path);
	}
	inline int do_delete(const char *path) {
		return send_cmd(_FTP_cmd[FTP_CMD_DELE], path);
	}
	inline int do_mkdir(const char *path) {
		return send_cmd(_FTP_cmd[FTP_CMD_MKD], path);
	}
	inline int do_pwd() {
		return send_cmd(_FTP_cmd[FTP_CMD_PWD], NULL);
	}
	inline int do_rmdir(const char *path) {
		return send_cmd(_FTP_cmd[FTP_CMD_RMD], path);
	}
	inline int do_type(const char *type) {
		return send_cmd(_FTP_cmd[FTP_CMD_TYPE], type);
	}
	inline int do_get(const char *in, const char *out) {
		return do_pasv(_FTP_cmd[FTP_CMD_RETR], in, out);
	}
	inline int do_list(const char *path, const char *out) {
		return do_pasv(_FTP_cmd[FTP_CMD_LIST], path, out);
	}
	inline int do_list_simple(const char *path, const char *out) {
		return do_pasv(_FTP_cmd[FTP_CMD_NLST], path, out);
	}

	int		_reply;
	int		_mode;
	fd_set		_fd_all;
	fd_set		_fd_read;
	struct timeval	_timeout;

	static uint16_t PORT;
	static uint16_t TIMEOUT;

	static const char* __cname;
private:
	FTP(const FTP&);
	void operator=(const FTP&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
