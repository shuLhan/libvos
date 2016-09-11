//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "List.hh"
#include "Buffer.hh"

namespace vos {

const char* List::__cname = "List";

List::List() : Object()
,	_head(NULL)
,	_tail(NULL)
,	_n(0)
,	_sep(',')
{}

List::~List()
{
	_n = 0;
	if (_tail) {
		// break circular link.
		_tail->_right = NULL;
		_head->_left = NULL;
	}
	while (_head) {
		_tail = _head->_right;
		delete _head;
		_head = _tail;
	}
	if (_v) {
		free(_v);
		_v = NULL;
	}
}

//
// `first_push()` will set head and tail to node.
//
void List::first_push(BNode* node)
{
	_head = node;
	_tail = node;
	_head->_left = _tail;
	_tail->_right = _head;
	_n++;
}

//
// `push_circular()` will push an `item` into circular list, and set `p` point
// to the node. `p` could be head or tail.
//
void List::push_circular(BNode** p, Object* item)
{
	BNode* node = new BNode(item);

	if (!_head) {
		first_push(node);
		return;
	}

	node->_left = _tail;
	node->_right = _head;

	_tail->_right = node;
	_head->_left = node;

	_n++;
	(*p) = node;
}

//
// `push_head()` will add new `item` as the first node in the list.
//
void List::push_head(Object* item)
{
	if (!item) {
		return;
	}

	push_circular(&_head, item);
}

//
// `push_tail()` will add new `item` to the end of the list.
//
void List::push_tail(Object* item)
{
	if (!item) {
		return;
	}

	push_circular(&_tail, item);
}

//
// `pop_head()` will remove the first node in the list and return the item
// that contains in it.
//
Object* List::pop_head()
{
	if (!_head) {
		return NULL;
	}

	BNode* oldhead = _head;

	if (_head == _tail) {
		_head = NULL;
		_tail = NULL;
	} else {
		_head = oldhead->_right;

		_tail->_right = _head;
		_head->_left = _tail;
	}

	Object* item = oldhead->_item;
	oldhead->_item = NULL;
	delete oldhead;
	_n--;

	return item;
}

//
// `pop_tail()` will remove the last node in the list, and return the item
// that contains in it.
//
Object* List::pop_tail()
{
	if (!_tail) {
		return NULL;
	}

	BNode* oldtail = _tail;

	if (_head == _tail) {
		_head = NULL;
		_tail = NULL;
	} else {
		_tail = oldtail->_left;

		_tail->_right = _head;
		_head->_left = _tail;
	}

	Object* item = oldtail->_item;
	oldtail->_item = NULL;
	delete oldtail;
	_n--;

	return item;
}

//
// `at()` return object in list at index `idx`, started from head to tail.
//  (0) It will return NULL if index is empty
//  (1) Index 0 is equal with `_head`,
//  (2) If `idx` is negative, object will be search in reverse order (from
//  tail to head).
//  (3) Index -1 is equal with `_tail`, and so on.
//
//	        _head <=> node <=> ... <=> _tail => _head
//	+idx      0         +1     ...      n-1      n
//	-idx     -n       -n+1     ...       -1      0
//
Object* List::at(int idx)
{
	if (_n == 0) {
		// (0.1)
		return NULL;
	}
	if (idx == 0) {
		// (1)
		return _head->_item;
	}

	uint8_t rev = 0;
	BNode *p = _head;

	// (2)
	if (idx < 0) {
		idx = idx * -1;
		rev = 1;
	}

	if (rev) {
		while (idx > 0) {
			p = p->_left;
			idx--;
		}
	} else {
		while (idx > 0) {
			p = p->_right;
			idx--;
		}
	}

	return p->_item;
}

//
// `chars()` will return string presentation of list where each node is
// printed from left to right and separated by `_sep` character.
// The returned string is in JSON array format.
//
const char* List::chars()
{
	if (_v) {
		free(_v);
		_v = NULL;
	}

	if (!_head) {
		return _v;
	}

	Buffer b;
	BNode* node = _head;

	if (_head == _tail) {
		b.concat("[\"", _head->chars(), "\"]", NULL);
		goto out;
	}

	b.appendc('[');
	while (node != _tail) {
		b.concat("\"", node->chars(), "\"", NULL);
		b.appendc(_sep);

		node = node->_right;
	}
	b.concat("\"", node->chars(), "\"", NULL);
	b.appendc(']');

out:
	_v = b._v;
	b._v = NULL;

	return _v;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
