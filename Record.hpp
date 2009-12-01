/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_RECORD_HPP
#define	_LIBVOS_RECORD_HPP	1

#include "Buffer.hpp"

namespace vos {

class Record : public Buffer {
public:
	Record();
	~Record();

	int init(const int bfr_size);
	void dump();

	Record *get_column(int n);
	int set_column(int n, Buffer *bfr);
	int set_column_number(int n, const int number);
	void columns_reset();

	static void ADD_COL(Record **row, Record *col);
	static void ADD_ROW(Record **rows, Record *row);
	static int INIT(Record **o, const int bfr_size);
	static int INIT_ROW(Record **o, int col_size, const int bfr_size);

	Record *_next_col;
	Record *_last_col;
	Record *_next_row;
	Record *_last_row;
private:
	DISALLOW_COPY_AND_ASSIGN(Record);
};

} /* namespace::vos */
#endif
