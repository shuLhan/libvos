/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Writer.hpp"

namespace vos {

/**
 * @method	: Writer::Writer
 * @desc	: Writer object constructor.
 */
Writer::Writer() :
	_line()
{}

/**
 * @method	: Writer::~Writer
 * @desc	: Writer object destructor.
 */
Writer::~Writer()
{}

/**
 * @method	: Writer::write
 * @param	:
 *	> cols	: record buffer, in list of column.
 *	> rmd	: record meta-data.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: write one row using 'rmd' as meta-data to file.
 */
int Writer::write(Record *cols, RecordMD *rmd)
{
	register int	s;
	register int	len;

	while (rmd && cols) {
		if (rmd->_start_p) {
			s = _line.resize(rmd->_start_p);
			if (s < 0)
				return s;
		}

		if (rmd->_left_q) {
			s = _line.appendc((char) rmd->_left_q);
			if (s < 0)
				return s;
		}

		switch (rmd->_type) {
		case RMD_T_STRING:
		case RMD_T_NUMBER:
		case RMD_T_DATE:
			s = _line.append_raw(cols->_v, cols->_i);
			if (s < 0)
				return s;
			break;
		case RMD_T_BLOB:
			s = cols->shiftr(RecordMD::BLOB_SIZE);
			if (s < 0)
				return s;

			memcpy(&cols->_v[0], &cols->_i, RecordMD::BLOB_SIZE);

			s = _line.append_raw(cols->_v, cols->_i);
			if (s < 0)
				return s;
			break;
		}

		if (rmd->_end_p) {
			if (rmd->_end_p < _line._i) {
				len = rmd->_end_p;
				if (rmd->_right_q)
					--len;
				if (rmd->_sep)
					--len;

				while (len < _line._i) {
					_line._v[_line._i] = ' ';
					--_line._i;
				}
			} else if (rmd->_end_p > _line._i) {
				_line._i = rmd->_end_p;
			}
		}
		if (rmd->_right_q) {
			s = _line.appendc((char) rmd->_right_q);
			if (s < 0)
				return s;
		}
		if (rmd->_sep) {
			s = _line.appendc((char) rmd->_sep);
			if (s < 0)
				return s;
		}
		
		cols	= cols->_next_col;
		rmd	= rmd->_next;
	}

	_line.appendc((char) _eol);

	len = _i + _line._i;
	if (len > _l) {
		flush();
	}

	s = append_raw(_line._v, _line._i);
	if (s < 0)
		return s;

	_line.reset();

	return 0;
}

/**
 * @method	: Writer::writes
 * @param	:
 *	> rows	: list of row of Record objects.
 *	> rmd	: Record meta-data.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: write all 'rows' to file.
 */
int Writer::writes(Record *rows, RecordMD *rmd)
{
	register int s;

	while (rows) {
		s = write(rows, rmd);
		if (s < 0)
			return s;
		rows = rows->_next_row;
	}

	return 0;
}

} /* namespace::vos */
