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
RBT::RBT(int (*fn_cmp)(Object*, Object*)
	, void (*fn_swap)(Object*, Object*))
:	Locker()
,	_root(NULL)
,	_red_nodes()
,	_n_black(-1)
,	_n_black_valid(1)
,	_fn_cmp(fn_cmp)
,	_fn_swap(fn_swap)
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

void RBT::_chars(Buffer* b, TreeNode* p, int level, int n_black)
{
	int x = 0;

	if (!p) {
		return;
	}

	if (p->is_black()) {
		n_black++;
	}

	if (p->_left) {
		_chars(b, p->_left, level+1, n_black);
	}

	for (; x < level; x++) {
		b->append_raw("..", 2);
	}

	b->append_raw(p->chars());

	if (!p->_left && !p->_right) {
		b->aprint("  N Black (%d)", n_black);

		if (_n_black < 0) {
			_n_black = n_black;
		} else {
			if (_n_black != n_black) {
				_n_black_valid = 0;
			}
		}
	}

	b->appendc('\n');

	if (p->is_red()) {
		if (_red_nodes.len() > 0) {
			_red_nodes.appendc(' ');
		}
		_red_nodes.append_raw(p->BNode::chars());
	}

	if (p->_right) {
		_chars(b, p->_right, level+1, n_black);
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

	if (__str) {
		free(__str);
		__str = NULL;
	}

	_red_nodes.reset();

	_n_black = -1;
	_n_black_valid = 1;

	_chars(&b, p, 0, 0);

	__str = b.detach();

	unlock();

	return __str;
}

void RBT::swap_content(TreeNode* x, TreeNode* y)
{
	Object* ox = x->get_content();
	Object* oy = y->get_content();
	x->set_content(oy);
	y->set_content(ox);

	if (_fn_swap) {
		_fn_swap(ox, oy);
	}
}

const char* RBT::get_red_node_chars()
{
	if (_red_nodes.is_empty()) {
		return "\0";
	}

	return _red_nodes.chars();
}

//
// `set_root_unsafe()` will set the root of tree to `node`.
//
void RBT::set_root_unsafe(TreeNode* node)
{
	_root = node;
}

/**
 * `get_root_unsafe()` will return the root of the tree as a node.
 * This is a non-thread safe operation.
 */
TreeNode* RBT::get_root_unsafe()
{
	return _root;
}

/**
 * `get_root()` will return the root of the tree.
 * This is a thread safe operation.
 */
TreeNode* RBT::get_root()
{
	lock();
	TreeNode* r = _root;
	unlock();

	return r;
}

/**
 *
 * `TREE_ROTATE_RIGHT()` will move the left-child of `x` as parent of
 * itself.
 * if left-child have right-child, then it will be the left child of `parent`
 * replacing previous left-child.
 *
 * ```
 *       ...               ...
 *        |                 |
 *        x                left
 *       / \                 \
 *    left  ...  ==>          x
 *       \                   / \
 *        gleft         gleft  ...
 * ```
 */
static TreeNode* TREE_ROTATE_RIGHT(TreeNode* root, TreeNode* x)
{
	TreeNode* parent = x->get_parent();
	TreeNode* left = x->get_left();
	TreeNode* granddaughter = left->get_right();

	x->set_left(granddaughter);

	if (granddaughter) {
		granddaughter->set_parent(x);
	}

	left->set_parent(parent);

	if (!parent) {
		root = left;
	} else {
		if (x->is_left_of(parent)) {
			parent->set_left(left);
		} else {
			parent->set_right(left);
		}
	}

	left->set_right(x);
	x->set_parent(left);

	return root;
}

/**
 * `TREE_ROTATE_LEFT()` will promote the right-child of `parent` as their
 * parent.
 * If right have son, then it will be the right child of `parent`.
 *
 *     ...                 ...
 *      |                   |
 *      x                right
 *     / \          ===>   /
 *  ...   right           x
 *        /              / \
 *      g-son          ...   g-son
 */
static TreeNode* TREE_ROTATE_LEFT(TreeNode* root, TreeNode* x)
{
	TreeNode* parent = x->get_parent();
	TreeNode* right = x->get_right();
	TreeNode* gleft = right->get_left();

	x->set_right(gleft);

	if (gleft) {
		gleft->set_parent(x);
	}

	right->set_parent(parent);

	if (!parent) {
		root = right;
	} else {
		if (x->is_left_of(parent)) {
			parent->set_left(right);
		} else {
			parent->set_right(right);
		}
	}

	right->set_left(x);
	x->set_parent(right);

	return root;
}

//
// 1. While parent is RED,
// 1.1. get grand-parent.
// 1.1.1. If grand-parent is NULL, stop.
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
// ... mirroring the left-side operation
//
// 1.4. Get node parent again and go to step 1.1
//
static TreeNode* INSERT_FIXUP(TreeNode* root, TreeNode* node)
{
	TreeNode* gp = NULL;
	TreeNode* aunt = NULL;

	// (1)
	TreeNode* parent = node->get_parent();
	while (parent && parent->is_red()) {
		gp = node->get_grand_parent();

		// (1.1.1)
		if (!gp) {
			break;
		}

		// (1.2)
		if (parent->is_left_of(gp)) {
			// (1.2.1)
			aunt = gp->get_right();

			// (1.2.2)
			if (aunt && aunt->is_red()) {
				parent->set_attr_to_black();
				aunt->set_attr_to_black();
				gp->set_attr_to_red();

				node = gp;
			} else {
				// (1.2.3)
				// (1.2.3.1)
				if (node->is_right_of(parent)) {
					node = parent;
					root = TREE_ROTATE_LEFT(root, node);
				}

				node->get_parent()->set_attr_to_black();
				gp->set_attr_to_red();

				root = TREE_ROTATE_RIGHT(root, gp);
			}
		} else {
			// (1.3.1)
			aunt = gp->get_left();

			// (1.3.2)
			if (aunt && aunt->is_red()) {
				parent->set_attr_to_black();
				aunt->set_attr_to_black();
				gp->set_attr_to_red();

				node = gp;
			} else {
				// (1.3.3)
				// (1.3.3.1)
				if (node->is_left_of(parent)) {
					node = parent;
					root = TREE_ROTATE_RIGHT(root, node);
				}

				node->get_parent()->set_attr_to_black();
				gp->set_attr_to_red();

				root = TREE_ROTATE_LEFT(root, gp);
			}
		}

		// (1.4)
		parent = node->get_parent();
	}

	root->set_attr_to_black();

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
		node->set_attr_to_black();
		set_root_unsafe(node);

		unlock();
		return node;
	}

	node->set_attr_to_red();

	while (p) {
		top = p;
		s = _fn_cmp(node->get_content(), p->get_content());

		if (s == 0) {
			if (replace) {
				p->replace_content(node);

				unlock();
				return p;
			}
			s = 1;
		}

		if (s < 0) {
			p = p->get_left();
		} else {
			p = p->get_right();
		}
	}

	if (s < 0) {
		top->set_left(node);
	} else {
		top->set_right(node);
	}

	node->set_parent(top);
	set_root_unsafe(INSERT_FIXUP(_root, node));
	unlock();
	return node;
}

void RBT::_do_rebalance(TreeNode* x)
{
	TreeNode* parent = NULL;
	TreeNode* sibling = NULL;
	TreeNode* siblingr = NULL;
	TreeNode* siblingl = NULL;

	while (x != _root) {
		parent = x->get_parent();

		if (x->is_left_of(parent)) {
			sibling = parent->get_right();

			if (!sibling) {
				break;
			}

			if (sibling->is_red()) {
				set_root_unsafe(TREE_ROTATE_LEFT(_root, parent));

				sibling->set_attr_to_black();

				if (parent->_right) {
					parent->_right->set_attr_to_red();
				}

				return;
			}

			siblingr = sibling->get_right();
			if (siblingr && siblingr->is_red()) {
				set_root_unsafe(TREE_ROTATE_LEFT(_root, parent));

				if (sibling->have_red_childs()) {
					sibling->set_attr_to_red();
				} else {
					sibling->set_attr_to_black();
				}
				sibling->set_childs_attr_to_black();

				return;
			}

			siblingl = sibling->get_left();
			if (siblingl && siblingl->is_red()) {
				set_root_unsafe(TREE_ROTATE_RIGHT(_root, sibling));
				sibling->swap_attr(siblingl);
				continue;
			}

			if (sibling->have_no_childs()
			||  sibling->have_black_childs()) {
				sibling->set_attr_to_red();

				if (parent->is_red()) {
					parent->set_attr_to_black();
					return;
				}

				if (parent == _root) {
					return;
				}
			}
		} else {
			sibling = parent->get_left();

			if (!sibling) {
				break;
			}

			if (sibling->is_red()) {
				set_root_unsafe(TREE_ROTATE_RIGHT(_root, parent));

				sibling->set_attr_to_black();

				if (parent->_left) {
					parent->_left->set_attr_to_red();
				}

				return;
			}

			siblingl = sibling->get_left();
			if (siblingl && siblingl->is_red()) {
				set_root_unsafe(TREE_ROTATE_RIGHT(_root, parent));

				if (sibling->have_red_childs()) {
					sibling->set_attr_to_red();
				} else {
					sibling->set_attr_to_black();
				}
				sibling->set_childs_attr_to_black();

				return;
			}

			siblingr = sibling->get_right();
			if (siblingr && siblingr->is_red()) {
				set_root_unsafe(TREE_ROTATE_LEFT(_root, sibling));
				sibling->swap_attr(siblingr);
				continue;
			}

			if (sibling->have_no_childs()
			||  sibling->have_black_childs()) {
				sibling->set_attr_to_red();

				if (parent->is_red()) {
					parent->set_attr_to_black();
					return;
				}

				if (parent == _root) {
					return;
				}
			}
		}

		x = parent;
	}

	_root->set_childs_attr_to_black();
}

TreeNode* RBT::_removed_have_no_child(TreeNode* x)
{
	TreeNode* parent = x->get_parent();

	if (!parent) {
		set_root_unsafe(NULL);
		x->detach();
		return x;
	}

	if (x->is_black()) {
		_do_rebalance(x);
	}

	if (x->is_left_of(parent)) {
		parent->set_left((TreeNode*) NULL);
	} else {
		parent->set_right((TreeNode*) NULL);
	}

	x->detach();

	return x;
}

TreeNode* RBT::_removed_have_both_childs(TreeNode* x)
{
	TreeNode* heir = x->get_left();

	// Find the largest on the left.
	while (heir->get_right()) {
		heir = heir->get_right();
	}

	swap_content(x, heir);

	return _remove_unsafe(heir);
}

TreeNode* RBT::_remove_unsafe(TreeNode* x)
{
	if (!x) {
		return NULL;
	}

	TreeNode* left = x->get_left();
	TreeNode* right = x->get_right();

	if (!left && !right) {
		return _removed_have_no_child(x);
	}
	if (left && !right) {
		swap_content(x, left);
		return _removed_have_no_child(left);
	}
	if (right && !left) {
		swap_content(x, right);
		return _removed_have_no_child(right);
	}

	return _removed_have_both_childs(x);
}

/**
 * `remove(o)` will remove node `o` from tree and return it.
 */
TreeNode* RBT::remove(TreeNode* x)
{
	if (!x) {
		return NULL;
	}

	lock();

	TreeNode* del = _remove_unsafe(x);

	unlock();
	return del;
}

/**
 * `find()` will search node in tree that have the same item. Return the node
 * object if found or NULL if not found.
 */
TreeNode* RBT::find(Object* item)
{
	int s = 0;

	lock();

	TreeNode* p = get_root_unsafe();

	while (p) {
		s = _fn_cmp(item, p->get_content());
		if (s == 0) {
			goto out;
		}
		if (s < 0) {
			p = p->get_left();
		} else {
			p = p->get_right();
		}
	}
out:
	unlock();
	return p;
}

/**
 * `_check_balance()` will count number of black node in each path. It will
 * return 1, if all path is balanced or 0 otherwise.
 */
int RBT::_check_balance(TreeNode* p, int nblack)
{
	if (!p) {
		return 1;
	}

	if (p->is_black()) {
		nblack++;
	}

	if (p->_left) {
		_check_balance(p->_left, nblack);
	}

	if (!p->_left && !p->_right) {
		if (_n_black < 0) {
			_n_black = nblack;
		} else {
			if (_n_black != nblack) {
				_n_black_valid = 0;
				return 0;
			}
		}
	}

	if (p->_right) {
		_check_balance(p->_right, nblack);
	}

	return 1;
}

/**
 * `is_balance()` will return `1` if number of every path from root to
 * each leave has the same number of black nodes; otherwise it will return
 * `0`.
 */
int RBT::is_balance()
{
	lock();
	_n_black = -1;
	_n_black_valid = _check_balance(_root, 0);
	unlock();
	return _n_black_valid;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
