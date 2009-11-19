/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef	_LIBVOS_ERROR_HPP
#define	_LIBVOS_ERROR_HPP	1

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macro.hpp"
#include "Buffer.hpp"

namespace vos {

extern int LIBVOS_DEBUG;

enum _errcode {
	E_OK		= 0,
	E_MEM		= 1,
	E_HOST_UNKNOWN,

	E_FILE_OPEN	= 4,
	E_FILE_READ,
	E_FILE_WRITE,
	E_FILE_SEEK,
	E_FILE_END,	/* 8 */

	E_CFG_BAD	= 12,

	E_SOCK_CREATE	= 16,
	E_SOCK_ADDR_INV,
	E_SOCK_ADDR_RESOLV,
	E_SOCK_ADDR_REUSE,
	E_SOCK_BIND,	/* 20 */
	E_SOCK_CONNECT,
	E_SOCK_READ,
	E_SOCK_WRITE,
	E_SOCK_SELECT,
	E_SOCK_ACCEPT,

	E_OCI		= 28,

	N_ERRCODE
};

enum _libvos_stat {
	STAT_OK	= 0,
	STAT_FAIL,
	N_STAT
};

extern const char *_errmsg[N_ERRCODE];
extern const char *_stat_msg[N_STAT];

class Error {
public:
	Error(const Error &e);
	Error(const int code ...);
	~Error();

	void print();

	int	_code;
	char	*_msg;
private:
	void operator=(const Error &e);
};

} /* namespace::vos */

#endif
