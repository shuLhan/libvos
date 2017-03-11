//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SSVReader.hh"

namespace vos {

SSVReader::SSVReader (const char comment) : File ()
,	_rows (NULL)
,	_comment_c (comment)
{}

SSVReader::~SSVReader ()
{
	if (_rows) {
		delete _rows;
	}
}

//
// `parse()` will parse line by separating it with white spaces, return NULL
// if its comment.
//
List* SSVReader::parse(Buffer* line)
{
	if (! line) {
		return NULL;
	}

	line->trim();

	// skip empty line.
	if (line->is_empty ()) {
		return NULL;
	}

	// skip line start with comment
	if (_comment_c != 0
	&& line->char_at(0) == _comment_c) {
		return NULL;
	}

	return line->split_by_whitespace();
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
	Buffer line;
	List* row = NULL;

	s = open (file);
	if (s != 0) {
		return -1;
	}

	if (! _rows) {
		_rows = new Rowset();
	}

	do {
		s = File::get_line (&line);
		if (s <= 0) {
			break;
		}

		row = parse (&line);

		if (row) {
			_rows->push_tail(row);
		}
	} while (s > 0);

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
// vi: ts=8 sw=8 tw=78:
