//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_WRITER_HH
#define _LIBVOS_WRITER_HH 1

#include "Record.hh"
#include "RecordMD.hh"

namespace vos {

/**
 * @class	: Writer
 * @attr	:
 *	- _line	: temporary buffer for collecting row data in a single line.
 * @desc	:
 *	module for writing Record object into file using Record Meta-Data.
 */
class Writer : public File {
public:
	Writer();
	~Writer();

	int write(Record* row, RecordMD* rmd);
	int writes(Record* rows, RecordMD* rmd);

	Buffer _line;
private:
	Writer(const Writer&);
	void operator=(const Writer&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
