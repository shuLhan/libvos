/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FTP_CLIENT_HPP
#define	_LIBVOS_FTP_CLIENT_HPP	1

#include "Dir.hpp"
#include "Socket.hpp"
#include "FTPCmd.hpp"

namespace vos {

enum _ftp_stat {
	FTP_STT_DISCONNECT	= vos::FILE_OPEN_NO
,	FTP_STT_CONNECTED	= O_RDWR
,	FTP_STT_LOGGED_IN	= O_RDWR << 1
,	FTP_STT_LOGGED_OUT	= O_RDWR << 2
};

/**
 * @class		: FTPClient
 * @attr		:
 *	- _s		: status of last command on this client.
 *	- _conn_stat	: status of client connection (see _ftp_stat).
 *	- _cmnd		: current command.
 *	- _cmnd_last	: last command.
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
class FTPClient {
public:
	FTPClient(Socket* socket = NULL);
	~FTPClient();

	void reset();
	void reply();
	void reply_raw(int code, const char* msg, const char* msg_add);

	int		_s;
	int		_conn_stat;
	FTPCmd		_cmnd;
	FTPCmd		_cmnd_last;
	Buffer		_wd;
	DirNode*	_wd_node;
	Buffer		_path;
	Buffer		_path_base;
	Buffer		_path_real;
	DirNode*	_path_node;
	Socket*		_sock;
	Socket*		_psrv;
	Socket*		_pclt;
	const char*	_rmsg;
	const char*	_rmsg_plus;
	FTPClient*	_next;
	FTPClient*	_last;

	static void ADD(FTPClient** list, FTPClient* client);
	static void REMOVE(FTPClient** list, FTPClient* client);
private:
	FTPClient(const FTPClient&);
	void operator=(const FTPClient&);
};

} /* namespace::vos */

#endif
