//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include "test.hh"

Dlogger D;

int expectString(const char* exp, const char* got, int to)
{
	int s = strcmp(exp, got);

	if (s == to) {
		return 0;
	}

	printf("[T] >>> Expecting: %s\n", exp);
	printf("[T] >>> Got: %s\n", got);

	exit(1);
}
