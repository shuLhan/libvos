//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "TestLocker.hh"

Test T("Locker");

void test_inheritance()
{
	T.start("TestLocker");

	TestLocker tl;

	T.expect_signed(1, tl.data == 0, vos::IS_EQUAL);

	tl.inc();
	T.expect_signed(1, tl.data == 1, vos::IS_EQUAL);

	tl.dec();
	T.expect_signed(1, tl.data == 0, vos::IS_EQUAL);

	T.ok();
}

void test_is_locked()
{
	T.start("is_locked()");

	TestLocker tl;

	T.expect_signed(1, tl.is_locked() == 0, vos::IS_EQUAL);

	tl.lock();

	T.expect_signed(1, tl.is_locked() == 1, vos::IS_EQUAL);

	tl.unlock();

	T.expect_signed(1, tl.is_locked() == 0, vos::IS_EQUAL);

	T.ok();
}

int main()
{
	test_inheritance();
	test_is_locked();

	return 0;
}

// vi: ts=8 sw=8 tw=80:
