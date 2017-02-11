//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../List.hh"
#include "../DSVRecordMD.hh"

using vos::DSVRecordMD;
using vos::List;

#define TEST_MD_0		\
":name:::'|',"			\
"':':stat:::'|':NUMBER,"	\
":ttl:':'::'|':DATE,"		\
":answer::3:'|':BLOB"

void test_INIT()
{
	int x = 0;
	List* list_md = NULL;
	DSVRecordMD* md = NULL;
	DSVRecordMD exp[4];

	exp[0]._name.copy_raw("name");
	exp[0]._sep = '|';

	exp[1]._name.copy_raw("stat");
	exp[1]._type = vos::RMD_T_NUMBER;
	exp[1]._left_q = ':';
	exp[1]._sep = '|';

	exp[2]._name.copy_raw("ttl");
	exp[2]._type = vos::RMD_T_DATE;
	exp[2]._right_q = ':';
	exp[2]._sep = '|';

	exp[3]._name.copy_raw("answer");
	exp[3]._type = vos::RMD_T_BLOB;
	exp[3]._start_p = 3;
	exp[3]._sep = '|';

	list_md = DSVRecordMD::INIT(TEST_MD_0);

	assert(list_md != NULL);
	assert(list_md->size() == 4);

	for (x = 0; x < list_md->size(); x++) {
		md = (DSVRecordMD*) list_md->at(x);
		expectString(exp[x].chars(), md->chars(), 0);
	}
}

int main()
{
	test_INIT();
	return 0;
}
