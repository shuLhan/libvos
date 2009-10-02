/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Dlogger.hpp"

namespace vos {

Dlogger::Dlogger(const char *logfile) :
	_lock()
{
	pthread_mutex_init(&_lock, 0);

	if (logfile) {
		open_wa(logfile);
	} else {
		_d = STDOUT_FILENO;
		_status = FILE_OPEN_W;
	}
}

Dlogger::~Dlogger()
{
	pthread_mutex_destroy(&_lock);

	if (_d == STDOUT_FILENO)
		_d = 0;
}

/**
 * @desc: write message to stdout and log file.
 */
void Dlogger::dloger(const char *fmt, ...)
{
	register int	s;
	va_list		args;
	va_list		args2;

	do { s = pthread_mutex_lock(&_lock); } while (s);

	va_start(args, fmt);

	__va_copy(args2, args);
	vfprintf(stdout, fmt, args2);
	write(fmt, args);
	va_end(args2);
	va_end(args);

	pthread_mutex_unlock(&_lock);
}

/**
 * @desc: write message to log file only.
 */
void Dlogger::dlogit(const char *fmt, ...)
{
	register int	s;
	va_list		args;

	do { s = pthread_mutex_lock(&_lock); } while (s);

	va_start(args, fmt);
	write(fmt, args);
	va_end(args);

	pthread_mutex_unlock(&_lock);
}

} /* namespace::vos */
