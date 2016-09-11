//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include "test.hh"
#include "../List.hh"

using vos::Object;
using vos::List;

#define EXP_PUSH_1	"[\"" STR_TEST_1 "\"]"
#define EXP_PUSH_2	"[\"" STR_TEST_1 "\",\"" STR_TEST_2 "\"]"
#define EXP_PUSH_3	"[\"" STR_TEST_0 "\",\"" STR_TEST_1 "\",\"" STR_TEST_2 "\"]"
#define EXP_POP_1	"[\"" STR_TEST_0 "\",\"" STR_TEST_1 "\"]"
#define EXP_POP_2	"[\"" STR_TEST_1 "\"]"

List list;

Object* item = NULL;
Buffer* b = NULL;

void test_push_tail(const char *str, int exp_n, const char *exp_chars)
{
	b = new Buffer();
	b->copy_raw(str);

	list.push_tail(b);
	assert(list._n == exp_n);

	assert(strcmp(exp_chars, list.chars()) == 0);
}

void test_push_head(const char *str, int exp_n, const char *exp_chars)
{
	b = new Buffer();
	b->copy_raw(str);

	list.push_head(b);
	assert(list._n == exp_n);

	assert(strcmp(exp_chars, list.chars()) == 0);
}

void test_at_0()
{
	assert(list._n == 0);

	item = list.at(0);
	assert(item == NULL);

	item = list.at(1);
	assert(item == NULL);
}

void test_at_1(const char* exp)
{
	assert(list._n == 1);

	// REMEMBER: circular!
	item = list.at(-1);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp) == 0);

	item = list.at(0);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp) == 0);

	// REMEMBER: circular!
	item = list.at(1);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp) == 0);
}

void test_at_2(const char* exp1, const char* exp2)
{
	assert(list._n == 2);

	item = list.at(-2);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp1) == 0);

	item = list.at(-1);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp2) == 0);

	item = list.at(0);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp1) == 0);

	item = list.at(1);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp2) == 0);

	item = list.at(2);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp1) == 0);
}

void test_at_3(const char* exp0, const char* exp1, const char* exp2)
{
	assert(list._n == 3);

	item = list.at(-3);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp0) == 0);

	item = list.at(-2);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp1) == 0);

	item = list.at(-1);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp2) == 0);

	item = list.at(0);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp0) == 0);

	item = list.at(1);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp1) == 0);

	item = list.at(2);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp2) == 0);

	item = list.at(3);
	assert(item != NULL);
	assert(strcmp(item->chars(), exp0) == 0);
}

void test_pop_tail(const char* exp_b, int exp_n, const char* exp_chars)
{
	b = (Buffer*) list.pop_tail();

	assert(list._n == exp_n);

	if (exp_chars) {
		assert(strcmp(exp_chars, list.chars()) == 0);
	}

	assert(strcmp(exp_b, b->chars()) == 0);
	delete b;
}

void test_pop_head(const char* exp_b, int exp_n, const char* exp_chars)
{
	b = (Buffer*) list.pop_head();

	assert(list._n == exp_n);

	if (exp_chars) {
		assert(strcmp(exp_chars, list.chars()) == 0);
	}

	assert(strcmp(exp_b, b->chars()) == 0);
	delete b;
}

int main()
{
	assert(list._n == 0);
	assert(NULL == list.chars());

	test_at_0();

	// TEST PUSH 1
	test_push_tail(STR_TEST_1, 1, EXP_PUSH_1);
	test_at_1(STR_TEST_1);

	// TEST PUSH 2
	test_push_tail(STR_TEST_2, 2, EXP_PUSH_2);
	test_at_2(STR_TEST_1, STR_TEST_2);

	// TEST PUSH 2
	test_push_head(STR_TEST_0, 3, EXP_PUSH_3);
	test_at_3(STR_TEST_0, STR_TEST_1, STR_TEST_2);

	// TEST POP
	test_pop_tail(STR_TEST_2, 2, EXP_POP_1);
	test_at_2(STR_TEST_0, STR_TEST_1);

	test_pop_head(STR_TEST_0, 1, EXP_POP_2);
	test_at_1(STR_TEST_1);

	test_pop_tail(STR_TEST_1, 0, NULL);
	test_at_0();
}

// vi: ts=8 sw=8 tw=78:
