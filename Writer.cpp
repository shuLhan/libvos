/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Writer.hpp"

namespace vos {

Writer::Writer() :
	_line()
{}

Writer::~Writer()
{}

int Writer::init()
{
	return _line.init(NULL);
}

/**
 * @desc: write one row using 'rmd' as meta-data to file.
 *
 * @param:
 *	> r	: record buffer.
 *	> rmd	: record meta-data.
 *
 * @return:
 *	< 1	: one record written.
 *	< 0	: success.
 *	< <0	: fail.
 */
int Writer::write(Record *cols, RecordMD *rmd)
{
	int	s;
	int	len;
	char	*p = NULL;

	while (rmd && cols) {
		if (rmd->_start_p) {
			s = _line.resize(rmd->_start_p);
			if (s < 0)
				return s;
		}

		if (rmd->_left_q) {
			s = _line.appendc(rmd->_left_q);
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
			len = sizeof(cols->_i);
			s = cols->shiftr(len);
			if (s < 0)
				return s;

			p = cols->_v;
			memcpy(p, &cols->_i, len);

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
			s = _line.appendc(rmd->_right_q);
			if (s < 0)
				return s;
		}
		if (rmd->_sep) {
			s = _line.appendc(rmd->_sep);
			if (s < 0)
				return s;
		}
		
		cols	= cols->_next_col;
		rmd	= rmd->_next;
	}

	_line.appendc(_eol);

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

int Writer::writes(Record *rows, RecordMD *rmd)
{
	int s;

	while (rows) {
		s = write(rows, rmd);
		if (s < 0)
			return s;
		rows = rows->_next_row;
	}

	return 0;
}

} /* namespace::vos */
