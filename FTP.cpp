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

FTP::FTP() : Socket(),
	_reply(0),
	_mode(FTP_STT_DISCONNECT)
{}

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
 *	< !0	: fail.
 * @desc	: create FTP connection to 'host:port'.
 */
int FTP::connect(const char *host, const int port, const int mode)
{
	int s;

	s = create_tcp();
	if (s < 0)
		return s;

	s = connect_to_raw(host, port);
	if (s < 0)
		return s;

	_mode = mode;

	if (mode == FTP_MODE_NORMAL) {
		/* get server header.
		 * why we loop here ? because some server send header more
		 * than once. so we read all the f*ing banner here before
		 * sending another commands.
		 */
		do {
			s = get_reply(TIMEOUT);
			if (_i == 0)
				break;
			/* get & set server EOL */
			if (_v[_i - 2] == GET_EOL_CHR(FILE_EOL_DOS)) {
				set_eol(FILE_EOL_DOS);
			}
		} while (s);
	}

	return 0;
}

int FTP::login(const char *username, const char *password)
{
	int s;

	s = send_cmd(FTP_CMD_USER, username);
	if (s)
		return s;

	s = send_cmd(FTP_CMD_PASS, password);
	if (s)
		return s;

	s = send_cmd(FTP_CMD_TYPE, "I");
	if (s)
		return s;

	return s;
}

void FTP::logout()
{
	if (_status & FTP_STT_LOGGED_IN) {
		send_cmd(FTP_CMD_QUIT, NULL);
	}
}

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
 * @return:
 *	< 0	: success.
 *	< <0	: fail.
 */
int FTP::recv(const int to_sec, const int to_usec)
{
	register int	s;
	fd_set		fd_all;
	fd_set		fd_read;

	reset();
	FD_ZERO(&fd_all);
	FD_ZERO(&fd_read);
	FD_SET(_d, &fd_all);

	fd_read			= fd_all;
	_timeout.tv_sec		= to_sec;
	_timeout.tv_usec	= to_usec;

	s = select(_d + 1, &fd_read, 0, 0, &_timeout);
	if (s < 0)
		return -E_SOCK_SELECT;

	if (FD_ISSET(_d, &fd_read)) {
		s = read();
		if (s >= 0) {
			if (LIBVOS_DEBUG && _mode == FTP_MODE_NORMAL) {
				dump();
			}
			return 0;
		}
	} else {
		if (LIBVOS_DEBUG) {
			printf("[FTP] timeout after %d.%d seconds.\n", to_sec,
				to_usec);
		}
		s = -E_SOCK_TIMEOUT;
	}

	return s;
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int FTP::send_cmd(const int cmd, const char *parm)
{
	register int s;

	if (_status == FTP_STT_DISCONNECT)
		return 1;

	reset();

	if (parm)
		s = concat(_ftp_cmd[cmd], " ", parm, GET_EOL_STR(_eol), NULL);
	else
		s = concat(_ftp_cmd[cmd], GET_EOL_STR(_eol), NULL);
	if (s < 0)
		return s;

	s = send(NULL);
	if (s < 0)
		return s;

	s = get_reply(TIMEOUT);

	return s;
}

/**
 * @return:
 *	< 0	: success.
 *	< <0	: fail.
 */
int FTP::get_reply(const int timeout)
{
	int s;

	s = recv(timeout, 0);
	if (s < 0) {
		return s;
	}
	if (!_i) {
		return 0;
	}

	_reply = atoi(_v);

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
		if (_status == FTP_STT_DISCONNECT)
			_status = FTP_STT_CONNECTED;
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
		return 1;

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
		return 1;
	default:
		fprintf(stderr, "[FTP-ERROR] unknown reply code %d!", _reply);
	}

	return 1;
}

/**
 * @return:
 *	< host: address of passive server.
 *	< port: port for passive connection.
 *	< 0   : success.
 *	< 1   : fail.
 */
int FTP::parsing_pasv_reply(Buffer *addr, int *port)
{
	int	s;
	char	*p = _v;

	/* get reply code : 227 */ 
	s = strtol(_v, &p, 0);
	if (LIBVOS_DEBUG)
		printf("[FTP] ftp pasv reply code : %d\n", s);

	/* get address */
	for (s = 1; s <= 4; ++s) {
		while (*p && ! isdigit(*p))
			++p;
		if (! *p)
			return 1;

		while (*p && isdigit(*p)) {
			addr->appendc(*p);
			++p;
		}
		if (! *p)
			return 1;

		if (s < 4)
			addr->appendc('.');
	}
	++p;

	/* get port */
	s	= strtol(p, &p, 0);
	*port	= s * 256;
	++p;
	s	= strtol(p, 0, 0);
	*port	+= s;

	if (LIBVOS_DEBUG)
		printf("[FTP] pasv '%s:%d'\n", addr->_v, *port);

	return 0;
}

int FTP::do_pasv(const int cmd, const char *parm, const char *out)
{
	int	port	= 0;
	int	s;
	Buffer	addr;
	File	fout;
	FTP	pasv;

	s = addr.init(NULL);
	if (s < 0)
		return s;

	s = fout.init();
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

int FTP::do_put(const char *path)
{
	int	port	= 0;
	int	s;
	Buffer	addr;
	File	fput;
	FTP	pasv;

	if (!path)
		return 0;

	s = addr.init(NULL);
	if (s < 0)
		return s;

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

int FTP::do_rename(const char *from, const char *to)
{
	int s;

	if (!from || !to)
		return 0;

	s = send_cmd(FTP_CMD_RNFR, from);
	if (s < 0)
		return s;

	s = send_cmd(FTP_CMD_RNTO, to);

	return s;
}

} /* namespace::vos */
