//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../Rowset.hh"

#define	EXP_ROW_0

#define	EXP_0	SB ( \
		SB( \
			V_STR(STR_TEST_0) SEP_ITEM \
			V_STR(STR_TEST_1) SEP_ITEM \
			V_STR(STR_TEST_2) \
		) \
	)

#define	EXP_1	SB ( \
		SB( \
			V_STR(STR_TEST_0) SEP_ITEM \
			V_STR(STR_TEST_1) SEP_ITEM \
			V_STR(STR_TEST_2) \
		) SEP_LINE \
		SB( \
			V_STR(STR_TEST_2) SEP_ITEM \
			V_STR(STR_TEST_1) SEP_ITEM \
			V_STR(STR_TEST_0) \
		) \
	)

using vos::Buffer;
using vos::List;
using vos::Rowset;

Rowset rowset;

List* create_row_0()
{
	Buffer* b = NULL;
	List* row = new List();

	b = new Buffer();
	b->copy_raw(STR_TEST_0);
	row->push_tail(b);

	b = new Buffer();
	b->copy_raw(STR_TEST_1);
	row->push_tail(b);

	b = new Buffer();
	b->copy_raw(STR_TEST_2);
	row->push_tail(b);

	return row;
}

List* create_row_1()
{
	Buffer* b = NULL;
	List* row = new List();

	b = new Buffer();
	b->copy_raw(STR_TEST_2);
	row->push_tail(b);

	b = new Buffer();
	b->copy_raw(STR_TEST_1);
	row->push_tail(b);

	b = new Buffer();
	b->copy_raw(STR_TEST_0);
	row->push_tail(b);

	return row;
}

int main()
{
	List* row;

	row = create_row_0();
	rowset.push_tail(row);

	printf("exp: '%s'\n", EXP_0);
	printf("got: '%s'\n", rowset.chars());

	assert(strcmp(EXP_0, rowset.chars()) == 0);

	row = create_row_1();
	rowset.push_tail(row);

	printf("exp: '%s'\n", EXP_1);
	printf("got: '%s'\n", rowset.chars());

	assert(strcmp(EXP_1, rowset.chars()) == 0);
}
