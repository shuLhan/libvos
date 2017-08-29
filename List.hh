//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_LIST_HH
#define _LIBVOS_LIST_HH 1

#include "Locker.hh"
#include "BNode.hh"

namespace vos {

/**
 * List is generic implementation of circular double linked list.
 *
 * Field `_n` will contain number of node in the list.
 * Field `_sep` is used to separate node when calling `chars`.
 * Field `_head` is a pointer to the first node in the list.
 * Field `_tail` is pointer to the last node in the list.
 *
 */
class List : public Locker {
public:
	// `__cname` contain canonical name of this object.
	static const char* __cname;

	explicit List(const char sep = ',');
	virtual ~List();

	BNode* head();
	BNode* tail();

	void first_push(BNode* node);
	void insert_before_unsafe(BNode* x, BNode* y);
	void insert_after_unsafe(BNode* x, BNode* y);

	BNode* push_circular(BNode** p, Object* item);

	BNode* push_head(Object* item);
	BNode* push_head_sorted(Object* item, int asc = 1
				, int (*fn_cmp)(Object*, Object*) = NULL);

	BNode* push_tail(Object* item);
	BNode* push_tail_sorted(Object* item, int asc = 1
				, int (*fn_cmp)(Object*, Object*) = NULL);

	void reset();
	void swap_by_idx_unsafe(int x, int y);
	void sort(int (*fn_compare)(Object*, Object*), int asc=1);
	void detach(BNode* node);

	void node_push_head_sorted(BNode* node, int asc = 1
				, int (*fn_cmp)(Object*, Object*) = NULL);
	void node_push_tail_sorted(BNode* node, int asc = 1
				, int (*fn_cmp)(Object*, Object*) = NULL);

	BNode* node_search(Object* item, int (*fn_compare)(Object*, Object*));
	BNode* node_pop_head();
	BNode* node_pop_tail();
	BNode* node_at(int idx);
	BNode* node_at_unsafe(int idx);

	Object* node_remove_unsafe(BNode* bnode);
	Object* pop_head();
	Object* pop_tail();
	Object* at(int idx);
	Object* at_unsafe(int idx);

	int remove(Object* item);
	int size();
	const char* chars();

protected:
	int _n;
	char _sep;
	BNode* _head;
	BNode* _tail;

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
// vi: ts=8 sw=8 tw=80:
