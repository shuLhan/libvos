//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "List.hh"
#include "Buffer.hh"

namespace vos {

const char* List::__cname = "List";

List::List(const char sep) : Object()
,	_head(NULL)
,	_tail(NULL)
,	_n(0)
,	_sep(',')
,	_locker()
{
	if (sep > 0) {
		_sep = sep;
	}
}

List::~List()
{
	_locker.lock();

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

	_locker.unlock();
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
	_locker.lock();

	BNode* node = new BNode(item);

	if (!_head) {
		first_push(node);
	} else {
		node->_left = _tail;
		node->_right = _head;

		_tail->_right = node;
		_head->_left = node;

		_n++;
		(*p) = node;
	}

	_locker.unlock();
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
	_locker.lock();

	Object* item = NULL;
	BNode* oldhead = NULL;

	if (!_head) {
		goto out;
	}

	oldhead = _head;

	if (_head == _tail) {
		_head = NULL;
		_tail = NULL;
	} else {
		_head = oldhead->_right;

		_tail->_right = _head;
		_head->_left = _tail;
	}

	item = oldhead->_item;
	oldhead->_item = NULL;
	delete oldhead;
	_n--;

out:
	_locker.unlock();
	return item;
}

//
// `pop_tail()` will remove the last node in the list, and return the item
// that contains in it.
//
Object* List::pop_tail()
{
	_locker.lock();

	Object* item = NULL;
	BNode* oldtail = NULL;

	if (!_tail) {
		goto out;
	}

	oldtail = _tail;

	if (_head == _tail) {
		_head = NULL;
		_tail = NULL;
	} else {
		_tail = oldtail->_left;

		_tail->_right = _head;
		_head->_left = _tail;
	}

	item = oldtail->_item;
	oldtail->_item = NULL;
	delete oldtail;
	_n--;

out:
	_locker.unlock();
	return item;
}

//
// `remove()` will find object in list that matched with `item`, and remove
// their item, the content of item will not be removed.
// If object found and removed it will return 0, otherwise return 1.
//
// Conditions,
// (C0) Item is null or list is empty, return 1
// (C1) No item found, return 1
// (C2) Item found, but its the only item in the list
// (C3) Item found, and its also the head
// (C4) Item found, and its also the tail
// (C5) Item found, and its maybe in the middle
//
int List::remove(Object* item)
{
	_locker.lock();

	register int s = 1;
	register int x = 0;
	BNode* p = NULL;

	// (C0)
	if (! item || !_head) {
		goto out;
	}

	p = _head;

	for (; x < _n; x++) {
		if (p->_item == item) {
			break;
		}

		p = p->_right;
	}

	// (C1)
	if (x >= _n) {
		goto out;
	}

	// (C2)
	if (_head == _tail) {
		_head = NULL;
		_tail = NULL;
	} else {
		// (C3)
		if (p == _head) {
			_head = _head->_right;
		}
		// (C4)
		if (p == _tail) {
			_tail = _tail->_left;
		}
		// (C5)
		p->_left->_right = p->_right;
		p->_right->_left = p->_left;
	}

	p->_left = NULL;
	p->_right = NULL;
	p->_item = NULL;

	delete p;
	_n--;
	s = 0;
out:
	_locker.unlock();
	return s;
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
	_locker.lock();

	uint8_t rev = 0;
	Object* item = NULL;
	BNode *p = NULL;

	// (0)
	if (_n == 0) {
		goto out;
	}

	// (1)
	if (idx == 0) {
		item = _head->_item;
		goto out;
	}

	p = _head;

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

	item = p->_item;
out:
	_locker.unlock();
	return item;
}

//
// `size()` will return number of node in list.
//
int List::size()
{
	return _n;
}

//
// `chars()` will return string presentation of list where each node is
// printed from left to right and separated by `_sep` character.
// The returned string is in JSON array format.
//
const char* List::chars()
{
	_locker.lock();

	Buffer b;
	BNode* node = _head;

	if (_v) {
		free(_v);
		_v = NULL;
	}

	if (!_head) {
		goto out_unlock;
	}

	if (_head == _tail) {
		b.concat("[ \"", _head->chars(), "\" ]", NULL);
		goto out;
	}

	b.append_raw("[ ");
	while (node != _tail) {
		b.concat("\"", node->chars(), "\"", NULL);
		b.appendc(_sep);

		node = node->_right;
	}
	b.concat("\"", node->chars(), "\"", NULL);
	b.append_raw(" ]");

out:
	_v = b._v;
	b._v = NULL;

out_unlock:
	_locker.unlock();
	return _v;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
