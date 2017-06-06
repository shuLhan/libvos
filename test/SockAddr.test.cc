/**
 * Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "test.hh"
#include "../Test.hh"
#include "../SockAddr.hh"

using vos::Test;
using vos::SockAddr;

Test T("SockAddr");

void test_set_ipv4()
{
	struct {
		const char     *desc;
		const char     *in_addr;
		const uint16_t in_port;
		const int      exp_res;
		const int      exp_t;
		const char     *exp_addr;
		const uint16_t exp_port;
		const char     *exp_chars;
	} const tests[] = {
		{
			"With NULL address",
			NULL,
			0,
			-1,
			IP_NO,
			"0.0.0.0",
			0,
			"",
		},
		{
			"With invalid IP",
			"444.0..1",
			0,
			-1,
			IP_NO,
			"0.0.0.0",
			0,
			"",
		},
		{
			"With IPv6",
			"::1",
			0,
			-1,
			IP_NO,
			"0.0.0.0",
			0,
			"",
		},
		{
			"With AF_INET, 8.8.8.8, 53",
			"8.8.8.8",
			53,
			0,
			IP_V4,
			"8.8.8.8",
			53,
			"8.8.8.8:53",
		},
	};

	SockAddr sa;
	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("set()", tests[x].desc);

		int got = sa.set(AF_INET, tests[x].in_addr, tests[x].in_port);

		if (tests[x].exp_res != 0) {
			T.expect_signed(tests[x].exp_res, got, vos::IS_EQUAL);
			T.ok();
			continue;
		}

		T.expect_signed(tests[x].exp_t, sa._t, vos::IS_EQUAL);
		T.expect_string(tests[x].exp_addr, sa.get_address(AF_INET), vos::IS_EQUAL);
		T.expect_signed(tests[x].exp_port, sa.get_port(AF_INET), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_chars, sa.chars(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_set_ipv6()
{
	struct {
		const char     *desc;
		const char     *in_addr;
		const uint16_t in_port;
		const int      exp_res;
		const int      exp_t;
		const char     *exp_addr;
		const uint16_t exp_port;
		const char     *exp_chars;
	} const tests[] = {
		{
			"With NULL address",
			NULL,
			0,
			-1,
			IP_NO,
			"0.0.0.0",
			0,
			"",
		},
		{
			"With invalid IP",
			"1:1:x:1",
			0,
			-1,
			IP_NO,
			"0.0.0.0",
			0,
			"",
		},
		{
			"With IPv4",
			"0.0.0.0",
			0,
			-1,
			IP_NO,
			"0.0.0.0",
			0,
			"",
		},
		{
			"With AF_INET6, 8.8.8.8, 53",
			"::1",
			53,
			0,
			IP_V6,
			"::1",
			53,
			"[::1]:53",
		},
	};

	SockAddr sa;
	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("set(AF_INET6)", tests[x].desc);

		int got = sa.set(AF_INET6, tests[x].in_addr, tests[x].in_port);

		if (tests[x].exp_res != 0) {
			T.expect_signed(tests[x].exp_res, got, vos::IS_EQUAL);
			T.ok();
			continue;
		}

		T.expect_signed(tests[x].exp_t, sa._t, vos::IS_EQUAL);
		T.expect_string(tests[x].exp_addr, sa.get_address(AF_INET6), vos::IS_EQUAL);
		T.expect_signed(tests[x].exp_port, sa.get_port(AF_INET6), vos::IS_EQUAL);
		T.expect_string(tests[x].exp_chars, sa.chars(), vos::IS_EQUAL);

		T.ok();
	}
}

void test_IS_IPV4()
{
	T.expect_signed(1, SockAddr::IS_IPV4("0.0.0.0"), vos::IS_EQUAL);
	T.expect_signed(1, SockAddr::IS_IPV4("127.0.0.1"), vos::IS_EQUAL);
	T.expect_signed(1, SockAddr::IS_IPV4("192.168.1.0"), vos::IS_EQUAL);
	T.expect_signed(1, SockAddr::IS_IPV4("192.168.1.1"), vos::IS_EQUAL);
	T.expect_signed(1, SockAddr::IS_IPV4("192.168.1.255"), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("192.168.1.265"), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("192.168.1a.254"), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("192.1680.1.254"), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("1.168.1.256"), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("1.168.1.2520"), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("1.168.1."), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("192.168.1"), vos::IS_EQUAL);
	T.expect_signed(0, SockAddr::IS_IPV4("192..168.1"), vos::IS_EQUAL);
}

int main()
{
	test_set_ipv4();
	test_set_ipv6();
	test_IS_IPV4();

	return 0;
}
