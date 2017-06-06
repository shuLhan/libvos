/**
 * Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "test.hh"
#include "../Test.hh"
#include "../List.hh"

using vos::Test;
using vos::List;

Test T("Buffer");

void test_constructor()
{
	T.start("constructor", "default");

	Buffer a;

	T.expect_string(a.__CNAME, "Buffer");
	T.expect_string(a.Object::__CNAME, "Object");

	T.expect_unsigned(a.size(), Buffer::DFLT_SIZE, vos::IS_EQUAL);
	T.expect_unsigned(a.len(), 0, vos::IS_EQUAL);
	T.expect_string(a.v(), "");

	T.ok();
}

void test_constructor_size()
{
	struct {
		const char* desc;
		size_t      in_size;
		size_t      exp_len;
		size_t      exp_size;
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

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_constructor_raw()
{
	struct {
		const char* desc;
		const char* in_v;
		size_t      in_len;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
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
		T.start("Buffer(const char*, size_t)", tests[x].desc);

		Buffer b(tests[x].in_v, tests[x].in_len);

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_constructor_buffer()
{
	struct {
		const char* desc;
		Buffer*     in_buffer;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
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
		T.start("Buffer(const Buffer*)", tests[x].desc);

		Buffer b(tests[x].in_buffer);

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		if (tests[x].in_buffer) {
			delete tests[x].in_buffer;
		}

		T.ok();
	}
}

void test_detach()
{
	struct {
		const char* desc;
		const char* in_v;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
		const char* exp_ret;
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

		char* got = b.detach();

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_ptr((void*) tests[x].exp_v, (void*) b.v(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_ret, got, vos::IS_EQUAL);

		if (got) {
			free(got);
		}

		T.ok();
	}
}

void test_release()
{
	struct {
		const char* desc;
		const char* in_v;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
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

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_ptr((void*) tests[x].exp_v, (void*) b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_reset()
{
	struct {
		const char* desc;
		const char* in_v;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
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

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_trim()
{
	struct {
		const char* desc;
		const char* in_v;
		const char* exp;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With space before",
			"	  a",
			"a",
			1,
			16,
		},
		{
			"With space after",
			"a	  ",
			"a",
			1,
			16,
		},
		{
			"With space before and after",
			"	  a	  ",
			"a",
			1,
			16,
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("trim()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in_v);
		b.trim();

		T.expect_string(tests[x].exp, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_truncate()
{
	struct {
		const char* desc;
		const char* in_v;
		size_t      in_len;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
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

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_is_empty()
{
	struct {
		const char* desc;
		const char* in_v;
		int         exp_res;
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

		T.expect_signed(tests[x].exp_res, b.is_empty(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_set_len()
{
	struct {
		const char* desc;
		size_t      in_len;
		size_t      exp_len;
		size_t      exp_size;
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

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_resize()
{
	struct {
		const char* desc;
		const char* in_v;
		size_t      in_resize_to;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
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

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_char_at()
{
	struct {
		const char* desc;
		const char* in;
		size_t      idx;
		char        exp;
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

		T.expect_signed(1, tests[x].exp == got, vos::IS_EQUAL);

		T.ok();
	}
}

void test_set_char_at()
{
	struct {
		const char* desc;
		const char* in;
		size_t      idx;
		char        v;
		Error       exp_err;
		const char* exp_res;
	} const tests[] = {
		{
			"With empty input",
			"",
			0,
			'x',
			vos::ErrBufferInvalidIndex,
			"",
		},
		{
			"With out of range index",
			"Test",
			10,
			'x',
			vos::ErrBufferInvalidIndex,
			"Test",
		},
		{
			"With index equal to length",
			"Test",
			4,
			'x',
			vos::ErrBufferInvalidIndex,
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

		Error err = b.set_char_at(tests[x].idx, tests[x].v);

		T.expect_signed(1, tests[x].exp_err == err, vos::IS_EQUAL);

		T.expect_string(tests[x].exp_res, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_copy()
{
	struct {
		const char*  desc;
		Buffer*      in;
		const size_t exp_len;
		const size_t exp_size;
		const char*  exp_v;
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
			new Buffer("1234567890 1234567890 1234567890"),
			32,
			32,
			"1234567890 1234567890 1234567890",
		},
	};

	Buffer b;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x< tests_len; x++) {
		T.start("copy()", tests[x].desc);

		b.copy(tests[x].in);

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		if (tests[x].in) {
			delete tests[x].in;
		}

		T.ok();
	}
}

void test_copy_raw()
{
	struct {
		const char*  desc;
		const char*  in;
		size_t       in_len;
		const size_t exp_len;
		const size_t exp_size;
		const char*  exp_v;
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
			"a",
			0,
			1,
			Buffer::DFLT_SIZE,
			"a",
		},
		{
			"With medium string",
			"1234567890 1234567890 1234567890",
			0,
			32,
			32,
			"1234567890 1234567890 1234567890",
		},
		{
			"With length",
			"1234567890 1234567890 1234567890",
			3,
			3,
			Buffer::DFLT_SIZE,
			"123",
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("copy_raw()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].in, tests[x].in_len);

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_copy_raw_at()
{
	struct {
		const char* desc;
		const char* init;
		size_t      in_idx;
		const char* in_v;
		size_t      in_len;
		size_t      exp_len;
		size_t      exp_size;
		const char* exp_v;
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

		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_set()
{
	struct {
		const char* desc;
		const char* in_v;
		Buffer*     in_bfr;
		Buffer*     in_dflt;
		const char* exp_v;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

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
		const char* desc;
		const char* in_v;
		const char* in_bfr;
		const char* in_dflt;
		const char* exp_v;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_shiftr()
{
	struct {
		const char*  desc;
		const char*  in_v;
		const size_t in_nbyte;
		const char*  exp_v;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With empty buffer",
			"",
			4,
			"\0\0\0\0",
			4,
			16,
		},
		{
			"With empty buffer and nbyte greater than buffer size",
			"",
			20,
			"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
			20,
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

		T.expect_mem(tests[x].exp_v, b.v(), tests[x].exp_len, vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_appendc()
{
	struct {
		const char*  desc;
		const char*  in_v;
		const char   in_c;
		const char*  exp_v;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_appendi()
{
	struct {
		const char*    desc;
		const char*    in_v;
		const long int in_i;
		const size_t   in_base;
		const char*    exp_v;
		const size_t   exp_len;
		const size_t   exp_size;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_appendui()
{
	struct {
		const char*         desc;
		const char*         in_v;
		const unsigned long in_i;
		const size_t        in_base;
		const char*         exp_v;
		const size_t        exp_len;
		const size_t        exp_size;
	} const tests[] = {
		{
			"With negative number",
			"",
			(unsigned long) -1,
			10,
			"18446744073709551615",
			20,
			20,
		},
		{
			"With big negative number",
			"",
			(unsigned long) -1234567890,
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_appendd()
{
	struct {
		const char*  desc;
		const char*  in_v;
		const double in_d;
		const size_t in_prec;
		const char*  exp_v;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_append()
{
	struct {
		const char*  desc;
		const char*  init;
		Buffer*      in;
		const char*  exp_v;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();

		if (tests[x].in) {
			delete tests[x].in;
		}
	}
}

void test_append_raw()
{
	struct {
		const char*  desc;
		const char*  init;
		const char*  in;
		const char*  exp_v;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_append_bin()
{
	const char* chars = "1234567890";
	int i = 1234567890;

	char* exp_i = (char*) calloc(14, 1);
	memcpy(exp_i, chars, 10);
	memcpy(&exp_i[10], &i, 4);

	struct {
		const char*  desc;
		const char*  init;
		const void*  bin;
		const size_t len;
		const void*  exp_v;
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

		T.expect_mem(tests[x].exp_v, b.v(), tests[x].len, vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}

	if (exp_i) {
		free(exp_i);
	}
}

void test_concat()
{
	struct {
		const char*  desc;
		const char*  init;
		const char*  in_v0;
		const char*  in_v1;
		const char*  in_v2;
		const char*  exp_v;
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

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_prepend()
{
	struct {
		const char*  desc;
		const char*  init;
		Buffer*      in;
		const char*  exp_v;
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
			new Buffer("klmnopqrst "),
			"klmnopqrst abcdefghij",
			21,
			21,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("prepend()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		b.prepend(tests[x].in);

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();

		if (tests[x].in) {
			delete tests[x].in;
		}
	}
}

void test_prepend_raw()
{
	struct {
		const char*  desc;
		const char*  init;
		const char*  in;
		const char*  exp_v;
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
			"uabcdefghijklmnopqrst",
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
			" klmnopqrstabcdefghij",
			21,
			21,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("prepend_raw()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		b.prepend_raw(tests[x].in);

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_subc()
{
	struct {
		const char*  desc;
		const char*  init;
		const char   from;
		const char   to;
		const char*  exp_v;
		const size_t exp_res;
		const size_t exp_len;
		const size_t exp_size;
	} const tests[] = {
		{
			"With empty buffer",
			"",
			'a',
			'b',
			"",
			0,
			0,
			16,
		},
		{
			"With no replacement found",
			"zxcvbnm",
			'a',
			'b',
			"zxcvbnm",
			0,
			7,
			16,
		},
		{
			"With success",
			"ababab",
			'a',
			'b',
			"bbbbbb",
			3,
			6,
			16,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("subc()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		size_t res = b.subc(tests[x].from, tests[x].to);

		T.expect_string(tests[x].exp_v, b.v(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_res, res, vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_len, b.len(), vos::IS_EQUAL);
		T.expect_unsigned(tests[x].exp_size, b.size(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_cmp()
{
	struct {
		const char* desc;
		const char* init;
		Buffer*     in;
		const int exp_res;
	} const tests[] = {
		{
			"With empty and NULL object",
			"",
			NULL,
			0,
		},
		{
			"With non-empty buffer and NULL object",
			"a",
			NULL,
			1,
		},
		{
			"With both buffer are empty",
			"",
			new Buffer(""),
			0,
		},
		{
			"With greater-than result",
			"z",
			new Buffer("a"),
			1,
		},
		{
			"With equal result",
			"a",
			new Buffer("a"),
			0,
		},
		{
			"With less-than result",
			"a",
			new Buffer("b"),
			-1,
		},
		{
			"Check sensitivity a - A",
			"a",
			new Buffer("A"),
			1,
		},
		{
			"Check sensitivity A - a",
			"A",
			new Buffer("a"),
			-1,
		},
		{
			"Check number vs lower char",
			"1",
			new Buffer("a"),
			-1,
		},
		{
			"Check number vs capital char",
			"1",
			new Buffer("A"),
			-1,
		},
		{
			"With different length 'abcd vs. a'",
			"abcd",
			new Buffer("a"),
			1,
		},
		{
			"With different length and number 'abcd vs. 1'",
			"abcd",
			new Buffer("1"),
			1,
		},
		{
			"With different length 'a vs. abcd'",
			"a",
			new Buffer("abcd"),
			-1,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("cmp()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		int got = b.cmp(tests[x].in);

		T.expect_signed(tests[x].exp_res, got, vos::IS_EQUAL);

		T.ok();

		if (tests[x].in) {
			delete tests[x].in;
		}
	}
}

void test_cmp_raw()
{
	struct {
		const char* desc;
		const char* init;
		const char* in_bfr;
		const int exp_res;
	} const tests[] = {
		{
			"With empty and NULL object",
			"",
			NULL,
			0,
		},
		{
			"With non-empty buffer and NULL buffer",
			"a",
			NULL,
			1,
		},
		{
			"With both buffer are empty",
			"",
			"",
			0,
		},
		{
			"With greater-than result",
			"z",
			"a",
			1,
		},
		{
			"With equal result",
			"a",
			"a",
			0,
		},
		{
			"With less-than result",
			"a",
			"b",
			-1,
		},
		{
			"Check sensitivity a - A",
			"a",
			"A",
			1,
		},
		{
			"Check sensitivity A - a",
			"A",
			"a",
			-1,
		},
		{
			"Check number vs lower char",
			"1",
			"a",
			-1,
		},
		{
			"Check number vs capital char",
			"1",
			"A",
			-1,
		},
		{
			"With different length 'abcd vs. a'",
			"abcd",
			"a",
			1,
		},
		{
			"With different length and number 'abcd vs. 1'",
			"abcd",
			"1",
			1,
		},
		{
			"With different length 'a vs. abcd'",
			"a",
			"abcd",
			-1,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("cmp_raw()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		int got = b.cmp_raw(tests[x].in_bfr);

		T.expect_signed(tests[x].exp_res, got, vos::IS_EQUAL);

		T.ok();
	}
}

void test_like()
{
	struct {
		const char* desc;
		const char* init;
		Buffer*     in;
		const int exp_res;
	} const tests[] = {
		{
			"With empty and NULL object",
			"",
			NULL,
			0,
		},
		{
			"With non-empty buffer and NULL object",
			"a",
			NULL,
			1,
		},
		{
			"With both buffer are empty",
			"",
			new Buffer(""),
			0,
		},
		{
			"With greater-than result",
			"z",
			new Buffer("a"),
			1,
		},
		{
			"With equal result",
			"a",
			new Buffer("a"),
			0,
		},
		{
			"With less-than result",
			"a",
			new Buffer("b"),
			-1,
		},
		{
			"Check sensitivity 'a vs. A'",
			"a",
			new Buffer("A"),
			0,
		},
		{
			"Check sensitivity 'A vs. a'",
			"A",
			new Buffer("a"),
			0,
		},
		{
			"Check number vs lower char",
			"1",
			new Buffer("a"),
			-1,
		},
		{
			"Check number vs capital char",
			"1",
			new Buffer("A"),
			-1,
		},
		{
			"With different length 'abcd vs. a'",
			"abcd",
			new Buffer("a"),
			1,
		},
		{
			"With different length and number 'abcd vs. 1'",
			"abcd",
			new Buffer("1"),
			1,
		},
		{
			"With different length 'a vs. abcd'",
			"a",
			new Buffer("abcd"),
			-1,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("like()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		int got = b.like(tests[x].in);

		T.expect_signed(tests[x].exp_res, got, vos::IS_EQUAL);

		T.ok();

		if (tests[x].in) {
			delete tests[x].in;
		}
	}
}

void test_like_raw()
{
	struct {
		const char* desc;
		const char* init;
		const char* in_bfr;
		const int   exp_res;
	} const tests[] = {
		{
			"With empty and NULL object",
			"",
			NULL,
			0,
		},
		{
			"With non-empty buffer and NULL object",
			"a",
			NULL,
			1,
		},
		{
			"With greater-than result",
			"z",
			"a",
			1,
		},
		{
			"With equal result",
			"a",
			"a",
			0,
		},
		{
			"With less-than result",
			"a",
			"b",
			-1,
		},
		{
			"Check sensitivity 'a vs. A'",
			"a",
			"A",
			0,
		},
		{
			"Check sensitivity 'A vs. a'",
			"A",
			"a",
			0,
		},
		{
			"Check number vs lower char",
			"1",
			"a",
			-1,
		},
		{
			"Check number vs capital char",
			"1",
			"A",
			-1,
		},
		{
			"With different length 'abcd vs. a'",
			"abcd",
			"a",
			1,
		},
		{
			"With different length and number 'abcd vs. 1'",
			"abcd",
			"1",
			1,
		},
		{
			"With different length 'a vs. abcd'",
			"a",
			"abcd",
			-1,
		},
	};

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("like_raw()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		int got = b.like_raw(tests[x].in_bfr);

		T.expect_signed(tests[x].exp_res, got, vos::IS_EQUAL);

		T.ok();
	}
}

void test_to_lint()
{
	struct {
		const char*    desc;
		const char*    init;
		const long int exp_v;
		const Error    exp_err;
	} const tests[] = {
		{
			"With empty buffer",
			"",
			0,
			0,
		},
		{
			"With alpha",
			"a",
			0,
			0,
		},
		{
			"With numeric and alpha",
			"123a",
			123,
			0,
		},
		{
			"With out of range number",
			"9223372036854775808",
			0,
			vos::ErrNumRange,
		},
		{
			"With success",
			"2147483647",
			2147483647,
			0,
		},
	};

	long int got = 0;
	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("to_lint()", tests[x].desc);

		Buffer b;

		b.copy_raw(tests[x].init);

		got = 0;

		Error err = b.to_lint(&got);

		T.expect_signed(tests[x].exp_v, got, vos::IS_EQUAL);
		T.expect_signed(1, tests[x].exp_err == err, vos::IS_EQUAL);

		T.ok();
	}

}

// INT_MAX = 2147483647
// LONG_MAX = 9223372036854775807
void test_PARSE_INT()
{
	struct {
		const char* desc;
		const char* init;
		const char* exp_p;
		const int   exp_v;
		const Error exp_err;
	} const tests[] = {
		{
			"With empty string",
			"",
			"",
			0,
			0,
		},
		{
			"With out of range LONG_MAX",
			"9223372036854775808",
			"9223372036854775808",
			0,
			vos::ErrNumRange,
		},
		{
			"With out of range LONG_MIN",
			"-9223372036854775808",
			"-9223372036854775808",
			0,
			vos::ErrNumRange,
		},
		{
			"With out of range INT_MAX",
			"2147483648",
			"2147483648",
			0,
			vos::ErrNumRange,
		},
		{
			"With out of range INT_MIN",
			"-2147483649",
			"-2147483649",
			0,
			vos::ErrNumRange,
		},
		{
			"With zero",
			"0",
			"",
			0,
			0,
		},
		{
			"With string (-asdf)",
			"-asdf",
			"-asdf",
			0,
			0,
		},
		{
			"With number start with negative -0 (-012345678)",
			"-012345678",
			"",
			-12345678,
			0,
		},
		{
			"With number start with positive 0 (012345678)",
			"012345678",
			"",
			12345678,
			0,
		},
		{
			"With number and string (1234asdf)",
			"1234asdf",
			"asdf",
			1234,
			0,
		},
		{
			"With string and number (asdf1234)",
			"asdf1234",
			"asdf1234",
			0,
			0,
		},

	};

	int v = 0;
	char* p = 0;
	Buffer b;
	Error err;

	size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("PARSE_INT()", tests[x].desc);

		err = b.copy_raw(tests[x].init);
		if (err != NULL) {
			printf("b.copy_raw error");
			exit(1);
		}

		p = (char*) b.v();

		Error err = Buffer::PARSE_INT(&p, &v);

		T.expect_signed(1, tests[x].exp_err == err, vos::IS_EQUAL);
		T.expect_signed(tests[x].exp_v, v, vos::IS_EQUAL);
		T.expect_string(tests[x].exp_p, p, vos::IS_EQUAL);

		T.ok();
	}
}

void test_append_fmt()
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
	b.append_fmt("%11209876543210", d);
	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);

	b.reset();
	b.append_fmt("%---##++", d);
	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);


	b.reset();
	b.append_fmt("%---#1", d);
	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);

	b.reset();
	b.append_fmt("%-.11209876543210", d);
	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);

	b.reset();
	b.append_fmt("%f", d);

	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);

	b.reset();
	b.append_fmt("%3d", 3);

	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);

	b.reset();
	b.append_fmt("%.1f", d);

	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);

	b.reset();
	b.append_fmt("%.3f", d);

	expectString(exps[exp_idx++], b.chars(), vos::IS_EQUAL);
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

	test_prepend();
	test_prepend_raw();

	test_subc();

	test_cmp();
	test_cmp_raw();
	test_like();
	test_like_raw();

	test_to_lint();

	test_PARSE_INT();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
