/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
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
