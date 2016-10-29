//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Object.hh"

namespace vos {

const char *Object::__cname = "Object";

Object::Object() :
	_v(NULL)
{}

Object::~Object()
{
	if (_v != NULL) {
		free(_v);
		_v = NULL;
	}
}

int Object::cmp(Object* that)
{
	if (!that) {
		return 1;
	}
	if (this == that) {
		return 0;
	}
	return strcmp(_v, that->chars());
}

const char* Object::chars()
{
	return _v;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
