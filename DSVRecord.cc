//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "DSVRecord.hh"

namespace vos {

/**
 * @method	: DSVRecord::DSVRecord
 * @desc	: DSVRecord object constructor.
 */
DSVRecord::DSVRecord() : Buffer(),
	_next_col(NULL),
	_last_col(this),
	_next_row(NULL),
	_last_row(this)
{}

/**
 * @method	: DSVRecord::~DSVRecord
 * @desc	: DSVRecord object destructor.
 */
DSVRecord::~DSVRecord()
{
	if (_next_col)
		delete _next_col;
	if (_next_row)
		delete _next_row;
}

/**
 * @method	: DSVRecord::dump
 * @desc	: print content of DSVRecord object to standard output.
 */
void DSVRecord::dump()
{
	Buffer	o;
	DSVRecord* row = this;
	DSVRecord* col = NULL;

	o.append_raw("[vos::DSVRecord__] dump:\n");
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
	printf("%s", o.chars());
}

/**
 * @method		: DSVRecord::get_column
 * @param		:
 *	> n		: index of column, started from zero.
 * @return		:
 *	< DSVRecord	: pointer to DSVRecord at column 'n'.
 * @desc		: get record in column 'n'.
 */
DSVRecord* DSVRecord::get_column(int n)
{
	DSVRecord* p = this;

	for (; n > 0 && p; --n) {
		p = p->_next_col;
	}

	return p;
}

/**
 * @method	: DSVRecord::set_column
 * @param	:
 *	> n	: index of column to set to, started from zero.
 *	> bfr	: content for new column value.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: set column 'n' value to 'bfr'.
 */
int DSVRecord::set_column(int n, Buffer* bfr)
{
	if (!bfr) {
		return 0;
	}

	register int	s	= 0;
	DSVRecord*	p	= this;

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
 * @method		: DSVRecord::set_column_number
 * @param		:
 *	> n		: index of column to set to, started from zero.
 *	> number	: content for a new column value.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: set column 'n' value to 'number'.
 */
int DSVRecord::set_column_number(int n, const int number)
{
	register int	s	= 0;
	DSVRecord*	p	= this;

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
 * @method		: DSVRecord::set_column_ulong
 * @param		:
 *	> n		: index of column to set to, started from zero.
 *	> number	: content for a new column value.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: set column 'n' value to 'number'.
 */
int DSVRecord::set_column_ulong(int n, const unsigned long number)
{
	register int	s	= 0;
	DSVRecord*	p	= this;

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
 * @method	: DSVRecord::columns_reset
 * @desc	: empties all columns.
 */
void DSVRecord::columns_reset()
{
	DSVRecord* p = this;

	while (p) {
		p->reset();
		p = p->_next_col;
	}
}

/**
 * @method	: DSVRecord::ADD_COL
 * @param	:
 *	> row	: header or row.
 *	> col	: a new column to be added to 'row'.
 * @desc	: add a new column 'col' to 'row'.
 */
void DSVRecord::ADD_COL(DSVRecord** row, DSVRecord* col)
{
	if (! (*row)) {
		(*row) = col;
	} else {
		(*row)->_last_col->_next_col	= col;
		(*row)->_last_col		= col;
	}
}

/**
 * @method	: DSVRecord::ADD_ROW
 * @param	:
 *	> rows	: pointer to the first row.
 *	> row	: a new row to be added to list of 'rows'.
 * @desc	: add a new 'row' to the list of 'rows'.
 */
void DSVRecord::ADD_ROW(DSVRecord** rows, DSVRecord* row)
{
	if (! (*rows)) {
		(*rows) = row;
	} else {
		(*rows)->_last_row->_next_row	= row;
		(*rows)->_last_row		= row;
	}
}

/**
 * @method		: DSVRecord::INIT_ROW
 * @param		:
 *	> row		: head of row or pointer to first column.
 *	> n_col		: number of column to be added to 'row'
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: add 'n_col' number of column to the 'row'.
 */
int DSVRecord::INIT_ROW(DSVRecord** row, int n_col)
{
	if (0 == n_col) {
		return 0;
	}

	DSVRecord* col = NULL;

	for (; n_col > 0; --n_col) {
		col = new DSVRecord();
		if (!col) {
			delete row;
			return -1;
		}
		ADD_COL(row, col);
	}

	return 0;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
