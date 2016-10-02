//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_FTP_CLIENT_HH
#define _LIBVOS_FTP_CLIENT_HH 1

#include "Dir.hh"
#include "SockServer.hh"
#include "FTPD_cmd.hh"

namespace vos {

/**
 * @class		: FTPD_client
 * @attr		:
 *	- _s		: status of last command on this client.
 *	- _conn_stat	: status of client connection (see _ftp_stat).
 *	- _cmd		: current command.
 *	- _cmd_last	: last command.
 *	- _wd		: current working directory.
 *	- _wd_node	: pointer to DirNode object, of working directory.
 *	- _path		: path of command parameter.
 *	- _path_base	: basename of path.
 *	- _path_real	: real path of command parameter.
 *	- _path_node	: pointer to DirNode object of '_path'.
 *	- _sock		: pointer to client command channel.
 *	- _psrv		: pointer to PASV server connection.
 *	- _pclt		: pointer to PASV client connection.
 *	- _rmsg		: pointer to client reply message.
 *	- _rmsg_plus	: pointer to additional reply message.
 * @desc		:
 * This class represent status and connection of FTP client on the server.
 */
class FTPD_client : public Object {
public:
	FTPD_client(Socket* socket = NULL);
	~FTPD_client();

	void reset();
	int reply();
	int reply_raw(int code, const char* msg, const char* msg_add);

	int		_s;
	int		_conn_stat;
	FTPD_cmd	_cmd;
	FTPD_cmd	_cmd_last;
	Buffer		_wd;
	DirNode*	_wd_node;
	Buffer		_path;
	Buffer		_path_base;
	Buffer		_path_real;
	DirNode*	_path_node;
	Socket*		_sock;
	SockServer*	_psrv;
	Socket*		_pclt;
	const char*	_rmsg;
	const char*	_rmsg_plus;

	static const char* __cname;
private:
	FTPD_client(const FTPD_client&);
	void operator=(const FTPD_client&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
