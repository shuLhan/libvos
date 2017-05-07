//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../Test.hh"
#include "../ListBuffer.hh"

using vos::Test;
using vos::Buffer;
using vos::List;

Test T("ListBuffer");

void test_SPLIT_BY_CHAR()
{
	struct {
		const char *desc;
		Buffer *in;
		const char sep;
		const int trim;
		const char *exp;
		const size_t exp_size;
	} const tests[] = {
		{
			"With empty buffer",
			NULL,
			',',
			0,
			NULL,
			0,
		},
		{
			"With no separator found",
			new Buffer("a"),
			',',
			0,
			"[ \"a\" ]",
			1,
		},
		{
			"With separator at the beginning without trimming",
			new Buffer(",a"),
			',',
			0,
			"[ \"\",\"a\" ]",
			2,
		},
		{
			"With separator at the beginning with trimming",
			new Buffer(",a"),
			',',
			1,
			"[ \"a\" ]",
			1,
		},
		{
			"With separator at the end without trimming",
			new Buffer("a,"),
			',',
			0,
			"[ \"a\",\"\" ]",
			2,
		},
		{
			"With separator at the end with trimming",
			new Buffer("a,"),
			',',
			1,
			"[ \"a\" ]",
			1,
		},
		{
			"With two values",
			new Buffer("a,b"),
			',',
			0,
			"[ \"a\",\"b\" ]",
			2,
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("SPLIT_BY_CHAR()", tests[x].desc);

		List *got = SPLIT_BY_CHAR(tests[x].in, tests[x].sep,
			tests[x].trim);

		if (tests[x].exp == NULL) {
			T.expect_ptr(tests[x].exp, got, 0);
		} else {
			T.expect_string(tests[x].exp, got->chars(), 0);
			T.expect_signed(tests[x].exp_size, got->size(), 0);
		}

		T.ok();

		if (tests[x].in) {
			delete tests[x].in;
		}
		if (got) {
			delete got;
		}
	}
}

void test_SPLIT_BY_WHITESPACE()
{
	struct {
		const char *desc;
		Buffer *in;
		const char *exp;
		const size_t exp_size;
	} const tests[] = {
		{
			"With empty buffer",
			NULL,
			NULL,
			0,
		},
		{
			"With one item",
			new Buffer("a"),
			"[ \"a\" ]",
			1,
		},
		{
			"With space at the beginning",
			new Buffer(" a"),
			"[ \"a\" ]",
			1,
		},
		{
			"With space at the end",
			new Buffer("a "),
			"[ \"a\" ]",
			1,
		},
		{
			"With space at before and end",
			new Buffer(" a "),
			"[ \"a\" ]",
			1,
		},
		{
			"With two values",
			new Buffer("a b"),
			"[ \"a\",\"b\" ]",
			2,
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("SPLIT_BY_WHITESPACE()", tests[x].desc);

		List *got = SPLIT_BY_WHITESPACE(tests[x].in);

		if (tests[x].exp == NULL) {
			T.expect_ptr(tests[x].exp, got, 0);
		} else {
			T.expect_string(tests[x].exp, got->chars(), 0);
			T.expect_signed(tests[x].exp_size, got->size(), 0);
		}

		T.ok();

		if (tests[x].in) {
			delete tests[x].in;
		}
		if (got) {
			delete got;
		}
	}

}

int main()
{
	test_SPLIT_BY_CHAR();
	test_SPLIT_BY_WHITESPACE();

	return 0;
}
