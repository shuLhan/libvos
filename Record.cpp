/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Record.hpp"

namespace vos {

/**
 * @method	: Record::Record
 * @desc	: Record object constructor.
 */
Record::Record() : Buffer(),
	_next_col(NULL),
	_last_col(this),
	_next_row(NULL),
	_last_row(this)
{}

/**
 * @method	: Record::~Record
 * @desc	: Record object destructor.
 */
Record::~Record()
{
	if (_next_col)
		delete _next_col;
	if (_next_row)
		delete _next_row;
}

/**
 * @method		: Record::init
 * @param		:
 *	> bfr_size	: initial buffer size.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: initialize Record object.
 */
int Record::init(const int bfr_size)
{
	return init_size(bfr_size);
}

/**
 * @method	: Record::dump
 * @desc	: print content of Record object to standard output.
 */
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

/**
 * @method		: Record::get_column
 * @param		:
 *	> n		: index of column, started from zero.
 * @return		:
 *	< Record	: pointer to Record at column 'n'.
 * @desc		: get record in column 'n'.
 */
Record *Record::get_column(int n)
{
	Record *p = this;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	return p;
}

/**
 * @method	: Record::set_column
 * @param	:
 *	> n	: index of column to set to, started from zero.
 *	> bfr	: content for new column value.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: set column 'n' value to 'bfr'.
 */
int Record::set_column(int n, Buffer *bfr)
{
	if (!bfr)
		return 0;

	register int	s	= 0;
	Record		*p	= this;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	if (n < 0 || NULL == p)
		return -E_RECORD_INV_COLUMN;

	if (bfr->_v) {
		s = p->copy_raw(bfr->_v, bfr->_i);
	}

	return s;
}

/**
 * @method		: Record::set_column_number
 * @param		:
 *	> n		: index of column to set to, started from zero.
 *	> number	: content for a new column value.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: set column 'n' value to 'number'.
 */
int Record::set_column_number(int n, const int number)
{
	register int	s	= 0;
	Record		*p	= this;

	for (; n > 0 && p; --n)
		p = p->_next_col;

	if (n < 0 || ! p)
		return -E_RECORD_INV_COLUMN;

	s = p->appendi(number);

	return s;
}

/**
 * @method	: Record::columns_reset
 * @desc	: empties all columns.
 */
void Record::columns_reset()
{
	Record *p = this;

	while (p) {
		p->reset();
		p = p->_next_col;
	}
}

/**
 * @method	: Record::ADD_COL
 * @param	:
 *	> row	: header or row.
 *	> col	: a new column to be added to 'row'.
 * @desc	: add a new column 'col' to 'row'.
 */
void Record::ADD_COL(Record **row, Record *col)
{
	if (! (*row)) {
		(*row) = col;
	} else {
		(*row)->_last_col->_next_col	= col;
		(*row)->_last_col		= col;
	}
}

/**
 * @method	: Record::ADD_ROW
 * @param	:
 *	> rows	: pointer to the first row.
 *	> row	: a new row to be added to list of 'rows'.
 * @desc	: add a new 'row' to the list of 'rows'.
 */
void Record::ADD_ROW(Record **rows, Record *row)
{
	if (! (*rows)) {
		(*rows) = row;
	} else {
		(*rows)->_last_row->_next_row	= row;
		(*rows)->_last_row		= row;
	}
}

/**
 * @method		: Record::INIT
 * @param		:
 *	> o		: return value, pointer to Record object.
 *	> bfr_size	: initial size for Record buffer.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		:
 *	create and initialize a new Record object 'o', using 'bfr_size' as
 *	initial buffer size for Record buffer.
 */
int Record::INIT(Record **o, const int bfr_size)
{
	register int s = -1;
	
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
 * @method		: Record::INIT_ROW
 * @param		:
 *	> row		: head of row or pointer to first column.
 *	> n_col		: number of column to be added to 'row'
 *	> bfr_size	: initial buffer size for each new column.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: add 'n_col' number of column to the 'row'.
 */
int Record::INIT_ROW(Record **row, int n_col, const int bfr_size)
{
	if (0 == n_col)
		return 0;

	register int	s;
	Record		*col = NULL;

	for (; n_col > 0; --n_col) {
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
