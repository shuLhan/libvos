//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_RBT_HH
#define _LIBVOS_RBT_HH 1

#include "Locker.hh"
#include "TreeNode.hh"
#include "Buffer.hh"

namespace vos {

enum _rbt_color {
	RBT_IS_RED	= 1
,	RBT_IS_BLACK	= 2
};

//
// `RBT` implement the Red-Black tree algorithm.
//
class RBT : public Locker {
public:
	RBT(int (*fn_cmp)(TreeNode*, TreeNode*));
	virtual ~RBT();

	const char* chars();

	void set_root_unsafe(TreeNode* node);
	TreeNode* get_root_unsafe();

	TreeNode* insert(TreeNode* node, int replace = 0);


	static const char* __cname;
private:
	TreeNode* _root;

	int (*_fn_cmp)(TreeNode*, TreeNode*);
	void _delete(TreeNode* p);
	void _chars(Buffer* b, TreeNode* node, int level = 0);

	RBT(const RBT&);
	void operator=(const RBT&);
};

} // namespace::vos

#endif
// vi: ts=8 sw=8 tw=78
