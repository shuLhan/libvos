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
int Buffer::CHAR_SIZE = sizeof(char);

/**
 *	Buffer object can be used as pointer to an raw string, by
 *	passing 0 to '_l' and later set '_v' point to array of char and use
 *	'_i' to count the length of '_v'.
 *
 *	if you set '_l' to value other than zero, Buffer destructor will
 *	destroy any value in '_v' at exit.
 */
Buffer::Buffer() :
	_i(0),
	_l(0),
	_v(NULL)
{}

Buffer::~Buffer()
{
	if (_v && _l) {
		free(_v);
		_v = NULL;
	}
}

/**
 * @desc	: initialize new Buffer using contents from 'bfr' object.
 *
 * @param	:
 *	> bfr	: pointer to Buffer object.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, not enough memory.
 */
int Buffer::init(const Buffer *bfr)
{
	int s = 0;

	if (_v) {
		return copy(bfr);
	} else if (!bfr) {
		s = init_raw(NULL, 0);
	} else if (bfr->_i > 0) {
		s = init_raw(bfr->_v, bfr->_i);
	}
	return s;
}

/**
 * @desc	: initialize new Buffer using 'bfr'.
 *
 * @param	:
 *	> bfr	: array of character.
 *	> len	: length of 'bfr'.
 *
 * @return	:
 *	< 0	: success.
 *	< -E_MEM: fail, not enough memory.
 */
int Buffer::init_raw(const char *bfr, const int len)
{
	if (_v) {
		return copy_raw(bfr, len);
	} else if (bfr) {
		if (!len) {
			_l = strlen(bfr);
		} else {
			_l = len;
		}

		_v = (char *) calloc(_l + CHAR_SIZE, CHAR_SIZE);
		if (!_v) {
			return -E_MEM;
		}

		memcpy(_v, bfr, _l);
		_i = _l;
	} else {
		_i = 0;
		_l = DFLT_SIZE;
		_v = (char *) calloc(_l + CHAR_SIZE, CHAR_SIZE);
		if (!_v) {
			return -E_MEM;
		}
	}
	return 0;
}

/**
 * @desc	: initialize buffer by size.
 *
 * @param	:
 *	> size	: initial size of new Buffer object.
 *
 * @return:
 *	< 0	: success.
 *	< -E_MEM: fail, out of memory.
 */
int Buffer::init_size(const int size)
{
	if (_v) {
		return resize(_i + size);
	} else if (size > 0) {
		_i = 0;
		_l = size;
		_v = (char *) calloc(_l + CHAR_SIZE, CHAR_SIZE);
		if (!_v) {
			return -E_MEM;
		}
	}
	return 0;
}

/**
 * @desc	: function to resize buffer allocation.
 *
 * @param	:
 *	> len	: the new length for buffer.
 *
 * @return	:
 *	< 0	: success.
 *	< -E_MEM: fail, out of memory.
 */
int Buffer::resize(const int len)
{
	if (_l < len) {
		_l = len;
		_v = (char *) realloc(_v, _l + CHAR_SIZE);
		if (!_v) {
			return -E_MEM;
		}
	}
	return 0;
}

/**
 * @desc	: reset Buffer object, keep an already allocated buffer and
 *		start index from zero again.
 */
void Buffer::reset()
{
	if (_i) {
		_i = 0;
		memset(_v, '\0', _l);
	}
}

/**
 * @desc	: remove leading and trailing white-space from buffer.
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
 * @desc		: append one character to the end of buffer.
 *
 * @param:
 *	> c		: a character to be added.
 *
 * @return:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 */
int Buffer::appendc(const char c)
{
	if (_i + CHAR_SIZE > _l) {
		_l += DFLT_SIZE;
		_v = (char *) realloc(_v, _l + CHAR_SIZE);
		if (!_v) {
			return -E_MEM;
		}
	}
	_v[_i]		= c;
	_v[++_i]	= '\0';
	return 0;
}

/**
 * @desc	: append a number to the end of buffer.
 *
 * @param:
 *	> i	: a number to be appended to the end buffer.
 *	> base	: optional, default is 10; base of number.
 *
 * @return:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Buffer::appendi(int i, const int base)
{
	int	s;
	int	x = 0;
	char	integer[32];

	if (i < 0) {
		s = appendc('-');
		if (s < 0)
			return s;

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
		s = appendc(integer[x]);
		if (s < 0)
			return s;

		--x;
	}
	return 0;
}

/**
 * @desc	: append a float number to the end of buffer.
 *
 * @param	:
 *	> f	: float number.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Buffer::appendd(double d)
{
	int	s;
	char	f[32];

	s = ::snprintf(f, 31, "%f", d);
	if (s < 0)
		return -E_PRINT;

	return append_raw(f, 0);
}

/**
 * @desc	: append a Buffer object data to the end of buffer.
 *
 * @param	:
 *	> bfr	: Buffer object to be appended to the end of buffer.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Buffer::append(const Buffer *bfr)
{
	if (!bfr)
		return 0;

	return append_raw(bfr->_v, bfr->_i);
}

/**
 * @desc	: append a raw buffer to the end of buffer.
 *
 * @param	:
 *	> bfr	: a raw buffer to be appended.
 *	> len	: optional, length of 'bfr'.
 *
 * @return:
 *	> >=0	: success, number of bytes appended to the end of buffer.
 *	< <0	: fail.
 */
int Buffer::append_raw(const char *bfr, int len)
{
	int s;

	if (!bfr)
		return 0;
	if (!len) {
		len = strlen(bfr);
		if (!len)
			return 0;
	}

	s = resize(_i + len);
	if (s < 0)
		return s;

	memcpy(&_v[_i], bfr, len);

	_i	+= len;
	_v[_i]	= '\0';

	return len;
}

/**
 * @desc	: append several raw buffer to the end of buffer.
 *
 * @param	:
 *	> bfr	: array of characters.
 *	> ...	: others buffer. (the last parameter must be NULL).
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Buffer::concat(const char *bfr, ...)
{
	int		s;
	int		len	= 0;
	va_list		al;
	const char	*p	= NULL;

	if (!bfr)
		return 0;

	va_start(al, bfr);
	p = bfr;
	while (p) {
		len	= strlen(p);
		s	= resize(_i + len);
		if (s < 0)
			return s;

		memcpy(&_v[_i], p, len);
		_i	+= len;
		p	= va_arg(al, const char *);
	}
	va_end(al);

	_v[_i] = '\0';

	return 0;
}

/**
 * @desc	: append a format string to the end of buffer.
 *	
 * @param	:
 *	> fmt	: format string and their value.
 *	> ...	: format string parameter.
 *
 * @return:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Buffer::aprint(const char *fmt, ...)
{
	int	s;
	int	len;
	va_list	args;

	va_start(args, fmt);
	len = Buffer::VSNPRINTF(0, 0, fmt, args);
	va_end(args);

	if (len < 0)
		return -E_MEM;

	++len;
	s = resize(_i + len);
	if (s < 0)
		return s;

	va_start(args, fmt);
	len = Buffer::VSNPRINTF(&_v[_i], len, fmt, args);
	va_end(args);

	if (len < 0)
		return -E_MEM;

	_i	+= len;
	_v[_i]	= '\0';

	return 0;
}

/**
 * @desc	: append a format string to the end of buffer.
 *
 *	NOTE	: vsnprintf() return length of string without '\0'.
 *
 * @param	:
 *	> fmt	: format string to be appended.
 *	> args	: list of arguments for 'fmt'.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, out of memory.
 */
int Buffer::vprint(const char *fmt, va_list args)
{
	int	s;
	int	len;
	va_list	args2;

	va_copy(args2, args);
	len = Buffer::VSNPRINTF(0, 0, fmt, args2);
	va_end(args2);

	if (len < 0)
		return -E_MEM;

	++len;
	s = resize(_i + len);
	if (s < 0)
		return s;

	len = Buffer::VSNPRINTF(&_v[_i], len, fmt, args);
	if (len < 0)
		return -E_MEM;

	_i	+= len;
	_v[_i]	= '\0';

	return 0;
}

/**
 * @desc	: move contents of buffer n bytes to the right.
 *
 * @param	:
 *	> nbyte	: size of buffer to be left on the right side.
 *
 * @return	:
 *	< 0	: success.
 *	< -E_MEM: fail.
 */
int Buffer::shiftr(const int nbyte)
{
	if (_i + nbyte > _l) {
		_l += nbyte;
		_v = (char *) realloc(_v, (_l + CHAR_SIZE));
		if (!_v) {
			return -E_MEM;
		}
	}

	memmove(&_v[nbyte], _v, _i);
	memset(_v, '\0', nbyte);
	_i += nbyte;

	return 0;
}

/**
 * @desc	: copy the content of Buffer object.
 *
 * @param	:
 *	> bfr	: a pointer to Buffer object.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, out of memory.
 */
int Buffer::copy(const Buffer *bfr)
{
	if (!bfr)
		return 0;

	return copy_raw(bfr->_v, bfr->_i);
}

/**
 * @desc		: copy the content of raw buffer.
 *
 * @param		:
 *	> bfr		: a pointer to raw buffer.
 *	> len		: optional, length of 'bfr'.
 *
 * @return:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 */
int Buffer::copy_raw(const char *bfr, int len)
{
	if (!bfr) {
		return 0;
	}
	if (!len) {
		len = strlen(bfr);
		if (!len)
			return 0;
	}
	if (_l < len) {
		_l = len;
		_v = (char *) realloc(_v, _l + CHAR_SIZE);
		if (!_v)
			return -E_MEM;
	}

	memcpy(_v, bfr, len);
	_i	= len;
	_v[_i]	= '\0';

	return 0;
}

/**
 * @desc	: set contents of Buffer to 'bfr'.
 *
 * @param	:
 *	> bfr	: pointer to Buffer object.
 *	> dflt	: if 'bfr' is empty or nil use contents of 'dflt'.
 *
 * @return	:
 *	< 0	: success, or 'bfr' is nil.
 *	< <0	: fail.
 */
int Buffer::set(const Buffer *bfr, const Buffer *dflt)
{
	int s = 0;

	if (bfr) {
		s = copy_raw(bfr->_v, bfr->_i);
	} else if (dflt) {
		s = copy_raw(dflt->_v, dflt->_i);
	}
	return s;
}

/**
 * @desc: set buffer to 'bfr' or to 'dflt' if 'bfr' is null/empty.
 *
 * @param:
 *	> bfr	: a pointer to raw buffer.
 *	> dflt	: default value to be copied to buffer if 'bfr' is empty.
 */
int Buffer::set_raw(const char *bfr, const char *dflt)
{
	int s = 0;

	if (bfr)
		s = copy_raw(bfr, 0);
	else if (dflt)
		s = copy_raw(dflt, 0);

	return s;
}

/**
 * @desc: move contents to another Buffer object, 'bfr',
 *	leave current object to be an empty Buffer object.
 *
 * @param:
 *	> bfr	: a pointer to another buffer.
 *
 * @return:
 *	< 0	: success.
 *	< -E_MEM: fail.
 */
int Buffer::move_to(Buffer **bfr)
{
	if ((*bfr)) {
		delete (*bfr);
	}

	(*bfr) = new Buffer();
	if (! (*bfr))
		return -E_MEM;

	(*bfr)->_l	= _l;
	(*bfr)->_i	= _i;
	(*bfr)->_v	= _v;
	_i		= 0;
	_l		= 0;
	_v		= NULL;

	return 0;
}

/**
 * @desc	: case sensitive compare, i.e: "A" != "a".
 *
 * @param	:
 *	> bfr	: pointer to Buffer object.
 *
 * @return	:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 */
int Buffer::cmp(const Buffer *bfr)
{
	if (!bfr)
		return 1;

	return cmp_raw(bfr->_v);
}

/**
 * @desc	: case sensitive compare, i.e: "A" != "a".
 *
 * @param	:
 *	> bfr	: array of character.
 *
 * @return	:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 */
int Buffer::cmp_raw(const char *bfr)
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
 * @desc	: case not sentisive compare, i.e: "A" == "a".
 *
 * @param	:
 *	> bfr	: pointer to Buffer object, to compare to.
 *
 * @return	:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 */
int Buffer::like(const Buffer *bfr)
{
	if (! bfr)
		return 1;

	return like_raw(bfr->_v);
}

/**
 * @desc	: case not sentisive compare, i.e: "A" == "a".
 *
 * @param	:
 *	> bfr	: array of characters.
 *
 * @return:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 */
int Buffer::like_raw(const char *bfr)
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
 * @desc	: function to check if buffer is empty or not;
 * @return:
 *	< 1	: if buffer is empty (_i == 0).
 *	< 0	: if buffer is not empty (_i != 0).
 */
int Buffer::is_empty()
{
	return !_i;
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
		if (!(i % 8)) {
			putchar('\t');
			for (; j < i; ++j) {
				printf(" %-2c", isprint(_v[j]) ? _v[j] : '.' );
			}
			j = i;
			putchar('\n');
		}
		printf(" %02X", _v[i] < 0 ? 0x80 & _v[i] : _v[i]);
	}

	if ((i % 8)) {
		for (int k = i % 8; k < 8; ++k) {
			printf("   ");
		}
	}

	putchar('\t');
	for (; j < i; ++j) {
		printf(" %-2c", isprint(_v[j]) ? _v[j] : '.' );
	}
	printf("\n\n");
}

/**
 * @desc	: create and initialize a new Buffer object based on
 *		data on 'bfr' object.
 *
 * @param	:
 *	> o	: output, a new Buffer object.
 *	> bfr	: pointer to Buffer object, to be copied to new object.
 *
 * @return	:
 *	< 0	: success, pointer to new Buffer object.
 *	< <0	: fail.
 */
int Buffer::INIT(Buffer **o, const Buffer *bfr)
{
	int s = -E_MEM;

	(*o) = new Buffer();
	if ((*o)) {
		s = (*o)->init(bfr);
		if (s != 0) {
			delete (*o);
			(*o) = NULL;
		}
	}
	return s;
}

/**
 * @desc	: create and initialized a new Buffer object based on
 *		raw buffer, 'bfr'.
 *
 * @param	:
 *	> o	: output, a new Buffer object.
 *	> bfr	: array of characters.
 *
 * @return	:
 *	< 0	: success, a new Buffer object.
 *	< <0	: fail.
 */
int Buffer::INIT_RAW(Buffer **o, const char *bfr)
{
	int s = -E_MEM;
	
	(*o) = new Buffer();
	if ((*o)) {
		s = (*o)->init_raw(bfr, 0);
		if (s < 0) {
			delete (*o);
			(*o) = NULL;
		}
	}
	return s;
}

/**
 * @desc	: create and initialized a new Buffer object with size
 *		is equal to 'size'.
 *
 * @param	:
 *	> o	: output, a new Buffer object.
 *	> size	: size of buffer for a new Buffer object.
 *
 * @return	:
 *	< 0	: success, a new Buffer object.
 *	< <0	: fail.
 */
int Buffer::INIT_SIZE(Buffer **o, const int size)
{
	int s = -E_MEM;

	(*o) = new Buffer();
	if ((*o)) {
		s = (*o)->init_size(size);
		if (s < 0) {
			delete (*o);
			(*o) = NULL;
		}
	}
	return s;
}

/**
 * @desc	: create an output of formatted string 'fmt' and their
 *		arguments, 'args', to buffer 'bfr'.
 *
 *	user must have allocated buffer prior calling these function.
 *
 * @param	:
 *	> bfr	: <out> return value as string as in 'fmt'.
 *	> len	: length of format string to be copied to 'bfr'.
 *	> fmt	: format string.
 *	> args	: list of arguments for 'fmt'.
 *
 * @return	:
 *	< >=0	: success, length of buffer.
 *	< <0	: fail.
 */
int Buffer::VSNPRINTF(char *bfr, int len, const char *fmt, va_list args)
{
	int		s;
	const char	*p = fmt;
	Buffer		b;

	b.init(NULL);
	while (*p) {
		while (*p && *p != '%') {
			s = b.appendc(*p);
			if (s < 0)
				return s;
			*p++;
		}
		if (!*p)
			break;

		*p++;
		if (!*p)
			break;

		switch (*p) {
		case 'c':
			s = b.appendc(va_arg(args, int));
			if (s < 0)
				return s;
			break;
		case 'd':
		case 'i':
			s = b.appendi(va_arg(args, int), 10);
			if (s < 0)
				return s;
			break;
		case 's':
			s = b.append_raw(va_arg(args, const char *), 0);
			if (s < 0)
				return s;
			break;
		case 'l':
			*p++;
			if (!*p)
				goto out;
			switch (*p) {
			case 'd':
			case 'i':
				s = b.appendi(va_arg(args, int), 10);
				if (s < 0)
					return s;
				break;
			default:
				s = b.appendc(*p);
				if (s < 0)
					return s;
				break;
			}
			break;
		case 'f':
			s = b.appendd(va_arg(args, double));
			if (s < 0)
				return s;
			break;
		default:
			s = b.appendc('%');
			if (s < 0)
				return s;

			s = b.appendc(*p);
			if (s < 0)
				return s;
			break;
		}
		*p++;
	}
out:
	if (bfr) {
		len = len < b._i ? len : b._i;
		memcpy(bfr, b._v, len);
	}

	return b._i;
}

/**
 * @desc	: remove leading and trailing white-space from buffer.
 *
 * @param	:
 *	> bfr 	: buffer to be trimmed.
 *	> len	: optional, length of 'bfr'.
 *
 * @return	:
 *	< >=0	: success, length of 'bfr' after trimmed left and right.
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
