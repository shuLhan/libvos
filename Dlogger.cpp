/*
 * Copyright (C) 2010 kilabit.org
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
	_lock()
,	_tmp()
,	_time_s(0)
,	_time()
,	_s(0)
,	_args()
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
 *	< -1		: fail.
 * @desc		: start the log daemon on the file 'logfile'.
 */
int Dlogger::open(const char* logfile)
{
	if (logfile) {
		close();

		_s = open_wa(logfile);
		if (_s < 0) {
			_d = STDERR_FILENO;
		}
		return _s;
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
 * @method		: Dlogger::_w
 * @param		:
 *	> stream	: File stream.
 *	> fmt		: format of messages.
 * @desc		: The generic method of writing a log messages.
 */
void Dlogger::_w(int fd, const char* fmt)
{
	add_timestamp();

	_s = _tmp.vprint(fmt, _args);
	if (_s <= 0) {
		_tmp.reset();
		return;
	}

	if (_d != STDERR_FILENO || !fd) {
		_s = write_raw(_tmp._v, _tmp._i);
	}
	if (fd) {
		::write(fd, _tmp._v, _tmp._i);
	}
	_tmp.reset();
}

/**
 * @method	: Dlogger::er
 * @param	:
 *	> fmt	: formatted string output.
 *	> ...	: one or more arguments for output.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: write message to standard error and log file.
 */
int Dlogger::er(const char* fmt, ...)
{
	do { _s = pthread_mutex_trylock(&_lock); } while (_s != 0);

	va_start(_args, fmt);
	_w(STDERR_FILENO, fmt);
	va_end(_args);

	pthread_mutex_unlock(&_lock);

	return _s;
}

/**
 * @method	: Dlogger::out
 * @param	:
 *	> fmt	: formatted string output.
 *	> ...	: one or more arguments for output.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: write message to standard output and log file.
 */
int Dlogger::out(const char* fmt, ...)
{
	do { _s = pthread_mutex_trylock(&_lock); } while (_s != 0);

	va_start(_args, fmt);
	_w(STDOUT_FILENO, fmt);
	va_end(_args);

	pthread_mutex_unlock(&_lock);

	return _s;
}

/**
 * @method	: Dlogger::it
 * @param	:
 *	> fmt	: formatted string output.
 *	> ...	: one or more arguments for output.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: write message to log file only.
 */
int Dlogger::it(const char* fmt, ...)
{
	do { _s = pthread_mutex_trylock(&_lock); } while (_s != 0);

	va_start(_args, fmt);
	_w(0, fmt);
	va_end(_args);

	pthread_mutex_unlock(&_lock);

	return _s;
}

} /* namespace::vos */
