//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "TestLocker.hh"

int main()
{
	TestLocker tl;

	assert(tl.data == 0);

	tl.inc();
	assert(tl.data == 1);

	tl.dec();
	assert(tl.data == 0);

	return 0;
}
