//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "test.hh"
#include "../RBT.hh"

using vos::BNode;
using vos::TreeNode;
using vos::RBT;

Buffer* b = NULL;

int main()
{
	RBT rbt(TreeNode::CMP);
	TreeNode* node = NULL;
	char v = 'a';
	int x = 0;

	const char* exp_root_contents[] = {
		"a","a"                                 // a - b
	,	"b","b","b","b","b"                     // c - g
	,	"d","d","d","d","d","d","d","d","d","d" // h - p
	,	"h","h","h","h","h","h","h","h","h","h" // q - z

	,	"h","h","h","h","h","h","h","h","h","h" // a2-j2
	,	"h","h","h","h","h","h","h","h","h","h" // k2-t2
	,	"h","h","h","h","h","h"                 // u2-z2
	};

	while (v <= 'z') {
		b = new Buffer();
		b->copy_raw(&v, 1);
		node = new TreeNode(b);

		rbt.insert(node);
		//printf("rbt:\n%s\n", rbt.chars());

		assert(strcmp(rbt.get_root_unsafe()->get_content()->chars()
			, exp_root_contents[x]) == 0);

		v++;
		x++;
	}

	v = 'a';
	while (v <= 'z') {
		b = new Buffer();
		b->copy_raw(&v, 1);
		node = new TreeNode(b);

		rbt.insert(node);
		//printf("rbt:\n%s\n", rbt.chars());

		assert(strcmp(rbt.get_root_unsafe()->get_content()->chars()
			, exp_root_contents[x]) == 0);

		v++;
		x++;
	}
}
