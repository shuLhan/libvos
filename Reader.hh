//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_READER_HH
#define _LIBVOS_READER_HH 1

#include "Record.hh"
#include "RecordMD.hh"

namespace vos {

/**
 * @class	: Reader
 * @desc	: a module for reading DSV file.
 */
class Reader : public File {
public:
	Reader();
	~Reader();

	int refill_buffer(const int read_min);
	int read(Record* r, List* list_md);
private:
	Reader(const Reader&);
	void operator=(const Reader&);
};

} /* namespace::vos */
#endif
// vi: ts=8 sw=8 tw=78:
