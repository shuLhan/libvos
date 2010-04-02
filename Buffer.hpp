/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_BUFFER_HPP
#define	_LIBVOS_BUFFER_HPP	1

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libvos.hpp"

namespace vos {

enum _number_base {
	NUM_BASE_8	= 8,
	NUM_BASE_10	= 10,
	NUM_BASE_16	= 16,
	NUM_DFLT_BASE	= 10
};

/**
 * @class		: Buffer
 * @attr		:
 *	- _i		: index of buffer.
 *	- _l		: current size of buffer.
 *	- _v		: pointer to the raw buffer.
 *	- DFLT_SIZE	: static, default buffer size for (re)allocating a new
 *                        buffer object.
 *	- CHAR_SIZE	: static, size of one character.
 *
 * @desc		:
 *
 *	Buffer object can be used as pointer to an raw string, by
 *	passing 0 to '_l' and later set '_v' point to array of char and use
 *	'_i' to count the length of '_v'.
 *
 *	if you set '_l' to value other than zero, Buffer destructor will
 *	destroy any value in '_v' at exit.
 */
class Buffer {
public:
	Buffer();
	virtual ~Buffer();

	int init(const Buffer *bfr);
	int init_raw(const char *bfr, const int len);
	int init_size(const int size);

	int resize(const int len);
	void reset();
	void trim();

	int appendc(const char c);
	int appendi(long int i, int base = NUM_BASE_10);
	int appendui(long unsigned int i, int base = NUM_BASE_10);
	int appendd(double d);
	int append(const Buffer *bfr);
	int append_raw(const char *bfr, int len = 0);
	int concat(const char *bfr, ...);
	int aprint(const char *fmt, ...);
	int vprint(const char *fmt, va_list args);

	int shiftr(const int nbyte, int c = 0);

	int copy(const Buffer *bfr);
	int copy_raw(const char *bfr, int len = 0);
	int set(const Buffer *bfr, const Buffer *dflt);
	int set_raw(const char *bfr, const char *dflt);
	int move_to(Buffer **bfr);

	int cmp(const Buffer *bfr);
	int cmp_raw(const char *bfr);
	int like(const Buffer *bfr);
	int like_raw(const char *bfr);

	int to_int();
	long to_lint();

	int is_empty();
	void dump();
	void dump_hex();

	static int INIT(Buffer **o, const Buffer *bfr);
	static int INIT_RAW(Buffer **o, const char *bfr);
	static int INIT_SIZE(Buffer **o, const int size);
	static int VSNPRINTF(char *bfr, int len, const char *fmt,
				va_list args);
	static int TRIM(char *bfr, int len);

	int		_i;
	int		_l;
	char		*_v;

	static int	DFLT_SIZE;
	static int	CHAR_SIZE;
private:
	Buffer(const Buffer&);
	void operator=(const Buffer&);
};

} /* namespace::vos */

#endif
