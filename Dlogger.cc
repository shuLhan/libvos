//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Dlogger.hh"

namespace vos {

const char* Dlogger::__cname = "Dlogger";

/**
 * @method	: Dlogger::Dlogger()
 * @desc	: initialize all Dlogger attributes, set standard error as
 *                default output.
 */
Dlogger::Dlogger () : File()
,	_locker()
,	_tmp()
,	_prefix()
,	_time_s(0)
,	_time()
,	_time_show(0)
#if defined(sun) || defined(__sun) || defined(__i386__)
,	_args()
#endif
,	_max_size (0)
{
	_d	= STDERR_FILENO;
	_status	= O_WRONLY;
}

/**
 * @method	: Dlogger::~Dlogger
 * @desc	: release Dlogger object to system.
 */
Dlogger::~Dlogger()
{}

/**
 * @method		: Dlogger::open
 * @param logfile	: a log file name, with or without leading path.
 * @param max_size	: maximum of file size in byte.
 * @param prefix	: string to be inserted at the beginning of each
 * line, after timestamp and before actual log output.
 * @param show_timestamp: `0` to disable timestamp on log output.
 * @return < 0		: success.
 * @return < -1		: fail.
 * @desc		: start the log daemon on the file 'logfile'.
 */
int Dlogger::open (const char* logfile, size_t max_size, const char* prefix
	, int show_timestamp)
{
	int s = 0;

	_prefix.copy_raw(prefix);
	_time_show = show_timestamp;

	if (logfile) {
		close();

		s = open_wa(logfile);
		if (s < 0) {
			_d = STDERR_FILENO;
		} else {
			_max_size = max_size;
		}

		return s;
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
		_prefix.reset();
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
	if (!_time_show) {
		return;
	}

	_time_s = time(NULL);
	localtime_r(&_time_s, &_time);

	_tmp.aprint("[%d.%02d.%02d %02d:%02d:%02d] ",
		1900 + _time.tm_year, 1 + _time.tm_mon, _time.tm_mday,
		_time.tm_hour, _time.tm_min, _time.tm_sec);
}

void Dlogger::add_prefix()
{
	_tmp.append(&_prefix);
}

/**
 * @method		: Dlogger::_w
 * @param		:
 *	> stream	: File stream.
 *	> fmt		: format of messages.
 * @desc		: The generic method of writing a log messages.
 */
ssize_t Dlogger::_w(int fd, const char* fmt)
{
	ssize_t s = 0;
	ssize_t ws = 0;

	add_timestamp();
	add_prefix();

	s = _tmp.vprint(fmt, _args);
	if (s <= 0) {
		_tmp.reset();
		return s;
	}

	if (_d != STDERR_FILENO || !fd) {
		// Check size of file
		if (_max_size > 0
		&& ((size_t(_size) + _i) > _max_size)) {
			truncate (FILE_TRUNC_FLUSH_NO);
		}

		s = write_raw(_tmp.v(), _tmp._i);
	}
	if (fd) {
		do {
			ws = ::write(fd, _tmp.v(size_t(ws)), _tmp._i - size_t(ws));
			if (ws < 0) {
				s = -1;
				break;
			}
		} while(size_t(ws) < _tmp._i);
	}
	_tmp.reset();

	return s;
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
ssize_t Dlogger::er(const char* fmt, ...)
{
	ssize_t s;

	_locker.lock();

	va_list args;
	va_start(args, fmt);
	va_copy(_args, args);

	s = _w(STDERR_FILENO, fmt);

	va_end(_args);
	va_end(args);

	_locker.unlock();

	return s;
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
ssize_t Dlogger::out(const char* fmt, ...)
{
	ssize_t s = 0;

	_locker.lock();

	va_list args;
	va_start(args, fmt);
	va_copy(_args, args);

	s = _w(STDOUT_FILENO, fmt);

	va_end(_args);
	va_end(args);

	_locker.unlock();

	return s;
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
ssize_t Dlogger::it(const char* fmt, ...)
{
	ssize_t s = 0;

	_locker.lock();

	va_list args;
	va_start(args, fmt);
	va_copy(_args, args);

	s = _w(0, fmt);

	va_end(_args);
	va_end(args);

	_locker.unlock();

	return s;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
