/**
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

	int refill_buffer(const int len);
	int read(Record *r, RecordMD *rmd);
private:
	DISALLOW_COPY_AND_ASSIGN(Reader);
};

}
#endif
