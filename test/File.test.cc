//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include "test.hh"
#include "../File.hh"

using vos::File;

Test T("File");

void test_GET_SIZE()
{
	struct {
		const char* desc;
		const char* file;
		const Error exp_err;
		const off_t exp_size;
	} const tests[] = {{
		"With NULL path"
	,	NULL
	,	vos::ErrFileNotFound
	,	0
	},{
		"With empty path"
	,	NULL
	,	vos::ErrFileNotFound
	,	0
	},{
		"With no file found"
	,	"asdfasdf"
	,	vos::ErrFileNotFound
	,	0
	},{
		"With empty file"
	,	"FILE_EMPTY"
	,	NULL
	,	0
	},{
		"With file found"
	,	"../LICENSE"
	,	NULL
	,	1949
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start(tests[x].desc);

		off_t got = 0;
		err = File::GET_SIZE(tests[x].file, &got);

		T.expect_signed(1, tests[x].exp_err == err, vos::IS_EQUAL);
		T.expect_signed(tests[x].exp_size, got, vos::IS_EQUAL);

		T.ok();
	}
}

void test_IS_EXIST()
{
	File::TOUCH("FILE_NOPERM", vos::FILE_OPEN_RW, 0);

	struct {
		const char*  desc;
		const char*  file;
		const size_t access_mode;
		const int    exp_res;
	} const tests[] = {{
		"With NULL path"
	,	NULL
	,	O_RDWR
	,	0
	},{
		"With empty path"
	,	NULL
	,	O_RDWR
	,	0
	},{
		"With no file found"
	,	"asdfasdf"
	,	O_RDWR
	,	0
	},{
		"With no read permission"
	,	"FILE_NOPERM"
	,	O_RDONLY
	,	0
	},{
		"With no write permission"
	,	"FILE_NOPERM"
	,	O_WRONLY
	,	0
	},{
		"With no read-write permission"
	,	"FILE_NOPERM"
	,	O_RDWR
	,	0
	},{
		"With read permission"
	,	"FILE_EMPTY"
	,	O_RDONLY
	,	1
	},{
		"With write permission"
	,	"FILE_EMPTY"
	,	O_WRONLY
	,	1
	},{
		"With read-write permission"
	,	"FILE_EMPTY"
	,	O_RDWR
	,	1
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start(tests[x].desc);

		int got = File::IS_EXIST(tests[x].file, tests[x].access_mode);

		T.expect_signed(tests[x].exp_res, got, vos::IS_EQUAL);

		T.ok();
	}

	unlink("FILE_NOPERM");
}

int main()
{
	test_GET_SIZE();
	test_IS_EXIST();

	return 0;
}

// vi: ts=8 sw=8 tw=80:
