/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_WRITER_HPP
#define	_LIBVOS_WRITER_HPP	1

#include "File.hpp"
#include "Record.hpp"
#include "RecordMD.hpp"

namespace vos {

class Writer : public File {
public:
	Writer();
	~Writer();

	int init();
	int write(Record *cols, RecordMD *rmd);
	int writes(Record *rows, RecordMD *rmd);

	Buffer _line;
private:
	DISALLOW_COPY_AND_ASSIGN(Writer);
};

} /* namespace::vos */

#endif
