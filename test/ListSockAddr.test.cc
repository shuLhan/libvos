//
// Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include "test.hh"
#include "../Test.hh"
#include "../ListSockAddr.hh"

using vos::Test;
using vos::List;

Test T("ListSockAddr");

void test_LISTSOCKADDR_CREATE()
{
	struct {
		const char *desc;
		const char *in;
		const char sep;
		const uint16_t def_port;
		const char *exp_v;
	} const tests[] = {
		{
			"With null list",
			NULL,
			',',
			53,
			NULL,
		},
		{
			"With invalid address",
			"127.0.0.1.1",
			',',
			53,
			NULL,
		},
		{
			"With invalid address",
			"127..0.1",
			',',
			53,
			NULL,
		},
		{
			"With single address",
			"127.0.0.1",
			',',
			53,
			"[ \"127.0.0.1:53\" ]",
		},
		{
			"With single address and empty port",
			"127.0.0.1:",
			',',
			53,
			"[ \"127.0.0.1:53\" ]",
		},
		{
			"With single address and port",
			"127.0.0.1:1000",
			',',
			53,
			"[ \"127.0.0.1:1000\" ]",
		},
		{
			"With two address, both are invalid",
			"127.0.0., 192.100..1",
			',',
			53,
			NULL,
		},
		{
			"With two address, one is invalid",
			"127.0.0., 192.168.100.1",
			',',
			53,
			"[ \"192.168.100.1:53\" ]",
		},
		{
			"With two address",
			"127.0.0.1, 192.168.100.1",
			',',
			53,
			"[ \"127.0.0.1:53\",\"192.168.100.1:53\" ]",
		},
		{
			"With two address and one port",
			"127.0.0.1, 192.168.100.1:1000",
			',',
			53,
			"[ \"127.0.0.1:53\",\"192.168.100.1:1000\" ]",
		},
		{
			"With two address and port",
			"127.0.0.1:54, 192.168.100.1:1000",
			',',
			53,
			"[ \"127.0.0.1:54\",\"192.168.100.1:1000\" ]",
		},
		{
			"With invalid hostname",
			"localhostasdasd",
			',',
			53,
			NULL,
		},
		{
			"With hostname",
			"localhost",
			',',
			53,
			"[ \"127.0.0.1:53\" ]",
		},
		{
			"With hostname and port",
			"localhost:80",
			',',
			53,
			"[ \"127.0.0.1:80\" ]",
		},
	};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		T.start("LISTSOCKADDR_CREATE", tests[x].desc);

		List *got = vos::LISTSOCKADDR_CREATE(tests[x].in,
			tests[x].sep, tests[x].def_port);

		if (got == NULL) {
			T.expect_ptr(tests[x].exp_v, got, 0);
		} else {
			T.expect_string(tests[x].exp_v, got->chars(), 0);
			delete got;
		}

		T.ok();
	}
}

int main()
{
	test_LISTSOCKADDR_CREATE();

	return 0;
}
