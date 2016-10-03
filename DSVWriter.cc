//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "DSVWriter.hh"

namespace vos {

/**
 * @method	: DSVWriter::DSVWriter
 * @desc	: DSVWriter object constructor.
 */
DSVWriter::DSVWriter() :
	_line()
{}

/**
 * @method	: DSVWriter::~DSVWriter
 * @desc	: DSVWriter object destructor.
 */
DSVWriter::~DSVWriter()
{}

/**
 * @method	: DSVWriter::write
 * @param	:
 *	> row	: record buffer, list of column data that will be written.
 *	> list_md: record meta-data.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: write one row using 'list_md' as meta-data to file.
 */
int DSVWriter::write(DSVRecord *row, List *list_md)
{
	register int	s;
	register int	len;
	int x = 0;
	DSVRecordMD* rmd = NULL;

	for (; x < list_md->size(); x++) {
		rmd = (DSVRecordMD*) list_md->at(x);

		if (rmd->_start_p) {
			s = _line.resize(rmd->_start_p);
			if (s < 0) {
				return -1;
			}
		}
		if (rmd->_left_q) {
			s = _line.appendc((char) rmd->_left_q);
			if (s < 0) {
				return -1;
			}
		}

		switch (rmd->_type) {
		case RMD_T_STRING:
		case RMD_T_NUMBER:
		case RMD_T_DATE:
			s = _line.append_raw(row->_v, row->_i);
			if (s < 0) {
				return -1;
			}
			break;
		case RMD_T_BLOB:
			s = _line.append_bin (&row->_i, DSVRecordMD::BLOB_SIZE);
			if (s < 0) {
				return -1;
			}

			s = _line.append_raw(row->_v, row->_i);
			if (s < 0) {
				return -1;
			}
			break;
		}

		if (rmd->_end_p) {
			if (rmd->_end_p < _line._i) {
				len = rmd->_end_p;
				if (rmd->_right_q) {
					--len;
				}
				if (rmd->_sep) {
					--len;
				}
				while (len < _line._i) {
					_line._v[_line._i] = ' ';
					--_line._i;
				}
			} else {
				_line._i = rmd->_end_p;
				/* it will resize by itself it the next
				 * call of append*() */
			}
		}
		if (rmd->_right_q) {
			s = _line.appendc((char) rmd->_right_q);
			if (s < 0) {
				return -1;
			}
		}
		if (rmd->_sep) {
			s = _line.appendc((char) rmd->_sep);
			if (s < 0) {
				return -1;
			}
		}
		row = row->_next_col;
	}

	_line.appendc((char) _eol);

	len = _i + _line._i;
	if (len > _l) {
		flush();
	}

	s = append_raw(_line._v, _line._i);
	if (s < 0) {
		return -1;
	}

	_line.reset();

	return 0;
}

/**
 * @method	: DSVWriter::writes
 * @param	:
 *	> rows	: list of row of DSVRecord objects.
 *	> list_md: list of record meta-data.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: write all 'rows' to file.
 */
int DSVWriter::writes(DSVRecord *rows, List *list_md)
{
	register int s;

	while (rows) {
		s = write(rows, list_md);
		if (s < 0) {
			return -1;
		}
		rows = rows->_next_row;
	}

	return 0;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
