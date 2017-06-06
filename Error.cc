//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Error.hh"

namespace vos {

const Error ErrNumRange("libvos: Numerical result out of range");
const Error ErrOutOfMemory("libvos: Out of memory");
Error ErrUnknown("libvos: unknown error");

const char* Error::__CNAME = "Error";

Error Error::SYS()
{
	int s;
	int errnum = errno;
	size_t len = 512;
	char* msg = (char*) calloc(len, 1);

	do {
		s = strerror_r(errnum, msg, len);
		if (s && errno == ERANGE) {
			len = len * 2;
			msg = (char*) realloc(msg, len);
		}
	} while (errno == ERANGE);

	if (errno == EINVAL) {
		return ErrUnknown.with(&errnum, sizeof(int));
	}

	return Error(msg);
}

Error::Error()
: Object()
, _data(NULL)
, _p_data(NULL)
, _len(0)
{
	__str = NULL;
}

Error::Error(const char* msg)
: Object()
, _data(NULL)
, _p_data(NULL)
, _len(0)
{
	__str = (char*) msg;
}

/**
 * Method Error(err) will create new error by referencing error message and
 * copying error data.
 */
Error::Error(const Error& err)
: Object()
, _data(NULL)
, _p_data(NULL)
, _len(0)
{
	__str = err.__str;
	if (err._data) {
		_p_data = err._data;
	}
	if (err._p_data) {
		_p_data = err._p_data;
	}
	_len = err._len;
}

/**
 * Method ~Error() will release error object from memory.
 */
Error::~Error()
{
	__str = NULL;
	if (_data != NULL) {
		free(_data);
		_data = NULL;
	}
	_p_data = NULL;
	_len = 0;
}

Error& Error::operator=(const Error& err)
{
	__str = err.__str;
	_data = err._data;
	_p_data = err._p_data;
	_len = err._len;
	return *this;
}

int Error::operator==(const Error& err) const
{
	return (__str == err.__str);
}

int Error::operator!=(const Error& err) const
{
	return (__str != err.__str);
}

Error Error::with(const void* data, size_t len)
{
	Error err(__str);

	if (data) {
		err._data = calloc(len, 1);
		memcpy(err._data, data, len);
		err._len = len;
	}

	return err;
}

void* Error::data() const
{
	if (_p_data) {
		return _p_data;
	}
	return _data;
}

char Error::data_as_char() const
{
	if (_len == 0) {
		return 0;
	}

	const char* str;

	if (_p_data) {
		str = (const char*) _p_data;
	} else {
		str = (const char*) _data;
	}

	return str[0];
}

const char* Error::data_as_string() const
{
	if (_len == 0) {
		return 0;
	}
	if (_p_data) {
		return (const char*) _p_data;
	}
	return (const char*) _data;
}

long int Error::data_as_signed() const
{
	long int data = 0;

	if (_len == 0) {
		return 0;
	}

	switch (_len) {
	case sizeof(int):
		if (_p_data) {
			data = *(int*) _p_data;
		} else {
			data = *(int*) _data;
		}
		break;
	case sizeof(long int):
		if (_p_data) {
			data = *(long int*) _p_data;
		} else {
			data = *(long int*) _data;
		}
		break;
	}

	return data;
}

unsigned long Error::data_as_unsigned() const
{
	unsigned long data = 0;

	if (_len == 0) {
		return 0;
	}

	switch (_len) {
	case sizeof(unsigned int):
		if (_p_data) {
			data =  *(unsigned int *) _p_data;
		} else {
			data =  *(unsigned int *) _data;
		}
		break;
	case sizeof(unsigned long):
		if (_p_data) {
			data =  *(unsigned long *) _p_data;
		} else {
			data =  *(unsigned long *) _data;
		}
		break;
	}

	return data;
}

double Error::data_as_double() const
{
	if (_len == 0) {
		return 0;
	}

	if (_p_data) {
		return *(double*) _p_data;
	}
	return *(double*) _data;
}

} // namespace vos
// vi: ts=8 sw=8 tw=80:
