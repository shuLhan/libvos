//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_OBJECT_HH
#define	_LIBVOS_OBJECT_HH	1

#include "libvos.hh"

namespace vos {

class Object {
public:
	Object();

	virtual ~Object();
	virtual int cmp(Object* that);
	virtual const char* chars();

	// `__cname` contain class name.
	static const char *__cname;

	// `_v` contain string representation of object.
	char *_v;
private:
	Object(const Object&);
	void operator=(const Object&);
};

} // namespace vos
#endif // _LIBVOS_OBJECT_HH
// vi: ts=8 sw=8 tw=78:
