//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../List.hh"

using vos::List;

#define TEST_SPLIT_BY_00_IN "127.0.0.1:53"
#define TEST_SPLIT_BY_00_OUT "[\"127.0.0.1:53\"]"

#define TEST_SPLIT_BY_01_IN "127.0.0.1:53"
#define TEST_SPLIT_BY_01_OUT "[\"127.0.0.1\",\"53\"]"

#define TEST_SPLIT_BY_02_IN "a b, c d e, fg hi, "
#define TEST_SPLIT_BY_02_OUT "[\"a b\",\" c d e\",\" fg hi\",\" \"]"
#define TEST_SPLIT_BY_02_OUT_TRIM "[\"a b\",\"c d e\",\"fg hi\"]"

#define TEST_SPLIT_BY_03_IN "a b,, c d e, , fg hi, "
#define TEST_SPLIT_BY_03_OUT "[\"a b\",\"\",\" c d e\",\" \",\" fg hi\",\" \"]"
#define TEST_SPLIT_BY_03_OUT_TRIM "[\"a b\",\"c d e\",\"fg hi\"]"

void test_split_by_char()
{
	Buffer in;
	List* lbuf;

	lbuf = in.split_by_char(',');
	assert(lbuf == NULL);

	in.copy_raw(TEST_SPLIT_BY_00_IN);
	lbuf = in.split_by_char(',');
	printf("    test_split_by_char 0: %s\n", lbuf->chars());
	assert(lbuf->size() == 1);
	assert(strcmp(TEST_SPLIT_BY_00_OUT, lbuf->chars()) == 0);
	delete lbuf;

	in.copy_raw(TEST_SPLIT_BY_01_IN);
	lbuf = in.split_by_char(':');
	printf("    test_split_by_char 1: %s\n", lbuf->chars());
	assert(lbuf->size() == 2);
	assert(strcmp(TEST_SPLIT_BY_01_OUT, lbuf->chars()) == 0);
	delete lbuf;

	in.copy_raw(TEST_SPLIT_BY_02_IN);

	lbuf = in.split_by_char(',');
	printf("    test_split_by_char 2: %s\n", lbuf->chars());
	assert(lbuf->size() == 4);
	assert(strcmp(TEST_SPLIT_BY_02_OUT, lbuf->chars()) == 0);
	delete lbuf;

	lbuf = in.split_by_char(',', 1);
	printf("    test_split_by_char 2 - trim: %s\n", lbuf->chars());
	assert(lbuf->size() == 3);
	assert(strcmp(TEST_SPLIT_BY_02_OUT_TRIM, lbuf->chars()) == 0);
	delete lbuf;

	in.copy_raw(TEST_SPLIT_BY_03_IN);

	lbuf = in.split_by_char(',');
	printf("    test_split_by_char 3: %s\n", lbuf->chars());
	assert(lbuf->size() == 6);
	assert(strcmp(TEST_SPLIT_BY_03_OUT, lbuf->chars()) == 0);
	delete lbuf;

	lbuf = in.split_by_char(',', 1);
	printf("    test_split_by_char 3 - trim: %s\n", lbuf->chars());
	assert(lbuf->size() == 3);
	assert(strcmp(TEST_SPLIT_BY_03_OUT_TRIM, lbuf->chars()) == 0);
	delete lbuf;
}

int main ()
{
	Buffer a;

	assert(strcmp("Buffer", a.__cname) == 0);
	assert(strcmp("Object", a.Object::__cname) == 0);

	assert(a._l == Buffer::DFLT_SIZE);
	assert(a._i == 0);
	assert(strcmp("", a.chars()) == 0);

	a.copy_raw(STR_TEST_0);

	assert(a._i == strlen(STR_TEST_0));
	assert(strcmp(STR_TEST_0, a.chars()) == 0);

	test_split_by_char();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
