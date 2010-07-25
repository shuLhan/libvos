/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_WRITER_HPP
#define	_LIBVOS_WRITER_HPP	1

#include "Record.hpp"
#include "RecordMD.hpp"

namespace vos {

/**
 * @class	: Writer
 * @attr	:
 *	- _line	: temporary buffer for collecting row data in a single line.
 * @desc	:
 *	module for writing of Record object into file using Record Meta-Data,
 *	or in another word, a module for writing DSV data to file.
 */
class Writer : public File {
public:
	Writer();
	~Writer();

	int write(Record *cols, RecordMD *rmd);
	int writes(Record *rows, RecordMD *rmd);

	Buffer _line;
private:
	Writer(const Writer&);
	void operator=(const Writer&);
};

} /* namespace::vos */

#endif
