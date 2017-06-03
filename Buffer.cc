/**
 * Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Buffer.hh"
#include "FmtParser.hh"

namespace vos {

const char* Buffer::__CNAME = "Buffer";

/**
 * `_digits` contains 16 characters for 16 base number.
 */
static const char __digits[17] = "0123456789ABCDEF";

enum __print_flag {
	FL_LEFT_ADJUST	= (1 << 0)
,	FL_SIGN		= (1 << 1)
,	FL_ZERO_PAD	= (1 << 2)
,	FL_WIDTH	= (1 << 3)
,	FL_PREC		= (1 << 4)
,	FL_OCTAL	= (1 << 5)
,	FL_HEX		= (1 << 6)
,	FL_NUMBER	= (1 << 7)
,	FL_ALT_OUT	= (1 << 8)
,	FL_SHORT	= (1 << 9)
,	FL_LONG		= (1 << 10)
,	FL_LONG_DBL	= (1 << 11)
};

/**
 * Variable CHAR_SIZE is a constant for size of char, to minimize calling the
 * sizeof.
 */
const uint8_t Buffer::CHAR_SIZE = sizeof(char);

/**
 * Variable DFLT_SIZE is a constant for default buffer size.
 */
const uint16_t Buffer::DFLT_SIZE = 16;


/**
 * Method CMP_OBJECTS() will compare object of buffer `x` and `y`.
 *
 * It will return,
 *
 * (0) 0 if x and y is NULL,
 * (1) 0 if x and y has the same content,
 * (2) -1 if x is NULL and y is not NULL,
 * (3) -1 if x is less than y
 * (4) 1 if x is not NULL and y is NULL
 * (5) 1 if x is greater than y
 */
int Buffer::CMP_OBJECTS(Object* x, Object* y)
{
	if (x == y) {
		return 0;
	}
	if (x == NULL) {
		return -1;
	}
	if (y == NULL) {
		return 1;
	}

	Buffer* bx = (Buffer*) x;

	return bx->cmp(y);
}

/**
 * Method PARSE_INT() will parse an integer value from string `pp` and save it
 * to `v`.
 *
 * It will return 0 if integer value successfully parsed and set `pp` to the
 * next invalid character or NULL if entire char is valid integer value.
 *
 * It will return `-1` if underflow/overflow or other error occured, value of
 * `v` and `pp` will not change.
 */
int Buffer::PARSE_INT(char** pp, int* v)
{
	long int lv = 0;
	char* p = (*pp);
	char* end = p;

	errno = 0;

	lv = strtol(p, &end, 10);

	if (errno) {
		return -1;
	}

	// value out of integer range.
	if (lv > INT_MAX || lv < INT_MIN) {
		errno = ERANGE;
		return -1;
	}

	(*pp) = end;
	(*v) = (int) lv;

	return 0;
}

/**
 * Method TRIM(bfr,len) will remove leading and trailing white-spaces from
 * buffer `bfr`.
 *
 * On success it will return length of `bfr` after trimmed.
 */
size_t Buffer::TRIM(char* bfr, size_t len)
{
	if (! bfr) {
		return 0;
	}
	if (! len) {
		len = strlen(bfr);
		if (! len) {
			return 0;
		}
	}

	do {
		--len;
	} while (len > 0 && isspace(bfr[len]));

	if (len == 0) {
		if (! isspace(bfr[len])) {
			++len;
		}
	} else {
		size_t x = 0;

		while (x < len && isspace(bfr[x])) {
			++x;
		}
		if (x > 0 && x <= len) {
			len = len - x + CHAR_SIZE;
			memmove(bfr, &bfr[x], len);
		} else {
			++len;
		}
	}

	bfr[len] = '\0';

	return len;
}

/**
 * Method `Buffer(size)` will allocated a buffer with `size`.
 */
Buffer::Buffer(const size_t size)
:	Object()
,	_i(0)
,	_l(0)
,	_v(NULL)
{
	if (size) {
		resize(size);
	} else {
		resize(Buffer::DFLT_SIZE);
	}
}

/**
 * Method `Buffer(v)` will create a new Buffer object and initialize
 * its content from `v` with length of `v` is defined by `vlen`.
 */
Buffer::Buffer(const char* v, size_t vlen)
:	Object()
,	_i(0)
,	_l(0)
,	_v(NULL)
{
	if (v && !vlen) {
		vlen = strlen(v);
	}
	if (vlen) {
		copy_raw(v, vlen);
	} else {
		resize(DFLT_SIZE);
	}
}

/**
 * Method `Buffer(bfr)` will create a new Buffer object and initialize its
 * content from another Buffer `bfr`.
 *
 * If `bfr` is empty, it will create the buffer with the same size as `bfr`.
 */
Buffer::Buffer(const Buffer* bfr)
:	Object()
,	_i(0)
,	_l(0)
,	_v(NULL)
{
	if (bfr) {
		if (bfr->is_empty()) {
			resize(bfr->size());
		} else {
			copy(bfr);
		}
	} else {
		resize(DFLT_SIZE);
	}
}

/**
 * Method `~Buffer()` will release the allocated buffer to the system.
 */
Buffer::~Buffer()
{
	release();
}

/**
 * Method `detach()` will release the allocated buffer and return it to be
 * used by the caller.
 */
char* Buffer::detach()
{
	char* v = _v;

	_v = NULL;
	_l = 0;
	_i = 0;

	return v;
}

/**
 * Method `release()` will release the allocated buffer to the system without
 * deleting the object it self.
 */
void Buffer::release()
{
	if (_v && _l) {
		free(_v);
		_v = NULL;
	}
	_l = 0;
	_i = 0;
}

/**
 * Method `reset(c)` will reset the content of buffer to `c`, all of them,
 * keep an already allocated buffer and start index from zero again.
 */
void Buffer::reset(int c)
{
	if (_i) {
		_i = 0;
		memset(_v, c, _l);
	}
}

/**
 * Method `trim()` will remove leading and trailing white-space in buffer.
 */
void Buffer::trim()
{
	_i = TRIM(_v, _i);
}

/**
 * Method `truncate(len, c)` will cut buffer until size `len`.
 */
void Buffer::truncate(const size_t len)
{
	if (len >= _i) {
		return;
	}
	_i = len;
	_v[_i] = 0;
}

/**
 * Method `is_empty()` will return `0` if current buffer is empty or any
 * positive integer if is not empty.
 */
int Buffer::is_empty() const
{
	return _i == 0;
}

/**
 * Method `len()` will return the length of buffer that has been used.
 */
size_t Buffer::len() const
{
	return _i;
}

/**
 * Method `set_len(len)` will set buffer length to `len`. If buffer size is
 * smaller than `len` then it will resized to `len + 1`.
 *
 * It will return `0` on success, or `-1` when fail to resize the buffer.
 */
int Buffer::set_len(size_t len)
{
	if (len > _l) {
		int s = resize(len + 1);
		if (s != 0) {
			return -1;
		}
	}

	_i = len;

	return 0;
}

/**
 * Method `size()` will return the size of buffer (used + unused).
 */
size_t Buffer::size() const
{
	return _l;
}

/**
 * Method `resize(size)` will resize the buffer to `size`.
 *
 * If 'size' is less than current buffer size, no reallocation will be
 * happenend.
 *
 * It will return `0` on success or `-1` when fail to reallocate more memory.
 */
int Buffer::resize(size_t size)
{
	char* newv = NULL;

	if (size <= _l) {
		return 0;
	}

	newv = (char*) realloc(_v, size + CHAR_SIZE);
	if (!newv) {
		return -1;
	}

	_v	= newv;
	_v[_i]	= '\0';
	_l	= size;

	return 0;
}

/**
 * Method `v()` will return content of buffer at index `idx`.
 *
 * If `idx` is not defined, then it will default to zero.
 *
 * If `idx` is out of current buffer index range, then it will return NULL.
 */
const char* Buffer::v(size_t idx) const
{
	if (idx > _i) {
		return NULL;
	}
	return &_v[idx];
}

/**
 * Method `char_at(idx)` will return a single byte character of buffer at
 * index `idx`.
 *
 * If `idx` is out of range of current buffer index it will return 0.
 */
char Buffer::char_at(size_t idx)
{
	if (idx > _i) {
		return 0;
	}
	return _v[idx];
}

/**
 * Method `set_char_at(idx, v)` will set a single byte of buffer at position
 * `idx` to `v`.
 *
 * It will return 0 on success, or `-1` if `idx` is out of range.
 */
int Buffer::set_char_at(size_t idx, char v)
{
	if (idx >= _i) {
		return -1;
	}

	_v[idx] = v;

	return 0;
}

/**
 * Method `copy(bfr)` will copy content of buffer `bfr` to its own buffer.
 *
 * If `bfr` is nil, no buffer changes happened.
 *
 * On success, it will return `0`, otherwise `-1`.
 */
int Buffer::copy(const Buffer* bfr)
{
	if (!bfr) {
		return 0;
	}
	return copy_raw(bfr->_v, bfr->_i);
}

/**
 * Method `copy_raw(bfr, len)` will copy `bfr` with length `len` to current
 * buffer.  If `len` is zero, and its the default value, then this function
 * will compute the buffer length.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::copy_raw(const char* bfr, size_t len)
{
	if (!bfr) {
		return 0;
	}
	if (len == 0) {
		len = strlen(bfr);
		if (!len) {
			return 0;
		}
	}
	if (resize(len) < 0) {
		return -1;
	}

	memcpy(_v, bfr, len);
	_i	= len;
	_v[_i]	= '\0';

	return 0;
}

/**
 * Method `copy_raw_at(idx, v, vlen)` will replace buffer at index `idx` with
 * the content of `v` with length of `vlen`.
 *
 * If `v` is null, it will truncate the buffer until `idx`.
 *
 * Unlike `copy_raw()` it will not move the buffer index if `vlen` is less
 * than current index.
 *
 * It will return `0` on success, or `-1` when no memory left.
 */
int Buffer::copy_raw_at(size_t idx, const char* v, size_t vlen)
{
	if (!v) {
		return 0;
	}

	size_t growth = idx;

	if (!vlen) {
		vlen = strlen(v);
		if (!vlen) {
			return 0;
		}

		growth += vlen;
	}

	int s = resize(growth);
	if (s) {
		return -1;
	}

	if (vlen == 1) {
		_v[idx] = v[0];
	} else {
		memcpy(&_v[idx], v, vlen);

		if (growth > _i) {
			_i = growth;
			_v[_i] = 0;
		}
	}

	return 0;
}

/**
 * Method `set(bfr, dflt)` will set contents of Buffer to 'bfr'. This method
 * is similar with `copy()` with additional parameter 'dflt'; if 'bfr' is nil
 * or empty then 'dflt' value will be used.
 *
 * If both are nil or empties then buffer content will not changes.
 *
 * On success, it will return `0`, or `-1` otherwise.
 */
int Buffer::set(const Buffer* bfr, const Buffer* dflt)
{
	if (bfr && !bfr->is_empty()) {
		return copy_raw(bfr->_v, bfr->_i);
	}
	if (dflt && !dflt->is_empty()) {
		return copy_raw(dflt->_v, dflt->_i);
	}
	return 0;
}

/**
 * Method `set_raw(bfr, dflt)` will set contents of Buffer to 'bfr', only if
 * `bfr` is not nil and not empty.
 * If `bfr` is nil or empty, then `dflt` will be used.
 * If both are nil or empties then buffer content will not changes.
 *
 * On success, it will return `0`, or `-1` otherwise.
 */
int Buffer::set_raw(const char* bfr, const char* dflt)
{
	size_t len;
	if (bfr) {
		len = strlen(bfr);
		if (len) {
			return copy_raw(bfr, len);
		}
	}

	if (dflt) {
		len = strlen(dflt);
		if (len) {
			return copy_raw(dflt, len);
		}
	}

	return 0;
}

/**
 * Method `shiftr(nbyte, c)` will move contents of buffer `nbyte` to the right
 * and fill the empty gap in the left with value of `c` (default to zero).
 *
 * If buffer is empty, its will behave like resize if `nbyte` is greater than
 * current buffer size, but move the buffer index to `nbyte`.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::shiftr(const size_t nbyte, int c)
{
	size_t growth = _i + nbyte;

	if (growth > _l) {
		int s = resize(growth);

		if (s) {
			return -1;
		}
	}

	memmove(&_v[nbyte], &_v[0], _i);
	memset(_v, c, nbyte);

	_i	+= nbyte;
	_v[_i]	= '\0';

	return 0;
}

/**
 * Method `appendc(c)` will append a character `c` to buffer.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::appendc(const char c)
{
	if (c < 0) {
		return 0;
	}

	size_t growth = _i + CHAR_SIZE;

	if (growth > _l) {
		int s = resize(growth);
		if (s) {
			return -1;
		}
	}

	_v[_i] = c;
	_i++;
	_v[_i] = '\0';

	return 0;
}

/**
 * Method `appendi(i, base)` will append a long integer `i` as a string to
 * buffer. The value of `i` will be assumed in base 10.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::appendi(long int i, size_t base)
{
	int s = 0;
	int x = -1;
	char angka[23];

	if (i < 0) {
		s = appendc('-');
		if (s < 0) {
			return -1;
		}
		i = -(i);
	}
	while (i >= 0) {
		++x;
		angka[x] = __digits[i % base];
		i = i / base;
		if (0 == i) {
			break;
		}
	}
	while (x >= 0) {
		s = appendc(angka[x]);
		if (s < 0) {
			return -1;
		}
		--x;
	}
	return 0;
}

/**
 * Method `appendui(i, base)` will append an unsigned long integer `i` as a
 * string to buffer. The value of `i` will be assumed in base 10.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::appendui(long unsigned int i, size_t base)
{
	int x = 0;
	char angka[23];

	angka[0] = '0';

	while (i > 0) {
		angka[x] = __digits[i % base];
		i = i / base;
		if (0 == i) {
			break;
		}
		++x;
	}

	while (x >= 0) {
		int s = appendc(angka[x]);
		if (s < 0) {
			return -1;
		}
		--x;
	}
	return 0;
}

/**
 * Method `appendd(d, prec)` will append a double number as a string to
 * buffer.  Maximum digit in precision is six digits.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::appendd(double d, size_t prec)
{
	int s;
	long int i = (long int) d;

	if (d < 0) {
		d = -(d);
		d = d + (double) i;
	} else {
		d = d - (double) i;
	}

	for (size_t x = 0; x < prec; x++) {
		d = d * 10;
	}

	long int frac = (long int) d;

	s = appendi(i);
	if (s) {
		return -1;
	}

	s = appendc('.');
	if (s) {
		return -1;
	}

	return appendi(frac);
}

/**
 * Method `append(bfr)` will append a content of Buffer object `bfr` to this
 * buffer.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::append(const Buffer* bfr)
{
	if (!bfr) {
		return 0;
	}
	return append_raw(bfr->_v, bfr->_i);
}

/**
 * Method `append_raw(bfr, len)` will append a raw buffer `bfr` to buffer with
 * maximum length is `len`.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::append_raw(const char* bfr, size_t len)
{
	if (!bfr) {
		return 0;
	}
	if (len == 0) {
		len = strlen(bfr);
		if (!len) {
			return 0;
		}
	}
	if (resize(_i + len) < 0) {
		return -1;
	}

	if (len == 1) {
		_v[_i] = bfr[0];
		_i++;
	} else {
		memcpy(&_v[_i], bfr, len);
		_i += len;
	}

	_v[_i] = '\0';

	return 0;
}

/**
 * Method `append_bin(bin,len)` will append binary data with length `len` to
 * buffer.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::append_bin(const void* bin, size_t len)
{
	if (!bin || len == 0) {
		return 0;
	}
	if (resize(_i + len) < 0) {
		return -1;
	}

	memcpy(&_v[_i], bin, len);
	_i += len;
	_v[_i] = 0;

	return 0;
}

/**
 * Method `append_fmt` will parse the formatted string `fmt` and apply any
 * value from arguments `...` and append the result to current buffer.
 *
 * On success it will return 0, otherwise it will return -1.
 */
int Buffer::append_fmt(const char* fmt, ...)
{
	if (!fmt) {
		return 0;
	}

	int s;
	va_list args;

	va_start(args, fmt);
	s = vappend_fmt(fmt, args);
	va_end(args);

	return s;
}

/**
 * Method `vappend_fmt(fmt, args)` will parse formatted string `fmt` and apply
 * any value from `args` and append their result to current buffer.
 *
 * On success it will return 0, otherwise it will return -1.
 */
int Buffer::vappend_fmt(const char* fmt, va_list args)
{
	FmtParser fmtp;

	int s = fmtp.parse(fmt, args);
	if (s) {
		return -1;
	}

	append(&fmtp);

	return 0;
}

/**
 * Method `concat(bfr ...)` will append one or more string to current buffer.
 *
 * NOTE: The last parameter must be NULL.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::concat(const char* bfr, ...)
{
	if (!bfr) {
		return 0;
	}

	const char* p;
	va_list al;
	int s = 0;

	va_start(al, bfr);
	p = bfr;
	while (p) {
		int s = append_raw(p, strlen(p));
		if (s < 0) {
			break;
		}
		p = va_arg(al, const char*);
	}
	va_end(al);

	return s;
}

/**
 * Method `prepend(bfr)` will add buffer content of `bfr` object to the
 * beginning of this object.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::prepend(Buffer* bfr)
{
	if (!bfr) {
		return 0;
	}
	return prepend_raw(bfr->_v, bfr->_i);
}

/**
 * Method `prepend_raw(bfr, len)` will add raw buffer `bfr` to the beginning
 * of buffer.
 *
 * On success it will return `0`, otherwise it will return `-1`.
 */
int Buffer::prepend_raw(const char* bfr, size_t len)
{
	if (!bfr) {
		return 0;
	}
	if (len == 0) {
		len = strlen(bfr);
		if (!len) {
			return 0;
		}
	}
	if (shiftr(len) < 0) {
		return -1;
	}

	memcpy(_v, bfr, len);

	return 0;
}

/**
 * Method `subc(from, to)` will replace each occurence of `from` character in
 * buffer with `to`.
 *
 * It will return number of character that has been replaced.
 */
size_t Buffer::subc(const char from, const char to)
{
	size_t n = 0;

	for (size_t x = 0; x < _i; x++) {
		if (_v[x] == from) {
			_v[x] = to;
			n++;
		}
	}

	return n;
}

/**
 * Method `cmp(bfr)` will compare Object chars representation with current
 * buffer.
 *
 * It will return,
 *
 * - `1` if this buffer greater than `bfr` or `bfr` is NULL
 * - `0` if this buffer equal with `bfr`
 * - `-1` if this buffer less than `bfr`
 *
 * This is a case sensitive comparison, where "A" != "a".
 */
int Buffer::cmp(Object* bfr)
{
	if (!bfr) {
		if (is_empty()) {
			return 0;
		}
		return 1;
	}
	return cmp_raw(bfr->chars());
}

/**
 * Method `cmp_raw(bfr, len)` will compare raw buffer `bfr` with current
 * buffer with maximum length to `len`.
 *
 * It will return,
 *
 * - `1` if this buffer greater than `bfr` or `bfr` is NULL
 * - `0` if this buffer equal with `bfr`
 * - `-1` if this buffer less than `bfr`
 *
 * This is a case sensitive comparison, where "A" != "a".
 */
int Buffer::cmp_raw(const char* bfr, size_t len)
{
	if (!bfr) {
		if (is_empty()) {
			return 0;
		}
		return 1;
	}

	if (len == 0) {
		len = strlen(bfr);
	}

	int s = strncmp(_v, bfr, len);
	if (s < 0) {
		return -1;
	}
	if (s > 0) {
		return 1;
	}
	if (_i > len) {
		return 1;
	}
	return 0;
}

/**
 * Method `like(bfr)` will compare content of current buffer with content of
 * buffer `bfr`.
 *
 * It will return,
 *
 * - `1` if this buffer greater than `bfr` or `bfr` is NULL, or if this buffer
 *   longer than `bfr` (e.g. "abcd" vs. "a").
 * - `0` if this buffer equal with `bfr` or both are empty.
 * - `-1` if this buffer less than `bfr`
 *
 *
 * This is a case non-sensitive comparison, where "A" == "a".
 */
int Buffer::like(const Buffer* bfr)
{
	if (!bfr) {
		if (is_empty()) {
			return 0;
		}

		return 1;
	}
	return like_raw(bfr->_v, bfr->_i);
}

/**
 * Method `like_raw(bfr, len)` will compare content of current buffer with
 * raw buffer `bfr`.
 *
 * It will return,
 *
 * - `1` if this buffer greater than `bfr` or `bfr` is NULL, or if this buffer
 *   longer than `bfr` (e.g. "abcd" vs. "a").
 * - `0` if this buffer equal with `bfr` or both are empty
 * - `-1` if this buffer less than `bfr`
 *
 * This is a case non-sensitive comparison, where "A" == "a".
 */
int Buffer::like_raw(const char* bfr, size_t len)
{
	if (!bfr) {
		if (is_empty()) {
			return 0;
		}
		return 1;
	}

	if (len == 0) {
		len = strlen(bfr);
	}

	int s = strncasecmp(_v, bfr, len);
	if (s < 0) {
		return -1;
	}
	if (s > 0) {
		return 1;
	}
	if (_i > len) {
		return 1;
	}
	return 0;
}

/**
 * Method `to_lint(v)` will convert content of buffer into long integer and
 * save their value to `res`.
 *
 * On success, it will return 0.
 * On fail, the value of `res` will not changed and it will return `-1`.
 */
int Buffer::to_lint(long int* res)
{
	if (!_v) {
		return 0;
	}

	long int li;

	errno = 0;

	li = strtol(_v, NULL, 0);

	if (errno == ERANGE) {
		return -1;
	}

	*res = li;

	return 0;
}

const char* Buffer::chars()
{
	return _v;
}

/**
 * Method `dump` will print buffer contents to standard output.
 */
void Buffer::dump()
{
	printf("[%s] dump: [%zu|%zu|%s]\n", __CNAME, _i, _l, _v);
}

/**
 * Method `dump_hex` will print buffer in two column, hexadecimal in the left
 * column and printable characters in the right column.
 */
void Buffer::dump_hex()
{
	size_t i = 0;
	size_t j = 0;
	int k = 0;
	Buffer o(_i * 6);

	o.append_raw("[vos::Buffer__] dump_hex:\n");

	for (; i < _i; ++i) {
		if ((i % 4) == 0) {
			o.append_raw("  ", 2);
		}
		if ((i % 8) == 0) {
			o.appendc('\t');
			for (; j < i; ++j) {
				if ((j % 4) == 0) {
					o.append_raw("  ", 2);
				}
				o.append_fmt(" %-2c", isprint(_v[j])
							? _v[j] : '.' );
			}
			j = i;
			o.append_fmt("\n %04X |", i);
		}
		o.append_fmt(" %02X", (unsigned char) _v[i]);
	}

	k = i % 8;
	for (; k < 8; ++k) {
		if ((k % 4) == 0) {
			o.append_raw("  ", 2);
		}
		o.append_raw("   ", 3);
	}

	o.appendc('\t');
	for (; j < i; ++j) {
		if ((j % 4) == 0) {
			o.append_raw("  ", 2);
		}
		o.append_fmt(" %-2c", isprint(_v[j]) ? _v[j] : '.' );
	}
	o.append_raw("\n\n", 2);

	printf("%s", o._v);
}

}// namespace::vos

// vi: ts=8 sw=8 tw=80:
