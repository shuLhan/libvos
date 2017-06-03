/**
 * Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Error.hh"

namespace vos {

const char* Error::__CNAME = "Error";

Error::Error()
: Object()
, _data(NULL)
{
	__str = NULL;
}

Error::Error(const char* msg)
: Object()
, _data(NULL)
{
	__str = (char*) msg;
}

Error::Error(const Error& err)
: Object()
, _data(NULL)
{
	__str = err.__str;
	_data = err._data;
}

Error::~Error()
{
	__str = NULL;
	if (_data) {
		free(_data);
		_data = NULL;
	}
}

Error& Error::operator=(const Error& err)
{
	this->__str = err.__str;
	this->_data = err._data;
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

void Error::With(const void* data, size_t len)
{
	if (_data) {
		free(_data);
	}

	_data = calloc(len, 1);
	memcpy(_data, data, len);
}

void* Error::Data()
{
	return _data;
}

const Error ErrOutOfMemory("libvos: Out of memory");
const Error ErrNumRange("libvos: Numerical result out of range");

} // namespace vos
// vi: ts=8 sw=8 tw=78:
