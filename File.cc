//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "File.hh"

namespace vos {

Error ErrFileEmpty("File: empty");
Error ErrFileNotFound("File: path is empty or invalid");

const char* File::__CNAME = "File";

uint16_t File::DFLT_SIZE = 4096;

const char* __eol[N_EOL_MODE] = {
	"\n",
	"\r\n"
};

/**
 * Method GET_SIZE(path) will get size of file `path` and save it to `size`.
 *
 * On success it will return the file size, otherwise it will return error:
 *
 * - ErrFileNotFound if `path` is empty of invalid.
 */
Error File::GET_SIZE(const char* path, off_t* size)
{
	if (!path) {
		return ErrFileNotFound.with(path, strlen(path));
	}

	int fd = 0;

	fd = ::open(path, O_RDONLY);
	if (fd < 0) {
		if (errno == ENOENT) {
			return ErrFileNotFound.with(path, strlen(path));
		}
		return Error::SYS();
	}

	*size = ::lseek(fd, 0, SEEK_END);
	if (*size < 0) {
		size = 0;
	}

	::close(fd);

	return NULL;
}

/**
 * @method		: File::IS_EXIST
 * @param		:
 *	> path		: a path to directory or file.
 *	> acc_mode	: access mode; read only (O_RDONLY), write only
 *                        (O_WRONLY), or read-write (O_RDWR).
 *                        Default to read-write.
 * @return		:
 *	< 1		: if 'path' is exist and accesible by user.
 *	< 0		: if 'path' does not exist or user does not have
 *                        permission to access it.
 * @desc		:
 *	check if 'path' is exist in file system and user had a permission to
 *	access it.
 */
int File::IS_EXIST(const char* path, int acc_mode)
{
	if (!path) {
		return 0;
	}

	int fd = 0;

	fd = ::open(path, acc_mode);
	if (fd < 0) {
		return 0;
	}

	::close(fd);

	return 1;
}

/**
 * @method		: File::BASENAME
 * @param		:
 *	> name		: return value, the last node of 'path'.
 *	> path		: a path to directory or file.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 * get the basename, last node, of path, it could be a file or directory.
 */
int File::BASENAME(Buffer* name, const char* path)
{
	if (!name) {
		return -1;
	}

	Error err;

	name->reset();

	if (!path) {
		err = name->appendc('.');
		if (err != NULL) {
			return -1;
		}
	} else {
		size_t len = strlen(path);
		if (path[0] == '/' && len == 1) {
			err = name->appendc('/');
			if (err != NULL) {
				return -1;
			}
		} else {
			size_t p = len - 1;
			while (p > 0 && path[p] == '/') {
				--len;
				--p;
			}
			while (p > 0 && path[p] != '/') {
				--p;
			}
			if (path[p] == '/' && path[p + 1] != '/') {
				++p;
			}
			err = name->copy_raw(&path[p], size_t(len - p));
			if (err != NULL) {
				return -1;
			}
		}
	}
	return 0;
}

/**
 * @method	: File::COPY
 * @param	:
 *	> src	: a path to source file to copy.
 *	> dst	: a path to destination file.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	:
 *
 * copy file 'src' to 'dst', create a new file if 'dst' is not exist, or
 * overwrite 'dst' if already exist.
 *
 * NOTE: use 'rename()' system call for easy and fast move.
 */
int File::COPY(const char* src, const char* dst)
{
	if (!src || !dst) {
		return -1;
	}

	ssize_t s = 0;
	File from;
	File to;

	s = from.open_ro(src);
	if (s < 0) {
		return -1;
	}

	s = to.open_wo(dst);
	if (s < 0) {
		return -1;
	}

	s = from.read();
	while (s > 0) {
		s = to.write(&from);
		if (s < 0) {
			return -1;
		}

		s = from.read();
	}

	return 0;
}

/**
 * @method		: File::TOUCH
 * @param		:
 *	> filename	: path to a file name to create or update.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 * touch a 'filename', create 'filename' if it not exist or update
 * access and modification time if it already exist.
 */
int File::TOUCH(const char* filename)
{
	if (!filename) {
		return -1;
	}

	int s;

	s = utime(filename, NULL);
	if (s < 0) {
		if (errno == ENOENT) {
			s = ::open(filename, FILE_OPEN_WA, S_IRUSR | S_IWUSR);
			if (s < 0) {
				perror(__CNAME);
				return -1;
			}
			::close(s);
			s = 0;
		}
	}

	return s;
}

/**
 * @method	: write_pid
 * @param	:
 *	> file	: path to a file, where PID will be written.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: function to write PID to 'file'.
 *
 *	This function usually used by process daemon.
 *
 *	When daemon started, daemon will write its process id to file, to :
 *	- make user know that daemon is already running, so
 *	- no other daemon running, just one, or
 *	- if daemon is not running but PID file exist, that mean is
 *	  last daemon is exit in abnormal state; so see log file for
 *	  further information.
 */
int File::WRITE_PID(const char* file)
{
	int s = 0;
	File f;

	s = f.open_wx(file);
	if (0 == s) {
		Error err = f.appendi(getpid());
		if (err != NULL) {
			return -1;
		}
	}

	return s;
}



File::File(const size_t bfr_size) : Buffer(bfr_size)
,	_d(0)
,	_p(0)
,	_status(FILE_OPEN_NO)
,	_perm (0)
,	_size (0)
,	_eol(__eol[EOL_NIX][0])
,	_eols(__eol[EOL_NIX])
,	_name()
{}

File::~File()
{
	close();
}

/**
 * @method	: File::open
 * @param	:
 *	> path	: path to a file name.
 *	> mode	: mode for opened file.
 *	> perm	: permission for a new file.
 *                default to 0600 (read-write for user only).
 * @return	:
 *	< 0	: success, or 'path' is nil.
 *	< -1	: fail, error at opening file.
 * @desc	:
 *	the generic method to open file with specific mode and permission.
 */
int File::_open(const char* path, const int mode, const int perm)
{
	Error err;

	if (!path) {
		return -1;
	}
	if (!_v) {
		err = resize(DFLT_SIZE);
		if (err != NULL) {
			return -1;
		}
	}

	_d = ::open(path, mode, perm);
	if (_d < 0) {
		perror(__CNAME);
		_d = 0;
		return -1;
	}

	err = _name.copy_raw(path);
	if (err != NULL) {
		return -1;
	}

	_size = get_size ();
	_status = (mode & (O_RDONLY | O_WRONLY | O_RDWR));
	_perm = perm;

	return 0;

}

/**
 * @method	: File::open
 * @param	:
 *	> path	: path to a file name.
 * @return	:
 *	< 0	: success, or 'path' is nil.
 *	< -1	: fail, error at opening file.
 * @desc	:
 *	open file for read and write, create a file if it is not exist.
 */
int File::open(const char* path)
{
	return _open(path, FILE_OPEN_RW);
}

/**
 * @method	: File::open_ro
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail, error at opening file.
 * @desc	: open file for read only.
 */
int File::open_ro(const char* path)
{
	return _open(path, FILE_OPEN_R);
}

/**
 * @method	: File::open_wo
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail, error at opening file.
 * @desc	: open file for write only.
 */
int File::open_wo(const char* path)
{
	return _open(path, FILE_OPEN_W);
}

/**
 * @method	: File::open_wx
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, if file exist make only one instances running
 * @desc	: open file for write only.
 */
int File::open_wx(const char* path)
{
	return _open(path, FILE_OPEN_WX);
}

/**
 * @method	: File::open_wa
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: open file for write and append.
 */
int File::open_wa(const char* path)
{
	return _open(path, FILE_OPEN_WA);
}

/**
 * @method	: File::truncate
 * @desc	: reset file size to 0.
 * @param flush_mode : if FILE_TRUNC_FLUSH_NO, buffer will not be flushed to file. If it FILE_TRUNC_FLUSH_FIRST, buffer will be reset first and then file will be truncated. If it FILE_TRUNC_FLUSH_LAST  then buffer will be flushed after file was truncated.
 * @return 0	: success.
 * @return -1	: fail to close.
 * @return -2	: fail to open.
 * @return -3	: fail at flushing the buffer;
 */
int File::truncate (uint8_t flush_mode)
{
	ssize_t s = 0;

	if (flush_mode & FILE_TRUNC_FLUSH_FIRST) {
		reset ();
	}

	// Try to close file.
	if (_d && (_d != STDOUT_FILENO && _d != STDERR_FILENO)) {
		s = ::close(_d);
		if (s != 0) {
			perror(__CNAME);
			return -1;
		}
	}

	// Reopen file by truncate.
	s = _open (_name.v(), _status | O_CREAT | O_TRUNC, _perm);
	if (s != 0) {
		return -2;
	}

	_size = 0;

	if (flush_mode & FILE_TRUNC_FLUSH_LAST) {
		s = flush ();
		if (s != 0) {
			return -3;
		}
	}

	return 0;
}

/**
 * `is_open()` will return positive integer if file descriptor is open, or `0`
 * if closed.
 */
int File::is_open()
{
	return (_d > 0);
}

/**
 * @method	: File::get_size
 * @return	:
 *	< >0	: size of file.
 *	< 0	: if file is empty.
 *	< -1	: fail, error at seek.
 * @desc	: get current size of file.
 */
off_t File::get_size()
{
	off_t s = 0;
	off_t cur = 0;
	off_t size = 0;

	cur = lseek(_d, 0, SEEK_CUR);
	if (cur < 0) {
		perror(__CNAME);
		return -1;
	}
	s = lseek(_d, 0, SEEK_SET);
	if (s < 0) {
		perror(__CNAME);
		return -1;
	}
	size = lseek(_d, 0, SEEK_END);
	if (size < 0) {
		perror(__CNAME);
		return -1;
	}
	cur = lseek(_d, cur, SEEK_SET);
	if (cur < 0) {
		perror(__CNAME);
		return -1;
	}
	return size;
}

/**
 * @method	: File::set_eol
 * @param	:
 *	> mode	: mode of end of line: Unix or DOS.
 * @desc	: set end of line mode for this current File object.
 */
void File::set_eol(const int mode)
{
	if (mode == EOL_NIX || mode == EOL_DOS) {
		_eol	= __eol[mode][0];
		_eols	= __eol[mode];
	}
}

/**
 * @method	: File::read
 * @return	:
 *	< >0	: success, return number of bytes read.
 *	< 0	: EOF
 *	< -1	: fail, error at reading descriptor.
 * @desc	: read contents of file and saved it to buffer.
 */
ssize_t File::read()
{
	if (_status == O_WRONLY) {
		return 0;
	}

	ssize_t s = 0;

	s = ::read(_d, &_v[0], _l);
	if (s < 0) {
		perror(__CNAME);
		return -1;
	}

	_i	= size_t(s);
	_p	= 0;
	_v[_i]	= '\0';

	return s;
}

/**
 * @method	: File::readn
 * @param	:
 *	> n	: number of bytes to be read from descriptor.
 * @return	:
 *	< >0	: success, return number of bytes read.
 *	< 0	: EOF, or file is not open.
 *	< -1	: fail.
 * @desc	:
 *	read n bytes of characters from file, automatically increase buffer if n
 *	is greater than File buffer size.
 */
ssize_t File::readn(size_t n)
{
	if (_status == O_WRONLY) {
		return 0;
	}

	if (n > _l) {
		Error err = resize(n);
		if (err != NULL) {
			return -1;
		}
	}
	_i = 0;
	while (n > 0) {
		ssize_t s = ::read(_d, &_v[_i], n);
		if (s < 0) {
			if (s == EAGAIN || s == EWOULDBLOCK) {
				break;
			}
			perror(__CNAME);
			return -1;
		}
		if (s == 0) {
			break;
		}
		_i += size_t(s);
		n -= size_t(s);
	}

	_p	= 0;
	_v[_i]	= '\0';

	return ssize_t(_i);
}

/**
 * @method		: File::refill
 * @param		:
 *	> read_min	: minimum length of buffer to fill, default to '0'.
 * @return		:
 *	< >=0		: success.
 *	< -1		: fail.
 * @desc		: Refill buffer with new data.
 *
 * All data from position of '_p' until '_i' will be moved to the beginning of
 * buffer and will not be replaced, new data will be filled in position after
 * (_i - _p).
 */
ssize_t File::refill(size_t read_min)
{
	if (_status == O_WRONLY) {
		return 0;
	}

	ssize_t s = 0;
	size_t move_len = 0;
	size_t len = 0;

	move_len = _i - _p;
	if (move_len > 0 && _p > 0) {
		memmove(&_v[0], &_v[_p], size_t(move_len));
	} else {
		// move_len == 0 || _p == 0
		_p = 0;
		return 0;
	}

	len = move_len + read_min;
	if (len > _l) {
		if (LIBVOS_DEBUG) {
			printf("[%s] refill: read resize from '%zu' to '%zu'\n"
				, __CNAME, _l, len);
		}
		resize(len);
		len -= move_len;
	} else {
		len = _l - move_len;
		if (len == 0) {
			len = _l * 2;
			if (LIBVOS_DEBUG) {
				printf("[%s] refill: read resize from '%zu' to '%zu'\n"
					, __CNAME, _l, len);
			}
			resize(len);
			len -= move_len;
		}
	}

	s = ::read(_d, &_v[move_len], len);
	if (s < 0) {
		perror(__CNAME);
		return -1;
	}

	_i = size_t(s);

	_i	+= move_len;
	_p	= 0;
	_v[_i]	= '\0';

	return s;
}

/**
 * @method	: File::get_line
 * @param	:
 *	> line	: out, pointer to Buffer object.
 * @return	:
 *	< 1	: success, one line read.
 *	< 0	: success, EOF.
 *	< -1	: fail.
 * @desc	: get one line at a time from buffer.
 *
 *	- this operation will change contents of file buffer.
 *	- this operation return a string without a new line character.
 */
int File::get_line(Buffer* line)
{
	if (_status == O_WRONLY || !line) {
		return 0;
	}

	ssize_t s = 0;
	size_t start = 0;
	ssize_t len = 0;

	if (_i == 0) {
		s = File::read();
		if (s <= 0) {
			perror(__CNAME);
			return -1;
		}
	}

	start = _p;
	while (_v[_p] != _eol) {
		if (_p >= _i) {
			_p = _p - start;
			memmove(&_v[0], &_v[start], _p);

			len = ssize_t(_l - _p);
			if (len == 0) {
				len = ssize_t(_l);
				resize(_l * 2);
			}

			_i = _p;
			while (len > 0) {
				s = ::read(_d, &_v[_i], size_t(len));
				if (s < 0) {
					perror(__CNAME);
					return -1;
				}
				if (s == 0) {
					break;
				}

				_i	+= size_t(s);
				len	-= s;
			}
			if (s == 0) {
				break;
			}

			start	= 0;
			_v[_i]	= '\0';
			if (_i == 0) {
				break;
			}
		} else {
			++_p;
		}
	}

	len = ssize_t(_p - start);
	if (len <= 0) {
		if (_p != 0 && _p < _i) {
			line->reset();
			_p++;
			return 1;
		}
		return 0;
	}

	Error err = line->copy_raw(&_v[start], size_t(len));
	if (err != NULL) {
		return -1;
	}

	_p++;

	return 1;
}

/**
 * @method	: File::write
 * @param	:
 *	> bfr	: Buffer object to be write to file.
 * @return	:
 *	< >=0	: success, number of bytes appended to File buffer.
 *	< -1	: fail, error at writing to descriptor.
 * @desc	: append buffer 'bfr' to File buffer for writing.
 */
ssize_t File::write(const Buffer* bfr)
{
	if (_status == O_RDONLY || !bfr) {
		return 0;
	}
	return write_raw(bfr->v(), bfr->len());
}

/**
 * @method	: File::write_raw
 * @param	:
 *	> bfr	: pointer to raw buffer.
 *	> len	: length of 'bfr' to write, default to zero.
 * @return	:
 *	< >=0	: success, number of bytes appended to File buffer.
 *	< -1	: fail, error at writing to descriptor.
 * @desc	: append buffer 'bfr' to File buffer for writing.
 */
ssize_t File::write_raw(const char* bfr, size_t len)
{
	if (_status == O_RDONLY || !bfr) {
		return 0;
	}
	if (!len) {
		len = strlen(bfr);
		if (!len) {
			return 0;
		}
	}

	ssize_t s = 0;

	// direct write
	if (len >= _l) {
		s = flush();
		if (s < 0) {
			return -1;
		}

		size_t x = 0;

		while (len > 0) {
			s = ::write(_d, &bfr[x], len);
			if (s < 0) {
				perror(__CNAME);
				return -1;
			}
			x	+= size_t(s);
			len	-= size_t(s);
		}
		len = x;
		_size += len;
	} else {
		if (_l < (_i + len)) {
			s = flush();
			if (s < 0) {
				return -1;
			}
		}
		Error err = append_raw(bfr, len);
		if (err != NULL) {
			return -1;
		}
		if (_status & O_SYNC) {
			flush();
		}
	}

	return ssize_t(len);
}

/**
 * @method	: File::writef
 * @param	:
 *	> fmt	: formatted string.
 *	> args	: list of arguments for formatted string.
 * @return	:
 *	< >=0	: success, return number of bytes written to file.
 *	< 0	: success, file is not open.
 *	< -1	: fail.
 * @desc	: write buffer of formatted string to file.
 */
ssize_t File::writef(const char* fmt, va_list args)
{
	if (_status == O_RDONLY || !fmt) {
		return 0;
	}

	Buffer b;

	Error err = b.vappend_fmt(fmt, args);
	if (err != NULL) {
		return -1;
	}

	return write_raw(b.v(), b.len());
}

/**
 * @method	: File::writes
 * @param	:
 *	> fmt	: formatted string.
 *	> ...	: any arguments for value in formatted string.
 * @return	:
 *	< >=0	: success, return number of bytes written to file.
 *	< -1	: fail.
 * @desc	: write buffer of formatted string to file.
 */
ssize_t File::writes(const char* fmt, ...)
{
	if (_status == O_RDONLY || !fmt) {
		return 0;
	}

	Buffer		b;
	va_list		al;

	va_start(al, fmt);
	Error err = b.vappend_fmt(fmt, al);
	va_end(al);

	if (err != NULL) {
		return -1;
	}

	return write_raw(b.v(), b.len());
}

/**
 * @method	: File::writec
 * @param	:
 *	> c	: a character to be appended to file.
 * @return	:
 *	< 1	: success.
 *	< -1	: fail.
 * @desc	: write one character to file.
 */
int File::writec(const char c)
{
	if (_status == O_RDONLY) {
		return 0;
	}

	if (_i + 1 >= _l) {
		ssize_t s = flush();
		if (s < 0) {
			return -1;
		}
	}

	_v[_i++] = c;

	if (_status & O_SYNC) {
		flush();
	}

	return 1;
}

/**
 * @method	: File::flush
 * @return	:
 *	< >=0	: success, size of buffer flushed to the system, in bytes.
 *	< -1	: fail, error at writing to descriptor.
 * @desc	: flush buffer cache; write all File buffer to disk.
 */
ssize_t File::flush()
{
	if (_status == O_RDONLY) {
		reset();
		return 0;
	}

	size_t x = 0;

	while (_i > 0) {
		ssize_t s = ::write(_d, &_v[x], _i);
		if (s < 0) {
			perror(__CNAME);
			return -1;
		}
		x	+= size_t(s);
		_i	-= size_t(s);
	}
	_i = x;
	_size += _i;

	reset();

	return ssize_t(x);
}

/**
 * @method	: File::close
 * @desc	: close file descriptor.
 */
void File::close()
{
	flush();
	_name.reset();

	if (_d && (_d != STDOUT_FILENO && _d != STDERR_FILENO)) {
		::close(_d);
	}

	_size	= 0;
	_status = FILE_OPEN_NO;
	_perm	= 0;
	_d	= 0;
}

/**
 * @method	: File::dump
 * @desc	: dump content of buffer to standard output.
 */
void File::dump()
{
	printf("[%s] dump:\n", __CNAME);
	printf("  descriptor  : %d\n", _d);
	printf("  name        : %s\n", _name.chars());
	printf("  size        : %ld\n", _size);
	printf("  contents    :\n[%s]\n", chars());
}

} // namespace::vos

// vi: ts=8 sw=8 tw=80:
