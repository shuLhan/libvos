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

//
// `RBT` implement the Red-Black tree algorithm.
//
class RBT : public Locker {
public:
	RBT(int (*fn_cmp)(Object*, Object*));
	virtual ~RBT();

	const char* chars();
	const char* get_red_node_chars();

	void set_root_unsafe(TreeNode* node);
	TreeNode* get_root_unsafe();
	TreeNode* get_root();

	TreeNode* insert(TreeNode* node, int replace = 0);
	TreeNode* remove(TreeNode* node);
	TreeNode* find(Object* item);

	int is_nblack_valid();

	static const char* __cname;
private:
	TreeNode* _root;
	Buffer _red_nodes;

	// Variable that count and save current black node in tree.
	int _n_black;

	// Variable that set to `1` if all path have the same number of black
	// nodes, or `0` if one of the path is not equal.
	int _n_black_valid;

	int (*_fn_cmp)(Object*, Object*);
	void _delete(TreeNode* p);
	void _chars(Buffer* b, TreeNode* node, int level = 0, int n_black = 0);

	void _do_rebalance(TreeNode* x);
	void _removed_have_no_child(TreeNode* x);
	TreeNode* _removed_have_both_childs(TreeNode* x);
	TreeNode* _remove_unsafe(TreeNode* x);

	RBT(const RBT&);
	void operator=(const RBT&);
};

} // namespace::vos

#endif
// vi: ts=8 sw=8 tw=78
