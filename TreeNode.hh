//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_TREENODE_HH
#define _LIBVOS_TREENODE_HH 1

#include "BNode.hh"

namespace vos {

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

	void set_attr(int x);
	int have_attr(int x);

	void set_left(TreeNode* node);
	TreeNode* get_left();

	void set_right(TreeNode* node);
	TreeNode* get_right();

	void set_parent(TreeNode* node);
	TreeNode* get_parent();
	TreeNode* get_grand_parent();

	void insert_left(TreeNode* node);

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
