//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_BNODE_HH
#define _LIBVOS_BNODE_HH 1

#include "Object.hh"

namespace vos {

//
// `BNode` implement binary node with two pointers to other node:
// `_left` and `_right`; and one pointer to the `_item` that its contain.
//
class BNode : public Object {
public:
	BNode(Object* item);
	virtual ~BNode();

	BNode* get_left_edge();
	BNode* get_right_edge();
	void push_left(BNode* node);
	void push_right(BNode* node);
	void insert_left(BNode* node);
	void insert_right(BNode* node);
	BNode* pop_left_edge();
	BNode* pop_right_edge();
	const char* chars();

	BNode* _left;
	BNode* _right;
	Object* _item;

	static const char* __cname;
private:
	BNode(const BNode&);
	void operator=(const BNode&);
};

} // namespace vos
#endif // _LIBVOS_BNODE_HH
// vi: ts=8 sw=8 tw=78:
