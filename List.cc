//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "List.hh"
#include "Buffer.hh"

namespace vos {

const char* List::__cname = "List";

List::List(const char sep) : Locker()
,	_head(NULL)
,	_tail(NULL)
,	_n(0)
,	_sep(',')
{
	if (sep > 0) {
		_sep = sep;
	}
}

List::~List()
{
	reset();

	lock();

	if (__str) {
		free(__str);
		__str = NULL;
	}

	unlock();
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
// `insert_before_unsafe()` will insert node `x` before node `y` in the list.
// If node `y` is head then `x` will be new head.
//
void List::insert_before_unsafe(BNode* x, BNode* y)
{
	x->_right = y;
	x->_left = y->_left;

	y->_left->_right = x;
	y->_left = x;

	if (y == _head) {
		_head = x;
	}
	_n++;
}

//
// `insert_after_unsafe()` will insert node `x` after node `y` in the list.
// If node `y` is tail then `x` will be the new tail.
//
void List::insert_after_unsafe(BNode* x, BNode* y)
{
	x->_right = y->_right;
	x->_left = y;

	y->_right->_left = x;
	y->_right = x;

	if (y == _tail) {
		_tail = x;
	}
	_n++;
}

//
// `push_circular()` will push an `item` into circular list, and set `p` point
// to the node. `p` could be head or tail.
// It will return node object that has been pushed to the list.
//
BNode* List::push_circular(BNode** p, Object* item)
{
	lock();

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

	unlock();

	return node;
}

//
// `push_head()` will add new `item` as the first node in the list.
// It will return node object that has been pushed to the list, or NULL if
// `item` itself is NULL.
//
BNode* List::push_head(Object* item)
{
	if (!item) {
		return NULL;
	}

	return push_circular(&_head, item);
}

//
// `push_head_sorted()` will insert new item into list in sorted order,
// default to ascending, start from the head.
// If `fn_cmp` is NULL, it will use the `Object.cmp` method.
//
BNode* List::push_head_sorted(Object* item, int asc
				, int (*fn_cmp)(Object*, Object*))
{
	BNode* node = new BNode(item);

	node_push_head_sorted(node, asc, fn_cmp);

	return node;
}

//
// `push_tail()` will add new `item` to the end of the list.
// It will return node object that has been pushed to the list, or NULL if
// `item` itself is NULL.
//
BNode* List::push_tail(Object* item)
{
	if (!item) {
		return NULL;
	}

	return push_circular(&_tail, item);
}

//
// `push_tail_sorted()` will insert new item into list in sorted order,
// default to ascending, started from the tail.
// If `fn_cmp` is NULL, it will use the `Object.cmp` method.
//
BNode* List::push_tail_sorted(Object* item, int asc
				, int (*fn_cmp)(Object*, Object*))
{
	BNode* node = new BNode(item);

	node_push_tail_sorted(node, asc, fn_cmp);

	return node;
}

//
// `reset()` will remove all items in the list.
//
void List::reset()
{
	lock();

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
	_n = 0;

	unlock();
}

//
// `swap_by_idx_unsafe` will swap content of node at index `x` with node at
// index `y`.  This function is not thread safe.
//
// (0) If both node or one of the node is NULL, no swap will be happened.
// (1) If both node point to the same object, due to circular linked list,
//     no swap will be happened.
//
void List::swap_by_idx_unsafe(int x, int y)
{
	BNode* nx = node_at_unsafe(x);
	BNode* ny = node_at_unsafe(y);
	Object* item = NULL;

	if (nx == NULL || ny == NULL) {
		return;
	}
	if (nx == ny) {
		return;
	}

	item = nx->_item;
	nx->_item = ny->_item;
	ny->_item = item;
}

//
// `sort_conqueror()` will merge two sorted list, `left` and `right` into one
// list.
//
void List::sort_conqueror(List* left, List* right
			, int (*fn_compare)(Object*, Object*), int asc)

{
	int s = 0;
	Object* tmp;

	BNode* pleft = left->_head;
	BNode* pright = right->_head;
	BNode* endleft = right->_head;
	BNode* endright = right->_tail->_right;

	while (pleft != endleft && pright != endright && pleft != pright) {
		s = fn_compare(pleft->_item, pright->_item);
		if (s == 0) {
			goto next;
		}
		if (asc) {
			// left < right
			if (s == -1) {
				pleft = pleft->_right;
				goto checkend;
			}
		} else {
			// left > right
			if (s == 1) {
				pleft = pleft->_right;
				goto checkend;
			}
		}

		// swap
		tmp = pleft->_item;
		pleft->_item = pright->_item;
		pright->_item = tmp;
next:
		pleft = pleft->_right;
		pright = pright->_right;

checkend:
		if (pleft == endleft) {
			if (pright != endright) {
				pleft = pright;
				pright = pright->_right;
				endleft = endright;
			}
		}
	}
}

//
// `sort_divide()` will divide the list into two parts, left and right, where
// left and right contain half of the list.
//
// This function assume that the list size is greater than 2.
//
void List::sort_divide(int (*fn_compare)(Object*, Object*), int asc)
{
	List left;
	List right;
	int mid = _n / 2;
	int n = 1;

	left._head = _head;
	left._tail = _head;
	left._n = mid;
	for (; n < mid; n++) {
		left._tail = left._tail->_right;
	}

	right._head = left._tail->_right;
	right._tail = _tail;
	right._n = _n - mid;

	left.sort(fn_compare, asc);
	right.sort(fn_compare, asc);

	int s = fn_compare(left._tail->_item, right._head->_item);

	if (s == 0) {
		return;
	}
	if (asc) {
		// left < right, its already sorted.
		if (s == -1) {
			return;
		}
	} else {
		// left > right, its already sorted.
		if (s == 1) {
			return;
		}
	}

	sort_conqueror(&left, &right, fn_compare, asc);

	// Reset left and right so it does not get destroyed.
	left._head = left._tail = NULL;
	right._head = right._tail = NULL;
}

//
// `sort()` will sort all items in ascending order using function pointed
// by `fn_compare`.
//
void List::sort(int (*fn_compare)(Object*, Object*), int asc)
{
	lock();

	int s = 0;

	if (_n <= 0) {
		goto out;
	}
	if (_n == 1) {
		goto out;
	}
	if (_n == 2) {
		s = (*fn_compare)(at_unsafe(0), at_unsafe(1));
		if (s == 0) {
			goto out;
		}
		if (asc) {
			// 0 > 1
			if (s == 1) {
				swap_by_idx_unsafe(0, 1);
			}
		} else {
			// 0 < 1
			if (s == -1) {
				swap_by_idx_unsafe(0, 1);
			}
		}
		goto out;
	}

	sort_divide(fn_compare, asc);
out:
	unlock();
}

//
// `detach()` will detach `node` from the list.
//
// (0) If `node` is head then the right node will be the new head.
// (1) If `node` is tail then the left node will be the new tail.
//
void List::detach(BNode* node)
{
	lock();

	if (_head == _tail) {
		_head = NULL;
		_tail = NULL;
		goto out;
	}

	node->_right->_left = node->_left;
	node->_left->_right = node->_right;

	// (0)
	if (node == _head) {
		_head = node->_right;
	}

	// (2)
	if (node == _tail) {
		_tail = node->_left;
	}

out:
	node->_right = NULL;
	node->_left = NULL;
	_n--;

	unlock();
}


//
// `node_push_head_sorted()` will insert new node into the list in sorted
// order, default to ascending (`asc` value 1), started from the head.
// If `fn_cmp` is NULL, it will use the `Object.cmp` method.
//
void List::node_push_head_sorted(BNode* node, int asc
				, int (*fn_cmp)(Object*, Object*))
{
	int s = 0;
	BNode* p = NULL;

	lock();

	if (!_head) {
		first_push(node);
		goto out;
	}

	p = _head;

	do {
		if (fn_cmp) {
			s = fn_cmp(p->_item, node->_item);
		} else {
			s = p->_item->cmp(node->_item);
		}

		// P > ITEM
		if (s > 0) {
			if (asc) {
				insert_before_unsafe(node, p);
				break;
			}
		}

		// ITEM > P
		if (s < 0) {
			if (!asc) { // desc
				insert_before_unsafe(node, p);
				break;
			}
		}

		p = p->_right;
	} while(p != _head);

	// node has not been inserted, push it to the bottom.
	if (!node->_left) {
		insert_after_unsafe(node, _tail);
	}
out:
	unlock();
}


//
// `node_push_tail_sorted()` will insert new node into the list in sorted
// order, default to ascending (`asc` value 1), started from the tail.
// If `fn_cmp` is NULL, it will use the `Object.cmp` method.
//
void List::node_push_tail_sorted(BNode* node, int asc
				, int (*fn_cmp)(Object*, Object*))
{
	int s = 0;
	BNode* p = NULL;

	lock();

	if (!_tail) {
		first_push(node);
		goto out;
	}

	p = _tail;

	do {
		if (fn_cmp) {
			s = fn_cmp(p->_item, node->_item);
		} else {
			s = p->_item->cmp(node->_item);
		}

		// P > ITEM
		if (s > 0) {
			if (!asc) { // desc
				insert_after_unsafe(node, p);
				break;
			}
		}

		// ITEM > P
		if (s < 0) {
			if (asc) { // desc
				insert_after_unsafe(node, p);
				break;
			}
		}

		p = p->_left;
	} while(p != _tail);

	// node has not been inserted, push it to the top.
	if (!node->_left) {
		insert_before_unsafe(node, _head);
	}
out:
	unlock();
}


//
// `node_search()` will find the `item` in the list that match using
// `fn_compare` as comparison. It will return node object on the first item
// that match, or `NULL` otherwise.
//
// Class that use List must pass compare function or implement `cmp()`.
//
BNode* List::node_search(Object* item, int (*fn_compare)(Object*, Object*))
{
	if (!_head) {
		return NULL;
	}

	lock();

	int s = 0;
	BNode* bnode = _head;

	do {
		if (fn_compare) {
			s = fn_compare(bnode->_item, item);
		} else {
			s = bnode->_item->cmp(item);
		}
		if (s == 0) {
			goto out;
		}

		bnode = bnode->_right;
	} while(bnode != _head);

	bnode = NULL;
out:
	unlock();

	return bnode;
}

//
// `node_pop_head()` will remove the first node in the list and return it.
//
BNode* List::node_pop_head()
{
	lock();

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

	oldhead->_left = NULL;
	oldhead->_right = NULL;

	_n--;
out:
	unlock();

	return oldhead;
}

//
// `node_pop_tail()` will remove that last node on the list and return it.
//
BNode* List::node_pop_tail()
{
	lock();

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

	oldtail->_left = NULL;
	oldtail->_right = NULL;

	_n--;
out:
	unlock();

	return oldtail;

}

//
// `node_at()` return node in list at index `idx`, started from head to tail.
//  (0) It will return NULL if index is empty
//  (1) Index 0 is equal with `_head`,
//  (2) If `idx` is negative, node will be search in reverse order (from
//  tail to head).
//  (3) Index -1 is equal with `_tail`, and so on.
//
//	        _head <=> node <=> ... <=> _tail => _head
//	+idx      0         +1     ...      n-1      n
//	-idx     -n       -n+1     ...       -1      0
//
BNode* List::node_at(int idx)
{
	lock();

	BNode *p = node_at_unsafe(idx);

	unlock();

	return p;
}

//
// `node_at_unsafe` will return node in list at index `idx`. Unsafe operation
// in multithread environment.
//
BNode* List::node_at_unsafe(int idx)
{
	uint8_t rev = 0;
	BNode *p = NULL;

	// (0)
	if (_n == 0) {
		return p;
	}

	// (1)
	if (idx == 0) {
		return _head;
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

	return p;
}

//
// `pop_head()` will remove the first node in the list and return the item
// that contains in it.
//
Object* List::pop_head()
{
	Object* item = NULL;
	BNode* oldhead = node_pop_head();

	if (!oldhead) {
		return NULL;
	}

	item = oldhead->_item;
	oldhead->_item = NULL;
	delete oldhead;

	return item;
}

//
// `pop_tail()` will remove the last node in the list, and return the item
// that contains in it.
//
Object* List::pop_tail()
{
	Object* item = NULL;
	BNode* oldtail = node_pop_tail();

	if (!oldtail) {
		return NULL;
	}

	item = oldtail->_item;
	oldtail->_item = NULL;
	delete oldtail;

	return item;
}

//
// `node_remove_unsafe()` will remove node from list.
// This is pthread unsafe operation, without locking/unlocking on list mutex.
//
// (C0) Node is the only item in the list
// (C1) Node is the head
// (C2) Node is also the tail
// (C3) Node is in the middle
//
Object* List::node_remove_unsafe(BNode* bnode)
{
	if (!bnode) {
		return NULL;
	}

	Object* item = bnode->_item;

	// (C0)
	if (_head == _tail) {
		_head = NULL;
		_tail = NULL;
	} else {
		// (C1)
		if (bnode == _head) {
			_head = _head->_right;
		}
		// (C2)
		if (bnode == _tail) {
			_tail = _tail->_left;
		}
		// (C3)
		bnode->_left->_right = bnode->_right;
		bnode->_right->_left = bnode->_left;
	}

	bnode->_left = NULL;
	bnode->_right = NULL;
	bnode->_item = NULL;

	delete bnode;
	_n--;

	return item;
}

//
// `remove()` will find object in list that matched with `item`, and remove
// their node object, the content of item will not be removed.
// If `item` found it will return 0, otherwise return 1.
//
// Conditions,
// (C0) Item is null or list is empty, return 1
// (C1) No item found, return 1
//
int List::remove(Object* item)
{
	lock();

	int s = 1;
	int x = 0;
	BNode* p = _head;

	// (C0)
	if (!item || !_head) {
		goto out;
	}

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

	node_remove_unsafe(p);
	s = 0;
out:
	unlock();
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
	BNode *p = node_at(idx);

	if (!p) {
		return NULL;
	}

	return p->_item;
}

Object* List::at_unsafe(int idx)
{
	BNode *p = node_at_unsafe(idx);

	if (!p) {
		return NULL;
	}

	return p->_item;
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
	lock();

	Buffer b;
	const char* p = NULL;
	BNode* node = _head;

	if (__str) {
		free(__str);
		__str = NULL;
	}

	if (!_head) {
		goto out;
	}

	b.append_raw("[ ");

	do {
		p = node->chars();
		if (p && p[0] != '{' && p[0] != '[') {
			b.concat("\"", p, "\"", 0);
		} else {
			b.append_raw(p);
		}

		node = node->_right;
		if (node != _head) {
			b.appendc(_sep);
		}
	} while (node != _tail->_right);

	b.append_raw(" ]");

	__str = b.detach();

out:
	unlock();
	return __str;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
