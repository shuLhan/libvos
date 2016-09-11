//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"

int main ()
{
	Buffer a;

	assert(strcmp("Buffer", a.__cname) == 0);
	assert(strcmp("Object", a.Object::__cname) == 0);

	assert(a._l == Buffer::DFLT_SIZE);
	assert(a._i == 0);
	assert(strcmp("", a.chars()) == 0);

	a.copy_raw(STR_TEST_0);

	assert(a._i == strlen(STR_TEST_0));
	assert(strcmp(STR_TEST_0, a.chars()) == 0);

	return 0;
}

// vi: ts=8 sw=8 tw=78:
