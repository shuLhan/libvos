/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_BUFFER_HPP
#define	_LIBVOS_BUFFER_HPP	1

#include <ctype.h>
#include <limits.h>
#include "Error.hpp"

namespace vos {

#define	DFLT_BASE	10

class Buffer {
public:
	Buffer();
	explicit Buffer(const int size);
	explicit Buffer(const char *bfr);
	explicit Buffer(const Buffer *bfr);
	virtual ~Buffer();

	void resize(const int len);
	void dump();
	void dump_hex();
	void reset();
	void trim();

	void appendc(const char c);
	void appendi(int i, const int base = DFLT_BASE);
	void appendd(double d);
	void append(const Buffer *bfr);
	void append(const char *bfr, int len = 0);
	void concat(const char *bfr, ...);
	void aprint(const char *fmt, ...);
	int vprint(const char *fmt, va_list args);

	void shiftr(const int nbyte);

	int copy(const Buffer *bfr);
	int copy(const char *bfr, int len = 0);
	void set(const char *bfr, const char *dflt = NULL);
	void move_to(Buffer **bfr);

	int cmp(const Buffer *bfr);
	int cmp(const char *bfr);
	int like(const Buffer *bfr);
	int like(const char *bfr);

	int is_empty();

	static int VSNPRINTF(char *bfr, int len, const char *fmt,
				va_list args);
	static int TRIM(char *bfr, int len = 0);

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
