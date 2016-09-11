//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "../SSVReader.hh"

int main (int argc, char** argv)
{
	register int s;
	vos::SSVReader reader;

	s = reader.load ("./hosts");
	if (s != 0) {
		return -1;
	}

	//reader._rows->dump ();

	reader.reset ();
	reader._comment_c = '#';

	s = reader.load ("./hosts");

	//reader._rows->dump ();

	return s;
}

// vi: ts=8 sw=8 tw=78:
