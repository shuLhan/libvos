//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../List.hh"

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

void test_constructor_size()
{
	Buffer b(256);

	assert(b.len() == 0);
	assert(b.size() == 256);
	expectString(b.v(), "", 0);
}

void test_constructor_raw()
{
	const char* exp = "Use of this source code is governed by a BSD-style";
	size_t exp_len = strlen(exp);

	Buffer b(exp, exp_len);

	assert(b.len() == exp_len);
	assert(b.size() == exp_len);
	expectString(b.v(), exp, 0);

	Buffer c(exp, 6);

	assert(c.len() == 6);
	assert(c.size() == 6);
	expectString(c.v(), "Use of", 0);
}

void test_constructor_buffer()
{
	const char* exp = "Use of this source code is governed by a BSD-style";
	size_t exp_len = strlen(exp);

	Buffer a(256);
	Buffer b(&a);

	assert(b.len() == 0);
	assert(b.size() == a.size());
	expectString(a.v(), b.v(), 0);

	a.copy_raw(exp);

	Buffer c(&a);

	assert(c.len() == exp_len);
	assert(c.size() == exp_len);
	expectString(c.v(), exp, 0);
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
	assert(b._i == sz);
	expectString(str, b.v(), 0);

	b.copy_raw(STR_TEST_0);

	assert(b._i == strlen(STR_TEST_0));
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
	Buffer a;

	expectString(a.__cname, "Buffer", 0);
	expectString(a.Object::__cname, "Object", 0);

	assert(a._l == Buffer::DFLT_SIZE);
	assert(a._i == 0);
	expectString(a.v(), "", 0);

	test_constructor_size();
	test_constructor_raw();
	test_constructor_buffer();

	test_copy_raw();

	test_split_by_char();

	test_split_by_whitespace();

	test_PARSE_INT();

	//test_aprint();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
