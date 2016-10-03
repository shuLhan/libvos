//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_READER_HH
#define _LIBVOS_READER_HH 1

#include "DSVRecord.hh"
#include "DSVRecordMD.hh"

namespace vos {

/**
 * @class	: DSVReader
 * @desc	: a module for reading DSV file.
 */
class DSVReader : public File {
public:
	DSVReader();
	~DSVReader();

	int refill_buffer(const int read_min);
	int read(DSVRecord* r, List* list_md);
private:
	DSVReader(const DSVReader&);
	void operator=(const DSVReader&);
};

} /* namespace::vos */
#endif
// vi: ts=8 sw=8 tw=78:
