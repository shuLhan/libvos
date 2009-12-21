/**
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
	int appendi(int i, const int base);
	int appendd(double d);
	int append(const Buffer *bfr);
	int append_raw(const char *bfr, int len);
	int concat(const char *bfr, ...);
	int aprint(const char *fmt, ...);
	int vprint(const char *fmt, va_list args);

	int shiftr(const int nbyte);

	int copy(const Buffer *bfr);
	int copy_raw(const char *bfr, int len);
	int set(const Buffer *bfr, const Buffer *dflt);
	int set_raw(const char *bfr, const char *dflt);
	int move_to(Buffer **bfr);

	int cmp(const Buffer *bfr);
	int cmp_raw(const char *bfr);
	int like(const Buffer *bfr);
	int like_raw(const char *bfr);

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
	DISALLOW_COPY_AND_ASSIGN(Buffer);
};

} /* namespace::vos */

#endif
