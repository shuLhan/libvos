/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- Mhd Sulhan (ms@kilabit.info)
 */

#include "SSVReader.hh"

namespace vos {

SSVReader::SSVReader (char comment) : File ()
,	_rows (NULL)
,	_comment_c (comment)
{}

SSVReader::~SSVReader ()
{
	if (_rows) {
		delete _rows;
	}
}

/**
 @method	: SSVReader::parse
 @param		:
 > line		: string.
 @return	:
 < 0		: success.
 < 1		: skip.
 @desc		: parse one row of 'line' for strings.
 */
int SSVReader::parse (Record** rows, Buffer* line)
{
	if (! line) {
		return 1;
	}

	// skip empty line.
	if (line->is_empty ()) {
		return 1;
	}

	// skip line start with comment
	if (_comment_c != 0
	&& line->_v[0] == _comment_c) {
		return 1;
	}

	register int s;
	register int start = 0;
	register int p = 0;
	Record* row = 0;
	Record* col = 0;

	while (p < line->_i) {
		if (isspace (line->_v[p])) {
			col = new Record ();
			if (! col) {
				return -1;
			}

			s = col->copy_raw (&line->_v[start], p - start);
			if (s != 0) {
				return -1;
			}

			Record::ADD_COL (&row, col);

			while (p < line->_i && isspace (line->_v[p])) {
				p++;
			}
			start = p;
		} else {
			p++;
		}
	}
	if (start < line->_i) {
		col = new Record ();
		if (! col) {
			return -1;
		}

		s = col->copy_raw (&line->_v[start], p - start);
		if (s != 0) {
			return -1;
		}

		Record::ADD_COL (&row, col);
	}

	Record::ADD_ROW (rows, row);

	return 0;
}

/**
 @method	: SSVReader::get_row
 @param		:
 > row		: list of strings.
 @return	:
 < 0		: success.
 < 1		: success, but no row read.
 < -1		: fail.
 @desc		: get list of string separated by space, per line/row in file.
 */
int SSVReader::get_row (Record** row)
{
	register int s;
	Buffer line;

	s = File::get_line (&line);
	if (s <= 0) {
		return s;
	}

	s = parse (row, &line);

	return s;
}

/**
 @method	: SSVReader::open
 @param		:
 > file		: path to file.
 @return	:
 < 0		: success.
 < -1		: fail.
 @desc		: open file for reading.
 */
int SSVReader::open (const char* file)
{
	register int s;

	if (! file) {
		return -1;
	}

	s = File::open_ro (file);

	return s;
}

/**
 @method	: SSVReader::load
 @param		:
 > file		: path to file.
 @return	:
 < 0		: success.
 < -1		: fail.
 @desc		: Load content of file. All line returned as table of string in _rows.
 */
int SSVReader::load (const char* file)
{
	register int s;

	s = open (file);
	if (s != 0) {
		return -1;
	}

	do {
		s = get_row (&_rows);
	} while (s >= 0);

	return 0;
}

void SSVReader::reset ()
{
	if (_rows) {
		delete _rows;
		_rows = NULL;
	}
	_comment_c = 0;

	close ();
}

} /* namespace::vos */
