/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- Mhd Sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_HOSTSREADER_HPP
#define	_LIBVOS_HOSTSREADER_HPP	1

#include "File.hh"
#include "Record.hh"

namespace vos {

/**
 @class		: SSVReader
 @attr		:
 - _rows	: list of strings, propagated after calling load.
 - _comment_c	: comment character. If it's set the line will be skipped
	if first character is matched.
 @desc		: Class to read content of file in Space Separated Value
	(e.g. hosts file).
 */
class SSVReader : public File {
public:
	SSVReader (char comment = 0);
	~SSVReader ();

	int parse (Record** rows, Buffer* line);
	int get_row (Record** row);
	int open (const char* file);
	int load (const char* file);

	void reset ();

	Record* _rows;
	char	_comment_c;
private:
	SSVReader (const SSVReader&);
	void operator= (const SSVReader&);
};

} /* namespace::vos */

#endif
