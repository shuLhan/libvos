//
// Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../User.hh"

using vos::User;

void test_switch()
{
	User me;

	me.switch_to_effective("bin");

	assert(me.get_effective_id() == 1);
	assert(me.get_effective_gid() == 1);
	assert(geteuid() == 1);
	assert(getegid() == 1);
	expectString(me.get_name(), getlogin(), 0);
	expectString(me.get_effective_name(), "bin", 0);

	me.switch_to_real();

	assert(me.get_effective_id() == 0);
	assert(me.get_effective_gid() == 0);
	assert(geteuid() == 0);
	assert(getegid() == 0);
}

int main()
{
	test_switch();

	return 0;
}
