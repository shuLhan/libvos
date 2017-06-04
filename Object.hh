/**
 * Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBVOS_OBJECT_HH
#define	_LIBVOS_OBJECT_HH	1

#include "libvos.hh"

#define K(k) "\"" #k "\""

namespace vos {

/**
 * Class Object represent base class in libvos.
 *
 * Field __cname contains the name of class in string.
 *
 * Field __str contains string representation of object.
 */
class Object {
public:
	static const char* __CNAME;

	static int CMP(Object* x, Object* y);

	Object();
	virtual ~Object();

	virtual int cmp(Object* that);
	virtual const char* chars();

	char* __str;

private:
	Object(const Object&);
	void operator=(const Object&);
};

} // namespace vos
#endif // _LIBVOS_OBJECT_HH
// vi: ts=8 sw=8 tw=80:
