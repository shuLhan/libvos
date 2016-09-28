//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Rowset.hh"
#include "Buffer.hh"

namespace vos {

const char* Rowset::__cname = "Rowset";

//
// `Rowset()` will create new set of row.
//
Rowset::Rowset() : List('\n')
{
}

//
// `~Rowset()` will release all rows memory to the system.
//
Rowset::~Rowset()
{}

//
// `chars()` will return string presentation of set, where each node is
// printed from top to bottom and separated by `_sep` character.
// The returned string is in JSON array format.
//
const char* Rowset::chars()
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
		b.concat("[ ", _head->chars(), " ]", NULL);
		goto out;
	}

	b.append_raw("[ ");
	while (node != _tail) {
		b.append_raw(node->chars());
		b.appendc(_sep);

		node = node->_right;
	}
	b.append_raw(node->chars());
	b.append_raw(" ]");

out:
	_v = b._v;
	b._v = NULL;

	return _v;
}


} // namespace vos
// vi: ts=8 sw=8 tw=78:
