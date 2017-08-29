//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "ListBuffer.hh"

namespace vos {

void _list_buffer_add(List* buffers, const char* v, size_t start, size_t end
	, int trim)
{
	Buffer* b = NULL;

	size_t len = end - start;
	if (len == 0) {
		if (trim) {
			return;
		}

		b = new Buffer(1);
	} else {
		b = new Buffer(len);
		b->copy_raw(&v[start], len);

		if (trim) {
			b->trim();

			if (b->is_empty()) {
				delete b;
				return;
			}
		}
	}

	buffers->push_tail(b);
}


/**
 * Method `split_by_char(b, sep, trim)` will split buffer `b` using separator
 * `sep` and return list of buffers.
 *
 * If buffer `b` is empty it will return NULL.
 *
 * If `trim` is non zero value, splitted buffer will be trimmed, and if result
 * of trimmed buffer is empty then it would not be added to the list.
 */
List* SPLIT_BY_CHAR(Buffer* b, const char sep, int trim)
{
	if (!b) {
		return NULL;
	}
	if (b->is_empty()) {
		return NULL;
	}

	List* buffers = new List();
	size_t x = 0;
	size_t start = 0;

	for (; x < b->len(); x++) {
		if (b->char_at(x) == sep) {
			_list_buffer_add(buffers, b->v(), start, x, trim);
			start = x + 1;
		}
	}

	if (x >= start) {
		_list_buffer_add(buffers, b->v(), start, x, trim);
	}

	return buffers;
}

/**
 * Method `split_by_whitespace(Buffer* b)` will split buffer using whitespace
 * and return list of buffers.
 */
List* SPLIT_BY_WHITESPACE(Buffer* b)
{
	if (!b) {
		return NULL;
	}

	b->trim();

	// skip empty line.
	if (b->is_empty()) {
		return NULL;
	}

	List* buffers = new List();
	size_t x = 0;
	size_t start = 0;

	for (; x < b->len(); x++) {
		if (isspace(b->char_at(x))) {
			_list_buffer_add(buffers, b->v(), start, x, 1);
			start = x + 1;
		}
	}

	if (x > start) {
		_list_buffer_add(buffers, b->v(), start, x, 1);
	}

	return buffers;
}

} // namespace::vos
// vi: ts=8 sw=8 tw=80:
