//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include "test.hh"
#include "../Dir.hh"

using vos::Dir;
using vos::DirNode;

struct ExpDirNode {
	const char* name;
	int is_dir;
	int is_file;
	int is_link;
};

void test_open()
{
	struct ExpDirNode exps[] = {
		{ "subdir", 1, 0, 0 }
	,	{ "test.a", 0, 1, 0 }
	,	{ "test.b", 0, 1, 0 }
	,	{ "test.c", 0, 1, 0 }
	,	{ "test.d", 0, 1, 0 }
	,	{ "test.link.a", 0, 0, 1 }
	};

	int s = 0;
	Dir dir;
	DirNode* inode;

	s = dir.open("./dir");

	assert(s == 0);

	inode = dir._ls;

	assert(inode->is_dir() == 1);
	assert(inode->_child != NULL);

	inode = inode->_child;

	s = 0;
	while (inode) {
		expectString(inode->_name.chars(), exps[s].name, 0);
		assert(inode->is_dir() == exps[s].is_dir);
		assert(inode->is_file() == exps[s].is_file);
		assert(inode->is_link() == exps[s].is_link);

		inode = inode->_next;
		s++;
	}
}

int main()
{
	test_open();
	return 0;
}
