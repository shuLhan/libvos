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

int test_n = 0;
Buffer in;
List* lbuf;

void test_copy_raw()
{
	Buffer b;
	const char* str = "a";

	b.copy_raw(str);

	assert(b._i == (int) strlen(str));
	assert(strcmp(b.chars(), str) == 0);

	b.copy_raw(STR_TEST_0);

	assert(b._i == strlen(STR_TEST_0));
	assert(strcmp(STR_TEST_0, b.chars()) == 0);
}

void test_split_by_char_n(const int x, const char* input, const char split
	, const int trim, const char* exp, const int exp_size)
{
	in.copy_raw(input);
	lbuf = in.split_by_char(split, trim);
	printf("    test_split_by_char %d got: %s\n", x, lbuf->chars());
	printf("    test_split_by_char %d exp: %s\n", x, exp);
	assert(lbuf->size() == exp_size);
	assert(strcmp(exp, lbuf->chars()) == 0);
	delete lbuf;
}

void test_split_by_char()
{
	in.reset();
	lbuf = in.split_by_char(',');
	assert(lbuf == NULL);

	test_split_by_char_n(test_n, TEST_SPLIT_BY_00_IN, ',', 0
		, TEST_SPLIT_BY_00_OUT, 1);

	test_split_by_char_n(test_n++, TEST_SPLIT_BY_01_IN, ':', 0
		, TEST_SPLIT_BY_01_OUT, 2);

	test_split_by_char_n(test_n++, TEST_SPLIT_BY_02_IN, ',', 0
		, TEST_SPLIT_BY_02_OUT, 4);

	test_split_by_char_n(test_n++, TEST_SPLIT_BY_02_IN, ',', 1
		, TEST_SPLIT_BY_02_OUT_TRIM, 3);

	test_split_by_char_n(test_n++, TEST_SPLIT_BY_03_IN, ',', 0
		, TEST_SPLIT_BY_03_OUT, 6);

	test_split_by_char_n(test_n++, TEST_SPLIT_BY_03_IN, ',', 1
		, TEST_SPLIT_BY_03_OUT_TRIM, 3);
}

void test_split_by_whitespace_n(const int x, const char* input
	, const char* exp, const int exp_size)
{
	in.copy_raw(input);
	lbuf = in.split_by_whitespace();
	printf("    test_split_by_whitespace %d got: %s\n", x, lbuf->chars());
	printf("    test_split_by_whitespace %d exp: %s\n", x, exp);
	assert(lbuf->size() == exp_size);
	assert(strcmp(exp, lbuf->chars()) == 0);
	delete lbuf;
}

void test_split_by_whitespace()
{
	in.reset();

	lbuf = in.split_by_whitespace();
	assert(lbuf == NULL);

	test_split_by_whitespace_n(0, TEST_SPLIT_BY_00_IN
		, TEST_SPLIT_BY_WS_00_OUT, 1);
	test_split_by_whitespace_n(0, TEST_SPLIT_BY_02_IN
		, TEST_SPLIT_BY_WS_02_OUT, 7);
	test_split_by_whitespace_n(0, TEST_SPLIT_BY_03_IN
		, TEST_SPLIT_BY_WS_03_OUT, 8);
	test_split_by_whitespace_n(0, TEST_04_IN, TEST_04_WS_OUT, 5);
}

int main()
{
	Buffer a;

	assert(strcmp("Buffer", a.__cname) == 0);
	assert(strcmp("Object", a.Object::__cname) == 0);

	assert(a._l == Buffer::DFLT_SIZE);
	assert(a._i == 0);
	assert(strcmp("", a.chars()) == 0);

	test_copy_raw();

	test_split_by_char();

	test_split_by_whitespace();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
