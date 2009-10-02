/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Record.hpp"

namespace vos {

Record::Record() :
	_next_col(NULL),
	_last_col(this),
	_next_row(NULL),
	_last_row(this)
{}

Record::~Record()
{
	if (_next_col)
		delete _next_col;
	if (_next_row)
		delete _next_row;
}

void Record::dump()
{
	Record *row = this;
	Record *col = NULL;

	while (row) {
		col = row;
		while (col) {
			printf("%s|", col->_v);
			col = col->_next_col;
		}

		printf("\n");
		row = row->_next_row;
	}
}

void Record::ADD_COL(Record **row, Record *col)
{
	if (! (*row)) {
		(*row) = col;
	} else {
		(*row)->_last_col->_next_col	= col;
		(*row)->_last_col		= col;
	}
}

void Record::ADD_ROW(Record **rows, Record *row)
{
	if (! (*rows)) {
		(*rows) = row;
	} else {
		(*rows)->_last_row->_next_row	= row;
		(*rows)->_last_row		= row;
	}
}

Record *Record::INIT_ROW(const int col_size)
{

	int	i	= 0;
	Record 	*row	= NULL;
	Record	*col	= NULL;

	if (! col_size)
		return NULL;

	for (; i < col_size; ++i) {
		col = new Record();
		ADD_COL(&row, col);
	}

	return row;
}

} /* namespace::vos */
