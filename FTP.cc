//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "FTP.hh"

namespace vos {

const char* FTP::__cname = "FTP";

uint16_t FTP::PORT = 21;
uint16_t FTP::TIMEOUT	= 3;

/**
 * @method	: FTP::FTP
 * @desc	: FTP object constructor.
 */
FTP::FTP() : Socket()
,	_reply(0)
,	_mode(FTP_MODE_NORMAL)
,	_fd_all()
,	_fd_read()
,	_timeout()
{}

/**
 * @method	: FTP::~FTP
 * @desc	: FTP object destructor.
 */
FTP::~FTP()
{
	if (_status & FTP_STT_LOGGED_IN) {
		logout();
	}
	if (_status & FTP_STT_CONNECTED) {
		disconnect();
	}
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
int FTP::connect(const char* host, const uint16_t port, const int mode)
{
	disconnect();

	int s = create();
	if (s < 0) {
		return -1;
	}

	s = connect_to_raw(host, port);
	if (s < 0) {
		return -1;
	}

	FD_ZERO(&_fd_all);
	FD_ZERO(&_fd_read);
	FD_SET(_d, &_fd_all);
	_mode = mode;

	if (mode == FTP_MODE_NORMAL) {
		s = get_reply(TIMEOUT);
		if (s < 0) {
			return -1;
		}

		/* get & set server EOL */
		if (_i > 2 && _v[_i - 2] == __eol[EOL_DOS][0]) {
			if (LIBVOS_DEBUG) {
				printf("[%s] connect: set EOL to DOS\n"
					, __cname);
			}
			set_eol(EOL_DOS);
		}
	}

	return 0;
}

/**
 * @method		: FTP::login
 * @param		:
 *	> username	: name of user on FTP server.
 *	> password	: password for 'username'.
 * @return		:
 * 	< 0		: success.
 *	< -1		: fail.
 * @desc		: login to FTP server.
 */
int FTP::login(const char* username, const char* password)
{
	int s = send_cmd(_FTP_cmd[FTP_CMD_USER], username);
	if (s < 0) {
		return -1;
	}

	s = send_cmd(_FTP_cmd[FTP_CMD_PASS], password);
	if (s < 0) {
		return -1;
	}

	s = send_cmd(_FTP_cmd[FTP_CMD_TYPE], "I");
	if (s < 0) {
		return -1;
	}

	return s;
}

/**
 * @method	: FTP::logout
 * @desc	: log out from FTP server, but keep the connection open.
 */
void FTP::logout()
{
	if (_status & FTP_STT_LOGGED_IN) {
		send_cmd(_FTP_cmd[FTP_CMD_QUIT], NULL);
	}
}

/**
 * @method	: FTP::disconnect
 * @desc	: completely close connection to FTP server.
 */
void FTP::disconnect()
{
	if (_status == FTP_STT_DISCONNECT) {
		return;
	}
	if (_status & FTP_STT_LOGGED_IN) {
		logout();
	}
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
 *	> to_usec	: time out in mili-seconds.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	Receive data from server.
 *	To check if function fail because of timeout or not, check the errno
 *	value.
 */
int FTP::recv(const unsigned int to_sec, const unsigned int to_usec)
{
	ssize_t s = 0;

	reset();

	_fd_read		= _fd_all;
	_timeout.tv_sec		= to_sec;
	_timeout.tv_usec	= to_usec;

	s = select(_d + 1, &_fd_read, 0, 0, &_timeout);
	if (s < 0) {
		return -1;
	}

	if (FD_ISSET(_d, &_fd_read)) {
		s = read();
		if (s >= 0) {
			if (LIBVOS_DEBUG && _mode == FTP_MODE_NORMAL) {
				dump();
			}
			s = 0;
		}
	} else {
		s = -1;
		if (LIBVOS_DEBUG) {
			printf(
"[%s] recv: timeout after '%u.%u' seconds.\n", __cname, to_sec, to_usec);
		}
	}

	return int(s);
}

/**
 * @method	: FTP::send_cmd
 * @param	:
 *	> cmd	: type of command to send.
 *	> parm	: parameter to be send, if needed.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: send 'cmd' to server with or without additional paramater
 *                'parm'.
 */
int FTP::send_cmd(const char* cmd, const char *parm)
{
	if (_status == FTP_STT_DISCONNECT) {
		return -1;
	}

	reset();

	ssize_t s = 0;
	Error err;

	if (parm) {
		err = concat(cmd, " ", parm, _eols, NULL);
	} else {
		err = concat(cmd, _eols, NULL);
	}
	if (err != NULL) {
		return -1;
	}

	s = flush();
	if (s < 0) {
		return -1;
	}

	return get_reply(TIMEOUT);
}

/**
 * @method		: FTP::get_reply
 * @param		:
 *	> timeout	: time out value, in seconds, for waiting a reply from
 *                        server.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: wait and get a reply from server.
 */
int FTP::get_reply(const unsigned int timeout)
{
	do {
		ssize_t s = recv(timeout);
		if (s < 0) {
			return -1;
		}
		if (_i == 0) {
			return 0;
		}

		if (isdigit(_v[0])) {
			_reply = atoi(_v);
		} else {
			_reply = 0;
		}
	} while (_i > 3 && _v[3] == '-');

	if (LIBVOS_DEBUG) {
		printf("[%s] get_reply: code '%d'\n", __cname, _reply);
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
		fprintf(stderr
			, "[%s] get_reply: server message: %s"
			, __cname, chars());
		return -1;
	default:
		fprintf(stderr
			, "[%s] get_reply: unknown reply code %d!"
			, __cname, _reply);
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
 *	< -1	: fail.
 */
int FTP::parsing_pasv_reply(Buffer* addr, uint16_t* port)
{
	int s;
	uint16_t tmp;
	char*		p = _v;

	/* get reply code : 227 */ 
	s = (int) strtol(_v, &p, 0);
	if (LIBVOS_DEBUG) {
		printf("[%s] parsing_pasv_reply: reply code : %d\n"
			, __cname, s);
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
	tmp	= (uint16_t) strtol(p, &p, 0);
	*port	= (uint16_t) (tmp * 256);
	++p;
	tmp	= (uint16_t) strtol(p, 0, 0);
	*port	= (uint16_t) (*port + tmp);

	if (LIBVOS_DEBUG) {
		printf("[%s] parsing_pasv_reply: '%s:%d'\n", __cname
			, addr->chars(), *port);
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
 *	< -1	: fail.
 * @desc	:
 *
 *	create a passive connection to server and send command 'cmd' after
 *	connection created, then save the 'cmd' output to file 'out'.
 *
 *	if 'out' is nil then output of 'cmd' will be printed to standard
 *	output.
 */
int FTP::do_pasv(const char* cmd, const char* parm, const char* out)
{
	uint16_t port = 0;
	int	s;
	Buffer	addr;
	File	fout;
	FTP	pasv;

	s = send_cmd(_FTP_cmd[FTP_CMD_PASV], NULL);
	if (s < 0) {
		return -1;
	}

	s = parsing_pasv_reply(&addr, &port);
	if (s < 0) {
		return -1;
	}

	s = pasv.connect(addr.v(), port, FTP_MODE_PASV);
	if (s < 0) {
		return -1;
	}

	s = send_cmd(cmd, parm);
	if (s < 0) {
		return -1;
	}

	if (out) {
		s = fout.open_wo(out);
	} else if (strncasecmp(cmd, _FTP_cmd[FTP_CMD_RETR], 4) == 0) {
		s = fout.open_wo(parm);
	} else {
		fout._d		= STDOUT_FILENO;
		fout._status	= O_WRONLY;
	}
	if (s < 0) {
		return -1;
	}

	pasv.recv(TIMEOUT);
	while (pasv.len() > 0) {
		fout.write(&pasv);
		pasv.recv(TIMEOUT);
	}
	fout.flush();
	pasv.disconnect();

	return get_reply(TIMEOUT);
}

/**
 * @method	: FTP::do_put
 * @param	:
 *	> path	: path to a file to be saved in server.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: send file 'path' to server.
 */
int FTP::do_put(const char* path)
{
	if (!path) {
		return 0;
	}

	uint16_t port = 0;
	int	s;
	Buffer	addr;
	File	fput;
	FTP	pasv;

	s = fput.open_ro(path);
	if (s < 0) {
		return -1;
	}

	s = send_cmd(_FTP_cmd[FTP_CMD_PASV], NULL);
	if (s < 0) {
		return -1;
	}

	s = parsing_pasv_reply(&addr, &port);
	if (s < 0) {
		return -1;
	}

	s = pasv.connect(addr.v(), port, FTP_MODE_PASV);
	if (s < 0) {
		return -1;
	}

	s = send_cmd(_FTP_cmd[FTP_CMD_STOR], path);
	if (s < 0) {
		return -1;
	}

	fput.read();
	while (fput.len() > 0) {
		pasv.write(&fput);
		fput.read();
	}
	pasv.flush();

	pasv.disconnect();

	return get_reply(TIMEOUT);
}

/**
 * @method	: FTP::do_rename
 * @param	:
 *	> from	: a file name to be renamed.
 *	> to	: a new name for file.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: rename file 'from' on server into 'to'.
 */
int FTP::do_rename(const char* from, const char* to)
{
	if (!from || !to) {
		return -1;
	}

	int s = send_cmd(_FTP_cmd[FTP_CMD_RNFR], from);
	if (s < 0) {
		return -1;
	}

	return send_cmd(_FTP_cmd[FTP_CMD_RNTO], to);
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
