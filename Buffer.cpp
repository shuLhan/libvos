/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Buffer.hpp"

namespace vos {

/* a 16 characters for <= 16 base digits */
static char __digits[17] = "0123456789ABCDEF";

/* default buffer size */
int Buffer::DFLT_SIZE = 15;
int Buffer::CHAR_SIZE = 1;

Buffer::Buffer() :
	_i(0),
	_l(DFLT_SIZE),
	_v(NULL)
{
	_v = (char *) calloc(_l + CHAR_SIZE, CHAR_SIZE);
	if (! _v)
		throw Error(E_MEM);
}

/**
 * @desc: initialize buffer.
 *
 *	Buffer object can be used as pointer to an old or raw string, by
 *	passing 0 to '_l' and later set '_v' point to pointer char and use
 *	'_i' to count the length of '_v'.
 *
 *	if you set '_l' to value other than zero, Buffer destructor will
 *	destroy any value in '_v' at exit.
 *
 * @param:
 *	> size : initial size of new Buffer object.
 *
 * @exception:
 *	< E_MEM: out of memory.
 */
Buffer::Buffer(const int size) :
	_i(0),
	_l(size),
	_v(NULL)
{
	if (size) {
		_v = (char *) calloc(size + CHAR_SIZE, CHAR_SIZE);
		if (! _v)
			throw Error(E_MEM);
	}
}

Buffer::Buffer(const char *bfr) :
	_i(0),
	_l(0),
	_v(NULL)
{
	if (! bfr) {
		_v = (char *) calloc(DFLT_SIZE + CHAR_SIZE, CHAR_SIZE);
		if (! _v)
			throw Error(E_MEM);
	} else {
		_l = strlen(bfr);

		_v = (char *) calloc(_l + CHAR_SIZE, CHAR_SIZE);
		if (! _v)
			throw Error(E_MEM);

		memcpy(_v, bfr, _l);
		_i = _l;
	}
}

Buffer::Buffer(const Buffer *bfr) :
	_i(0),
	_l(0),
	_v(NULL)
{
	if (! bfr) {
		_v = (char *) calloc(DFLT_SIZE + CHAR_SIZE, CHAR_SIZE);
		if (! _v)
			throw Error(E_MEM);
	} else if (bfr->_i) {
		_l = bfr->_i;

		_v = (char *) calloc(_l + CHAR_SIZE, CHAR_SIZE);
		if (! _v)
			throw Error(E_MEM);

		memcpy(_v, bfr->_v, _l);
		_i = _l;
	}
}

Buffer::~Buffer()
{
	if (_v && _l) {
		free(_v);
		_v = NULL;
	}
}

/**
 * @desc:
 *	function to resize buffer allocation.
 *
 * @param:
 *	> len	: the new length for buffer.
 *
 * @exception:
 *	< E_MEM	: out of memory.
 */
void Buffer::resize(const int len)
{
	if (_l < (_i + len)) {
		_l += len;
		_v = (char *) realloc(_v, (_l + CHAR_SIZE));
		if (! _v)
			throw Error(E_MEM);
	}
}

void Buffer::dump()
{
	printf("%d|%d|%s|\n", _i, _l, _v);
}

/**
 * @desc: dump buffer in two column, hexadecimal in left and printable
 *	characters in the right.
 */
void Buffer::dump_hex()
{
	int i = 0;
	int j = 0;

	for (; i < _i; ++i) {
		if (! (i % 8)) {
			putchar('\t');
			for (; j < i; ++j) {
				printf(" %-2c", isprint(_v[j]) ? _v[j] : '.' );
			}
			j = i;
			putchar('\n');
		}
		printf(" %02X", _v[i] < 0 ? 0x80 & _v[i] : _v[i]);
	}

	for (int k = i % 8; k < 8; ++k) {
		printf("   ");
	}

	putchar('\t');
	for (; j < i; ++j) {
		printf(" %-2c", isprint(_v[j]) ? _v[j] : '.' );
	}
	printf("\n\n");
}

/**
 * @desc:
 *	reset Buffer object, keep an already allocated buffer and start index
 *	from zero again.
 */
void Buffer::reset()
{
	if (_i) {
		_i = 0;
		memset(_v, '\0', _l);
	}
}

/**
 * @desc:
 *	remove leading and trailing white-space from buffer.
 */
void Buffer::trim()
{
	int x = 0;

	do {
		--_i;
	} while (_i >= 0 && isspace(_v[_i]));

	while (x < _i && isspace(_v[x])) {
		++x;
	}

	if (x > 0 && x <= _i) {
		_i = _i - x + CHAR_SIZE;
		memmove(_v, &_v[x], _i);
	} else {
		++_i;
	}

	_v[_i] = '\0';
}

/**
 * @desc:
 *	append one character to the end of buffer.
 *
 * @param:
 *	> c	: a character to be add to buffer.
 *
 * @exception:
 *	< E_MEM	: out of memory.
 */
void Buffer::appendc(const char c)
{
	if (_i + CHAR_SIZE > _l) {
		_l += DFLT_SIZE;
		_v = (char *) realloc(_v, (_l + CHAR_SIZE));
		if (! _v)
			throw Error(E_MEM);
	}
	_v[_i]		= c;
	_v[++_i]	= '\0';
}

/**
 * @desc: append a number to the end of buffer.
 *
 * @param:
 *	> i	: a number to be appended to the end buffer.
 *	> base	: optional, default is 10; base of number.
 */
void Buffer::appendi(int i, const int base)
{
	int	x = 0;
	char	integer[32];

	if (i < 0) {
		appendc('-');
		i = -(i);
	}
	while (i >= 0) {
		integer[x] = __digits[i % base];
		i = i / base;
		++x;
		if (0 == i)
			break;
	}
	--x;
	while (x >= 0) {
		appendc(integer[x]);
		--x;
	}
}

/**
 * @desc: append a float number to the end of buffer.
 *
 * @param:
 *	> f : float number.
 */
void Buffer::appendd(double d)
{
	char f[32];

	snprintf(f, 31, "%f", d);

	append(f, 0);
}

/**
 * @desc: append a Buffer object data to the end of buffer.
 *
 * @param:
 *	> bfr : Buffer object to be appended to the end of buffer.
 */
void Buffer::append(const Buffer *bfr)
{
	if (bfr)
		append(bfr->_v, bfr->_i);
}

/**
 * @desc: append a raw buffer to the end of buffer.
 *
 * @param:
 *	> bfr : a raw buffer to be appended.
 *	> len : optional, length of 'bfr'.
 */
void Buffer::append(const char *bfr, int len)
{
	if (! bfr)
		return;
	if (! len) {
		len = strlen(bfr);
		if (! len)
			return;
	}

	resize(len);
	memcpy(&_v[_i], bfr, len);

	_i	+= len;
	_v[_i]	= '\0';
}

/**
 * @desc: append several raw buffer to the end of buffer.
 *
 *	NOTE: the last parameter must be NULL.
 *
 * @param:
 *	> bfr ... : one or more buffer data.
 */
void Buffer::concat(const char *bfr, ...)
{
	int		len	= 0;
	va_list		al;
	const char	*p	= NULL;

	if (! bfr)
		return;

	va_start(al, bfr);
	p = bfr;
	while (p) {
		len = strlen(p);
		resize(len);
		memcpy(&_v[_i], p, len);
		_i += len;
		p = va_arg(al, const char *);
	}
	va_end(al);

	_v[_i] = '\0';
}

/**
 * @desc: append a format string to the end of buffer.
 *	
 * @param:
 *	> fmt	: format string and their value.
 *	> ...	: format string parameter.
 * @return:
 *	< 0	: success.
 *	< E_MEM	: fail, out of memory.
 */
void Buffer::aprint(const char *fmt, ...)
{
	int	len;
	va_list	args;

	va_start(args, fmt);
	len = Buffer::VSNPRINTF(0, 0, fmt, args);
	va_end(args);

	if (len < 0)
		throw Error(E_MEM);

	++len;
	resize(len);

	va_start(args, fmt);
	len = Buffer::VSNPRINTF(&_v[_i], len, fmt, args);
	va_end(args);

	if (len < 0)
		throw Error(E_MEM);

	_i	+= len;
	_v[_i]	= '\0';
}

/**
 * @desc: append a format string to the end of buffer.
 *
 *	NOTE: vsnprintf() return length of string without '\0'.
 *
 * @param:
 *	> fmt	: format string to be appended.
 *	> args	: list of arguments for 'fmt'.
 * @return:
 *	< 0	: success.
 *	< E_MEM	: fail, out of memory.
 */
int Buffer::vprint(const char *fmt, va_list args)
{
	int	len;
	va_list	args2;

	va_copy(args2, args);
	len = Buffer::VSNPRINTF(0, 0, fmt, args2);
	va_end(args2);

	if (len < 0)
		return E_MEM;

	++len;
	resize(len);

	len = Buffer::VSNPRINTF(&_v[_i], len, fmt, args);
	if (len < 0)
		return E_MEM;

	_i	+= len;
	_v[_i]	= '\0';

	return 0;
}

/**
 * @desc: move contents of buffer n bytes to the right.
 */
void Buffer::shiftr(const int nbyte)
{
	if (_i + nbyte > _l) {
		_l += nbyte;
		_v = (char *) realloc(_v, (_l + CHAR_SIZE));
		if (! _v)
			throw Error(E_MEM);
	}

	memmove(&_v[nbyte], _v, _i);
	memset(_v, '\0', nbyte);
	_i += nbyte;
}

/**
 * @desc: copy the content of raw buffer.
 *
 * @param:
 *	> bfr	: a pointer to Buffer object.
 * @return:
 *	< 0	: success.
 *	< E_MEM	: fail, out of memory.
 */
int Buffer::copy(const Buffer *bfr)
{
	if (! bfr)
		return 0;

	return copy(bfr->_v, bfr->_i);
}

/**
 * @desc: copy the content of raw buffer.
 *
 * @param:
 *	> bfr	: a pointer to raw buffer.
 *	> len	: optional, length of 'bfr'.
 * @return:
 *	< 0	: success.
 *	< E_MEM	: fail, out of memory.
 */
int Buffer::copy(const char *bfr, int len)
{
	if (! bfr)
		return 0;

	if (! len) {
		len = strlen(bfr);
		if (! len)
			return 0;
	}

	if (_l < len) {
		_l = len;
		_v = (char *) realloc(_v, (_l + CHAR_SIZE));
		if (! _v)
			return E_MEM;
	}

	memcpy(_v, bfr, len);
	_i	= len;
	_v[_i]	= '\0';

	return 0;
}

/**
 * @desc: set buffer to 'bfr' or to 'dflt' if 'bfr' is null/empty.
 *
 * @param:
 *	> bfr	: a pointer to raw buffer.
 *	> dflt	: default value to be copied to buffer if 'bfr' is empty.
 */
void Buffer::set(const char *bfr, const char *dflt)
{
	if (bfr)
		copy(bfr, 0);
	else if (dflt)
		copy(dflt, 0);
}

/**
 * @desc: move buffer to bfr, leave current object to nil.
 *
 * @param:
 *	> bfr	: a pointer to another buffer.
 */
void Buffer::move_to(Buffer **bfr)
{
	if ((*bfr)) {
		delete (*bfr);
	}

	(*bfr)		= new Buffer(0);
	(*bfr)->_l	= _l;
	(*bfr)->_i	= _i;
	(*bfr)->_v	= _v;
	_i		= 0;
	_l		= 0;
	_v		= NULL;
}

/**
 * @desc: case sensitive compare, i.e: "A" != "a"
 *
 * @return:
 *	< 1 : this > bfr
 *	< 0 : this == bfr
 *	< -1: this < bfr
 */
int Buffer::cmp(const Buffer *bfr)
{
	if (!bfr)
		return 1;

	return cmp(bfr->_v);
}

/**
 * @desc: case sensitive compare, i.e: "A" != "a"
 *
 * @return:
 *	< 1 : this > bfr
 *	< 0 : this == bfr
 *	< -1: this < bfr
 */
int Buffer::cmp(const char *bfr)
{
	register int s;

	if (!bfr)
		return 1;

	s = strcmp(_v, bfr);
	if (s < 0)
		return -1;
	if (s > 0)
		return 1;

	return 0;
}

/**
 * @desc: case not sentisive compare, i.e: "A" == "a".
 *
 * @return:
 *	< 1	: this > bfr
 *	< 0	: this == bfr
 *	< -1	: this < bfr
 */
int Buffer::like(const Buffer *bfr)
{
	if (! bfr)
		return 1;

	return like(bfr->_v);
}

/**
 * @desc: case not sentisive compare, i.e: "A" == "a".
 *
 * @return:
 *	< 1	: this > bfr
 *	< 0	: this == bfr
 *	< -1	: this < bfr
 */
int Buffer::like(const char *bfr)
{
	register int s;

	if (! bfr)
		return 1;

	s = strcasecmp(_v, bfr);
	if (s < 0)
		return -1;
	if (s > 0)
		return 1;

	return 0;
}

/**
 * @return:
 *	< 1 : if buffer is empty (_i == 0).
 *	< 0 : if buffer is not empty (_i != 0).
 */
int Buffer::is_empty()
{
	return !_i;
}

/**
 * @desc:
 *	user must have allocated buffer prior calling these function.
 *
 * @param:
 *	> bfr	: <out> return value as string as in 'fmt'.
 *	> len	: length of format string to be copied to 'bfr'.
 *	> fmt	: format string.
 *	> args	: list of arguments for 'fmt'.
 *
 * @return:
 *	< int	: length of buffer.
 */
int Buffer::VSNPRINTF(char *bfr, int len, const char *fmt, va_list args)
{
	Buffer		b;
	const char	*p = fmt;

	while (*p) {
		while (*p && *p != '%') {
			b.appendc(*p);
			++p;
		}
		if (! *p)
			break;

		++p;
		if (! *p)
			break;

		switch (*p) {
		case 'c':
			b.appendc(va_arg(args, int));
			break;
		case 'd':
		case 'i':
			b.appendi(va_arg(args, int), 10);
			break;
		case 's':
			b.append(va_arg(args, const char *), 0);
			break;
		case 'l':
			++p;
			if (! *p)
				goto out;
			switch (*p) {
			case 'd':
			case 'i':
				b.appendi(va_arg(args, int), 10);
				break;
			default:
				b.appendc(*p);
				break;
			}
			break;
		case 'f':
			b.appendd(va_arg(args, double));
			break;
		default:
			b.appendc('%');
			b.appendc(*p);
			break;
		}
		++p;
	}
out:
	if (bfr) {
		len = len < b._i ? len : b._i;
		memcpy(bfr, b._v, len);
	}

	return b._i;
}

/**
 * @desc:
 *	remove leading and trailing white-space from buffer.
 *
 * @param:
 *	> bfr 	: buffer to be trimmed.
 *	> len	: optional, length of 'bfr'.
 *
 * @return:
 *	< int	: length of 'bfr' after trimmed left and right.
 */
int Buffer::TRIM(char *bfr, int len)
{
	int x = 0;

	if (! bfr)
		return 0;
	if (! len) {
		len = strlen(bfr);
		if (! len)
			return 0;
	}

	do {
		--len;
	} while (len >= 0 && isspace(bfr[len]));

	while (x < len && isspace(bfr[x]))
		++x;

	if (x > 0 && x <= len) {
		len = len - x + CHAR_SIZE;
		memmove(bfr, &bfr[x], len);
	} else {
		++len;
	}

	bfr[len] = '\0';

	return len;
}

} /* namespace::vos */
