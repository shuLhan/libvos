/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_READER_HPP
#define	_LIBVOS_READER_HPP	1

#include "File.hpp"
#include "Record.hpp"
#include "RecordMD.hpp"

namespace vos {

class Reader : public File {
public:
	Reader();
	~Reader();

	int refill_buffer(const int read_min);
	int read(Record *r, RecordMD *rmd);
private:
	Reader(const Reader &);
	void operator=(const Reader &);
};

} /* namespace::vos */
#endif
