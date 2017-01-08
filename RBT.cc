//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "RBT.hh"

namespace vos {

const char* RBT::__cname = "RBT";

//
// `RBT()` will create new Red-Black Tree.
// `fn_cmp` parameter is a pointer to function that will be used to compare
// nodes when doing insertion.
//
RBT::RBT(int (*fn_cmp)(TreeNode*, TreeNode*)) : Locker()
,	_root(NULL)
,	_fn_cmp(fn_cmp)
{}

void RBT::_delete(TreeNode* p)
{
	if (!p) {
		return;
	}
	if (p->_left) {
		_delete(p->_left);
	}
	if (p->_right) {
		_delete(p->_right);
	}
	delete p;
	p = NULL;
}

//
// `~RBT()` will delete left and right node in tree recursively.
//
RBT::~RBT()
{
	lock();

	_delete(_root);

	unlock();
}

void RBT::_chars(Buffer* b, TreeNode* p, int level)
{
	int x = 0;

	if (p->_left) {
		_chars(b, p->_left, level+1);
	}

	for (; x < level; x++) {
		b->appendc(' ');
	}

	b->append_raw(p->chars());
	b->appendc('\n');

	if (p->_right) {
		_chars(b, p->_right, level+1);
	}
}

//
// `RBT::chars` will return the string representation of this tree.
//
const char* RBT::chars()
{
	lock();

	Buffer b;
	TreeNode* p = _root;

	if (_v) {
		free(_v);
		_v = NULL;
	}

	_chars(&b, p, 0);

	_v = b._v;
	b._v = NULL;

	unlock();

	return _v;
}

//
// `set_root_unsafe()` will set the root of tree to `node`.
//
void RBT::set_root_unsafe(TreeNode* node)
{
	_root = node;
}

//
// `get_root_unsafe()` will return the root of the tree as a node.
//
TreeNode* RBT::get_root_unsafe()
{
	return _root;
}

//
// `TREE_ROTATE_RIGHT()` will move the left-child (son) of `parent` as parent
// of itself.
// if son have daughter, then it will be the left child of `parent`.
//
// ```
//   ...                ...                |
//     \                  \                |
//      parent             son             |
//       / \                 \             |
//    son   r-child  ==>      parent       |
//     \                        / \        |
//      g-daughter    g-daughter   r-chilf |
// ```
//
static TreeNode* TREE_ROTATE_RIGHT(TreeNode* root, TreeNode* parent)
{
	TreeNode* gp = parent->get_parent();
	TreeNode* son = parent->get_left();
	TreeNode* granddaughter = son->get_right();

	parent->set_left(granddaughter);

	if (granddaughter) {
		granddaughter->set_parent(parent);
	}

	son->set_parent(gp);

	if (!gp) {
		root = son;
	} else {
		if (gp->get_right() == parent) {
			gp->set_right(son);
		} else {
			gp->set_left(son);
		}
	}

	son->set_right(parent);
	parent->set_parent(son);

	return root;
}

//
// `TREE_ROTATE_LEFT()` will promote the right-child (daughter) of `parent` as
// their parent.
// If daughter have son, then it will be the right child of `parent`.
//
// ```
//  ...                        ...            |
//    \                          \            |
//     parent                     daughter    |
//      / \          ===>         /           |
//   ...   daughter         parent            |
//            /              / \              |
//       g-son            ...   g-son         |
// ```
//
static TreeNode* TREE_ROTATE_LEFT(TreeNode* root, TreeNode* parent)
{
	TreeNode* gp = parent->get_parent();
	TreeNode* daughter = parent->get_right();
	TreeNode* grandson = daughter->get_left();

	parent->set_right(grandson);

	if (grandson) {
		grandson->set_parent(parent);
	}

	daughter->set_parent(gp);

	if (!gp) {
		root = daughter;
	} else {
		if (gp->get_left() == parent) {
			gp->set_left(daughter);
		} else {
			gp->set_right(daughter);
		}
	}

	daughter->set_left(parent);
	parent->set_parent(daughter);

	return root;
}

//
// 1. While parent is RED,
// 1.1. get grand-parent.
// 1.2. If parent is the left child of grand-parent,
// 1.2.1. get sibling of parent (grand-parent's right child), lets call it
// aunt.
// 1.2.2. If aunt is RED (parent is also RED),
// 1.2.2.1. set aunt and parent to BLACK
// 1.2.2.2. set grand-parent to RED
// 1.2.2.3. set node as grand-parent
// 1.2.3. otherwise, if aunt is BLACK,
// 1.2.3.1. and node is right child of parent,
// 1.2.3.1.1. set node to parent,
// 1.2.3.1.2. rotate parent tree to the left.
// 1.2.3.2. set parent to BLACK
// 1.2.3.3. set grand-parent to RED
// 1.2.3.4. rotate grand-parent's tree to right
//
// 1.3. If parent is the right child of grand-parent,
// 1.3.1. set aunt as left child of grand-parent.
// 1.3.2. If aunt is RED (parent is also RED),
// 1.3.2.1. set aunt and parent to BLACK
// 1.3.2.2. set grand-parent to RED
// 1.3.2.3. set node as grand-parent
// 1.3.3. otherwise, if aunt is BLACK,
// 1.3.3.1. and node is left child of parent,
// 1.3.3.1.1. set node to parent,
// 1.3.3.1.2. rotate parent tree to the right.
// 1.3.3.2. set parent to BLACK
// 1.3.3.3. set grand-parent to RED
// 1.3.3.4. rotate grand-parent's tree to left.
//
// 1.4. Get node parent again.
//
static TreeNode* INSERT_FIXUP(TreeNode* root, TreeNode* node)
{
	TreeNode* parent = NULL;
	TreeNode* gp = NULL;
	TreeNode* aunt = NULL;

	// (1)
	parent = node->get_parent();
	while (parent && parent->have_attr(RBT_IS_RED)) {
		// (1.1)
		gp = node->get_grand_parent();
		if (!gp) {
			break;
		}

		// (1.2)
		if (parent->is_left_of(gp)) {
			// (1.2.1)
			aunt = gp->get_right();

			// (1.2.2)
			if (aunt && aunt->have_attr(RBT_IS_RED)) {
				parent->set_attr(RBT_IS_BLACK);
				aunt->set_attr(RBT_IS_BLACK);
				gp->set_attr(RBT_IS_RED);

				node = gp;
			} else {
				// (1.2.3)
				// (1.2.3.1)
				if (node->is_right_of(parent)) {
					node = parent;
					root = TREE_ROTATE_LEFT(root, node);
				}

				parent->set_attr(RBT_IS_BLACK);
				gp->set_attr(RBT_IS_RED);

				root = TREE_ROTATE_RIGHT(root, gp);
			}
		} else {
			// (1.3.1)
			aunt = gp->get_left();

			// (1.3.2)
			if (aunt && aunt->have_attr(RBT_IS_RED)) {
				parent->set_attr(RBT_IS_BLACK);
				aunt->set_attr(RBT_IS_BLACK);
				gp->set_attr(RBT_IS_RED);

				node = gp;
			} else {
				// (1.3.3)
				// (1.3.3.1)
				if (node->is_left_of(parent)) {
					node = parent;
					root = TREE_ROTATE_RIGHT(root, node);
				}

				parent->set_attr(RBT_IS_BLACK);
				gp->set_attr(RBT_IS_RED);

				root = TREE_ROTATE_LEFT(root, gp);
			}
		}

		// (1.4)
		parent = node->get_parent();
	}

	root->set_attr(RBT_IS_BLACK);

	return root;
}

//
// `RBT::insert` will insert `node` into tree using `fn_cmp` to compare each
// node while traversing the tree from top to bottom.
//
// It will return `node` that has been inserted, or `node` that has been
// replaced in tree.
//
TreeNode* RBT::insert(TreeNode* node, int replace)
{
	if (!node) {
		return node;
	}

	lock();

	int s = 0;
	TreeNode* p = get_root_unsafe();
	TreeNode* top = NULL;

	if (!p) {
		node->set_attr(RBT_IS_RED);
		set_root_unsafe(node);

		unlock();
		return node;
	}

	while (p) {
		top = p;
		s = _fn_cmp(p, node);

		if (s == 0) {
			if (replace) {
				p->replace_content(node);

				unlock();
				return p;
			}

			p->insert_left(node);
			node->set_attr(RBT_IS_RED);

			set_root_unsafe(INSERT_FIXUP(_root, node));

			unlock();
			return node;
		} else if (s < 0) {
			p = p->get_right();
		} else {
			p = p->get_left();
		}
	}

	if (s < 0) {
		top->set_right(node);
	} else {
		top->set_left(node);
	}

	node->set_parent(top);
	node->set_attr(RBT_IS_RED);

	set_root_unsafe(INSERT_FIXUP(_root, node));

	unlock();
	return node;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
