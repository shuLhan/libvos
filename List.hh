//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_LIST_HH
#define _LIBVOS_LIST_HH 1

#include "Buffer.hh"
#include "BNode.hh"

namespace vos {

//
// List is generic implementation of double linked list.
//
class List : public Object {
public:
	List();
	virtual ~List();

	void push_head(Object* item);
	void push_tail(Object* item);
	Object* pop_head();
	Object* pop_tail();
	const char* chars();

	// _head is pointer to the first node in the list.
	BNode* _head;

	// _tail is pointer to the last node in the list.
	BNode* _tail;

	// _n will contain number of node in the list.
	int _n;

	// _sep is used to separate node when calling `chars`.
	char _sep;

	// `__cname` contain canonical name of this object.
	static const char* __cname;
private:
	List(const List&);
	void operator=(const List&);
};

} // namespace vos
#endif // _LIBVOS_LIST_HH
// vi: ts=8 sw=8 tw=78:
