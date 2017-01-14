//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_DLOGGER_HH
#define _LIBVOS_DLOGGER_HH 1

#include "File.hh"
#include "Locker.hh"

namespace vos {

/**
 * @class	: Dlogger
 * @desc	:
 *	a module for writing formatted output log to a file or standard error,
 *	or both. If Dlogger object is not initialized, by calling open(), all
 *	log output from calling er() or it() will be printed to standard error.
 * @attr _max_size : Maximum log file size.
 */
class Dlogger : public File {
public:
	Dlogger();
	~Dlogger();

	int open(const char* logfile, off_t max_size = 0
		, const char* prefix = 0
		, int show_timestamp = 1);
	void close();

	int er(const char* fmt, ...);
	int out(const char* fmt, ...);
	int it(const char* fmt, ...);
private:
	Dlogger(const Dlogger&);
	void operator=(const Dlogger&);

	void add_timestamp();
	void add_prefix();
	void _w(int fd, const char* fmt);

	Locker		_locker;
	Buffer		_tmp;
	Buffer		_prefix;
	time_t		_time_s;
	struct tm	_time;
	int		_time_show;
	int		_s;
	va_list		_args;
	off_t		_max_size;
};

} /* namespace::vos */
#endif
// vi: ts=8 sw=8 tw=78:
