/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "FTP.hpp"

namespace vos {

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
 * @desc: create an FTP connection that connect to 'host' at port 'port'.
 *
 * @param:
 *	> host : FTP server name or address.
 *	> port : FTP server port (default to 21).
 */
FTP::FTP(const char *host, int port) :
	_reply(0),
	_mode(FTP_MODE_NORMAL),
	_tout()
{
	_tout.tv_sec	= FTP_TIMEOUT;
	_tout.tv_usec	= FTP_UTIMEOUT_INC;
	if (host)
		FTP::connect(host, port);
}

FTP::~FTP()
{
	if (_status & FTP_STT_LOGGED_IN)
		logout();
	if (_status & FTP_STT_CONNECTED)
		disconnect();
}

void FTP::connect(const char *host, const int port, const int mode)
{
	int s;

	create();
	connect_to(host, port);

	_mode = mode;

	if (mode == FTP_MODE_NORMAL) {
		/* get server header.
		 * why we loop here ? because some server send header more
		 * than once.
		 */
		do {
			s = get_reply(0);
			if (s) {
				return;
			}
			/* get & set server EOL */
			if (_v[_i - 2] == _file_eol[FILE_EOL_DOS][0]) {
				_eol = _file_eol[FILE_EOL_DOS][0];
			}
		} while (s == 0);
	}
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
	if (!(_status & FTP_STT_LOGGED_IN))
		return;

	send_cmd(FTP_CMD_QUIT);
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
 * @desc: send all buffer contents.
 */
void FTP::send()
{
	if (LIBVOS_DEBUG)
		dump_send();
	flush();
}

/**
 * @return:
 *	< 0 : success.
 *	< 1 : fail, socket timeout.
 */
int FTP::recv(const int timeout)
{
	int		s;
	struct timeval	to;
	fd_set		read_set;

	do {
		FD_ZERO(&read_set);
		FD_SET(_d, &read_set);

		to.tv_sec	= _tout.tv_sec;
		to.tv_usec	= _tout.tv_usec;

		s = select(_d + 1, &read_set, 0, 0, &to);
		if (s < 0)
			throw Error(E_SOCK_SELECT);

		/* time out */
		if (s == 0 || !(FD_ISSET(_d, &read_set))) {
			_tout.tv_usec += FTP_UTIMEOUT_INC;
			_tout.tv_sec += (_tout.tv_usec / FTP_UTIMEOUT_MAX);
			_tout.tv_usec %=  FTP_UTIMEOUT_MAX;

			printf(" -- tout.tv_sec >= timeout: %ld >= %d\n",
				_tout.tv_sec, timeout);
			if (_tout.tv_sec >= timeout) {
				_tout.tv_sec	= 0;
				_tout.tv_usec	= FTP_UTIMEOUT_INC;
				return 1;
			}
		}
	} while (s <= 0);

	reset();
	read();
	if (LIBVOS_DEBUG && _mode == FTP_MODE_NORMAL)
		dump_receive();

	return 0;
}

void FTP::dump_send()
{
	printf("\n >> %s", _v);
}

void FTP::dump_receive()
{
	printf(" << %s\n", _v);
}

/**
 * @return:
 *	< 0  : success.
 *	< 1  : fail.
 */
int FTP::send_cmd(const int cmd, const char *parm)
{
	int s;

	if (_status == FTP_STT_DISCONNECT)
		return 1;

	reset();
	if (parm)
		concat(_ftp_cmd[cmd], " ", parm, __eol, NULL);
	else
		concat(_ftp_cmd[cmd], __eol, NULL);

	send();
	s = get_reply();

	return s;
}

/**
 * @return:
 *	< 0  : success.
 *	< 1  : fail.
 */
int FTP::get_reply(const int timeout)
{
	int s = recv(timeout);
	if (s)
		return s;

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
		_status &= FTP_STT_LOGGED_OUT;
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

	s = send_cmd(FTP_CMD_PASV);
	if (s)
		return s;

	s = parsing_pasv_reply(&addr, &port);
	if (s)
		return s;

	pasv.connect(addr._v, port, FTP_MODE_PASV);

	s = send_cmd(cmd, parm);
	if (s)
		return s;

	if (out) {
		s = fout.open_wo(out);
	} else if (cmd == FTP_CMD_RETR) {
		s = fout.open_wo(parm);
	} else {
		fout._d		= STDOUT_FILENO;
		fout._status	= FILE_OPEN_W;
	}
	if (s)
		return s;

	pasv.recv();
	while (pasv._i > 0) {
		fout.write(&pasv);
		pasv.recv();
	}
	fout.flush();
	pasv.disconnect();

	s = get_reply();

	return s;
}

int FTP::do_put(const char *path)
{
	int	port	= 0;
	int	s;
	Buffer	addr;
	File	fput;
	FTP	pasv;

	if (! path)
		return 0;

	s = fput.open_ro(path);
	if (s)
		return s;

	s = send_cmd(FTP_CMD_PASV);
	if (s)
		return s;

	s = parsing_pasv_reply(&addr, &port);
	if (s)
		return s;

	pasv.connect(addr._v, port, FTP_MODE_PASV);

	s = send_cmd(FTP_CMD_STOR, path);
	if (s)
		return s;

	fput.read();
	while (fput._i > 0) {
		pasv.write(&fput);
		fput.read();
	}
	pasv.flush();

	pasv.disconnect();
	s = get_reply();

	return s;
}

int FTP::do_rename(const char *from, const char *to)
{
	int s;

	if (! from || ! to)
		return 0;

	s = send_cmd(FTP_CMD_RNFR, from);
	if (s)
		return s;

	s = send_cmd(FTP_CMD_RNTO, to);

	return s;
}

} /* namespace::vos */
