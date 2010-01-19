/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_MACRO_HPP
#define	_LIBVOS_MACRO_HPP	1

#include <stdlib.h>

namespace vos {
/**
 * @ref:
 * http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
 *
 * A macro to disallow the copy constructor and 'operator=' functions.
 * This should be used in the 'private:' declarations for a class.
 */
#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
	TypeName(const TypeName&);		\
	void operator=(const TypeName&)


extern int LIBVOS_DEBUG;

enum _errcode {
	E_OK		= 0,

	E_INVALID_PARAM	= 2,
	E_MEM,
	E_PRINT,
	E_HOST_UNKNOWN,

	E_FILE_OPEN,
	E_FILE_READ,
	E_FILE_WRITE,
	E_FILE_SEEK,
	E_FILE_END,

	E_CFG_BAD,

	E_SOCK_CREATE,
	E_SOCK_ADDR_INV,
	E_SOCK_ADDR_RESOLV,
	E_SOCK_ADDR_REUSE,

	E_SOCK_BIND,
	E_SOCK_LISTEN,
	E_SOCK_CONNECT,
	E_SOCK_READ,

	E_SOCK_WRITE,
	E_SOCK_SELECT,
	E_SOCK_ACCEPT,
	E_SOCK_TIMEOUT,

	E_RECORD_INV_COLUMN,
	E_RECORD_REJECT,

	E_OCI,

	N_ERRCODE
};

extern const char *_errmsg[N_ERRCODE];

enum _libvos_stat {
	STAT_OK	= 0,
	STAT_FAIL,
	N_STAT
};

} /* namespace::vos */

#endif
