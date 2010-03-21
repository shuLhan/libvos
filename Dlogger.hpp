/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
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
 */
class Dlogger : public File {
public:
	Dlogger();
	~Dlogger();

	int open(const char *logfile);
	void close();
	void add_timestamp();

	int er(const char *fmt, ...);
	int er_b(Buffer *bfr);
	int it(const char *fmt, ...);
private:
	Dlogger(const Dlogger&);
	void operator=(const Buffer&);

	pthread_mutex_t	_lock;
	Buffer		_tmp;
	time_t		_time_s;
	struct tm	_time;
};

} /* namespace::vos */

#endif
