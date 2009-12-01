/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef	_LIBVOS_ERROR_HPP
#define	_LIBVOS_ERROR_HPP	1

#include "Buffer.hpp"

using vos::Buffer;

namespace vos {

extern const char *_errmsg[N_ERRCODE];
extern const char *_stat_msg[N_STAT];

class Error {
public:
	Error();
	~Error();

	int init(int code, ...);
	void print();

	int	_code;
	Buffer	_msg;
private:
	DISALLOW_COPY_AND_ASSIGN(Error);
};

} /* namespace::vos */

#endif
