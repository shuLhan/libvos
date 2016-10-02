//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../FTPD.hh"

using vos::FTPD;

#define	TEST_USER_0	"user00"
#define	TEST_USER_1	"user01"
#define	TEST_USER_2	"user02"
#define	TEST_USER_3	"user0"

int s;
FTPD ftpd;

void test_user_add()
{
	s = ftpd.user_is_exist(TEST_USER_0);
	assert(s == 0);

	ftpd.user_add(TEST_USER_0, TEST_USER_0);
	ftpd.user_add(TEST_USER_1, TEST_USER_1);
	ftpd.user_add(TEST_USER_2, TEST_USER_2);

	// Test if user is exist.
	s = ftpd.user_is_exist(TEST_USER_0);
	assert(s == 1);
	s = ftpd.user_is_exist(TEST_USER_1);
	assert(s == 1);
	s = ftpd.user_is_exist(TEST_USER_2);
	assert(s == 1);
	s = ftpd.user_is_exist(TEST_USER_0, TEST_USER_0);
	assert(s == 1);
	s = ftpd.user_is_exist(TEST_USER_1, TEST_USER_1);
	assert(s == 1);
	s = ftpd.user_is_exist(TEST_USER_2, TEST_USER_2);
	assert(s == 1);

	// Test if user is not exist.
	s = ftpd.user_is_exist(TEST_USER_3);
	assert(s == 0);
	s = ftpd.user_is_exist(TEST_USER_0, TEST_USER_1);
	assert(s == 0);
	s = ftpd.user_is_exist(TEST_USER_0, TEST_USER_2);
	assert(s == 0);
	s = ftpd.user_is_exist(TEST_USER_1, TEST_USER_0);
	assert(s == 0);
	s = ftpd.user_is_exist(TEST_USER_1, TEST_USER_2);
	assert(s == 0);
	s = ftpd.user_is_exist(TEST_USER_2, TEST_USER_0);
	assert(s == 0);
	s = ftpd.user_is_exist(TEST_USER_2, TEST_USER_1);
	assert(s == 0);
}

int main()
{
	test_user_add();
	return 0;
}
