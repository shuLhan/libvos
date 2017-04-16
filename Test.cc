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

int Test::expect_string(const char* exp, const char* got, int to)
{
	int s = strcmp(exp, got);

	if (s == to) {
		return 0;
	}

	printf("%s.%s\n", _header.v(), _prefix.v());
	printf("    Expecting: %s\n", exp);
	printf("    got      : %s\n", got);

	exit(1);
}

} // namespace::vos
// vi: ts=8 sw=8 tw=78:
