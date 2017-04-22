//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../Test.hh"
#include "../List.hh"

using vos::Test;
using vos::List;

#define TEST_SPLIT_BY_00_IN "127.0.0.1:53"
#define TEST_SPLIT_BY_00_OUT SB(V_STR(TEST_SPLIT_BY_00_IN))
#define TEST_SPLIT_BY_WS_00_OUT SB(V_STR(TEST_SPLIT_BY_00_IN))

#define TEST_SPLIT_BY_01_IN "127.0.0.1:53"
#define TEST_SPLIT_BY_01_OUT SB(K(127.0.0.1) SEP_ITEM K(53))

#define TEST_SPLIT_BY_02_IN "a b, c d e, fg hi, "
#define	TEST_SPLIT_BY_WS_02_OUT SB( \
		V_STR("a") SEP_ITEM \
		V_STR("b,") SEP_ITEM \
		V_STR("c") SEP_ITEM \
		V_STR("d") SEP_ITEM \
		V_STR("e,") SEP_ITEM \
		V_STR("fg") SEP_ITEM \
		V_STR("hi,") \
	)

#define TEST_SPLIT_BY_02_OUT SB( \
		K(a b) SEP_ITEM \
		V_STR(" c d e") SEP_ITEM \
		V_STR(" fg hi") SEP_ITEM \
		V_STR(" ") \
	)

#define TEST_SPLIT_BY_02_OUT_TRIM SB( \
		K(a b) SEP_ITEM \
		K(c d e) SEP_ITEM \
		K(fg hi) \
	)

#define TEST_SPLIT_BY_03_IN "a b,, c d e, , fg hi, "
#define TEST_SPLIT_BY_03_OUT SB( \
		V_STR("a b") SEP_ITEM \
		V_STR("") SEP_ITEM \
		V_STR(" c d e") SEP_ITEM \
		V_STR(" ") SEP_ITEM \
		V_STR(" fg hi") SEP_ITEM \
		V_STR(" ") \
	)
#define TEST_SPLIT_BY_03_OUT_TRIM SB( \
		K(a b) SEP_ITEM \
		K(c d e) SEP_ITEM \
		K(fg hi) \
	)
#define TEST_SPLIT_BY_WS_03_OUT SB( \
		V_STR("a") SEP_ITEM \
		V_STR("b,,") SEP_ITEM \
		V_STR("c") SEP_ITEM \
		V_STR("d") SEP_ITEM \
		V_STR("e,") SEP_ITEM \
		V_STR(",") SEP_ITEM \
		V_STR("fg") SEP_ITEM \
		V_STR("hi,") \
	)

#define	TEST_04_IN "	  a   b   c		d	e	"
#define	TEST_04_WS_OUT SB( \
		V_STR("a") SEP_ITEM \
		V_STR("b") SEP_ITEM \
		V_STR("c") SEP_ITEM \
		V_STR("d") SEP_ITEM \
		V_STR("e") \
	)

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
	T.start("constructor", "with size");

	Buffer b(256);

	assert(b.len() == 0);
	assert(b.size() == 256);

	T.expect_string(b.v(), "");

	T.ok();
}

void test_constructor_raw()
{
	T.start("constructor", "with raw char");

	const char* exp = "Use of this source code is governed by a BSD-style";
	size_t exp_len = strlen(exp);

	Buffer b(exp);

	assert(b.len() == exp_len);
	assert(b.size() == exp_len);
	T.expect_string(b.v(), exp);

	T.ok();

	T.start(0, "with raw char and size");

	Buffer c(exp, 6);

	assert(c.len() == 6);
	assert(c.size() == 6);
	T.expect_string(c.v(), "Use of");

	T.ok();
}

void test_constructor_buffer()
{
	T.start("constructor", "with empty buffer");

	const char* exp = "Use of this source code is governed by a BSD-style";
	size_t exp_len = strlen(exp);

	Buffer a(256);
	Buffer b(&a);

	assert(b.len() == 0);
	assert(b.size() == a.size());
	T.expect_string(a.v(), b.v());

	T.ok();

	T.start(0, "with non-empty buffer");

	a.copy_raw(exp);

	Buffer c(&a);

	assert(c.len() == exp_len);
	assert(c.size() == exp_len);
	T.expect_string(c.v(), exp);

	T.ok();
}

void test_detach()
{
	T.start("detach");

	Buffer a;
	char *v;

	v = a.detach();

	assert(a.len() == 0);
	assert(a.size() == 0);
	assert(a.v() == NULL);
	assert(v != NULL);

	free(v);

	T.ok();
}

void test_release()
{
	T.start("release");

	Buffer a;

	assert(a.len() == 0);
	assert(a.size() == Buffer::DFLT_SIZE);
	assert(a.v() != NULL);

	a.release();

	assert(a.len() == 0);
	assert(a.size() == 0);
	assert(a.v() == NULL);

	T.ok();
}

void test_reset()
{
	T.start("reset");

	const char* exp = "a string";
	const size_t exp_len = strlen(exp);
	Buffer a;

	a.copy_raw(exp);

	assert(a.len() == exp_len);
	assert(a.size() == Buffer::DFLT_SIZE);
	T.expect_string(exp, a.v(), 0);

	a.reset();

	assert(a.len() == 0);
	assert(a.size() == Buffer::DFLT_SIZE);
	T.expect_string("", a.v(), 0);

	T.ok();
}

void test_trim()
{
	T.start("trim");

	const size_t in_len = 3;
	const char* in[in_len] = {
			"	  a"
		,	"a	  "
		,	"	  a	  "
		};
	const char* exp = "a";

	Buffer a;

	for (size_t x = 0; x < in_len; x++) {
		a.copy_raw(in[x]);
		a.trim();
		T.expect_string(exp, a.v(), 0);
	}

	T.ok();
}

void test_truncate()
{
	T.start("truncate");

	const char* input = "abcdefghijklmnopqrstuvwxyz";
	const char* exp = "abcd";
	const size_t input_len = strlen(input);
	const size_t exp_len = strlen(exp);

	Buffer a;

	a.copy_raw(input);

	assert(a.len() == input_len);
	assert(a.size() == input_len);

	a.truncate(3);

	assert(a.len() == exp_len);
	assert(a.size() == input_len);

	T.expect_string(exp, a.v(), 0);

	T.ok();
}

void test_is_empty()
{
	T.start("is_empty", "true");

	Buffer a;

	assert(a.is_empty() == 1);

	T.ok();

	T.start("is_empty", "false");

	a.copy_raw("any");

	assert(a.is_empty() == 0);

	T.ok();
}

void test_set_len()
{
	T.start("set_len", "it should not resize the buffer");

	Buffer b;

	assert(b.len() == 0);
	assert(b.size() == Buffer::DFLT_SIZE);

	b.set_len(b.size());

	assert(b.len() == b.size());
	assert(b.size() == Buffer::DFLT_SIZE);

	T.ok();

	T.start(0, "it should resize the buffer");

	size_t new_len = 24;

	b.set_len(new_len);

	assert(b.len() == new_len);
	assert(b.size() == (new_len + 1));

	T.ok();
}

void test_resize()
{
	T.start("resize", "greater than current size");

	size_t resize_to = 256;
	const char* exp = "buffer";
	const size_t exp_len = strlen(exp);
	Buffer a;

	a.copy_raw(exp);

	assert(a.len() == exp_len);
	assert(a.size() == Buffer::DFLT_SIZE);
	T.expect_string(exp, a.v(), 0);

	a.resize(resize_to);

	assert(a.len() == exp_len);
	assert(a.size() == resize_to);
	T.expect_string(exp, a.v(), 0);

	T.ok();

	T.start(0, "less than current size");

	const size_t resize_to_less = 16;

	a.resize(resize_to_less);

	assert(a.len() == exp_len);
	assert(a.size() == resize_to);
	T.expect_string(exp, a.v(), 0);

	T.ok();
}

void test_char_at()
{
	struct TestInput {
		const char* desc;
		const char* in;
		size_t      idx;
		char        exp;
	};

	TestInput const testIns[] = {
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

	Buffer       b;
	char         got;
	const size_t lenTestIns = sizeof(testIns)/sizeof(TestInput);

	for (size_t x = 0; x < lenTestIns; x++) {
		T.start("char_at", testIns[x].desc);

		b.copy_raw(testIns[x].in);

		got = b.char_at(testIns[x].idx);

		assert(testIns[x].exp == got);

		T.ok();
	}
}

int test_n = 0;
Buffer in;
List* lbuf;

void test_copy_raw()
{
	Buffer b;
	const char* str = "a";
	size_t sz = 0;

	b.copy_raw(str);

	sz = strlen(str);
	assert(b.len() == sz);
	expectString(str, b.v(), 0);

	b.copy_raw(STR_TEST_0);

	assert(b.len() == strlen(STR_TEST_0));
	expectString(STR_TEST_0, b.v(), 0);
}

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

	test_copy_raw();

	test_split_by_char();

	test_split_by_whitespace();

	test_PARSE_INT();

	//test_aprint();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
