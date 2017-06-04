#//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../Test.hh"
#include "../FmtParser.hh"

using vos::Test;
using vos::FmtParser;

Test T("FmtParser");

void test_parse()
{
	struct {
		const char *desc;
		const char *fmt;
		const char *exp_v;
	} const tests[] = {
		{
			"With '%' at the end of string",
			"test %",
			"test %",
		},
		{
			"With escaped format (%%)",
			"%% test",
			"% test",
		},
		{
			"With duplicate flag chars (%--+)",
			"%--+ test",
			"%--+ test",
		},
		{
			"With duplicate flag chars (%++#)",
			"%++# test",
			"%++# test",
		},
		{
			"With duplicate flag chars (%##0)",
			"%##0 test",
			"%##0 test",
		},
		{
			"With duplicate flag chars (%00-)",
			"%00- test",
			"%00- test",
		},
		{
			"With duplicate flag chars (%00-)",
			"%00- test",
			"%00- test",
		},
		{
			"With zero padding and field width (%010c)",
			"%010c test",
			"         0 test",
		},
		{
			"With field width and zero precision (%010.c)",
			"%010.c test",
			"         0 test",
		},
		{
			"With field width and precision (%010.4c)",
			"%010.4c test",
			"         0 test",
		},
		{
			"With field width, precision and invalid conversion (%010.4v)",
			"%010.4v test",
			"%010.4v test",
		},
		{
			"With length modifier",
			"%c for char, %hd for short int, %li for long int, and %Lf for long double",
			"0 for char, 1 for short int, 2 for long int, and 3.0 for long double",
		},
		{
			"With invalid conversion",
			"%hj for short int %ly for long int and %Lt for long double",
			"%hj for short int %ly for long int and %Lt for long double",
		},
		{
			"With valid conversion",
			"%c %d %i %f %o %s %u %x %X",
			"0 1 2 3.0 4 5 6 A A",
		},
	};

	FmtParser fmtp;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("parse()", tests[x].desc);

		fmtp.parse(tests[x].fmt, '0', 1, 2, 3.0, 4, "5", 6, 10, 10);

		T.expect_string(tests[x].exp_v, fmtp.v(), 0);

		T.ok();
	}
}

void test_parse_integer()
{
	struct {
		const char *desc;
		const char *fmt;
		const int in;
		const char *exp_v;
	} const tests[] = {
		{
			"With no field width and no precision",
			"%d test",
			123,
			"123 test",
		},
		{
			"With field width and no precision",
			"%4d test",
			123,
			" 123 test",
		},
		{
			"With 4 width and 2 precision",
			"%4.2d test",
			123,
			" 123 test",
		},
		{
			"With 6 width and 2 precision",
			"'%6.2d' test",
			123,
			"'   123' test",
		},
		{
			"With '0' flag, 6 width, and 2 precision",
			"%06.2d test",
			123,
			"000123 test",
		},
		{
			"With '-' flag, 6 width, and 2 precision",
			"'%-6.2d' test",
			123,
			"'123   ' test",
		},
		{
			"With '-0' flag, 6 width, and 2 precision",
			"'%-06.2d' test",
			123,
			"'123   ' test",
		},
		{
			"With '0-' flag, 6 width, and 2 precision",
			"'%0-6.2d' test",
			123,
			"'123   ' test",
		},
		{
			"With '+-' flag, 6 width, and 2 precision",
			"'%+-6.2d' test",
			-123,
			"'-123  ' test",
		},
		{
			"With '-+' flag, 6 width, and 2 precision",
			"'%-+6.2d' test",
			-123,
			"'-123  ' test",
		},
		{
			"With '+-' flag, no width, and 2 precision",
			"'%+-.2d' test",
			123,
			"'+123' test",
		},
		{
			"With zero padding and field width (%010d)",
			"%010d test",
			123,
			"0000000123 test",
		},
		{
			"With field width and zero precision (%010.d)",
			"%010.d test",
			123,
			"0000000123 test",
		},
		{
			"With field width and precision (%010.4d)",
			"%010.4d test",
			123,
			"0000000123 test",
		},
	};

	FmtParser fmtp;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("test_parse_integer()", tests[x].desc);

		fmtp.parse(tests[x].fmt, tests[x].in);

		T.expect_string(tests[x].exp_v, fmtp.v(), 0);

		T.ok();
	}
}

void test_parse_float()
{
	struct {
		const char *desc;
		const char *fmt;
		const double in;
		const char *exp_v;
	} const tests[] = {
		{
			"With no field width and no precision",
			"%f test",
			1.23456789,
			"1.234567 test",
		},
		{
			"With field width and no precision",
			"%4f test",
			1.23456789,
			"1.234567 test",
		},
		{
			"With 4 width and 2 precision",
			"%4.2f test",
			1.23456789,
			"1.23 test",
		},
		{
			"With 6 width and 2 precision",
			"'%6.2f' test",
			1.23456789,
			"'  1.23' test",
		},
		{
			"With '0' flag, 6 width, and 2 precision",
			"%06.2f test",
			1.23456789,
			"001.23 test",
		},
		{
			"With '-' flag, 6 width, and 2 precision",
			"'%-6.2f' test",
			1.23456789,
			"'1.23  ' test",
		},
		{
			"With '-0' flag, 6 width, and 2 precision",
			"'%-06.2f' test",
			1.23456789,
			"'1.23  ' test",
		},
		{
			"With '0-' flag, 6 width, and 2 precision",
			"'%0-6.2f' test",
			1.23456789,
			"'1.23  ' test",
		},
		{
			"With '+-' flag, 6 width, and 2 precision",
			"'%+-6.2f' test",
			-1.23456789,
			"'-1.23 ' test",
		},
		{
			"With '-+' flag, 6 width, and 2 precision",
			"'%-+6.2f' test",
			-1.23456789,
			"'-1.23 ' test",
		},
		{
			"With '+-' flag, no width, and 2 precision",
			"'%+-.2f' test",
			1.23456789,
			"'+1.23' test",
		},

	};

	FmtParser fmtp;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("test_parse_float()", tests[x].desc);

		fmtp.parse(tests[x].fmt, tests[x].in);

		T.expect_string(tests[x].exp_v, fmtp.v(), 0);

		T.ok();
	}
}

void test_parse_string()
{
	struct {
		const char *desc;
		const char *fmt;
		const char *in;
		const char *exp_v;
	} const tests[] = {
		{
			"With no field width and no precision",
			"%s test",
			"abcd",
			"abcd test",
		},
		{
			"With field width",
			"'%8s' test",
			"abcd",
			"'    abcd' test",
		},
		{
			"With left padding",
			"'%-8s' test",
			"abcd",
			"'abcd    ' test",
		},
	};

	FmtParser fmtp;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("test_parse_string()", tests[x].desc);

		fmtp.parse(tests[x].fmt, tests[x].in);

		T.expect_string(tests[x].exp_v, fmtp.v(), 0);

		T.ok();
	}
}

int main()
{
	test_parse();
	test_parse_integer();
	test_parse_float();
	test_parse_string();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
