/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "FTP.hpp"

namespace vos {

unsigned int FTP::PORT		= 21;
unsigned int FTP::TIMEOUT	= 3;
unsigned int FTP::UTIMEOUT	= 0;

const char *_ftp_cmd[N_FTP_CMD] = {
	"USER",	/* USER	username		*/
	"PASS",	/* PASS	password		*/
	"ACCT",	/* ACCT	username		*/
	"CWD" ,	/* CWD	directory		*/
	"CDUP",	/* CDUP				*/
	"PASV",	/* PASV				*/
	"TYPE",	/* TYPE	[A|I]			*/
	"RETR",	/* RETR	/path/to/filename	*/
	"STOR",	/* STOR	/path/to/filename	*/
	"LIST",	/* LIST	[directory]		*/
	"NLST",	/* NLST	[directory]		*/
	"DELE",	/* DELE	/path/to/filename	*/
	"RNFR",	/* RNFR	/path/to/filename.old	*/
	"RNTO",	/* RNTO	/path/to/filename.new	*/
	"RMD" ,	/* RMD	directory		*/
	"MKD" ,	/* MKD	directory		*/
	"PWD" ,	/* PWD				*/
	"QUIT"	/* QUIT				*/
};

/**
 * @method	: FTP::FTP
 * @desc	: FTP object constructor.
 */
FTP::FTP() : Socket(),
	_reply(0),
	_mode(FTP_STT_DISCONNECT)
{}

/**
 * @method	: FTP::~FTP
 * @desc	: FTP object destructor.
 */
FTP::~FTP()
{
	if (_status & FTP_STT_LOGGED_IN)
		logout();
	if (_status & FTP_STT_CONNECTED)
		disconnect();
}

/**
 * @method	: FTP::connect
 * @param	:
 *	> host	: hostname or IP address destination.
 *	> port	: port number for FTP service on destination.
 *	> mode	: mode of connection (NORMAL | PASV).
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create FTP connection to 'host:port'.
 */
int FTP::connect(const char *host, const int port, const int mode)
{
	register int s;

	s = create();
	if (s < 0) {
		return -1;
	}

	s = connect_to_raw(host, port);
	if (s < 0) {
		return -1;
	}

	_mode = mode;

	if (mode == FTP_MODE_NORMAL) {
		/* get server header.
		 * why we loop here ? because some server send header more
		 * than once. so we read all the f*ing banner here before
		 * sending another commands.
		 */
		do {
			s = get_reply(TIMEOUT);
			/* get & set server EOL */
			if (_v[_i - 2] == __eol[EOL_DOS][0]) {
				fprintf(stderr,"[FTP] EOL set to DOS\n");
				set_eol(EOL_DOS);
			}
		} while (_reply > 0 && _i > 0);
	}

	return 0;
}

/**
 * @method		: FTP::login
 * @param		:
 *	> username	: name of user on FTP server.
 *	> password	: password for 'username'.
 * @return		:
 *	< 1		: socket is not connected.
 * 	< 0		: success.
 *	< <0		: fail.
 * @desc		: login to FTP server.
 */
int FTP::login(const char *username, const char *password)
{
	register int s;

	s = send_cmd(FTP_CMD_USER, username);
	if (s < 0)
		return s;

	s = send_cmd(FTP_CMD_PASS, password);
	if (s < 0)
		return s;

	s = send_cmd(FTP_CMD_TYPE, "I");
	if (s < 0)
		return s;

	return s;
}

/**
 * @method	: FTP::logout
 * @desc	: log out from FTP server, but keep the connection open.
 */
void FTP::logout()
{
	if (_status & FTP_STT_LOGGED_IN) {
		send_cmd(FTP_CMD_QUIT, NULL);
	}
}

/**
 * @method	: FTP::disconnect
 * @desc	: completely close connection to FTP server.
 */
void FTP::disconnect()
{
	if (_status == FTP_STT_DISCONNECT)
		return;
	if (_status & FTP_STT_LOGGED_IN)
		logout();

	if (_d) {
		::close(_d);
		_d = 0;
	}
	_status = FTP_STT_DISCONNECT;
}

/**
 * @method		: FTP::recv
 * @param		:
 *	> to_sec	: time out in seconds.
 *	> to_usec	: time out in micro-seconds.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		:
 *	Receive data from server.
 *	To check if function fail because of timeout or not, check the errno
 *	value.
 */
int FTP::recv(const int to_sec, const int to_usec)
{
	register int	s;
	fd_set		fd_all;
	fd_set		fd_read;
	struct timeval	_timeout;

	reset();
	FD_ZERO(&fd_all);
	FD_ZERO(&fd_read);
	FD_SET(_d, &fd_all);

	fd_read			= fd_all;
	_timeout.tv_sec		= to_sec;
	_timeout.tv_usec	= to_usec;

	s = select(_d + 1, &fd_read, 0, 0, &_timeout);
	if (s < 0)
		return -1;

	if (FD_ISSET(_d, &fd_read)) {
		s = read();
		if (s >= 0) {
			if (LIBVOS_DEBUG && _mode == FTP_MODE_NORMAL) {
				dump();
			}
			return 0;
		}
	} else {
		s = -1;
		if (LIBVOS_DEBUG) {
			printf("[FTP] timeout after %d.%d seconds.\n", to_sec,
				to_usec);
		}
	}

	return s;
}

/**
 * @method	: FTP::send_cmd
 * @param	:
 *	> cmd	: type of command to send.
 *	> parm	: parameter to be send, if needed.
 * @return	:
 *	< 1	: socket is not connected.
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: send 'cmd' to server with or without additional paramater
 *                'parm'.
 */
int FTP::send_cmd(const int cmd, const char *parm)
{
	register int s;

	if (_status == FTP_STT_DISCONNECT) {
		return 1;
	}

	reset();

	if (parm) {
		s = concat(_ftp_cmd[cmd], " ", parm, _eols, NULL);
	} else {
		s = concat(_ftp_cmd[cmd], _eols, NULL);
	}
	if (s < 0) {
		return s;
	}

	s = flush();
	if (s < 0) {
		return s;
	}

	s = get_reply(TIMEOUT);

	return s;
}

/**
 * @method		: FTP::get_reply
 * @param		:
 *	> timeout	: time out value, in seconds, for waiting a reply from
 *                        server.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: wait and get a reply from server.
 */
int FTP::get_reply(const int timeout)
{
	register int s;

	s = recv(timeout, 0);
	if (s < 0) {
		return -1;
	}
	if (!_i) {
		return 0;
	}

	if (isdigit(_v[0])) {
		_reply = atoi(_v);
	} else {
		_reply = 0;
	}

	if (LIBVOS_DEBUG) {
		printf("[FTP] reply code : %d\n", _reply);
	}

	switch (_reply) {
	case   0: /* reply without number, usually come after 220 */
		  /* i.e: server banner */

	case 125: /* data connection already open, transfer starting */
	case 127: /* */
	case 150: /* file status okay, about to open data connection */

	case 200: /* command okay */
	case 202: /* command superflous */
		return 0;
	case 220: /* service ready */
		if (_status == FTP_STT_DISCONNECT) {
			_status = FTP_STT_CONNECTED;
		}
		return 0;
	case 221: /* logout successful */
		_status = (FTP_STT_CONNECTED | FTP_STT_LOGGED_OUT);
		return 0;
	case 230: /* login successful */
		_status |= FTP_STT_LOGGED_IN;
		return 0;
	case 226: /* closing data connection, transfer complete */
	case 227: /* entering passive mode */
	case 250: /* requested file action okay, completed */
	case 257: /* "PATHNAME" created */

	case 331: /* need password to continue */
	case 332: /* need account to continue */
	case 350: /* requested file action pending, send me next command */
		return 0;

	case 421: /* service not available, closing control connection */
	case 425: /* can not open data connection */
	case 426: /* connection closed, transfor aborted */
		_status = FTP_STT_DISCONNECT;
		return -1;

	case 120: /* service not ready yet */

	case 450: /* file unavailable, e.g., file busy */
	case 451: /* requested action aborted, local error in processing */

	case 500: /* command unrecognized */
	case 501: /* syntax error in parameter */
	case 502: /* command not implemented */
	case 503: /* bad sequence of command */
	case 530: /* not logged in */
	case 532: /* need account for storing file */
	case 550:
		/* requested file action not taken, e.g., file not found,
		 * no access, removing non empty directory. */
	case 551: /* request action aborted; page type unknown */
	case 552: /* requested file action aborted. exceeded storage allocation */
	case 553: /* requested action not taken, file name not allowed */
		fprintf(stderr, "[FTP] server message: %s", v());
		return -1;
	default:
		fprintf(stderr, "[FTP-ERROR] unknown reply code %d!", _reply);
	}

	return -1;
}

/**
 * @method	: FTP::parsing_pasv_reply
 * @param	:
 *	> addr	: return value, address of server that will accepting passive
 *                connection.
 *	> port	: return value, port that server will be accepting for passive
 *                connection.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int FTP::parsing_pasv_reply(Buffer *addr, int *port)
{
	register int	s;
	char		*p = _v;

	/* get reply code : 227 */ 
	s = (int) strtol(_v, &p, 0);
	if (LIBVOS_DEBUG) {
		printf("[FTP] ftp pasv reply code : %d\n", s);
	}

	/* get address */
	for (s = 1; s <= 4; ++s) {
		while (*p && ! isdigit(*p)) {
			++p;
		}
		if (!*p) {
			return -1;
		}

		while (*p && isdigit(*p)) {
			addr->appendc(*p);
			++p;
		}
		if (! *p) {
			return -1;
		}
		if (s < 4) {
			addr->appendc('.');
		}
	}
	++p;

	/* get port */
	s	= (int) strtol(p, &p, 0);
	*port	= s * 256;
	++p;
	s	= (int) strtol(p, 0, 0);
	*port	+= s;

	if (LIBVOS_DEBUG) {
		printf("[FTP] pasv '%s:%d'\n", addr->v(), *port);
	}

	return 0;
}

/**
 * @method	: FTP::do_pasv
 * @param	:
 *	> cmd	: type of command to be passed after in passive mode.
 *	> parm	: parameter for command 'cmd'.
 *	> out	: file name where output of 'cmd' will be saved.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *
 *	create a passive connection to server and send command 'cmd' after
 *	connection created, then save the 'cmd' output to file 'out'.
 *
 *	if 'out' is nil then output of 'cmd' will be printed to standard
 *	output.
 */
int FTP::do_pasv(const int cmd, const char *parm, const char *out)
{
	int	port	= 0;
	int	s;
	Buffer	addr;
	File	fout;
	FTP	pasv;

	s = send_cmd(FTP_CMD_PASV, NULL);
	if (s < 0)
		return s;

	s = parsing_pasv_reply(&addr, &port);
	if (s < 0)
		return s;

	s = pasv.connect(addr._v, port, FTP_MODE_PASV);
	if (s < 0)
		return s;

	s = send_cmd(cmd, parm);
	if (s < 0)
		return s;

	if (out) {
		s = fout.open_wo(out);
	} else if (cmd == FTP_CMD_RETR) {
		s = fout.open_wo(parm);
	} else {
		fout._d		= STDOUT_FILENO;
		fout._status	= O_WRONLY;
	}
	if (s < 0)
		return s;

	pasv.recv(TIMEOUT, 0);
	while (pasv._i > 0) {
		fout.write(&pasv);
		pasv.recv(TIMEOUT, 0);
	}
	fout.flush();
	pasv.disconnect();

	s = get_reply(TIMEOUT);

	return s;
}

/**
 * @method	: FTP::do_put
 * @param	:
 *	> path	: path to a file to be saved in server.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: send file 'path' to server.
 */
int FTP::do_put(const char *path)
{
	int	port	= 0;
	int	s;
	Buffer	addr;
	File	fput;
	FTP	pasv;

	if (!path)
		return 0;

	s = fput.open_ro(path);
	if (s < 0)
		return s;

	s = send_cmd(FTP_CMD_PASV, NULL);
	if (s < 0)
		return s;

	s = parsing_pasv_reply(&addr, &port);
	if (s < 0)
		return s;

	s = pasv.connect(addr._v, port, FTP_MODE_PASV);
	if (s < 0)
		return s;

	s = send_cmd(FTP_CMD_STOR, path);
	if (s < 0)
		return s;

	fput.read();
	while (fput._i > 0) {
		pasv.write(&fput);
		fput.read();
	}
	pasv.flush();

	pasv.disconnect();
	s = get_reply(TIMEOUT);

	return s;
}

/**
 * @method	: FTP::do_rename
 * @param	:
 *	> from	: a file name to be renamed.
 *	> to	: a new name for file.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: rename file 'from' on server into 'to'.
 */
int FTP::do_rename(const char *from, const char *to)
{
	register int s;

	if (!from || !to)
		return 0;

	s = send_cmd(FTP_CMD_RNFR, from);
	if (s < 0)
		return s;

	s = send_cmd(FTP_CMD_RNTO, to);

	return s;
}

} /* namespace::vos */
