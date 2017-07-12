//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "BNode.hh"

namespace vos {

const char* BNode::__cname = "BNode";

/**
 * `BNode()` will create new binary node that contain `item`.
 * Note that you must pass an object that allocated with `new` not passed as
 * `&item`, because it will be deleted by this object not automatically by
 * system.
 */
BNode::BNode(Object* item) : Object()
,	_left(NULL)
,	_right(NULL)
,	_item(item)
{}

/**
 * `~BNode()` will release node memory. It is the job of BNode implementer to
 * free left and right node (see List).
 */
BNode::~BNode()
{
	_left = NULL;
	_right = NULL;

	if (_item) {
		delete _item;
		_item = NULL;
	}
}

/**
 * `delete_content()` will delete the content of node and set it to NULL.
 */
void BNode::delete_content()
{
	delete _item;
	_item = NULL;
}

/**
 * `replace_content()` will replace content of current node with content in
 * `node`. Old content will be deleted and content of `node` will be removed.
 */
void BNode::replace_content(BNode* node)
{
	delete_content();
	set_content(node->get_content());
	node->set_content(NULL);
}

/**
 * `set_content()` will set node content to `item`.
 */
void BNode::set_content(Object* item)
{
	_item = item;
}

/**
 * `get_content()` will return the content of node.
 */
Object* BNode::get_content()
{
	return _item;
}

/**
 * `swap_content()` will swap the content of node `x` with `y`.
 */
void BNode::swap_content(BNode* x)
{
	Object* x_item = x->get_content();
	x->set_content(get_content());
	set_content(x_item);
}

/**
 * `set_left()` will set the left child to `node`.
 */
void BNode::set_left(BNode* node)
{
	_left = node;
}

/**
 * `get_left()` will return the left node.
 */
BNode* BNode::get_left()
{
	return _left;
}

/**
 * `get_left_edge()` will return the outer left node of this node, or this
 * node if left is NULL.
 */
BNode* BNode::get_left_edge()
{
	BNode* p = this;
	while (p->_left != NULL) {
		p = p->_left;
	}
	return p;
}

/**
 * `set_right()` will set the right child to `node`.
 */
void BNode::set_right(BNode* node)
{
	_right = node;
}

/**
 * `get_right()` will return the right node.
 */
BNode* BNode::get_right()
{
	return _right;
}

/**
 * `get_right_edge()` will return the outer right node of this node, or this
 * node if right is NULL.
 */
BNode* BNode::get_right_edge()
{
	BNode* p = this;
	while (p->_right != NULL) {
		p = p->_right;
	}
	return p;
}

/**
 * `is_left_of()` will return `1` if current node is left child
 * of `node`.
 */
int BNode::is_left_of(BNode* node)
{
	if (node->get_left() == this) {
		return 1;
	}
	return 0;
}

/**
 * `is_right_of()` will return `1` if current node is right child of
 * `node`.
 */
int BNode::is_right_of(BNode* node)
{
	if (node->get_right() == this) {
		return 1;
	}
	return 0;
}

/**
 * `push_left()` will add new `node` to the outer left of this node.
 *
 * - If this node left pointer is not NULL then the new `node` will be
 * inserted at the edge of left node.
 *
 * - If node right is not NULL then the left edge of this node will point to
 * the right edge of node.
 *
 * Example:
 *
 * ```
 * Precondition:
 * 	- NULL <= ... <=> node <=> ... => NULL
 * 	- NULL <= ... <=> this <=> ... => NULL
 * Postcondition:
 * 	- NULL <= ... <=> node <=> ... <=> this <=> ... => NULL
 * ```
 */
void BNode::push_left(BNode* node)
{
	if (!node) {
		return;
	}

	BNode* left_edge = get_left_edge();
	BNode* node_right_edge = node->get_right_edge();

	node_right_edge->_right = left_edge;
	left_edge->_left = node_right_edge;
}

/**
 * `push_right()` will add new `node` to the outer right of this node.
 *
 * - If this node right pointer is not NULL then the new `node` will be
 * inserted at the edge of right node.
 *
 * - If node left is not NULL then the right edge of this node will point to
 * the left edge of node.
 *
 * Example:
 *
 * ```
 * Precondition:
 * 	* NULL <= ... <=> node <=> ... => NULL
 * 	* NULL <= ... <=> this <=> ... => NULL
 * Postcondition:
 * 	* NULL <= ... <=> this <=> ... <=> node <=> ... => NULL
 * ```
 */
void BNode::push_right(BNode* node)
{
	if (!node) {
		return;
	}

	BNode* right_edge = get_right_edge();
	BNode* node_left_edge = node->get_left_edge();

	node_left_edge->_left = right_edge;
	right_edge->_right = node_left_edge;
}

/**
 * `insert_left()` will add new `node` to the left pointer.
 *
 * - If this left pointer is not NULL then the new `node` will be inserted
 * between left node and this node.
 *
 * Example:
 * ```
 * Precondition:
 * 	* NULL <= ... <=> node <=> ... => NULL
 * 	* NULL <= ... <=> left-node <=> this <=> right-node => NULL
 * Postcondition:
 *	* NULL <= ... <=> left-node <=> ... <=> node <=> ... <=> this <=> ... => NULL
 * ```
 */
void BNode::insert_left(BNode* node)
{
	BNode* prev_node_left = _left;
	BNode* node_right_edge = node->get_right_edge();
	BNode* node_left_edge = node->get_left_edge();

	_left = node_right_edge;
	node_right_edge->_right = this;

	if (prev_node_left) {
		prev_node_left->_right = node_left_edge;
		node_left_edge->_left = prev_node_left;
	}
}

/**
 * `insert_right()` will insert new `node` to the right pointer.
 *
 * - If right pointer is not NULL then the new `node` will be inserted between
 * right node and this node.
 *
 * Example:
 * ```
 * Precondition:
 * 	* NULL <= ... <=> node <=> ... => NULL
 * 	* NULL <= ... <=> this <=> right-node <=> ... => NULL
 * Postcondition:
 *	* NULL <= ... <=> this <=> ... <=> node <=> ... <=> right-node <=> ... => NULL
 * ```
 */
void BNode::insert_right(BNode* node)
{
	BNode* prev_node_right = _right;
	BNode* node_left_edge = node->get_left_edge();
	BNode* node_right_edge = node->get_right_edge();

	_right = node_left_edge;
	node_left_edge->_left = this;

	if (prev_node_right) {
		prev_node_right->_left = node_right_edge;
		node_right_edge->_right = prev_node_right;
	}
}

/**
 * `pop_left_edge()` will detach the outer left node.
 *
 * (1) If left edge is this node, it will return NULL.
 */
BNode* BNode::pop_left_edge()
{
	BNode* left_edge = get_left_edge();

	// (1)
	if (left_edge == this) {
		return NULL;
	}

	left_edge->_right->_left = NULL;
	left_edge->_right = NULL;

	return left_edge;
}

/**
 * `pop_right_edge()` will detach the outer right node.
 *
 * (1) If right edge is this node, it will return NULL.
 */
BNode* BNode::pop_right_edge()
{
	BNode* right_edge = get_right_edge();

	// (1)
	if (right_edge == this) {
		return NULL;
	}

	right_edge->_left->_right = NULL;
	right_edge->_left = NULL;

	return right_edge;
}

/**
 * `chars()` will return representation of this node as string.
 */
const char* BNode::chars()
{
	if (!_item) {
		return NULL;
	}
	return _item->chars();
}

} // namespace vos
// vi: ts=8 sw=8 tw=80:
