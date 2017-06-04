//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <assert.h>

#include "test.hh"
#include "../Object.hh"
#include "../Test.hh"

using vos::Object;
using vos::Test;

Test T(Object::__CNAME);

int main()
{
	Object o;

	T.expect_string(o.__CNAME, "Object", 0);
	T.expect_ptr(o.chars(), NULL, 0);
}

// vi: ts=8 sw=8 tw=80:
