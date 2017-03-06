//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_TREENODE_HH
#define _LIBVOS_TREENODE_HH 1

#include "BNode.hh"

namespace vos {

enum _node_attr {
	RBT_IS_RED	= 1
,	RBT_IS_BLACK	= 2
};

//
// `TreeNode` implement node for tree data structure, with
//   - two pointers to child nodes: `_left` and `_right`,
//   - one pointer to parent node,
//   - and one pointer to the `_item` that its contain,
//   - and one field for node attribute `_attr`.
//
class TreeNode : public BNode {
public:
	TreeNode(Object* item);
	virtual ~TreeNode();
	void detach();

	void set_attr(int x);
	void set_attr_to_red();
	void set_attr_to_black();
	int get_attr();
	void swap_attr(TreeNode* node);

	int have_attr(int x);
	int is_red();
	int is_black();
	int is_left_red();
	int is_right_red();

	using BNode::set_left;
	void set_left(TreeNode* node);
	TreeNode* get_left();

	using BNode::set_right;
	void set_right(TreeNode* node);
	TreeNode* get_right();

	void set_parent(TreeNode* node);
	TreeNode* get_parent();
	TreeNode* get_grand_parent();

	int have_no_childs();
	int have_red_childs();
	int have_black_childs();

	void set_childs_attr(int x);
	void set_childs_attr_to_red();
	void set_childs_attr_to_black();

	using BNode::insert_left;
	void insert_left(TreeNode* node);

	using BNode::insert_right;
	void insert_right(TreeNode* node);

	const char* chars();

	int _attr;
	TreeNode* _left;
	TreeNode* _right;
	TreeNode* _top;

	static int CMP(TreeNode* x, TreeNode* y);

	static const char* __cname;
private:
	TreeNode(const TreeNode&);
	void operator=(const TreeNode&);
};

} // namespace vos
#endif // _LIBVOS_TREENODE_HH
// vi: ts=8 sw=8 tw=78:
