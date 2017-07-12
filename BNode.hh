//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_BNODE_HH
#define _LIBVOS_BNODE_HH 1

#include "Object.hh"

namespace vos {

/**
 * `BNode` implement binary node with two pointers to other node:
 * `_left` and `_right`; and one pointer to the `_item` that its contain.
 */
class BNode : public Object {
public:
	static const char* __cname;

	explicit BNode(Object* item);
	virtual ~BNode();

	void delete_content();
	void replace_content(BNode* node);
	void set_content(Object* item);
	Object* get_content();
	virtual void swap_content(BNode* x);

	virtual void set_left(BNode* node);
	virtual BNode* get_left();
	virtual BNode* get_left_edge();

	virtual void set_right(BNode* node);
	virtual BNode* get_right();
	virtual BNode* get_right_edge();

	virtual int is_left_of(BNode* node);
	virtual int is_right_of(BNode* node);

	void push_left(BNode* node);
	void push_right(BNode* node);

	virtual void insert_left(BNode* node);
	virtual void insert_right(BNode* node);

	BNode* pop_left_edge();
	BNode* pop_right_edge();

	const char* chars();

protected:
	BNode* _left;
	BNode* _right;
	Object* _item;

private:
	BNode(const BNode&);
	void operator=(const BNode&);
};

} // namespace vos
#endif // _LIBVOS_BNODE_HH
// vi: ts=8 sw=8 tw=80:
