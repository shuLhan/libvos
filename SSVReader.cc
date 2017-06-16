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

	return SPLIT_BY_WHITESPACE(line);
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
Error SSVReader::open (const char* file)
{
	if (! file) {
		return ErrFileNotFound;
	}

	return File::open_ro (file);
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
Error SSVReader::load (const char* file)
{
	Error err = open(file);
	if (err != NULL) {
		return err;
	}

	if (! _rows) {
		_rows = new Rowset();
	}

	Buffer line;
	List* row = NULL;

	do {
		err = File::get_line(&line);
		if (err == ErrFileEnd) {
			return NULL;
		}
		if (err != NULL) {
			return err;
		}

		row = parse (&line);

		if (row) {
			_rows->push_tail(row);
		}
	} while (err == NULL);

	return NULL;
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
