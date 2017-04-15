//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_RECORD_HH
#define _LIBVOS_RECORD_HH 1

#include "Buffer.hh"

namespace vos {

/**
 * @class		: DSVRecord
 * @attr		:
 *	- _next_col	: pointer to the next column.
 *	- _last_col	: pointer to the last column.
 *	- _next_row	: pointer to the next row.
 *	- _last_row	: pointer to the last row.
 * @desc		:
 *	module to handle field data when parsing DSV file.
 */
class DSVRecord : public Buffer {
public:
	DSVRecord();
	~DSVRecord();

	void dump();

	DSVRecord* get_column(int n);
	int set_column(int n, Buffer* bfr);
	int set_column_number(int n, const int number);
	int set_column_ulong(int n, const unsigned long number);
	void columns_reset();

	static void ADD_COL(DSVRecord** row, DSVRecord* col);
	static void ADD_ROW(DSVRecord** rows, DSVRecord* row);
	static int INIT_ROW(DSVRecord** row, int n_col);

	DSVRecord* _next_col;
	DSVRecord* _last_col;
	DSVRecord* _next_row;
	DSVRecord* _last_row;
private:
	DSVRecord(const DSVRecord&);
	void operator=(const DSVRecord&);
};

} /* namespace::vos */
#endif
// vi: ts=8 sw=8 tw=78:
