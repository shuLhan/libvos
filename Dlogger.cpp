/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Dlogger.hpp"

namespace vos {

/**
 * @method	: Dlogger::Dlogger()
 * @desc	: initialize all Dlogger attributes, set standard error as
 *                default output.
 */
Dlogger::Dlogger() :
	_lock(),
	_tmp(),
	_time_s(0),
	_time()
{
	pthread_mutex_init(&_lock, NULL);

	_d	= STDERR_FILENO;
	_status	= O_WRONLY;
}

/**
 * @method	: Dlogger::~Dlogger
 * @desc	: release Dlogger object to system.
 */
Dlogger::~Dlogger()
{
	pthread_mutex_destroy(&_lock);
}

/**
 * @method		: Dlogger::open
 * @param		:
 *	> logfile	: a log file name, with or without leading path.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: start the log daemon on the file 'logfile'.
 */
int Dlogger::open(const char *logfile)
{
	close();

	if (logfile) {
		File::init();
		return open_wa(logfile);
	}
	return 0;
}

/**
 * @method	: Dlogger::close
 * @desc	:
 *	close log file, and revert the log output back to standard error.
 */
void Dlogger::close()
{
	if (_d && _d != STDERR_FILENO) {
		File::close();
		_d	= STDERR_FILENO;
		_status	= O_WRONLY;
	}
}

/**
 * @method	: Dlogger::add_timestamp
 * @desc	:
 *	add timestamp to log output.
 *	Timestamp format: YEAR.MONTH.DAY HOUR:MINUTE:SECOND.
 */
inline void Dlogger::add_timestamp()
{
	_time_s = time(NULL);
	localtime_r(&_time_s, &_time);

	_tmp.aprint("[%d.%02d.%02d %02d:%02d:%02d] ",
		1900 + _time.tm_year, 1 + _time.tm_mon, _time.tm_mday,
		_time.tm_hour, _time.tm_min, _time.tm_sec);
}

/**
 * @method	: Dlogger::er
 * @param	:
 *	> fmt	: formatted string output.
 *	> ...	: one or more arguments for output.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: write message to standard error and log file.
 */
int Dlogger::er(const char *fmt, ...)
{
	register int	s;
	va_list		args;

	do { s = pthread_mutex_trylock(&_lock); } while (s != 0);

	va_start(args, fmt);

	add_timestamp();
	_tmp.vprint(fmt, args);

	if (_d != STDERR_FILENO) {
		s = write(&_tmp);
	}
	fprintf(stderr, "%s", _tmp._v);

	_tmp.reset();
	va_end(args);

	pthread_mutex_unlock(&_lock);

	return s;
}

/**
 * @method	: Dlogger::it
 * @param	:
 *	> fmt	: formatted string output.
 *	> ...	: one or more arguments for output.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: write message to log file only.
 */
int Dlogger::it(const char *fmt, ...)
{
	register int	s;
	va_list		args;

	do { s = pthread_mutex_trylock(&_lock); } while (s != 0);

	va_start(args, fmt);
	add_timestamp();
	_tmp.vprint(fmt, args);
	s = write(&_tmp);
	_tmp.reset();
	va_end(args);

	pthread_mutex_unlock(&_lock);

	return s;
}

} /* namespace::vos */
