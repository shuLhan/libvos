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
{}

/**
 * Method start(prefix,msg) will print a test message `header.prefix - msg`
 * that indicate the test unit that will be running.
 */
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

/**
 * Method ok() will print string "OK" with new line if test run succesfully.
 */
void Test::ok()
{
	out("OK\n");
}

/**
 * Method `expect_mem(exp, got, len, e)` will compare content of memory
 * pointed by `exp` with `got` with length no more than `len`.
 *
 * It will return 0 if
 *
 * - e is IS_LESS_THAN and `exp` greater than `got`,
 * - e is IS_EQUAL and `exp` is equal with `got`,
 * - e is IS_GREATER_THAN and `exp` less than `got`.
 *
 * Otherwise it will terminate the program with exit code `1`.
 */
int Test::expect_mem(const void *exp, const void *got, size_t len
	, enum EQUALITY e)
{
	int res = memcmp(exp, got, len);

	if (res == e) {
		return 0;
	}

	append_fmt("\n");
	append_fmt("    Expecting: %p\n", exp);
	append_fmt("    got      : %p\n", got);

	exit(1);
}

/**
 * Method `expect_ptr(exp, got, e)` will compare pointer of `exp` and
 * `got`.
 *
 * It will return `0` if,
 *
 * - `exp` and `got` has the same pointer and e is IS_EQUAL.
 * - `exp` and `got` is not pointing to the same memory location and e is
 *   IS_NOT_EQUAL.
 *
 * Otherwise it will terminate the program with exit status `1`.
 */
int Test::expect_ptr(const void *exp, const void *got
	, enum EQUALITY e)
{
	const char *str_e;

	switch (e) {
	case IS_EQUAL:
		if (exp == got) {
			return 0;
		}
		str_e = "equal";
		break;
	case IS_LESS_THAN:
	case IS_NOT_EQUAL:
		if (exp != got) {
			return 0;
		}
		str_e = "not equal";
		break;
	}

	append_fmt("\n");
	append_fmt("    Expecting pointer '%p' %s '%p'\n", exp, str_e, got);

	exit(1);
}

/**
 * Method `expect_string(exp, got, to)` will compare expected string value
 * `exp` with `got` value.
 *
 * Value of e IS_LESS_THAN means the function expect the `exp` value to be
 * less than `got` value.
 * Value of e IS_EQUAL means its `exp` equal with `got`.
 * Value of e IS_GREATER_THAN means the function expect the `exp` value
 * greater than `got` value.
 *
 * On success it will return `0`, if expected and got value true to their `e`
 * value.
 *
 * On fail it will exit with status `1`.
 */
int Test::expect_string(const char* exp, const char* got
	, enum EQUALITY e)
{
	int s = strncmp(exp, got, strlen(exp));

	if (s == e) {
		return 0;
	}

	append_fmt("\n");
	append_fmt("    Expecting: %s\n", exp);
	append_fmt("    got      : %s\n", got);

	exit(1);
}

/**
 * Method `expect_signed(exp, got, e)` will check if `exp` value is
 * either less, equal, or greater than `got` value.
 *
 * On success it will return `0`, if expected and got value true to their `e`
 * value.
 *
 * On fail it will exit with status `1`.
 */
int Test::expect_signed(const ssize_t exp, const ssize_t got
	, enum EQUALITY e)
{
	const char *str_e;

	switch (e) {
	case IS_LESS_THAN:
		if (exp < got) {
			return 0;
		}
		str_e = "less than";
		break;
	case IS_EQUAL:
		if (exp == got) {
			return 0;
		}
		str_e = "equal";
		break;
	case IS_GREATER_THAN:
		if (exp > got) {
			return 0;
		}
		str_e = "greater than";
		break;
	}

	append_fmt("\n");
	append_fmt("    Expecting '%d' %s '%d'\n", exp, str_e, got);

	exit(1);
}

/**
 * Method `expect_unsigned(exp, got, e)` will check if `exp` value is
 * either less, equal, or greater than `got` value.
 *
 * On success it will return `0`, if expected and got value true to their `e`
 * value.
 *
 * On fail it will exit with status `1`.
 */
int Test::expect_unsigned(const size_t exp, const size_t got
	, enum EQUALITY e)
{
	const char *str_e;

	switch (e) {
	case IS_LESS_THAN:
		if (exp < got) {
			return 0;
		}
		str_e = "less than";
		break;
	case IS_EQUAL:
		if (exp == got) {
			return 0;
		}
		str_e = "equal";
		break;
	case IS_GREATER_THAN:
		if (exp > got) {
			return 0;
		}
		str_e = "greater than";
		break;
	}

	append_fmt("\n");
	append_fmt("    Expecting '%u' %s '%u'\n", exp, str_e, got);

	exit(1);

}

/**
 * Method expect_double(exp, got, e) will check if `exp` value is
 * either less, equal, or greater than `got` value.
 *
 * On success it will return `0`, if expected and got value true to their e
 * value.
 *
 * On fail, it will exit with status `1`.
 */
int Test::expect_double(const double exp, const double got
	, enum EQUALITY e)
{
	const char *str_e;

	switch (e) {
	case IS_LESS_THAN:
		if (exp < got) {
			return 0;
		}
		str_e = "less than";
		break;
	case IS_EQUAL:
		if (exp == got) {
			return 0;
		}
		str_e = "equal";
		break;
	case IS_GREATER_THAN:
		if (exp > got) {
			return 0;
		}
		str_e = "greater than";
		break;
	}

	append_fmt("\n");
	append_fmt("    Expecting '%f' %s '%f'\n", exp, str_e, got);

	exit(1);
}

} // namespace::vos

// vi: ts=8 sw=8 tw=80:
