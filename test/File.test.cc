//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include "test.hh"
#include "../File.hh"

using vos::File;

Test T("File");

void test_file_open_mode()
{
	T.out("FILE_OPEN_NO: %d\n", vos::FILE_OPEN_NO);
	T.out("FILE_OPEN_RO: %d\n", vos::FILE_OPEN_RO);
	T.out("FILE_OPEN_WO: %d\n", vos::FILE_OPEN_WO);
	T.out("FILE_OPEN_WOCA: %d\n", vos::FILE_OPEN_WOCA);
	T.out("FILE_OPEN_WOCT: %d\n", vos::FILE_OPEN_WOCT);
	T.out("FILE_OPEN_WOCX: %d\n", vos::FILE_OPEN_WOCX);
	T.out("FILE_OPEN_RWCA: %d\n", vos::FILE_OPEN_RWCA);
	T.out("FILE_OPEN_RWCT: %d\n", vos::FILE_OPEN_RWCT);
	T.out("FILE_OPEN_RWCX: %d\n", vos::FILE_OPEN_RWCX);
	T.out("FILE_OPEN_SOCK: %d\n", vos::FILE_OPEN_SOCK);
}

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
		T.start("GET_SIZE()", tests[x].desc);

		off_t got = 0;
		err = File::GET_SIZE(tests[x].file, &got);

		T.expect_error(tests[x].exp_err, err);
		T.expect_signed(tests[x].exp_size, got);

		T.ok();
	}
}

void test_IS_EXIST()
{
	File::TOUCH("FILE_NOPERM", vos::FILE_OPEN_RWCT, 0);

	struct {
		const char*  desc;
		const char*  file;
		const int    access_mode;
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
		T.start("IS_EXIST()", tests[x].desc);

		int got = File::IS_EXIST(tests[x].file, tests[x].access_mode);

		T.expect_signed(tests[x].exp_res, got);

		T.ok();
	}

	unlink("FILE_NOPERM");
}

void test_BASENAME()
{
	struct {
		const char* desc;
		const char* path;
		Error       exp_err;
		const char* exp;
	} tests[] = {{
		"With NULL path"
	,	NULL
	,	NULL
	,	"."
	},{
		"With single char: /"
	,	"/"
	,	NULL
	,	"/"
	},{
		"With single char: a"
	,	"a"
	,	NULL
	,	"a"
	},{
		"With relative path: ./"
	,	"./"
	,	NULL
	,	"."
	},{
		"With relative path: ../"
	,	"../"
	,	NULL
	,	".."
	},{
		"With relative path: ../../.."
	,	"../../.."
	,	NULL
	,	".."
	},{
		"With relative path: ../a///"
	,	"../a///"
	,	NULL
	,	"a"
	},{
		"With relative path: ../a///b/"
	,	"../a///b/"
	,	NULL
	,	"b"
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;
	Buffer name;

	for (int x = 0; x < tests_len; x++) {
		T.start("BASENAME()", tests[x].desc);

		err = File::BASENAME(&name, tests[x].path);

		T.expect_error(tests[x].exp_err, err);
		T.expect_string(tests[x].exp, name.v());

		T.ok();
	}
}

void test_COPY()
{
	unlink("LICENSE.copy");

	struct {
		const char* desc;
		const char* src;
		const char* dst;
		Error       exp_err;
		size_t      exp_size;
	} tests[] = {{
		"With NULL src and dest"
	,	NULL
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	},{
		"With NULL src"
	,	NULL
	,	"copy"
	,	vos::ErrFileNameEmpty
	,	0
	},{
		"With NULL dest"
	,	"asdf"
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	},{
		"With not existen source"
	,	"asdf"
	,	"copy"
	,	vos::ErrFileNotFound
	,	0
	},{
		"With success"
	,	"../LICENSE"
	,	"LICENSE.copy"
	,	NULL
	,	1949
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("COPY()", tests[x].desc);

		err = File::COPY(tests[x].src, tests[x].dst);

		if (err != NULL) {
			T.expect_error(tests[x].exp_err, err);
		} else {
			off_t size;

			File::GET_SIZE(tests[x].dst, &size);

			T.expect_signed(tests[x].exp_size, size);
		}

		T.ok();
	}

	unlink("LICENSE.copy");
}

void test_TOUCH()
{
	unlink("test.FILE_TOUCH");

	struct {
		const char* desc;
		const char* filename;
		Error       exp_err;
		off_t       exp_size;
	} tests[] = {{
		"With NULL parameter"
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	},{
		"With file exist"
	,	"../LICENSE"
	,	NULL
	,	1949
	},{
		"With non-existent file"
	,	"test.FILE_TOUCH"
	,	NULL
	,	0
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("TOUCH()", tests[x].desc);

		err = File::TOUCH(tests[x].filename);

		T.expect_error(tests[x].exp_err, err);

		if (err == NULL) {
			off_t size;

			File::GET_SIZE(tests[x].filename, &size);

			T.expect_signed(tests[x].exp_size, size);
		}

		T.ok();
	}

	unlink("test.FILE_TOUCH");
}

void test_WRITE_PID()
{
	unlink("test.WRITE_PID");

	struct {
		const char* desc;
		const char* filename;
		Error       exp_err;
		off_t       exp_size;
	} tests[] = {{
		"With NULL parameter"
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	},{

		"With success"
	,	"test.WRITE_PID"
	,	NULL
	,	1
	},{
		"With file exist"
	,	"test.WRITE_PID"
	,	vos::ErrFileExist
	,	0
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("WRITE_PID()", tests[x].desc);

		err = File::WRITE_PID(tests[x].filename);

		T.expect_error(tests[x].exp_err, err);

		if (err == NULL) {
			off_t size;

			File::GET_SIZE(tests[x].filename, &size);

			T.expect_signed(size, tests[x].exp_size
				, vos::IS_GREATER_THAN);
		}

		T.ok();
	}

	unlink("test.WRITE_PID");
}

void test_open()
{
	struct {
		const char* desc;
		const char* path;
		Error       exp_err;
		const off_t exp_size;
		const int   exp_status;
	} tests[] = {{
		"With open(NULL)"
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	,	vos::FILE_OPEN_NO
	},{
		"With open(newfile)"
	,	"newfile"
	,	NULL
	,	0
	,	O_RDWR
	},{
		"With open(../LICENSE)"
	,	"../LICENSE"
	,	NULL
	,	1949
	,	O_RDWR
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("open()", tests[x].desc);

		File f;

		err = f.open(tests[x].path);

		T.expect_error(tests[x].exp_err, err);

		T.expect_signed(tests[x].exp_status, f.status());

		if (err == NULL) {
			T.expect_string(tests[x].path, f.name());

			T.expect_unsigned(tests[x].exp_size, f.size());

			T.expect_signed(f.fd(), 0, vos::IS_GREATER_THAN);
		}

		T.ok();
	}

	unlink("newfile");
}

void test_open_ro()
{
	struct {
		const char* desc;
		const char* path;
		Error       exp_err;
		const off_t exp_size;
		const int   exp_status;
	} tests[] = {{
		"With open(NULL)"
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	,	vos::FILE_OPEN_NO
	},{
		"With open(newfile)"
	,	"newfile"
	,	vos::ErrFileNotFound
	,	0
	,	vos::FILE_OPEN_NO
	},{
		"With open(../LICENSE)"
	,	"../LICENSE"
	,	NULL
	,	1949
	,	O_RDONLY
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;
	Buffer b("test write buffer");

	for (int x = 0; x < tests_len; x++) {
		T.start("open_ro()", tests[x].desc);

		File f;

		err = f.open_ro(tests[x].path);

		T.expect_error(tests[x].exp_err, err);

		T.expect_signed(tests[x].exp_status, f.status());

		if (err == NULL) {
			T.expect_string(tests[x].path, f.name());

			T.expect_unsigned(tests[x].exp_size, f.size());

			T.expect_signed(f.fd(), 0, vos::IS_GREATER_THAN);

			// Test write
			err = f.write(&b);
			T.expect_error(vos::ErrFileReadOnly, err);

			err = f.write_raw("test write raw");
			T.expect_error(vos::ErrFileReadOnly, err);

			err = f.writef("test write raw");
			T.expect_error(vos::ErrFileReadOnly, err);

			err = f.writec('x');
			T.expect_error(vos::ErrFileReadOnly, err);
		}

		T.ok();
	}
}

void test_open_wo()
{
	struct {
		const char* desc;
		const char* path;
		Error       exp_err;
		const off_t exp_size;
		const int   exp_status;
	} tests[] = {{
		"With open(NULL)"
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	,	vos::FILE_OPEN_NO
	},{
		"With open(newfile)"
	,	"newfile"
	,	NULL
	,	0
	,	O_WRONLY
	},{
		"With open(../LICENSE)"
	,	"../LICENSE"
	,	NULL
	,	1949
	,	O_WRONLY
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("open_wo()", tests[x].desc);

		File f;

		err = f.open_wo(tests[x].path);

		T.expect_error(tests[x].exp_err, err);

		T.expect_signed(tests[x].exp_status, f.status());

		if (err == NULL) {
			T.expect_string(tests[x].path, f.name());

			T.expect_unsigned(tests[x].exp_size, f.size());

			T.expect_signed(f.fd(), 0, vos::IS_GREATER_THAN);

			// Test read.
			err = f.read();
			T.expect_error(NULL, err);

			err = f.readn(1);
			T.expect_error(NULL, err);
		}

		T.ok();
	}

	unlink("newfile");
}

void test_open_wx()
{
	struct {
		const char* desc;
		const char* path;
		Error       exp_err;
		const off_t exp_size;
		const int   exp_status;
	} tests[] = {{
		"With open(NULL)"
	,	NULL
	,	vos::ErrFileNameEmpty
	,	0
	,	vos::FILE_OPEN_NO
	},{
		"With open(../LICENSE)"
	,	"../LICENSE"
	,	vos::ErrFileExist
	,	0
	,	-1
	},{
		"With open(newfile)"
	,	"newfile"
	,	NULL
	,	0
	,	O_WRONLY
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("open_wx()", tests[x].desc);

		File f;

		err = f.open_wx(tests[x].path);

		T.expect_error(tests[x].exp_err, err);

		T.expect_signed(tests[x].exp_status, f.status());

		if (err == NULL) {
			T.expect_string(tests[x].path, f.name());

			T.expect_unsigned(tests[x].exp_size, f.size());

			T.expect_signed(f.fd(), 0, vos::IS_GREATER_THAN);

			// Test read.
			err = f.read();
			T.expect_error(NULL, err);

			err = f.readn(1);
			T.expect_error(NULL, err);

			unlink(tests[x].path);
		}

		T.ok();
	}
}

void test_truncate_ro()
{
	struct {
		const char* desc;
		const char* path;
		Error       exp_err;
		const off_t exp_size;
		const int   exp_status;
	} tests[] = {{
		"With open_ro(LICENSE.copy)"
	,	"LICENSE.copy"
	,	vos::ErrFileReadOnly
	,	0
	,	vos::FILE_OPEN_NO
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("truncate()", tests[x].desc);

		File::COPY("../LICENSE", tests[x].path);

		File f;

		err = f.open_ro(tests[x].path);

		T.expect_error(NULL, err);

		err = f.truncate();

		T.expect_error(tests[x].exp_err, err);

		unlink(tests[x].path);

		T.ok();
	}
}

void test_truncate_wo()
{
	struct {
		const char* desc;
		const char* path;
		Error       exp_err;
		const off_t exp_size;
		const int   exp_status;
	} tests[] = {{
		"With open_wo(LICENSE.copy)"
	,	"LICENSE.copy"
	,	NULL
	,	0
	,	O_WRONLY
	}};

	int tests_len = ARRAY_SIZE(tests);
	Error err;

	for (int x = 0; x < tests_len; x++) {
		T.start("truncate()", tests[x].desc);

		err = File::COPY("../LICENSE", tests[x].path);
		T.expect_error(NULL, err);

		File f;

		err = f.open_wo(tests[x].path);

		T.expect_error(NULL, err);

		err = f.truncate();

		T.expect_error(tests[x].exp_err, err);
		T.expect_signed(tests[x].exp_size, f.size());
		T.expect_signed(tests[x].exp_status, f.status());

		if (err == NULL) {
			unlink(tests[x].path);
		}

		T.ok();
	}
}

void test_truncate()
{
	test_truncate_ro();
	test_truncate_wo();
}

void test_is_open()
{
	T.start("is_open()");

	File f;

	T.expect_signed(0, f.is_open());

	f.open_ro("../LICENSE");

	T.expect_signed(1, f.is_open());

	T.ok();
}

void test_set_eol()
{
	struct {
		const char* desc;
		enum vos::file_eol_mode mode;
		const char* exp_eol;
	} tests[] = {{
		"With EOL_NIX"
	,	vos::FILE_EOL_NIX
	,	"\n"
	},{
		"With EOL_DOS"
	,	vos::FILE_EOL_DOS
	,	"\r\n"
	},{
		"With unknown mode"
	,	vos::N_FILE_EOL_MODE
	,	"\n"
	}};

	int tests_len = ARRAY_SIZE(tests);
	File f;

	for (int x = 0; x < tests_len; x++) {
		T.start("set_eol()", tests[x].desc);

		f.set_eol(tests[x].mode);

		T.expect_string(tests[x].exp_eol, f.eol());

		T.ok();
	}
}

int main()
{
	test_file_open_mode();

	test_GET_SIZE();
	test_IS_EXIST();
	test_BASENAME();
	test_COPY();
	test_TOUCH();
	test_WRITE_PID();

	test_open();
	test_open_ro();
	test_open_wo();
	test_open_wx();

	test_truncate();

	test_is_open();
	test_set_eol();

	return 0;
}

// vi: ts=8 sw=8 tw=80:
