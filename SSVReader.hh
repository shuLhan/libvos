//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_HOSTSREADER_HH
#define _LIBVOS_HOSTSREADER_HH 1

#include "File.hh"
#include "Rowset.hh"

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
	SSVReader (const char comment = 0);
	~SSVReader ();

	int load (const char* file);
	void reset ();

	Rowset* _rows;
	char	_comment_c;
private:
	List* parse(Buffer* line);
	int open(const char* file);

	SSVReader (const SSVReader&);
	void operator= (const SSVReader&);
};

} // namespace::vos
#endif
// vi: ts=8 sw=8 tw=78:
