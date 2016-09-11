/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_READER_HPP
#define	_LIBVOS_READER_HPP	1

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
	int read(Record* r, RecordMD* rmd);
private:
	Reader(const Reader&);
	void operator=(const Reader&);
};

} /* namespace::vos */
#endif
