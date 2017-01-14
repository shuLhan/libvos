//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../Dlogger.hh"

using vos::Dlogger;

Dlogger dlog;

#define EXP_PREFIX "[rescached] test with prefix\n"
#define EXP_NON_PREFIX "test without prefix\n"

void test_prefix()
{
	dlog.open("log", 0, "[rescached] ", 0);

	dlog.out("test with prefix\n");

	expectString(dlog.chars(), EXP_PREFIX, 0);

	dlog.close();

	// Test without prefix.
	dlog.open("log", 0, "", 0);

	dlog.out("test without prefix\n");
	expectString(dlog.chars(), EXP_NON_PREFIX, 0);

	dlog.close();
}

int main()
{
	test_prefix();

	return 0;
}
