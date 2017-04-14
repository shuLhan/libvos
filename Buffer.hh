//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_BUFFER_HH
#define _LIBVOS_BUFFER_HH 1

#include "List.hh"

namespace vos {

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
class Buffer : public Object {
public:
	explicit Buffer(const size_t size = DFLT_SIZE);
	Buffer(const char* v, const size_t vlen = 0);
	explicit Buffer(const Buffer* v);
	virtual ~Buffer();
	void release();
	char* detach();

	int is_empty() const;
	size_t len() const;
	size_t size() const;

	const char* v(size_t idx = 0) const;
	void set(char* v);
	int set_at(size_t idx, const char* v, size_t vlen);

	char char_at(size_t idx);
	int set_char_at(size_t idx, char v);

	int resize(size_t len);
	void reset(int c = 0);
	void trim();

	int copy(const Buffer* bfr);
	int copy_raw(const char* bfr, size_t len = 0);

	int set(const Buffer* bfr, const Buffer* dflt);
	int set_raw(const char* bfr, const char* dflt);

	int move_to(Buffer** bfr);
	int shiftr(const size_t nbyte, int c = 0);

	int appendc(const char c);
	int appendi(long int i, unsigned int base = 10);
	int appendui(long unsigned int i, size_t base = 10);
	int appendd(double d, int prec = 6);
	int append(const Buffer* bfr);
	int append_raw(const char* bfr, size_t len = 0);
	int append_bin(void *bin, size_t len);

	int concat(const char* bfr, ...);
	int aprint(const char* fmt, ...);
	int vprint(const char* fmt, va_list args);

	int prepend(Buffer* bfr);
	int prepend_raw(const char* bfr, size_t len = 0);

	int subc(int from, int to);

	int cmp(Object* bfr);
	int cmp_raw(const char* bfr, size_t len = 0);
	int like(const Buffer* bfr);
	int like_raw(const char* bfr, size_t len = 0);

	long int to_lint();

	List* split_by_char(const char sep, int trim = 0);
	List* split_by_whitespace();

	const char* chars();
	void dump();
	void dump_hex();

	size_t _i;

	static int PARSE_INT(char** pp, int* v);
	static int VSNPRINTF(char *bfr, int len, const char *fmt,
				va_list args);
	static size_t TRIM(char *bfr, size_t len);
	static int CMP_OBJECTS(Object* x, Object* y);

	static uint16_t	DFLT_SIZE;
	static uint8_t	CHAR_SIZE;

	static const char* __cname;

protected:
	size_t _l;
	char* _v;

private:
	Buffer(const Buffer&);
	void operator=(const Buffer&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
