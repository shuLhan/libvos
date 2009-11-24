/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Dlogger.hpp"

namespace vos {

Dlogger::Dlogger() :
	_lock()
{
	pthread_mutex_init(&_lock, 0);

	_d = STDOUT_FILENO;
	_status = FILE_OPEN_W;
}

Dlogger::~Dlogger()
{
	pthread_mutex_destroy(&_lock);

	if (_d == STDOUT_FILENO)
		_d = 0;
}

/**
 * @desc: start the log daemon on the file 'logfile'.
 *
 * @param:
 *	> logfile	: a log file name, with or without leading path.
 * @return:
 *	< 0	: success.
 *	< !0	: fail.
 */
int Dlogger::open(const char *logfile)
{
	if (_d && _d != STDOUT_FILENO) {
		File::close();
	}
	if (!logfile) {
		_d = STDOUT_FILENO;
		_status = FILE_OPEN_W;
	} else {
		return open_wa(logfile);
	}
	return 0;
}

/**
 * @desc: close log file, and revert the log output back to standard output.
 */
void Dlogger::close()
{
	if (_d && _d != STDOUT_FILENO) {
		File::close();
		_d = STDOUT_FILENO;
		_status = FILE_OPEN_W;
	}
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
