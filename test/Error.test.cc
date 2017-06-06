/**
 * Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "test.hh"

Test T("Error");

void test_equality()
{
	const char* err_data = "This is it";
	const size_t err_data_len = strlen(err_data);

	struct {
		const char* desc;
		Error       in;
		int         exp;
		const Error exp_err;
		void*       exp_data;
		size_t      exp_data_len;
	} const tests[] = {{
		"With NULL"
	,	NULL
	,	1
	,	NULL
	,	NULL
	,	0
	},{
		"With ErrOutOfMemory"
	,	vos::ErrOutOfMemory
	,	1
	,	vos::ErrOutOfMemory
	,	NULL
	,	0
	},{
		"With ErrUnknown.with(data)"
	,	vos::ErrUnknown.with(err_data, err_data_len)
	,	1
	,	vos::ErrUnknown
	,	(char*) err_data
	,	err_data_len
	}};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("operator==", tests[x].desc);

		T.expect_signed(tests[x].exp, tests[x].in == tests[x].exp_err
			, vos::IS_EQUAL);

		if (tests[x].in != NULL && tests[x].exp_data) {
			void* got_data = tests[x].in.data();

			T.expect_mem(tests[x].exp_data, got_data
				, tests[x].exp_data_len, vos::IS_EQUAL);
		}

		T.ok();
	}
}

void test_operator_assign()
{
	Error err = vos::ErrUnknown;

	T.start("operator=");

	T.out("\n");
	T.out(">>> ErrUnknown       : %p\n", &vos::ErrUnknown);
	T.out(">>> err              : %p\n", &err);

	T.out(">>> ErrUnknown.__str : %p\n", vos::ErrUnknown.chars());
	T.out(">>> err.__str        : %p\n", err.chars());

	T.expect_signed(1, err == vos::ErrUnknown, vos::IS_EQUAL);
	T.expect_ptr(vos::ErrUnknown.chars(), err.chars(), vos::IS_EQUAL);

	T.ok();
}

void test_with()
{
	int data = 10;
	size_t data_len = sizeof(int);

	T.start("with(data,len)");

	T.out("\n\n");
	T.out(">>> ErrUnknown       : %p\n", &vos::ErrUnknown);
	T.out(">>> ErrUnknown.__str : %p\n", vos::ErrUnknown.chars());
	T.out(">>> ErrUnknown.data  : %p\n\n", vos::ErrUnknown.data());

	Error err = vos::ErrUnknown.with(&data, data_len);
	Error err2 = err;

	T.out(">>> ErrUnknown       : %p\n", &vos::ErrUnknown);
	T.out(">>> err              : %p\n", &err);
	T.out(">>> err2             : %p\n\n", &err2);

	T.out(">>> ErrUnknown.__str : %p\n", vos::ErrUnknown.chars());
	T.out(">>> err.__str        : %p\n", err.chars());
	T.out(">>> err2.__str       : %p\n\n", err2.chars());

	T.out(">>> ErrUnknown.data  : %p\n", vos::ErrUnknown.data());
	T.out(">>> err.data         : %p\n", err.data());
	T.out(">>> err2.data        : %p\n\n", err2.data());

	T.out(">>> err.data         : %d\n", err.data_as_signed());
	T.out(">>> err2.data        : %d\n", err2.data_as_unsigned());

	T.expect_signed(1, vos::ErrUnknown == err, vos::IS_EQUAL);
	T.expect_signed(1, vos::ErrUnknown == err2, vos::IS_EQUAL);
	T.expect_signed(1, err == err2, vos::IS_EQUAL);

	T.expect_ptr(vos::ErrUnknown.chars(), err.chars(), vos::IS_EQUAL);
	T.expect_ptr(vos::ErrUnknown.chars(), err2.chars(), vos::IS_EQUAL);
	T.expect_ptr(err.chars(), err2.chars(), vos::IS_EQUAL);

	T.expect_mem(err2.data(), err.data(), data_len, vos::IS_EQUAL);

	T.ok();
}

void test_data_as()
{
	char c = 'c';
	const char* str = "string";
	int i = 222;
	unsigned long ul = 333;
	double d = 444.444;

	Error errChar   = vos::ErrUnknown.with(&c, 1);
	Error errString = vos::ErrUnknown.with(str, 6);
	Error errSigned = vos::ErrUnknown.with(&i, sizeof(int));
	Error errUnsign = vos::ErrUnknown.with(&ul, sizeof(unsigned long));
	Error errDouble = vos::ErrUnknown.with(&d, sizeof(double));

	T.start("data_as_xxx()");

	T.expect_signed(1, c == errChar.data_as_char(), vos::IS_EQUAL);
	T.expect_string(str, errString.data_as_string(), vos::IS_EQUAL);
	T.expect_signed(i, errSigned.data_as_signed(), vos::IS_EQUAL);
	T.expect_unsigned(ul, errUnsign.data_as_unsigned(), vos::IS_EQUAL);
	T.expect_double(d, errDouble.data_as_double(), vos::IS_EQUAL);

	T.ok();
}

int main()
{
	test_equality();
	test_operator_assign();
	test_with();
	test_data_as();

	return 0;
}

// vi: ts=8 sw=8 tw=80:
