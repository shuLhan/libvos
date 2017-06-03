/**
 * Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "test.hh"
#include "../Test.hh"
#include "../Error.hh"

using vos::Test;
using vos::Error;

Test T("Error");

void test_assignment()
{
	struct {
		const char* desc;
		const Error in;
		int         exp;
		const Error expErr;
	} const tests[] = {{
		"With NULL"
	,	NULL
	,	1
	,	NULL
	},{
		"With ErrOutOfMemory"
	,	vos::ErrOutOfMemory
	,	1
	,	vos::ErrOutOfMemory
	}};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start(tests[x].desc);

		T.expect_signed(tests[x].exp, tests[x].in == tests[x].expErr, 0);

		T.ok();
	}
}

int main()
{
	test_assignment();

	return 0;
}
