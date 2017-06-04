//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "DSVWriter.hh"

namespace vos {

const char* DSVWriter::__cname = "DSVWriter";

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
	ssize_t len = 0;
	size_t blob_size = 0;
	int x = 0;
	DSVRecordMD* rmd = NULL;
	Error err;

	for (; x < list_md->size(); x++) {
		rmd = (DSVRecordMD*) list_md->at(x);

		if (rmd->_start_p) {
			err = _line.resize(rmd->_start_p);
			if (err != NULL) {
				return -1;
			}
		}
		if (rmd->_left_q) {
			err = _line.appendc((char) rmd->_left_q);
			if (err != NULL) {
				return -1;
			}
		}

		switch (rmd->_type) {
		case RMD_T_STRING:
		case RMD_T_NUMBER:
		case RMD_T_DATE:
			err = _line.append_raw(row->v(), row->len());
			if (err != NULL) {
				return -1;
			}
			break;
		case RMD_T_BLOB:
			blob_size = row->len();
			err = _line.append_bin(&blob_size, DSVRecordMD::BLOB_SIZE);
			if (err != NULL) {
				return -1;
			}

			err = _line.append_raw(row->v(), row->len());
			if (err != NULL) {
				return -1;
			}
			break;
		}

		if (rmd->_end_p) {
			if (rmd->_end_p < _line.len()) {
				len = ssize_t(rmd->_end_p);
				if (rmd->_right_q) {
					--len;
				}
				if (rmd->_sep) {
					--len;
				}

				_line.truncate(size_t(len));
			} else {
				_line.set_len(rmd->_end_p);
				/* it will resize by itself it the next
				 * call of append */
			}
		}
		if (rmd->_right_q) {
			err = _line.appendc((char) rmd->_right_q);
			if (err != NULL) {
				return -1;
			}
		}
		if (rmd->_sep) {
			err = _line.appendc((char) rmd->_sep);
			if (err != NULL) {
				return -1;
			}
		}
		row = row->_next_col;
	}

	_line.appendc((char) _eol);

	len = ssize_t(_i + _line.len());
	if (size_t(len) > _l) {
		flush();
	}

	err = append_raw(_line.v(), _line.len());
	if (err != NULL) {
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
	while (rows) {
		int s = write(rows, list_md);
		if (s < 0) {
			return -1;
		}
		rows = rows->_next_row;
	}

	return 0;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
