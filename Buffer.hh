//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_BUFFER_HH
#define _LIBVOS_BUFFER_HH 1

#include "Error.hh"

namespace vos {

extern const Error ErrBufferInvalidIndex;

/**
 * Class `Buffer` represent generic buffer (list of characters).
 *
 * Field `_i` contains index of buffer.
 * Field `_l` contains size of buffer.
 * Field `_v` contains pointer of buffer in memory.
 */
class Buffer : public Object {
public:
	static const char* __CNAME;

	static const uint8_t CHAR_SIZE;
	static const uint16_t DFLT_SIZE;

	static int CMP(Object* x, Object* y);
	static Error PARSE_INT(char** pp, int* v);
	static size_t TRIM(char* bfr, size_t len);

	explicit Buffer(const size_t size = DFLT_SIZE);
	Buffer(const char* v, size_t vlen = 0);
	explicit Buffer(const Buffer* v);

	virtual ~Buffer();

	char* detach();
	void release();
	void reset(int c = 0);
	void trim();
	void truncate(const size_t len);

	int is_empty() const;

	size_t len() const;
	Error set_len(size_t len);

	size_t size() const;
	Error resize(size_t len);

	const char* v(size_t idx = 0) const;

	char char_at(size_t idx);
	Error set_char_at(size_t idx, char v);

	Error copy(const Buffer* bfr);
	Error copy_raw(const char* bfr, size_t len = 0);
	Error copy_raw_at(size_t idx, const char* v, size_t vlen);

	Error set(const Buffer* bfr, const Buffer* dflt);
	Error set_raw(const char* bfr, const char* dflt);

	Error shiftr(const size_t nbyte, int c = 0);

	Error appendc(const char c);
	Error appendi(long int i, size_t base = 10);
	Error appendui(long unsigned int i, size_t base = 10);
	Error appendd(double d, size_t prec = 6);
	Error append(const Buffer* bfr);
	Error append_raw(const char* bfr, size_t len = 0);
	Error append_bin(const void* bin, size_t len);
	Error append_fmt(const char* fmt, ...);
	Error vappend_fmt(const char* fmt, va_list args);
	Error concat(const char* bfr, ...);

	Error prepend(Buffer* bfr);
	Error prepend_raw(const char* bfr, size_t len = 0);

	size_t subc(const char from, const char to);

	int cmp(Object* bfr);
	int cmp_raw(const char* bfr, size_t len = 0);
	int like(const Buffer* bfr);
	int like_raw(const char* bfr, size_t len = 0);

	Error to_lint(long int* res);

	const char* chars();
	const char* dump();
	const char* dump_hex();

protected:
	size_t _i;
	size_t _l;
	char* _v;

private:
	Buffer(const Buffer&);
	void operator=(const Buffer&);
};

}// namespace::vos
#endif

// vi: ts=8 sw=8 tw=80:
