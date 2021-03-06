//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include "test.hh"
#include "../List.hh"

using vos::Object;
using vos::List;
using vos::BNode;
using vos::Test;

Test T("List");

#define EXP_0		SB(V_STR(STR_TEST_0))
#define EXP_1		SB(V_STR(STR_TEST_1))
#define EXP_2		SB(V_STR(STR_TEST_2))
#define EXP_1_2		SB( \
				V_STR(STR_TEST_1) SEP_ITEM \
				V_STR(STR_TEST_2) \
			)
#define EXP_0_1_2	SB( \
				V_STR(STR_TEST_0) SEP_ITEM \
				V_STR(STR_TEST_1) SEP_ITEM \
				V_STR(STR_TEST_2) \
			)
#define EXP_0_2		SB( \
				V_STR(STR_TEST_0) SEP_ITEM \
				V_STR(STR_TEST_2) \
			)
#define EXP_0_1		SB( \
				V_STR(STR_TEST_0) SEP_ITEM \
				V_STR(STR_TEST_1) \
			)

#define	EXP_0_0		SB( \
				V_STR(STR_TEST_0) SEP_ITEM \
				V_STR(STR_TEST_0) \
			)
#define	EXP_0_0_0	SB( \
				V_STR(STR_TEST_0) SEP_ITEM \
				V_STR(STR_TEST_0) SEP_ITEM \
				V_STR(STR_TEST_0) \
			)
#define	EXP_2_1_0	SB( \
				V_STR(STR_TEST_2) SEP_ITEM \
				V_STR(STR_TEST_1) SEP_ITEM \
				V_STR(STR_TEST_0) \
			)


List list;
int s;
Object* item = NULL;
Buffer* b = NULL;

Buffer* test_push_tail(const char* str, int exp_n, const char* exp_chars)
{
	b = new Buffer();
	b->copy_raw(str);

	list.push_tail(b);
	T.expect_signed(list.size(), exp_n);

	T.expect_string(exp_chars, list.chars());

	return b;
}

void test_push_head(const char* str, int exp_n, const char* exp_chars)
{
	b = new Buffer();
	b->copy_raw(str);

	list.push_head(b);
	T.expect_signed(list.size(), exp_n);

	T.expect_string(exp_chars, list.chars());
}

void test_at_0()
{
	T.expect_signed(list.size(), 0);

	item = list.at(0);
	T.expect_ptr(item, NULL);

	item = list.at(1);
	T.expect_ptr(item, NULL);
}

void test_at_1(const char* exp)
{
	T.expect_signed(list.size(), 1);

	// REMEMBER: circular!
	item = list.at(-1);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp);

	item = list.at(0);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp);

	// REMEMBER: circular!
	item = list.at(1);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp);
}

void test_at_2(const char* exp1, const char* exp2)
{
	T.expect_signed(list.size(), 2);

	item = list.at(-2);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp1);

	item = list.at(-1);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp2);

	item = list.at(0);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp1);

	item = list.at(1);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp2);

	item = list.at(2);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp1);
}

void test_at_3(const char* exp0, const char* exp1, const char* exp2)
{
	T.expect_signed(list.size(), 3);

	item = list.at(-3);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp0);

	item = list.at(-2);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp1);

	item = list.at(-1);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp2);

	item = list.at(0);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp0);

	item = list.at(1);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp1);

	item = list.at(2);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp2);

	item = list.at(3);
	T.expect_ptr(item, NULL, vos::IS_NOT_EQUAL);
	T.expect_string(item->chars(), exp0);
}

void test_pop_tail(const char* exp_b, int exp_n, const char* exp_chars)
{
	b = (Buffer*) list.pop_tail();

	T.expect_signed(list.size(), exp_n);

	if (exp_chars) {
		T.expect_string(exp_chars, list.chars());
	}

	T.expect_string(exp_b, b->chars());
	delete b;
}

void test_pop_head(const char* exp_b, int exp_n, const char* exp_chars)
{
	b = (Buffer*) list.pop_head();

	T.expect_signed(list.size(), exp_n);

	if (exp_chars) {
		T.expect_string(exp_chars, list.chars());
	}

	T.expect_string(exp_b, b->chars());
	delete b;
}

void test_remove()
{
	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_0);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_0);

	// (C0) Item not found, parameter is empty.
	s = list.remove(NULL);
	T.expect_signed(s, 1);

	// (C1) Item not found, list is empty.
	s = list.remove(b0);
	T.expect_signed(s, 1);

	// (C1) Item not found, when searching different item.
	list.push_tail(b0);
	T.expect_signed(list.size(), 1);

	s = list.remove(b1);
	T.expect_signed(s, 1);
	T.expect_signed(list.size(), 1);

	// (C2) Item found, but its the only item in the list
	s = list.remove(b0);
	T.expect_signed(s, 0);
	T.expect_signed(list.size(), 0);

	// (C1) Item not found, because its already removed.
	s = list.remove(b0);
	T.expect_signed(s, 1);

	// (C3) Item found, and its also the head
	list.push_tail(b0);
	list.push_tail(b1);
	list.push_tail(b2);
	T.expect_signed(list.size(), 3);

	s = list.remove(b0);
	T.expect_signed(s, 0);
	T.expect_signed(list.size(), 2);
	T.expect_string(list.chars(), EXP_0_0);

	// (C1) Item not found
	s = list.remove(b0);
	T.expect_signed(s, 1);

	// (C4) Item found, and its also the tail
	s = list.remove(b2);
	T.expect_signed(s, 0);
	T.expect_signed(list.size(), 1);
	T.expect_string(list.chars(), EXP_0);

	// (C1) Item not found
	s = list.remove(b0);
	T.expect_signed(s, 1);
	s = list.remove(b2);
	T.expect_signed(s, 1);

	// (C2) Item found, but its the only item in the list
	s = list.remove(b1);
	T.expect_signed(s, 0);
	T.expect_signed(list.size(), 0);

	// (C1) Item not found
	s = list.remove(b0);
	T.expect_signed(s, 1);
	s = list.remove(b1);
	T.expect_signed(s, 1);
	s = list.remove(b2);
	T.expect_signed(s, 1);

	// (C5) Item found, and its in the middle
	list.push_tail(b0);
	list.push_tail(b1);
	list.push_tail(b2);
	T.expect_signed(list.size(), 3);

	s = list.remove(b1);
	T.expect_signed(s, 0);
	T.expect_signed(list.size(), 2);

	s = list.remove(b0);
	T.expect_signed(s, 0);
	T.expect_signed(list.size(), 1);

	s = list.remove(b2);
	T.expect_signed(s, 0);
	T.expect_signed(list.size(), 0);

	// the end.
	delete b2;
	delete b1;
	delete b0;
}

void test_sort()
{
	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_1);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_2);

	T.expect_signed(list.size(), 0);

	list.push_tail(b2);
	list.sort(Buffer::CMP);
	T.expect_signed(list.size(), 1);
	T.expect_string(EXP_2, list.chars());

	list.push_tail(b1);
	list.sort(Buffer::CMP);
	T.expect_signed(list.size(), 2);
	T.expect_string(EXP_1_2, list.chars());

	list.push_tail(b0);
	list.sort(Buffer::CMP);
	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	list.reset();
}

void test_search()
{
	BNode* node_found;
	Buffer* bdel;

	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_1);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_2);

	T.expect_signed(list.size(), 0);

	node_found = list.node_search(b1, NULL);
	T.expect_ptr(node_found, NULL);

	list.push_tail(b0);
	list.push_tail(b1);
	list.push_tail(b2);

	node_found = list.node_search(b1, NULL);
	T.expect_ptr(node_found, NULL, vos::IS_NOT_EQUAL);
	T.expect_ptr(node_found, list.node_at(1));

	node_found = list.node_search(b2, Buffer::CMP);
	T.expect_ptr(node_found, NULL, vos::IS_NOT_EQUAL);
	T.expect_ptr(node_found, list.node_at(2));

	bdel = (Buffer*) list.node_remove_unsafe(node_found);

	T.expect_signed(list.size(), 2);
	T.expect_ptr(bdel, b2);

	node_found = list.node_search(b2, Buffer::CMP);
	T.expect_ptr(node_found, NULL);

	list.reset();

	delete b2;
}

void test_push_head_sorted_asc()
{
	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_1);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_2);

	T.expect_signed(list.size(), 0);

	list.push_head_sorted(b2);
	list.push_head_sorted(b1);
	list.push_head_sorted(b0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_head_sorted(b0);
	list.push_head_sorted(b1);
	list.push_head_sorted(b2);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_head_sorted(b2);
	list.push_head_sorted(b0);
	list.push_head_sorted(b1);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	list.reset();
}

void test_push_head_sorted_desc()
{
	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_1);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_2);

	T.expect_signed(list.size(), 0);

	list.push_head_sorted(b2, 0);
	list.push_head_sorted(b1, 0);
	list.push_head_sorted(b0, 0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_2_1_0, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_head_sorted(b0, 0);
	list.push_head_sorted(b1, 0);
	list.push_head_sorted(b2, 0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_2_1_0, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_head_sorted(b0, 0);
	list.push_head_sorted(b2, 0);
	list.push_head_sorted(b1, 0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_2_1_0, list.chars());

	list.reset();
}

void test_push_tail_sorted_asc()
{
	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_1);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_2);

	T.expect_signed(list.size(), 0);

	list.push_tail_sorted(b2);
	list.push_tail_sorted(b1);
	list.push_tail_sorted(b0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_tail_sorted(b0);
	list.push_tail_sorted(b1);
	list.push_tail_sorted(b2);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_tail_sorted(b0);
	list.push_tail_sorted(b2);
	list.push_tail_sorted(b1);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	list.reset();
}

void test_push_tail_sorted_desc()
{
	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_1);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_2);

	T.expect_signed(list.size(), 0);

	list.push_tail_sorted(b0, 0);
	list.push_tail_sorted(b1, 0);
	list.push_tail_sorted(b2, 0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_2_1_0, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_tail_sorted(b2, 0);
	list.push_tail_sorted(b1, 0);
	list.push_tail_sorted(b0, 0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_2_1_0, list.chars());

	list.pop_tail();
	list.pop_tail();
	list.pop_tail();
	T.expect_signed(list.size(), 0);

	list.push_tail_sorted(b0, 0);
	list.push_tail_sorted(b2, 0);
	list.push_tail_sorted(b1, 0);

	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_2_1_0, list.chars());

	list.reset();
}

void test_detach()
{
	BNode* node;

	Buffer* b0 = new Buffer();
	b0->copy_raw(STR_TEST_0);

	Buffer* b1 = new Buffer();
	b1->copy_raw(STR_TEST_1);

	Buffer* b2 = new Buffer();
	b2->copy_raw(STR_TEST_2);

	T.expect_signed(list.size(), 0);

	list.push_tail(b0);
	list.push_tail(b1);
	list.push_tail(b2);
	T.expect_signed(list.size(), 3);

	node = list.node_at(0);
	list.detach(node);
	T.expect_signed(list.size(), 2);
	T.expect_string(EXP_1_2, list.chars());

	node->set_content(NULL);
	delete node;

	list.push_head(b0);
	T.expect_signed(list.size(), 3);
	T.expect_string(EXP_0_1_2, list.chars());

	node = list.node_at(1);
	list.detach(node);
	T.expect_signed(list.size(), 2);
	T.expect_string(EXP_0_2, list.chars());

	delete node;

	node = list.node_at(0);
	list.detach(node);
	T.expect_signed(list.size(), 1);
	T.expect_string(EXP_2, list.chars());

	delete node;

	node = list.node_at(0);
	list.detach(node);
	T.expect_signed(list.size(), 0);
	T.expect_ptr(list.chars(), 0);

	delete node;
}

int main()
{
	T.expect_signed(list.size(), 0);
	T.expect_ptr(NULL, list.chars());

	test_at_0();

	// TEST PUSH 1
	test_push_tail(STR_TEST_1, 1, EXP_1);
	test_at_1(STR_TEST_1);

	// TEST PUSH 2
	test_push_tail(STR_TEST_2, 2, EXP_1_2);
	test_at_2(STR_TEST_1, STR_TEST_2);

	// TEST PUSH 2
	test_push_head(STR_TEST_0, 3, EXP_0_1_2);
	test_at_3(STR_TEST_0, STR_TEST_1, STR_TEST_2);

	// TEST POP
	test_pop_tail(STR_TEST_2, 2, EXP_0_1);
	test_at_2(STR_TEST_0, STR_TEST_1);

	test_pop_head(STR_TEST_0, 1, EXP_1);
	test_at_1(STR_TEST_1);

	test_pop_tail(STR_TEST_1, 0, NULL);
	test_at_0();

	test_remove();

	test_sort();

	test_search();

	test_push_head_sorted_asc();
	test_push_head_sorted_desc();

	test_push_tail_sorted_asc();
	test_push_tail_sorted_desc();

	test_detach();
}

// vi: ts=8 sw=8 tw=78:
