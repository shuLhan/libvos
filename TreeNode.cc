//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "TreeNode.hh"
#include "Buffer.hh"

namespace vos {

const char* TreeNode::__cname = "TreeNode";

//
// `TreeNode(item)` will create new TreeNode object that contain `item`.
//
TreeNode::TreeNode(Object* item) : BNode(item)
,	_attr(0)
,	_left(NULL)
,	_right(NULL)
,	_top(NULL)
{}

//
// `~TreeNode()` will set all fields to NULL. Note that, the destructor will
// not release any memory allocated in left, right, or top; that is the job
// for class that use the TreeNode module.
//
TreeNode::~TreeNode()
{
	_attr = 0;
	detach();
}

//
// `detach()` will unlink all node pointers to other nodes, make it detached
// from tree.
//
void TreeNode::detach()
{
	set_left((TreeNode*) NULL);
	set_right((TreeNode*) NULL);
	set_parent(NULL);
}

//
// `set_attr()` will set the attribute to `x`.
//
void TreeNode::set_attr(int x)
{
	_attr = x;
}

//
// `set_attr_to_red` will set current attribute to RED.
//
void TreeNode::set_attr_to_red()
{
	set_attr(RBT_IS_RED);
}

//
// `set_attr_to_black` will set current attribute to BLACK.
//
void TreeNode::set_attr_to_black()
{
	set_attr(RBT_IS_BLACK);
}

//
// `get_attr()` will return current node attribute.
//
int TreeNode::get_attr()
{
	return _attr;
}

//
// `swap_attr` will swap current attribute with `node`.
//
void TreeNode::swap_attr(TreeNode* node)
{
	int tmp = node->get_attr();
	node->set_attr(get_attr());
	set_attr(tmp);
}

//
// `have_attr()` will return non-zero value if attribute contain `x`.
//
int TreeNode::have_attr(int x)
{
	return (_attr & x);
}

//
// `is_red()` will return non-zero value if attribute is RBT RED.
//
int TreeNode::is_red()
{
	return (_attr & RBT_IS_RED);
}

//
// `is_black()` will return non-zero value if attribute is RBT BLACK.
//
int TreeNode::is_black()
{
	return (_attr & RBT_IS_BLACK);
}

//
// `is_left_red()` will return non-zero if left node is RED, or `0` otherwise.
//
int TreeNode::is_left_red()
{
	if (_left && _left->is_red()) {
		return 1;
	}
	return 0;
}

//
// `is_right_red()` will return non-zero if right node is RED, or `0`
// otherwise.
//
int TreeNode::is_right_red()
{
	if (_right && _right->is_red()) {
		return 1;
	}
	return 0;
}

//
// `set_left(o)` will set the left child point to `o`.
//
void TreeNode::set_left(TreeNode* o)
{
	_left = o;
}

//
// `get_left()` will return pointer to the left child.
//
TreeNode* TreeNode::get_left()
{
	return _left;
}

//
// `set_right(o)` will set the right child point to `o`.
//
void TreeNode::set_right(TreeNode* o)
{
	_right = o;
}

//
// `get_right()` will return pointer to the right child.
// 
TreeNode* TreeNode::get_right()
{
	return _right;
}

//
// `set_parent()` will set the parent to `o`.
//
void TreeNode::set_parent(TreeNode* o)
{
	_top = o;
}

//
// `get_parent()` will return pointer to parent object.
//
TreeNode* TreeNode::get_parent()
{
	return _top;
}

//
// `get_grand_parent()` will return pointer to parent of parent of object.
//
TreeNode* TreeNode::get_grand_parent()
{
	if (!_top) {
		return NULL;
	}
	return _top->_top;
}

//
// `have_no_childs()` will return 1 if node does not any childs; or 0 if node
// have one child or both childs.
//
int TreeNode::have_no_childs()
{
	if (_left || _right) {
		return 0;
	}
	return 1;
}

//
// `have_red_childs` will return 1 if both of its childs are RED, otherwise
// it will return 0.
//
int TreeNode::have_red_childs()
{
	if ((_left && _left->is_red()) && (_right && _right->is_red())) {
		return 1;
	}
	return 0;
}

//
// `have_black_childs` will return 1 if both of its childs are BLACK,
// otherwise it will return 0.
//
int TreeNode::have_black_childs()
{
	if ((_left && _left->is_black()) && (_right && _right->is_black())) {
		return 1;
	}
	return 0;
}

//
// `set_childs_attr` will set the attribute of both of child to `x`.
//
void TreeNode::set_childs_attr(int x)
{
	if (_left) {
		_left->set_attr(x);
	}
	if (_right) {
		_right->set_attr(x);
	}
}

//
// `set_childs_attr_to_red` will set the attribute of both of child to RED.
//
void TreeNode::set_childs_attr_to_red()
{
	set_childs_attr(RBT_IS_RED);
}

//
// `set_childs_attr_to_black` will set the attribute of both of child to
// BLACK.
//
void TreeNode::set_childs_attr_to_black()
{
	set_childs_attr(RBT_IS_BLACK);
}

//
// `insert_left()` will insert `node` to left-child. If current left
// child is not empty then it will be the left-child of `node`.
//
void TreeNode::insert_left(TreeNode* node)
{
	node->set_left(_left);
	node->set_parent(this);

	if (_left) {
		_left->set_parent(node);
	}

	_left = node;
}

//
// `insert_right()` will insert `node` to right-child. If current right
// child is not empty then it will be the right-child of `node`.
//
void TreeNode::insert_right(TreeNode* node)
{
	node->set_right(_right);
	node->set_parent(this);

	if (_right) {
		_right->set_parent(node);
	}

	_right = node;
}

//
// `chars()` will return representation of object as string.
//
const char* TreeNode::chars()
{
	if (!_item) {
		return NULL;
	}
	if (__str) {
		free(__str);
		__str = 0;
	}

	Buffer b;

	if (_attr) {
		b.aprint("{ %d:\"%s\" }", _attr, _item->chars());
	} else {
		b.append_raw(_item->chars());
	}

	__str = b._v;
	b._v = NULL;

	return __str;
}

//
// `CMP()` will compare content of object `x` with content of object `y`, and
// return,
//
// - `0` if both objects are null or have the same content;
// - `-1`, if `x` is NULL or content of `x < y`;
// - `1` if `y` is NULL or content if `x > y`.
//
int TreeNode::CMP(TreeNode* x, TreeNode* y)
{
	if (!x && !y) {
		return 0;
	}
	if (!x) {
		return -1;
	}
	if (!y) {
		return 1;
	}
	return strcmp(x->get_content()->chars(), y->get_content()->chars());
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
