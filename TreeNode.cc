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
	_left = NULL;
	_right = NULL;
	_top = NULL;
}

//
// `set_attr()` will set the attribute to `x`.
//
void TreeNode::set_attr(int x)
{
	_attr = x;
}

//
// `have_attr()` will return non-zero value if attribute contain `x`.
//
int TreeNode::have_attr(int x)
{
	return (_attr & x);
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
// `insert_left()` will insert `o` to left child. If current left
// child is not empty then it will be the left child of `o`.
//
void TreeNode::insert_left(TreeNode* o)
{
	o->set_left(_left);
	o->set_parent(this);

	if (_left) {
		_left->set_parent(o);
	}

	_left = o;
}

//
// `chars()` will return representation of object as string.
//
const char* TreeNode::chars()
{
	if (!_item) {
		return NULL;
	}
	if (_v) {
		free(_v);
		_v = 0;
	}

	Buffer b;

	if (_attr) {
		b.aprint("{ %d:\"%s\" }", _attr, _item->chars());
	} else {
		b.append_raw(_item->chars());
	}

	_v = b._v;
	b._v = NULL;

	return _v;
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
