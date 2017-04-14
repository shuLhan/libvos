//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
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

	ssize_t refill_buffer(const size_t read_min);
	int read(DSVRecord* r, List* list_md);

	static const char* __cname;
private:
	DSVReader(const DSVReader&);
	void operator=(const DSVReader&);
};

} /* namespace::vos */
#endif
// vi: ts=8 sw=8 tw=78:
