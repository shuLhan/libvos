//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Test.hh"

namespace vos {

Test::Test(const char* header)
: Dlogger()
, _header(header)
, _prefix((const char*) 0)
{
	out(">>> Testing %s\n", header);
}

Test::~Test()
{
}

void Test::start(const char* prefix, const char* msg)
{
	if (prefix) {
		_prefix.copy_raw(prefix);
	}

	out("  %s.%s", _header.v(), _prefix.v());

	if (msg) {
		out(" - %s", msg);
	}

	out(" : ");
}

void Test::ok()
{
	out("OK\n");
}

/**
 * Method `expect_mem(exp, got, len, equality)` will compare content of memory
 * pointed by `exp` with `got` with length no more than `len`.
 *
 * It will return 0 if
 *
 * - equality is `1` and `exp` greater than `got`,
 * - equality is `0` and `exp` is equal with `got`,
 * - equality is `-1` and `exp` less than `got`.
 *
 * Otherwise it will terminate the program with exit code `1`.
 */
int Test::expect_mem(const void *exp, const void *got, size_t len,
	int equality)
{
	int res = memcmp(exp, got, len);

	if (res == equality) {
		return 0;
	}

	printf("\n");
	printf("    Expecting: %p\n", exp);
	printf("    got      : %p\n", got);

	exit(1);
}

/**
 * Method `expect_ptr(exp, got, equality)` will compare pointer of `exp` and
 * `got`.
 *
 * It will return `0` if,
 *
 * - `exp` and `got` has the same pointer and equality is true (`1`).
 * - `exp` and `got` is not point to the same memory and equality is false
 *   (`0`).
 *
 * Otherwise it will terminate the program with exit status `1`.
 */
int Test::expect_ptr(const void *exp, const void *got, int equality)
{
	const char *str_equality;

	switch (equality) {
	case 0:
		if (exp == got) {
			return 0;
		}
		str_equality = "equal";
		break;
	case 1:
		if (exp != got) {
			return 0;
		}
		str_equality = "not equal";
		break;
	}

	printf("\n");
	printf("    Expecting pointer '%p' %s '%p'\n", exp, str_equality, got);

	exit(1);
}

/**
 * Method `expect_string(exp, got, to)` will compare expected string value
 * `exp` with `got` value.
 *
 * Value of equality `-1` means the function expect the `exp` value to be less
 * than `got` value.
 * Value of equality `0` means its `exp` equal with `got`.
 * Value of equality `1` means the function expect the `exp` value greater
 * than `got` value.
 *
 * It will return `0` if string comparison of `exp` with `got` value match
 * with `equality` value; otherwise it will exit the test with status 1.
 */
int Test::expect_string(const char* exp, const char* got, int equality)
{
	int s = strcmp(exp, got);

	if (s == equality) {
		return 0;
	}

	printf("\n");
	printf("    Expecting: %s\n", exp);
	printf("    got      : %s\n", got);

	exit(1);
}

/**
 * Method `expect_signed(exp, got, equality)` will check if `exp` value is
 * either,
 *
 * - less (equality is -1),
 * - equal (equality is 0), or
 * - greater (equality is 1)
 *
 * than `got` value.
 *
 * It will return `0` if expected and got value true to their equality value.
 * Otherwise, it will exit with status `1`.
 */
int Test::expect_signed(const ssize_t exp, const ssize_t got, int equality)
{
	const char *str_equality;

	switch (equality) {
	case -1:
		if (exp < got) {
			return 0;
		}
		str_equality = "less than";
		break;
	case 0:
		if (exp == got) {
			return 0;
		}
		str_equality = "equal";
		break;
	case 1:
		if (exp > got) {
			return 0;
		}
		str_equality = "greater than";
		break;
	}

	printf("\n");
	printf("    Expecting '%zd' %s '%zd'\n", exp, str_equality, got);

	exit(1);
}

/**
 * Method `expect_unsigned(exp, got, equality)` will check if `exp` value is
 * either,
 *
 * - less than (equality is -1),
 * - equal (equality is 0), or
 * - greater than (equality is 1)
 *
 * `got` value.
 *
 * It will return `0` if expected and got value true to their equality value.
 * Otherwise, it will exit with status `1`.
 */
int Test::expect_unsigned(const size_t exp, const size_t got, int equality)
{
	const char *str_equality;

	switch (equality) {
	case -1:
		if (exp < got) {
			return 0;
		}
		str_equality = "less than";
		break;
	case 0:
		if (exp == got) {
			return 0;
		}
		str_equality = "equal";
		break;
	case 1:
		if (exp > got) {
			return 0;
		}
		str_equality = "greater than";
		break;
	}

	printf("\n");
	printf("    Expecting '%zu' %s '%zu'\n", exp, str_equality, got);

	exit(1);

}

} // namespace::vos
// vi: ts=8 sw=8 tw=78:
