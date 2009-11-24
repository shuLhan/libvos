/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Record.hpp"

namespace vos {

Record::Record(int bfr_size) : Buffer(bfr_size),
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

Record *Record::INIT_ROW(int col_size, const int bfr_size)
{

	Record 	*row	= NULL;
	Record	*col	= NULL;

	if (! col_size)
		return NULL;

	for (; col_size > 0; --col_size) {
		col = new Record(bfr_size);
		ADD_COL(&row, col);
	}

	return row;
}

Record *Record::get_column(int n)
{
	Record *p = this;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	return p;
}

int Record::set_column(int n, Buffer *bfr)
{
	Record *p = this;

	if (! bfr)
		return 0;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	if (n < 0 || ! p)
		return 1;

	if (bfr->_v) {
		p->copy(bfr->_v, bfr->_i);
	}

	return 0;
}

int Record::set_column(int n, const int number)
{
	Record *p = this;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	if (n < 0 || ! p)
		return 1;

	p->appendi(number);

	return 0;
}

void Record::columns_reset()
{
	Record *p = this;

	while (p) {
		p->reset();
		p = p->_next_col;
	}
}

} /* namespace::vos */
