/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Error.hpp"

namespace vos {

int LIBVOS_DEBUG = getenv("LIBVOS_DEBUG") == 0
			? 0 : atoi(getenv("LIBVOS_DEBUG"));

const char *_errmsg[N_ERRCODE] = {
	"\0",
	"[LIBVOS] Error: Out of memory!\n",
	"[LIBVOS] Error: Unknown host or address '%s'!\n",
	"\0",
	"[LIBVOS] File error: cannot open file '%s'!\n",	/* 4 */
	"[LIBVOS] File error: at reading on file '%s'!\n",
	"[LIBVOS] File error: at writing on file '%s'!\n",
	"[LIBVOS] File error: at seeking on file '%s'!\n",
	"[LIBVOS] end of file.\n",				/* 8 */
	"\0",
	"\0",
	"\0",
	"[LIBVOS] Config error: line %d column %d\n",		/* 12 */
	"\0",
	"\0",
	"\0",
	"[LIBVOS] Socket error: create!%s\n",			/* 16 */
	"[LIBVOS] Socket error: invalid address '%s'!\n",
	"[LIBVOS] Socket error: cannot resolve address '%s'!\n",
	"[LIBVOS] Socket error: cannot reuse address '%s' at port '%d'!\n",
	"[LIBVOS] Socket error: cannot bind to address '%s' at port '%d'!\n",
	"[LIBVOS] Socket error: cannot connect to host '%s' at port '%d'!\n",
	"[LIBVOS] Socket error: read operation failed!\n",
	"[LIBVOS] Socket error: write operation failed!\n",
	"[LIBVOS] Socket error: select operation failed!\n",	/* 24 */
	"[LIBVOS] Socket error: failed to accept new connection!\n",
	"\0",
	"\0",
	"[LIBVOS] OCI Error: %d - %s\n"				/* 28 */
};

const char *_stat_msg[N_STAT] = {
	"[OK]",
	"[FAIL]"
};

Error::Error(const Error &e) :
	_code(e._code),
	_msg(e._msg)
{}

/**
 * @desc: initialize Error object based on error code.
 * @param:
 *	> code	: error code.
 *	> ...   : arguments for error message, if applicable.
 */
Error::Error(const int code ...) :
	_code(code),
	_msg((char *) _errmsg[E_OK])
{
	int	len;
	va_list	args;
	va_list	args2;

	if (code >= N_ERRCODE)
		return;

	va_start(args, code);

	__va_copy(args2, args);
	len = Buffer::VSNPRINTF(0, 0, _errmsg[code], args2);
	if (len < 0) {
		perror(0);
		return;
	}
	va_end(args2);

	_msg = (char *) calloc(len + 1, sizeof(_msg));
	if (! _msg) {
		perror(0);
		return;
	}

	len = Buffer::VSNPRINTF(_msg, len, _errmsg[code], args);
	if (len < 0) {
		perror(0);
		return;
	}
}

Error::~Error()
{
	if (_msg && _msg != _errmsg[E_OK])
		free(_msg);
}

void Error::print()
{
	if (errno)
		perror(0);

	fprintf(stdout, "%s\n", _msg);
}

} /* namespace::vos */
