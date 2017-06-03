/**
 * Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Object.hh"

namespace vos {

const char* Object::__CNAME = "Object";

Object::Object() :
	__str(NULL)
{}

Object::~Object()
{
	if (__str != NULL) {
		free(__str);
		__str = NULL;
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
	return strcmp(__str, that->chars());
}

const char* Object::chars()
{
	return __str;
}

/**
 * Method CMP is a static function to compare two object `x` and `y` using
 * value of `chars()` function. It will return,
 *
 * - `-1` if `x` is less than `y`, or `x` is nil and `y` is not nil.
 * - `1` if `x` is greater than `y`, or `x` is not nil while `y` is nil.
 * - `0` if `x` and `y` is equal or both are nil.
 */
int Object::CMP(Object* x, Object* y)
{
	if (!x && !y) {
		return 0;
	}
	if (!x) {
		return -1;
	}
	if (!y) {
		return 1;
	}
	return strcmp(x->chars(), y->chars());
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
