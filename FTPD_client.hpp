/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FTP_CLIENT_HPP
#define	_LIBVOS_FTP_CLIENT_HPP	1

#include "Dir.hpp"
#include "SockServer.hpp"
#include "FTPD_cmd.hpp"

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
 *	- _next		: pointer to the next FTP client.
 *	- _last		: pointer to the last FTP client.
 * @desc		:
 * This class represent status and connection of FTP client on the server.
 */
class FTPD_client {
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
	FTPD_client*	_next;
	FTPD_client*	_prev;
	FTPD_client*	_last;

	static void ADD(FTPD_client** list, FTPD_client* client);
	static void REMOVE(FTPD_client** list, FTPD_client* client);
private:
	FTPD_client(const FTPD_client&);
	void operator=(const FTPD_client&);
};

} /* namespace::vos */

#endif
