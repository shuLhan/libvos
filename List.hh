//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_LIST_HH
#define _LIBVOS_LIST_HH 1

#include "Locker.hh"
#include "BNode.hh"

namespace vos {

//
// List is generic implementation of circular double linked list.
//
class List : public Object {
public:
	List(const char sep = ',');
	virtual ~List();

	void first_push(BNode* node);
	void push_circular(BNode** p, Object* item);
	void push_head(Object* item);
	void push_tail(Object* item);
	void reset();
	void swap_by_idx_unsafe(int x, int y);
	void sort(int (*fn_compare)(Object* x, Object* y), int asc=1);
	BNode* node_at(int idx);
	BNode* node_at_unsafe(int idx);
	Object* pop_head();
	Object* pop_tail();
	Object* at(int idx);
	Object* at_unsafe(int idx);
	int remove(Object* item);
	int size();
	const char* chars();

	// _head is pointer to the first node in the list.
	BNode* _head;

	// _tail is pointer to the last node in the list.
	BNode* _tail;

	// `__cname` contain canonical name of this object.
	static const char* __cname;
protected:
	// _n will contain number of node in the list.
	int _n;

	// _sep is used to separate node when calling `chars`.
	char _sep;

	// _locker will lock list every call to push or pop.
	Locker _locker;
private:
	void sort_divide(int (*fn_compare)(Object*, Object*), int asc);
	void sort_conqueror(List* left, List* right
			, int (*fn_compare)(Object*, Object*), int asc);

	List(const List&);
	void operator=(const List&);
};

typedef List Queue;

} // namespace vos
#endif // _LIBVOS_LIST_HH
// vi: ts=8 sw=8 tw=78:
