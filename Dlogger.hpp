/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_DLOGGER_HPP
#define	_LIBVOS_DLOGGER_HPP	1

#include <pthread.h>
#include "File.hpp"

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

	int open(const char* logfile, off_t max_size = 0);
	void close();
	void add_timestamp();

	int er(const char* fmt, ...);
	int out(const char* fmt, ...);
	int it(const char* fmt, ...);
private:
	Dlogger(const Dlogger&);
	void operator=(const Dlogger&);

	void _w(int fd, const char* fmt);

	pthread_mutex_t	_lock;
	Buffer		_tmp;
	time_t		_time_s;
	struct tm	_time;
	int		_s;
	va_list		_args;
	off_t		_max_size;
};

} /* namespace::vos */

#endif
