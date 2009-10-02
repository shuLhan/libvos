#include "Writer.hpp"

namespace vos {

Writer::Writer()
{}

Writer::~Writer()
{}

/**
 * @desc: write one row using 'rmd' as meta-data to file.
 *
 * @param:
 *	> r	: record buffer.
 *	> rmd	: record meta-data.
 *
 * @return:
 *	< 1	: one record writen.
 */
void Writer::write(Record *cols, RecordMD *rmd)
{
	int len;

	while (rmd && cols) {
		if (rmd->_start_p) {
			_line.resize(rmd->_start_p);
		}

		if (rmd->_left_q) {
			_line.appendc(rmd->_left_q);
		}

		_line.append(cols->_v, cols->_i);

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
			_line.appendc(rmd->_right_q);
		}
		if (rmd->_sep) {
			_line.appendc(rmd->_sep);
		}
		
		cols	= cols->_next_col;
		rmd	= rmd->_next;
	}

	_line.appendc(_eol);

	len = _i + _line._i;
	if (len > _l) {
		flush();
	}

	append(_line._v, _line._i);
	_line.reset();
}

void Writer::writes(Record *rows, RecordMD *rmd)
{
	while (rows) {
		write(rows, rmd);
		rows = rows->_next_row;
	}
}

} /* namespace::vos */
