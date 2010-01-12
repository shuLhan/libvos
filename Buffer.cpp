/*
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
 * @method		: Buffer::init
 * @param		:
 *	> bfr		: pointer to Buffer object.
 * @return		:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 * @desc		:
 *	Initialize a new Buffer object using contents from 'bfr' object.
 *	If 'bfr' is nil then a new empty buffer will allocated with the size
 *	is equal to DFLT_SIZE.
 */
int Buffer::init(const Buffer *bfr)
{
	if (bfr) {
		return init_raw(bfr->_v, bfr->_i);
	}
	return init_raw(NULL, 0);
}

/**
 * @method		: Buffer::init_raw
 * @param		:
 *	> bfr		: raw buffer.
 *	> len		: length of raw buffer.
 * @return		:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 * @desc		: initialize new Buffer object using raw buffer.
 */
int Buffer::init_raw(const char *bfr, const int len)
{
	if (_v) {
		return copy_raw(bfr, len);
	}
	if (bfr) {
		_i = _l = len ? len : strlen(bfr);
		if (0 == _l)
			return 0;

		_v = (char *) calloc(_l + CHAR_SIZE, CHAR_SIZE);
		if (!_v) {
			return -E_MEM;
		}

		memcpy(_v, bfr, _l);
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
 * @method		: Buffer::init_size
 * @param		:
 *	> size		: initial size of new Buffer object.
 * @return		:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 * @desc		: create Buffer object with buffer size equal to
 *                        'size'.
 */
int Buffer::init_size(const int size)
{
	if (_v) {
		return resize(_i + size);
	}
	if (size > 0) {
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
 * @method		: Buffer::resize
 * @param		:
 *	> len		: the new length for buffer.
 * @return		:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 * @desc		: change the size of buffer to 'size'.
 */
int Buffer::resize(const int size)
{
	if (_l < size) {
		_v = (char *) realloc(_v, size + CHAR_SIZE);
		if (!_v) {
			return -E_MEM;
		}
		_l	= size;
		_v[_i]	= '\0';
	}
	return 0;
}

/**
 * @method	: Buffer::reset
 * @desc	: reset Buffer object, keep an already allocated buffer and
 *                start index from zero again.
 */
void Buffer::reset()
{
	if (_i) {
		_i = 0;
		memset(_v, '\0', _l);
	}
}

/**
 * @method	: Buffer::trim
 * @desc	: remove leading and trailing white-space from buffer.
 */
void Buffer::trim()
{
	register int x = 0;

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
 * @method		: Buffer::appendc
 * @param		:
 *	> c		: a character to be added to buffer.
 * @return:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 * @desc		: append one character to the end of buffer.
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
 * @method		: Buffer::appendi
 * @param		:
 *	> i		: a number to be appended to the end buffer.
 * @return		:
 *	< >=0		: success, number of bytes appended to the end of
 *                        buffer.
 *	< -E_MEM	: fail.
 * @desc		: append an integer 'i' to the end of buffer.
 */
int Buffer::appendi(int i)
{
	register int	s	= 0;
	register int	x	= 0;
	char		rebmun[32];

	if (i < 0) {
		s = appendc('-');
		if (s < 0) {
			return s;
		}
		i = -(i);
	}
	while (i >= 0) {
		rebmun[x]	= __digits[i % NUM_BASE_10];
		i		= i / NUM_BASE_10;
		if (0 == i) {
			break;
		}
		++x;
	}
	while (x >= 0) {
		s = appendc(rebmun[x]);
		if (s < 0) {
			return s;
		}
		--x;
	}
	return 0;
}

/**
 * @method		: Buffer::appendd
 * @param		:
 *	> f		: float or double number.
 * @return		:
 *	< >=0		: success, number of bytes appended to the end of
 *                        buffer.
 *	< -E_MEM	: fail, out of memory.
 *	< -E_PRINT	: fail, invalid number format.
 * @desc		: append a float number to the end of buffer.
 *	maximum digit in fraction is six digits.
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
 * @method		: Buffer::append
 * @param		:
 *	> bfr		: Buffer object to be appended to the end of buffer.
 * @return		:
 *	< >=0		: success, number of bytes appended to the end of
 *                        buffer.
 *	< -E_MEM	: fail.
 * @desc		: append a Buffer object data to the end of buffer.
 */
int Buffer::append(const Buffer *bfr)
{
	if (!bfr)
		return 0;

	return append_raw(bfr->_v, bfr->_i);
}

/**
 * @method		: Buffer::append_raw
 * @param		:
 *	> bfr		: a raw buffer to be appended.
 *	> len		: optional, length of 'bfr'.
 * @return		:
 *	> >=0		: success, number of bytes appended to the end of
 *                        buffer.
 *	< -E_MEM	: fail.
 * @desc		: append a raw buffer to the end of buffer.
 */
int Buffer::append_raw(const char *bfr, int len)
{
	register int s;

	if (!bfr)
		return 0;
	if (!len) {
		len = strlen(bfr);
		if (len <= 0)
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
 * @method	: Buffer::concat
 * @param	:
 *	> bfr	: array of characters.
 *	> ...	: others buffer. (the last parameter must be NULL).
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: append several raw buffer to the end of buffer.
 */
int Buffer::concat(const char *bfr, ...)
{
	register int		s;
	register int		len	= 0;
	va_list			al;
	register const char	*p	= NULL;

	if (!bfr)
		return 0;

	va_start(al, bfr);
	p = bfr;
	while (p) {
		len = strlen(p);
		if (len > 0) {
			s = resize(_i + len);
			if (s < 0)
				return s;

			memcpy(&_v[_i], p, len);
			_i += len;
		}
		p = va_arg(al, const char *);
	}
	va_end(al);

	_v[_i] = '\0';

	return 0;
}

/**
 * @method	: Buffer::aprint
 * @param	:
 *	> fmt	: format string and their value.
 *	> ...	: format string parameter.
 * @return:
 *	< >=0	: success, number of bytes appended.
 *	< <0	: fail.
 * @desc	: append a formatted string to the end of buffer.
 */
int Buffer::aprint(const char *fmt, ...)
{
	register int	s;
	va_list		args;

	va_start(args, fmt);
	s = vprint(fmt, args);
	va_end(args);

	return s;
}

/**
 * @method	: Buffer::vprint
 * @param	:
 *	> fmt	: format string to be appended.
 *	> args	: list of arguments for 'fmt'.
 * @return	:
 *	< >=0	: success, number of bytes appended.
 *	< <0	: fail.
 * @desc	: append a formatted string to the end of buffer.
 *                vsnprintf() return length of string without '\0'.
 */
int Buffer::vprint(const char *fmt, va_list args)
{
	register int	s;
	register int	len;
	va_list		args2;

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

	return len;
}

/**
 * @method	: Buffer::shiftr
 * @param	:
 *	> nbyte	: size of buffer to be left on the right side.
 * @return	:
 *	< 0	: success.
 *	< -E_MEM: fail.
 * @desc	: move contents of buffer n bytes to the right.
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

	_i	+= nbyte;
	_v[_i]	= '\0';

	return 0;
}

/**
 * @method	: Buffer::copy
 * @param	:
 *	> bfr	: a pointer to Buffer object.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, out of memory.
 * @desc	: copy the content of Buffer object.
 */
int Buffer::copy(const Buffer *bfr)
{
	if (!bfr)
		return 0;

	return copy_raw(bfr->_v, bfr->_i);
}

/**
 * @method		: Buffer::copy_raw
 * @param		:
 *	> bfr		: a pointer to raw buffer.
 *	> len		: optional, length of 'bfr'.
 * @return		:
 *	< 0		: success.
 *	< -E_MEM	: fail, out of memory.
 * @desc		: copy the content of raw buffer.
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
 * @method	: Buffer::set
 * @param	:
 *	> bfr	: pointer to Buffer object.
 *	> dflt	: if 'bfr' is empty or nil use contents of 'dflt'.
 * @return	:
 *	< 0	: success, or 'bfr' is nil.
 *	< <0	: fail.
 * @desc	:
 *	set contents of Buffer to 'bfr'. This method is similar with copy()
 *	with additional parameter 'dflt', if 'bfr' is nil then 'dflt' value
 *	will be used.
 */
int Buffer::set(const Buffer *bfr, const Buffer *dflt)
{
	if (bfr) {
		return copy_raw(bfr->_v, bfr->_i);
	}
	return copy_raw(dflt->_v, dflt->_i);
}

/**
 * @method	: Buffer::set_raw
 * @param	:
 *	> bfr	: a pointer to raw buffer.
 *	> dflt	: default value to be copied to buffer if 'bfr' is empty.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: set buffer to 'bfr' or to 'dflt' if 'bfr' is null/empty.
 */
int Buffer::set_raw(const char *bfr, const char *dflt)
{
	if (bfr) {
		return copy_raw(bfr, 0);
	}
	return copy_raw(dflt, 0);
}

/**
 * @method	: Buffer::move_to
 * @param	:
 *	> bfr	: a pointer to another buffer.
 * @return	:
 *	< 0	: success.
 *	< -E_MEM: fail.
 * @desc	: move contents to another Buffer object, 'bfr',
 *                leave current object to be an empty Buffer object.
 */
int Buffer::move_to(Buffer **bfr)
{
	if ((*bfr)) {
		free((*bfr)->_v);
	} else {
		(*bfr) = new Buffer();
		if (! (*bfr))
			return -E_MEM;
	}

	(*bfr)->_l	= _l;
	(*bfr)->_i	= _i;
	(*bfr)->_v	= _v;
	_i		= 0;
	_l		= 0;
	_v		= NULL;

	return 0;
}

/**
 * @method	: Buffer::cmp
 * @param	:
 *	> bfr	: pointer to Buffer object.
 * @return	:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 * @desc	: case sensitive compare, i.e: "A" != "a".
 */
int Buffer::cmp(const Buffer *bfr)
{
	if (!bfr)
		return 1;

	return cmp_raw(bfr->_v);
}

/**
 * @method	: Buffer::cmp_raw
 * @param	:
 *	> bfr	: array of character.
 * @return	:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 * @desc	: case sensitive compare, i.e: "A" != "a".
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
 * @method	: Buffer::like
 * @param	:
 *	> bfr	: pointer to Buffer object, to compare to.
 * @return	:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 * @desc	: case not sentisive compare, i.e: "A" == "a".
 */
int Buffer::like(const Buffer *bfr)
{
	if (! bfr)
		return 1;

	return like_raw(bfr->_v);
}

/**
 * @method	: Buffer::like_raw
 * @param	:
 *	> bfr	: array of characters.
 * @return	:
 *	< 1	: this > bfr.
 *	< 0	: this == bfr.
 *	< -1	: this < bfr.
 * @desc	: case not sentisive compare, i.e: "A" == "a".
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
 * @method	: Buffer::is_empty
 * @return	:
 *	< 1	: if buffer is empty (_i == 0).
 *	< 0	: if buffer is not empty (_i != 0).
 * @desc	: function to check if buffer is empty or not;
 */
int Buffer::is_empty()
{
	return !_i;
}

/**
 * @method	: Buffer::dump
 * @desc	: dump buffer contents to standard output.
 */
void Buffer::dump()
{
	printf("%d|%d|%s|\n", _i, _l, _v ? _v : "");
}

/**
 * @method	: Buffer::dump_hex
 * @desc	: dump buffer in two column, hexadecimal in left and printable
 *                characters in the right column.
 */
void Buffer::dump_hex()
{
	register int i = 0;
	register int j = 0;

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
 * @method	: Buffer::INIT
 * @param	:
 *	> o	: output, a new Buffer object.
 *	> bfr	: pointer to Buffer object, to be copied to new object.
 * @return	:
 *	< 0	: success, pointer to new Buffer object.
 *	< <0	: fail.
 * @desc	: create and initialize a new Buffer object based on
 *                data on 'bfr' object.
 */
int Buffer::INIT(Buffer **o, const Buffer *bfr)
{
	register int s = -E_MEM;

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
 * @method	: Buffer::INIT_RAW
 * @param	:
 *	> o	: output, a new Buffer object.
 *	> bfr	: array of characters.
 * @return	:
 *	< 0	: success, a new Buffer object.
 *	< <0	: fail.
 * @desc	: create and initialized a new Buffer object based on
 *                raw buffer, 'bfr'.
 */
int Buffer::INIT_RAW(Buffer **o, const char *bfr)
{
	register int s = -E_MEM;
	
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
 * @method	: Buffer::INIT_SIZE
 * @param	:
 *	> o	: output, a new Buffer object.
 *	> size	: size of buffer for a new Buffer object.
 * @return	:
 *	< 0	: success, a new Buffer object.
 *	< <0	: fail.
 * @desc	: create and initialized a new Buffer object with size
 *                is equal to 'size'.
 */
int Buffer::INIT_SIZE(Buffer **o, const int size)
{
	register int s = -E_MEM;

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
 * @method	: Buffer::VSNPRINTF
 * @param	:
 *	> bfr	: <out> return value as string as in 'fmt'.
 *	> len	: length of format string to be copied to 'bfr'.
 *	> fmt	: format string.
 *	> args	: list of arguments for 'fmt'.
 * @return	:
 *	< >=0	: success, length of buffer.
 *	< <0	: fail.
 * @desc	:
 *	create an output of formatted string 'fmt' and their arguments,
 *	'args', to buffer 'bfr'.
 *	user must have allocated buffer prior calling these function.
 */
int Buffer::VSNPRINTF(char *bfr, int len, const char *fmt, va_list args)
{
	register int		s;
	register const char	*p	= fmt;
	Buffer			b;

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
			s = b.appendi(va_arg(args, int));
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
				s = b.appendi(va_arg(args, int));
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
 * @method	: Buffer::TRIM
 * @param	:
 *	> bfr 	: buffer to be trimmed.
 *	> len	: optional, length of 'bfr'.
 * @return	:
 *	< >=0	: success, length of 'bfr' after trimmed left and right.
 * @desc	: remove leading and trailing white-space from buffer.
 */
int Buffer::TRIM(char *bfr, int len)
{
	register int x = 0;

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
