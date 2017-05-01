//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Buffer.test.hh"

Test T("Buffer");

void test_constructor()
{
	T.start("constructor", "default");

	Buffer a;

	T.expect_string(a.__cname, "Buffer");
	T.expect_string(a.Object::__cname, "Object");

	assert(a.size() == Buffer::DFLT_SIZE);
	assert(a.len() == 0);
	T.expect_string(a.v(), "");

	T.ok();
}

void test_constructor_size()
{
	struct {
		const char *desc;
		size_t     in_size;
		size_t     exp_len;
		size_t     exp_size;
	} const tests[] = {
		{
			"Without size or zero size",
			0,
			0,
			Buffer::DFLT_SIZE,
		},
		{
			"With size greater than zero",
			256,
			0,
			256,
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("Buffer(size_t)", tests[x].desc);

		Buffer b(tests[x].in_size);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_constructor_raw()
{
	struct {
		const char *desc;
		const char *in_v;
		size_t     in_len;
		size_t     exp_len;
		size_t     exp_size;
		const char *exp_v;
	} const tests[] = {
		{
			"With empty string",
			"",
			0,
			0,
			16,
			"",
		},
		{
			"With non empty string",
			"abcdefghij",
			0,
			10,
			10,
			"abcdefghij",
		},
		{
			"With non empty string and length",
			"abcdefghij",
			4,
			4,
			4,
			"abcd",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("Buffer(const char *, size_t)", tests[x].desc);

		Buffer b(tests[x].in_v, tests[x].in_len);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();
	}
}

void test_constructor_buffer()
{
	struct {
		const char *desc;
		Buffer     *in_buffer;
		size_t     exp_len;
		size_t     exp_size;
		const char *exp_v;
	} const tests[] = {
		{
			"With empty buffer",
			new Buffer(256),
			0,
			256,
			"",
		},
		{
			"With non empty buffer",
			new Buffer("abcdefghij"),
			10,
			10,
			"abcdefghij",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("Buffer(const Buffer *)", tests[x].desc);

		Buffer b(tests[x].in_buffer);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		if (tests[x].in_buffer) {
			delete tests[x].in_buffer;
		}

		T.ok();
	}
}

void test_detach()
{
	struct {
		const char *desc;
		const char *in_v;
		size_t     exp_len;
		size_t     exp_size;
		const char *exp_v;
		const char *exp_ret;
	} const tests[] = {
		{
			"With empty buffer",
			"",
			0,
			0,
			NULL,
			"",
		},
		{
			"With non empty buffer",
			"abcdefghij",
			0,
			0,
			NULL,
			"abcdefghij",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("detach()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		char *got = b.detach();

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_ptr((void *) tests[x].exp_v, (void *) b.v(), 0);
		T.expect_string(tests[x].exp_ret, got, 0);

		if (got) {
			free(got);
		}

		T.ok();
	}
}

void test_release()
{
	struct {
		const char *desc;
		const char *in_v;
		size_t     exp_len;
		size_t     exp_size;
		const char *exp_v;
	} const tests[] = {
		{
			"With empty buffer",
			"",
			0,
			0,
			NULL,
		},
		{
			"With non empty buffer",
			"abcdefghij",
			0,
			0,
			NULL,
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("release()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.release();

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_ptr((void *) tests[x].exp_v, (void *) b.v(), 0);

		T.ok();
	}
}

void test_reset()
{
	struct {
		const char *desc;
		const char *in_v;
		size_t     exp_len;
		size_t     exp_size;
		const char *exp_v;
	} const tests[] = {
		{
			"When buffer empty",
			"",
			0,
			Buffer::DFLT_SIZE,
			"",
		},
		{
			"With buffer not empty",
			"abcdefghij",
			0,
			Buffer::DFLT_SIZE,
			"",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("reset()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.reset();

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();
	}
}

void test_trim()
{
	struct {
		const char *desc;
		const char *in_v;
		const char *exp;
	} const tests[] = {
		{
			"With space before",
			"	  a",
			"a",
		},
		{
			"With space after",
			"a	  ",
			"a",
		},
		{
			"With space before and after",
			"	  a	  ",
			"a",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("trim()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);
		b.trim();

		T.expect_string(tests[x].exp, b.v(), 0);

		T.ok();
	}
}

void test_truncate()
{
	struct {
		const char *desc;
		const char *in_v;
		size_t     in_len;
		size_t     exp_len;
		size_t     exp_size;
		const char *exp_v;
	} const tests[] = {
		{
			"With length greater than buffer len",
			"abcdefghij",
			20,
			10,
			Buffer::DFLT_SIZE,
			"abcdefghij",
		},
		{
			"With length smaller than buffer",
			"abcdefghij",
			3,
			3,
			Buffer::DFLT_SIZE,
			"abc",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("truncate()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.truncate(tests[x].in_len);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();
	}
}

void test_is_empty()
{
	struct {
		const char *desc;
		const char *in_v;
		int        exp_res;
	} const tests[] = {
		{
			"Empty",
			"",
			1,
		},
		{
			"Not empty",
			"a",
			0,
		},
	};

	Buffer       b;
	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("is_empty()", tests[x].desc);

		b.copy_raw(tests[x].in_v);

		T.expect_signed(tests[x].exp_res, b.is_empty(), 0);

		T.ok();
	}
}

void test_set_len()
{
	struct {
		const char *desc;
		size_t     in_len;
		size_t     exp_len;
		size_t     exp_size;
	} const tests[] = {
		{
			"With new len less than current size",
			1,
			1,
			Buffer::DFLT_SIZE,
		},
		{
			"With new len greater than current size",
			256,
			256,
			257,
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("set_len()", tests[x].desc);

		Buffer b;

		b.set_len(tests[x].in_len);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_resize()
{
	struct {
		const char *desc;
		const char *in_v;
		size_t      in_resize_to;
		size_t      exp_len;
		size_t      exp_size;
		const char *exp_v;
	} const tests[] = {
		{
			"Greater than current size",
			"Buffer",
			256,
			6,
			256,
			"Buffer",
		},
		{
			"Less than current size",
			"Buffer",
			1,
			6,
			Buffer::DFLT_SIZE,
			"Buffer",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("resize()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);
		b.resize(tests[x].in_resize_to);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();
	}
}

void test_char_at()
{
	struct {
		const char *desc;
		const char *in;
		size_t     idx;
		char       exp;
	} const tests[] = {
		{
			"With empty buffer",
			"",
			0,
			0,
		},
		{
			"With index out of range",
			"Test",
			100,
			0,
		},
		{
			"With index same as input length",
			"Test",
			4,
			0,
		},
		{
			"With correct index",
			"Test",
			0,
			'T',
		},
		{
			"With correct index",
			"Test",
			3,
			't',
		},
	};

	Buffer b;
	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("char_at()", tests[x].desc);

		b.copy_raw(tests[x].in);

		char got = b.char_at(tests[x].idx);

		assert(tests[x].exp == got);

		T.ok();
	}
}

void test_set_char_at()
{
	struct {
		const char *desc;
		const char *in;
		size_t     idx;
		char       v;
		int        exp_ret;
		const char *exp_res;
	} const tests[] = {
		{
			"With empty input",
			"",
			0,
			'x',
			-1,
			"",
		},
		{
			"With out of range index",
			"Test",
			10,
			'x',
			-1,
			"Test",
		},
		{
			"With index equal to length",
			"Test",
			4,
			'x',
			-1,
			"Test",
		},
		{
			"Within range",
			"Test",
			3,
			'x',
			0,
			"Tesx",
		},
	};

	Buffer b;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("set_char_at()", tests[x].desc);

		b.copy_raw(tests[x].in);

		int got_ret = b.set_char_at(tests[x].idx, tests[x].v);

		assert(tests[x].exp_ret == got_ret);

		T.expect_string(tests[x].exp_res, b.v(), 0);

		T.ok();
	}
}

void test_copy()
{
	struct {
		const char   *desc;
		Buffer       *in;
		const size_t exp_len;
		const size_t exp_size;
		const char   *exp_v;
	} const tests[] = {
		{
			"With null",
			NULL,
			0,
			Buffer::DFLT_SIZE,
			"",
		},
		{
			"With buffer greater than default size",
			new Buffer(TEST_IN_MED),
			test_in_med_len,
			test_in_med_len,
			TEST_IN_MED,
		},
	};

	Buffer b;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x< tests_len; x++) {
		T.start("copy()", tests[x].desc);

		b.copy(tests[x].in);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		if (tests[x].in) {
			delete tests[x].in;
		}

		T.ok();
	}
}

void test_copy_raw()
{
	struct {
		const char   *desc;
		const char   *in;
		size_t       in_len;
		const size_t exp_len;
		const size_t exp_size;
		const char   *exp_v;
	} const tests[] = {
		{
			"With empty string",
			NULL,
			0,
			0,
			Buffer::DFLT_SIZE,
			"",
		},
		{
			"With short string",
			TEST_IN_SHORT,
			0,
			test_in_short_len,
			Buffer::DFLT_SIZE,
			TEST_IN_SHORT,
		},
		{
			"With medium string",
			TEST_IN_MED,
			0,
			test_in_med_len,
			test_in_med_len,
			TEST_IN_MED,
		},
		{
			"With length",
			TEST_IN_MED,
			3,
			3,
			Buffer::DFLT_SIZE,
			"Use",
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("copy_raw()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in, tests[x].in_len);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();
	}
}

void test_copy_raw_at()
{
	struct {
		const char *desc;
		const char *init;
		size_t     in_idx;
		const char *in_v;
		size_t     in_len;
		size_t     exp_len;
		size_t     exp_size;
		const char *exp_v;
	} const tests[] = {
		{
			"With empty buffer: zero index and empty value",
			"",
			0,
			NULL,
			0,
			0,
			Buffer::DFLT_SIZE,
			"",
		},
		{
			"With empty buffer: zero index and non-empty value",
			"",
			0,
			"Test copy_raw_at()",
			0,
			18,
			18,
			"Test copy_raw_at()",
		},
		{
			"With non-empty buffer: zero index and empty value",
			"Test copy_raw_at()",
			0,
			NULL,
			0,
			18,
			18,
			"Test copy_raw_at()",
		},
		{
			"With non-empty buffer: zero index and non-empty value",
			"Test copy_raw_at()",
			0,
			"XXXX",
			0,
			18,
			18,
			"XXXX copy_raw_at()",
		},
		{
			"With non-empty buffer: zero index and value longer than buffer",
			"Test copy_raw_at()",
			0,
			"Test copy_raw_at() 123456",
			0,
			25,
			25,
			"Test copy_raw_at() 123456",
		},
		{
			"With non-empty buffer: replace middle buffer",
			"1234567890",
			2,
			"XXX",
			0,
			10,
			Buffer::DFLT_SIZE,
			"12XXX67890",
		},
		{
			"With non-empty buffer: replace middle buffer longer than buffer",
			"1234567890",
			2,
			"abcdefghij",
			0,
			12,
			Buffer::DFLT_SIZE,
			"12abcdefghij",
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("copy_raw_at()", tests[x].desc);

		Buffer b;
		b.copy_raw(tests[x].init);

		b.copy_raw_at(tests[x].in_idx, tests[x].in_v, tests[x].in_len);

		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);
		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();
	}
}

void test_set()
{
	struct {
		const char *desc;
		const char *in_v;
		Buffer     *in_bfr;
		Buffer     *in_dflt;
		const char *exp_v;
	} const tests[] = {
		{
			"With both parameters are NULL",
			"initial",
			NULL,
			NULL,
			"initial",
		},
		{
			"With bfr is empty and default is NULL",
			"initial",
			new Buffer(),
			NULL,
			"initial",
		},
		{
			"With bfr is not empty and default is NULL",
			"initial",
			new Buffer("buffer"),
			NULL,
			"buffer",
		},
		{
			"With bfr is NULL and default is empty",
			"initial",
			NULL,
			new Buffer(),
			"initial",
		},
		{
			"With bfr is NULL and default is not empty",
			"initial",
			NULL,
			new Buffer("default"),
			"default",
		},
		{
			"With both parameters are empty",
			"initial",
			new Buffer(),
			new Buffer(),
			"initial",
		},
		{
			"With both parameters are not empty",
			"initial",
			new Buffer("buffer"),
			new Buffer("default"),
			"buffer",
		},
	};

	Buffer b;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("set()", tests[x].desc);

		b.copy_raw(tests[x].in_v);

		b.set(tests[x].in_bfr, tests[x].in_dflt);

		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();

		if (tests[x].in_bfr) {
			delete tests[x].in_bfr;
		}
		if (tests[x].in_dflt) {
			delete tests[x].in_dflt;
		}
	}
}

void test_set_raw()
{
	struct {
		const char *desc;
		const char *in_v;
		const char *in_bfr;
		const char *in_dflt;
		const char *exp_v;
	} const tests[] = {
		{
			"With both parameters are NULL",
			"initial",
			NULL,
			NULL,
			"initial",
		},
		{
			"With bfr is empty and default is NULL",
			"initial",
			"",
			NULL,
			"initial",
		},
		{
			"With bfr is not empty and default is NULL",
			"initial",
			"buffer",
			NULL,
			"buffer",
		},
		{
			"With bfr is NULL and default is empty",
			"initial",
			NULL,
			"",
			"initial",
		},
		{
			"With bfr is NULL and default is not empty",
			"initial",
			NULL,
			"default",
			"default",
		},
		{
			"With both parameters are empty",
			"initial",
			"",
			"",
			"initial",
		},
		{
			"With both parameters are not empty",
			"initial",
			"buffer",
			"default",
			"buffer",
		},
	};

	Buffer b;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("set_raw()", tests[x].desc);

		b.copy_raw(tests[x].in_v);

		b.set_raw(tests[x].in_bfr, tests[x].in_dflt);

		T.expect_string(tests[x].exp_v, b.v(), 0);

		T.ok();
	}
}

void test_shiftr()
{
	struct {
		const char *desc;
		const char *in_v;
		const size_t in_nbyte;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With empty buffer",
			"",
			4,
			"",
			0,
			16,
		},
		{
			"With empty buffer and nbyte greater than buffer size",
			"",
			20,
			"",
			0,
			20,
		},
		{
			"With non empty buffer",
			"abcd",
			4,
			"\0\0\0\0abcd",
			8,
			16,
		},
		{
			"With non empty buffer and nbyte greater than buffer size",
			"abcdefghij",
			10,
			"\0\0\0\0\0\0\0\0\0\0abcdefghij",
			20,
			20,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("shiftr()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.shiftr(tests[x].in_nbyte);

		T.expect_mem(tests[x].exp_v, b.v(), tests[x].exp_len, 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_appendc()
{
	struct {
		const char *desc;
		const char *in_v;
		const char in_c;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With negative char",
			"",
			-1,
			"",
			0,
			16,
		},
		{
			"With empty buffer",
			"",
			'a',
			"a",
			1,
			16,
		},
		{
			"With non empty buffer",
			"abcd",
			'e',
			"abcde",
			5,
			16,
		},
		{
			"With buffer length equal to size",
			"1234567890123456",
			'a',
			"1234567890123456a",
			17,
			17,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("appendc()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.appendc(tests[x].in_c);

		T.expect_string(tests[x].exp_v, b.v(), 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_appendi()
{
	struct {
		const char *desc;
		const char *in_v;
		const long int in_i;
		const long int in_base;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With negative number",
			"",
			-1,
			10,
			"-1",
			2,
			16,
		},
		{
			"With big negative number",
			"",
			-1234567890,
			10,
			"-1234567890",
			11,
			16,
		},
		{
			"With zero",
			"",
			0,
			10,
			"0",
			1,
			16,
		},
		{
			"With positive number",
			"",
			1234567890,
			10,
			"1234567890",
			10,
			16,
		},
		{
			"With base 16 (0)",
			"",
			0,
			16,
			"0",
			1,
			16,
		},
		{
			"With base 16 (15)",
			"",
			15,
			16,
			"F",
			1,
			16,
		},
		{
			"With base 16 (4096)",
			"",
			4096,
			16,
			"1000",
			4,
			16,
		},
		{
			"With base 16 (3545088211)",
			"",
			3545088211,
			16,
			"D34DC0D3",
			8,
			16,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("appendi()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.appendi(tests[x].in_i, tests[x].in_base);

		T.expect_string(tests[x].exp_v, b.v(), 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_appendui()
{
	struct {
		const char *desc;
		const char *in_v;
		const long int in_i;
		const long int in_base;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With negative number",
			"",
			-1,
			10,
			"18446744073709551615",
			20,
			20,
		},
		{
			"With big negative number",
			"",
			-1234567890,
			10,
			"18446744072474983726",
			20,
			20,
		},
		{
			"With zero",
			"",
			0,
			10,
			"0",
			1,
			16,
		},
		{
			"With positive number",
			"",
			1234567890,
			10,
			"1234567890",
			10,
			16,
		},
		{
			"With base 16 (0)",
			"",
			0,
			16,
			"0",
			1,
			16,
		},
		{
			"With base 16 (15)",
			"",
			15,
			16,
			"F",
			1,
			16,
		},
		{
			"With base 16 (4096)",
			"",
			4096,
			16,
			"1000",
			4,
			16,
		},
		{
			"With base 16 (3545088211)",
			"",
			3545088211,
			16,
			"D34DC0D3",
			8,
			16,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("appendui()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.appendui(tests[x].in_i, tests[x].in_base);

		T.expect_string(tests[x].exp_v, b.v(), 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_appendd()
{
	struct {
		const char *desc;
		const char *in_v;
		const double in_d;
		const size_t in_prec;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With negative number (-1, 0)",
			"",
			-1,
			3,
			"-1.0",
			4,
			16,
		},
		{
			"With negative number (-1, 3)",
			"",
			-1,
			3,
			"-1.0",
			4,
			16,
		},
		{
			"With negative number (-1.234, 3)",
			"",
			-1.23456,
			3,
			"-1.234",
			6,
			16,
		},
		{
			"With negative number (-1.234567, 3)",
			"",
			-1.234567,
			3,
			"-1.234",
			6,
			16,
		},
		{
			"With zero (0, 3)",
			"",
			0,
			3,
			"0.0",
			3,
			16,
		},
		{
			"With positive number (1, 3)",
			"",
			1,
			3,
			"1.0",
			3,
			16,
		},
		{
			"With postitive number (1.234, 3)",
			"",
			1.234,
			3,
			"1.234",
			5,
			16,
		},
		{
			"With positive number (1.234567, 3)",
			"",
			1.234567,
			3,
			"1.234",
			5,
			16,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("appendd()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);

		b.appendd(tests[x].in_d, tests[x].in_prec);

		T.expect_string(tests[x].exp_v, b.v(), 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_append()
{
	struct {
		const char *desc;
		const char *init;
		Buffer *in;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With nil buffer",
			"",
			NULL,
			"",
			0,
			16,
		},
		{
			"With empty buffer",
			"",
			new Buffer(""),
			"",
			0,
			16,
		},
		{
			"With init and empty buffer",
			"abcdefghij",
			new Buffer(""),
			"abcdefghij",
			10,
			16,
		},
		{
			"With non empty buffer",
			"abcdefghij",
			new Buffer(" klmnopqrst"),
			"abcdefghij klmnopqrst",
			21,
			21,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("append()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		b.append(tests[x].in);

		T.expect_string(tests[x].exp_v, b.v(), 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();

		if (tests[x].in) {
			delete tests[x].in;
		}
	}
}

void test_append_raw()
{
	struct {
		const char *desc;
		const char *init;
		const char *in;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With nil buffer",
			"",
			NULL,
			"",
			0,
			16,
		},
		{
			"With empty buffer",
			"",
			"",
			"",
			0,
			16,
		},
		{
			"With single char",
			"",
			"a",
			"a",
			1,
			16,
		},
		{
			"With init and single char",
			"abcdefghijklmnopqrst",
			"u",
			"abcdefghijklmnopqrstu",
			21,
			21,
		},
		{
			"With init and empty buffer",
			"abcdefghij",
			"",
			"abcdefghij",
			10,
			16,
		},
		{
			"With non empty buffer",
			"abcdefghij",
			" klmnopqrst",
			"abcdefghij klmnopqrst",
			21,
			21,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("append_raw()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		b.append_raw(tests[x].in);

		T.expect_string(tests[x].exp_v, b.v(), 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_append_bin()
{
	const char *chars = "1234567890";
	int i = 1234567890;

	char *exp_i = (char *) calloc(14, 1);
	memcpy(exp_i, chars, 10);
	memcpy(&exp_i[10], &i, 4);

	struct {
		const char *desc;
		const char *init;
		const void *bin;
		const size_t len;
		const void *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With null on empty buffer",
			"",
			NULL,
			0,
			"",
			0,
			16,
		},
		{
			"With chars on empty buffer",
			"",
			chars,
			4,
			"1234",
			4,
			16,
		},
		{
			"With chars on non empty buffer",
			"abcd",
			chars,
			4,
			"abcd1234",
			8,
			16,
		},
		{
			"With integer on empty buffer",
			"",
			&i,
			4,
			&i,
			4,
			16,
		},
		{
			"With integer on non empty buffer",
			"1234567890",
			&i,
			4,
			exp_i,
			14,
			16,
		},

	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("append_bin()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		b.append_bin(tests[x].bin, tests[x].len);

		T.expect_mem(tests[x].exp_v, b.v(), tests[x].len, 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

void test_concat()
{
	struct {
		const char *desc;
		const char *init;
		const char *in_v0;
		const char *in_v1;
		const char *in_v2;
		const char *exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With NULL",
			"",
			NULL,
			NULL,
			NULL,
			"",
			0,
			16,
		},
		{
			"With empty strings",
			"",
			"",
			"",
			NULL,
			"",
			0,
			16,
		},
		{
			"With empty string on first param",
			"",
			"",
			"abcd",
			NULL,
			"abcd",
			4,
			16,
		},
		{
			"With init",
			"abcd",
			"efgh",
			"ijkl",
			NULL,
			"abcdefghijkl",
			12,
			16,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("concat()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		b.concat(tests[x].in_v0, tests[x].in_v1, tests[x].in_v2);

		T.expect_string(tests[x].exp_v, b.v(), 0);
		T.expect_unsigned(tests[x].exp_len, b.len(), 0);
		T.expect_unsigned(tests[x].exp_size, b.size(), 0);

		T.ok();
	}
}

Buffer in;
List* lbuf;

void test_split_by_char_n(const char* input, const char split
	, const uint8_t trim, const char* exp, const int exp_size)
{
	in.copy_raw(input);
	lbuf = in.split_by_char(split, trim);

	expectString(exp, lbuf->chars(), 0);

	assert(lbuf->size() == exp_size);
	expectString(exp, lbuf->chars(), 0);
	delete lbuf;
}

void test_split_by_char()
{
	in.reset();
	lbuf = in.split_by_char(',');
	assert(lbuf == NULL);

	test_split_by_char_n(TEST_SPLIT_BY_00_IN, ',', 0
		, TEST_SPLIT_BY_00_OUT, 1);

	test_split_by_char_n(TEST_SPLIT_BY_01_IN, ':', 0
		, TEST_SPLIT_BY_01_OUT, 2);

	test_split_by_char_n(TEST_SPLIT_BY_02_IN, ',', 0
		, TEST_SPLIT_BY_02_OUT, 4);

	test_split_by_char_n(TEST_SPLIT_BY_02_IN, ',', 1
		, TEST_SPLIT_BY_02_OUT_TRIM, 3);

	test_split_by_char_n(TEST_SPLIT_BY_03_IN, ',', 0
		, TEST_SPLIT_BY_03_OUT, 6);

	test_split_by_char_n(TEST_SPLIT_BY_03_IN, ',', 1
		, TEST_SPLIT_BY_03_OUT_TRIM, 3);
}

void test_split_by_whitespace_n(const char* input
	, const char* exp, const int exp_size)
{
	in.copy_raw(input);
	lbuf = in.split_by_whitespace();

	expectString(exp, lbuf->chars(), 0);

	assert(lbuf->size() == exp_size);
	expectString(exp, lbuf->chars(), 0);
	delete lbuf;
}

void test_split_by_whitespace()
{
	in.reset();

	lbuf = in.split_by_whitespace();
	assert(lbuf == NULL);

	test_split_by_whitespace_n(TEST_SPLIT_BY_00_IN
		, TEST_SPLIT_BY_WS_00_OUT, 1);
	test_split_by_whitespace_n(TEST_SPLIT_BY_02_IN
		, TEST_SPLIT_BY_WS_02_OUT, 7);
	test_split_by_whitespace_n(TEST_SPLIT_BY_03_IN
		, TEST_SPLIT_BY_WS_03_OUT, 8);
	test_split_by_whitespace_n(TEST_04_IN, TEST_04_WS_OUT, 5);
}

void test_PARSE_INT()
{
	int s = 0;
	int v = 0;
	char* str = (char*) calloc(64, sizeof(char));
	char* p;

	strcpy(str, "\0");
	p = str;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 0);
	assert(*p == 0);

	strcpy(str, "-asdf");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 0);
	assert(*p == '-');

	strcpy(str, "0");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 0);
	assert(*p == 0);

	strcpy(str, "0123456");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 123456);
	assert(*p == 0);

	strcpy(str, "-0123456");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == -123456);
	assert(*p == 0);

	strcpy(str, "0123as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 123);
	assert(*p == 'a');

	strcpy(str, "-0123as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == -123);
	assert(*p == 'a');

	// overflow
	strcpy(str, "9876543210as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == -1);
	assert(v == 0);
	assert(*p == '9');

	// underflow
	strcpy(str, "-9876543210as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == -1);
	assert(v == 0);
	assert(*p == '-');

	free(str);
}

void test_aprint()
{
	const char* exps[] = {
			"%11209876543210"
		,	"%---##++"
		,	"%---#1"
		,	"%-.11209876543210"
		,	"112.987654"
		,	"  3"
		,	"113.0"
		,	"112.988"
		};

	Buffer b;
	double d = 112.987654321;
	int exp_idx = 0;

	b.reset();
	b.aprint("%11209876543210", d);
	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%---##++", d);
	expectString(exps[exp_idx++], b.chars(), 0);


	b.reset();
	b.aprint("%---#1", d);
	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%-.11209876543210", d);
	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%f", d);

	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%3d", 3);

	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%.1f", d);

	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%.3f", d);

	expectString(exps[exp_idx++], b.chars(), 0);
}

int main()
{
	test_constructor();
	test_constructor_size();
	test_constructor_raw();
	test_constructor_buffer();

	test_detach();
	test_release();
	test_reset();
	test_trim();
	test_truncate();

	test_is_empty();

	// skip testing `len()`, because its already done on other tests.
	test_set_len();

	// skip testing `size()`, because its already done on other tests.
	test_resize();

	// skip testing `v()`, because its already done on other tests.

	test_char_at();
	test_set_char_at();

	test_copy();
	test_copy_raw();
	test_copy_raw_at();

	test_set();
	test_set_raw();

	test_shiftr();

	test_appendc();
	test_appendi();
	test_appendui();
	test_appendd();
	test_append();
	test_append_raw();
	test_append_bin();

	test_concat();

	test_split_by_char();

	test_split_by_whitespace();

	test_PARSE_INT();

	//test_aprint();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
