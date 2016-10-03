//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_WRITER_HH
#define _LIBVOS_WRITER_HH 1

#include "DSVRecord.hh"
#include "DSVRecordMD.hh"

namespace vos {

/**
 * @class	: DSVWriter
 * @attr	:
 *	- _line	: temporary buffer for collecting row data in a single line.
 * @desc	:
 *	module for writing DSVRecord object into file using DSVRecord Meta-Data.
 */
class DSVWriter : public File {
public:
	DSVWriter();
	~DSVWriter();

	int write(DSVRecord* row, List* rmd);
	int writes(DSVRecord* rows, List* list_md);

	Buffer _line;
private:
	DSVWriter(const DSVWriter&);
	void operator=(const DSVWriter&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
