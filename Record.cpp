/*
 * Copyright (C) 2010 kilabit.org
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
 * @method	: Record::dump
 * @desc	: print content of Record object to standard output.
 */
void Record::dump()
{
	Buffer	o;
	Record* row = this;
	Record* col = NULL;

	o.append_raw("[vos::Record__] dump:\n");
	while (row) {
		col = row;
		while (col) {
			o.append(col);
			o.appendc('|');
			col = col->_next_col;
		}
		o.appendc('\n');
		row = row->_next_row;
	}
	printf("%s", o.v());
}

/**
 * @method		: Record::get_column
 * @param		:
 *	> n		: index of column, started from zero.
 * @return		:
 *	< Record	: pointer to Record at column 'n'.
 * @desc		: get record in column 'n'.
 */
Record* Record::get_column(int n)
{
	Record* p = this;

	for (; n > 0 && p; --n) {
		p = p->_next_col;
	}

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
int Record::set_column(int n, Buffer* bfr)
{
	if (!bfr) {
		return 0;
	}

	register int	s	= 0;
	Record*		p	= this;

	for (; n > 0 && p; --n) {
		p = p->_next_col;
	}
	if (n < 0 || NULL == p) {
		return -1;
	}
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
	Record*		p	= this;

	for (; n > 0 && p; --n) {
		p = p->_next_col;
	}
	if (n < 0 || NULL == p) {
		return -1;
	}
	p->reset();
	s = p->appendi(number);

	return s;
}

/**
 * @method		: Record::set_column_ulong
 * @param		:
 *	> n		: index of column to set to, started from zero.
 *	> number	: content for a new column value.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: set column 'n' value to 'number'.
 */
int Record::set_column_ulong(int n, const unsigned long number)
{
	register int	s	= 0;
	Record*		p	= this;

	for (; n > 0 && p; --n) {
		p = p->_next_col;
	}
	if (n < 0 || NULL == p) {
		return -1;
	}
	p->reset();
	s = p->appendui(number);

	return s;
}

/**
 * @method	: Record::columns_reset
 * @desc	: empties all columns.
 */
void Record::columns_reset()
{
	Record* p = this;

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
void Record::ADD_COL(Record** row, Record* col)
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
void Record::ADD_ROW(Record** rows, Record* row)
{
	if (! (*rows)) {
		(*rows) = row;
	} else {
		(*rows)->_last_row->_next_row	= row;
		(*rows)->_last_row		= row;
	}
}

/**
 * @method		: Record::INIT_ROW
 * @param		:
 *	> row		: head of row or pointer to first column.
 *	> n_col		: number of column to be added to 'row'
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: add 'n_col' number of column to the 'row'.
 */
int Record::INIT_ROW(Record** row, int n_col)
{
	if (0 == n_col) {
		return 0;
	}

	Record* col = NULL;

	for (; n_col > 0; --n_col) {
		col = new Record();
		if (!col) {
			delete row;
			return -1;
		}
		ADD_COL(row, col);
	}

	return 0;
}

} /* namespace::vos */
