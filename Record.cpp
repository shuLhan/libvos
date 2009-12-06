/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Record.hpp"

namespace vos {

Record::Record() : Buffer(),
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

int Record::init(const int bfr_size)
{
	return init_size(bfr_size);
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

Record *Record::get_column(int n)
{
	Record *p = this;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	return p;
}

/**
 * @desc	: set column 'n' value to 'bfr'.
 *
 * @param	:
 *	> n	: column number.
 *	> bfr	: content for new column value.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Record::set_column(int n, Buffer *bfr)
{
	int	s	= 0;
	Record	*p	= this;

	if (!bfr)
		return 0;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	if (n < 0 || ! p)
		return -E_RECORD_INV_COLUMN;

	if (bfr->_v) {
		s = p->copy_raw(bfr->_v, bfr->_i);
	}

	return s;
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Record::set_column_number(int n, const int number)
{
	int	s	= 0;
	Record	*p	= this;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	if (n < 0 || ! p)
		return -E_RECORD_INV_COLUMN;

	s = p->appendi(number, DFLT_BASE);

	return s;
}

void Record::columns_reset()
{
	Record *p = this;

	while (p) {
		p->reset();
		p = p->_next_col;
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

int Record::INIT(Record **o, const int bfr_size)
{
	int s = -E_MEM;
	
	(*o) = new Record();

	if ((*o)) {
		s = (*o)->init(bfr_size);
		if (s != 0) {
			delete (*o);
			(*o) = NULL;
		}
	}
	return s;
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Record::INIT_ROW(Record **row, int col_size, const int bfr_size)
{
	int	s;
	Record	*col = NULL;

	if (0 == col_size)
		return 0;

	for (; col_size > 0; --col_size) {
		s = Record::INIT(&col, bfr_size);
		if (s != 0) {
			delete row;
			return s;
		}
		ADD_COL(row, col);
	}

	return 0;
}

} /* namespace::vos */
