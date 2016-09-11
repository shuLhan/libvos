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
// `push_head()` will add new `item` as the first node in the list.
//
void List::push_head(Object* item)
{
	if (!item) {
		return;
	}

	BNode* node = new BNode(item);
	_n++;

	if (!_head) {
		_head = node;
		_tail = _head;
		return;
	}

	_head->_left = node;
	node->_right = _head;
	_head = node;
}

//
// `push_tail()` will add new `item` to the end of the list.
//
void List::push_tail(Object* item)
{
	if (!item) {
		return;
	}

	BNode* node = new BNode(item);
	_n++;

	if (!_tail) {
		_head = node;
		_tail = _head;
		return;
	}

	_tail->_right = node;
	node->_left = _tail;
	_tail = node;
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

	Object* item = _head->_item;
	BNode* oldhead = _head;

	_head->_item = NULL;

	_head = oldhead->_right;
	if (!_head) {
		_tail = NULL;
	} else {
		_head->_left = NULL;
	}

	oldhead->_right = NULL;
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

	Object* item = _tail->_item;
	BNode* oldtail = _tail;

	_tail->_item = NULL;
	_tail = oldtail->_left;
	if (!_tail) {
		_head = NULL;
	} else {
		_tail->_right = NULL;
	}

	oldtail->_left = NULL;
	delete oldtail;
	_n--;

	return item;
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

	b.appendc('[');
	while (node) {
		b.concat("\"", node->chars(), "\"", NULL);

		if (node->_right) {
			b.appendc(_sep);
		}

		node = node->_right;
	}
	b.appendc(']');

	_v = b._v;
	b._v = NULL;

	return _v;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
