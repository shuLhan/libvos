//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_ERROR_HH
#define	_LIBVOS_ERROR_HH	1

#include "Object.hh"

namespace vos {

/**
 * Class Error represent error type as object with string and data.
 *
 * Field _data contains any value that cause the error.
 */
class Error : public Object {
public:
	static const char* __CNAME;

	static Error SYS();

	Error();
	Error(const char* msg);
	Error(const Error&);

	~Error();

	Error& operator=(const Error& err);
	int operator==(const Error& err) const;
	int operator!=(const Error& err) const;

	Error with(const void* data, size_t len);

	void* data() const;
	char data_as_char() const;
	const char* data_as_string() const;
	long int data_as_signed() const;
	unsigned long data_as_unsigned() const;
	double data_as_double() const;

private:
	void* _data;
	void* _p_data;
	size_t _len;
};

extern const Error ErrNumRange;
extern const Error ErrOutOfMemory;
extern Error ErrUnknown;

} // namespace vos
#endif // _LIBVOS_EROR_HH
// vi: ts=8 sw=8 tw=80:
