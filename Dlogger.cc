//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
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
,	_args()
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
Error Dlogger::open (const char* logfile, size_t max_size, const char* prefix
	, int show_timestamp)
{
	_prefix.copy_raw(prefix);
	_time_show = show_timestamp;

	if (logfile) {
		close();

		Error err = open_wa(logfile);
		if (err != NULL) {
			_d = STDERR_FILENO;
			return err;
		}

		_max_size = max_size;
	}
	return NULL;
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

	_tmp.append_fmt("[%d.%02d.%02d %02d:%02d:%02d] ",
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
Error Dlogger::_w(int fd, const char* fmt)
{
	add_timestamp();
	add_prefix();

	Error err = _tmp.vappend_fmt(fmt, _args);
	if (err != NULL) {
		_tmp.reset();
		return err;
	}

	if (_d != STDERR_FILENO || !fd) {
		// Check size of file
		if (_max_size > 0
		&& ((size_t(_size) + _i) > _max_size)) {
			truncate(FLUSH_NO);
		}

		err = write_raw(_tmp.v(), _tmp.len());
		if (err != NULL) {
			return err;
		}
	}
	if (fd) {
		ssize_t ws = 0;
		do {
			ws = ::write(fd, _tmp.v(size_t(ws))
				, _tmp.len() - size_t(ws));
			if (ws < 0) {
				return Error::SYS();
			}
		} while(size_t(ws) < _tmp.len());
	}
	_tmp.reset();

	return NULL;
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
Error Dlogger::er(const char* fmt, ...)
{
	_locker.lock();

	va_list args;
	va_start(args, fmt);
	va_copy(_args, args);

	Error err = _w(STDERR_FILENO, fmt);

	va_end(_args);
	va_end(args);

	_locker.unlock();

	return err;
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
Error Dlogger::out(const char* fmt, ...)
{
	_locker.lock();

	va_list args;
	va_start(args, fmt);
	va_copy(_args, args);

	Error err = _w(STDOUT_FILENO, fmt);

	va_end(_args);
	va_end(args);

	_locker.unlock();

	return err;
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
Error Dlogger::it(const char* fmt, ...)
{
	_locker.lock();

	va_list args;
	va_start(args, fmt);
	va_copy(_args, args);

	Error err = _w(0, fmt);

	va_end(_args);
	va_end(args);

	_locker.unlock();

	return err;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=80:
