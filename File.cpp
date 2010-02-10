/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "File.hpp"

namespace vos {

const char *__eol[N_FILE_EOL_TYPE] = {
	"\n",
	"\r\n"
};

unsigned int File::DFLT_BUFFER_SIZE = 8192;

File::File() : Buffer(),
	_d(0),
	_p(0),
	_status(FILE_OPEN_NO),
	_eol(FILE_EOL_NIX),
	_name()
{}

File::~File()
{
	close();
}

/**
 * @method		: File::init
 * @param		:
 *	> bfr_size	: size of File buffer, default to DFLT_BUFFER_SIZE.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: initialize File object with specific buffer size.
 */
int File::init(const int bfr_size)
{
	register int s;

	s = Buffer::init_size(bfr_size);
	if (s == 0)
		s = _name.init_size(Buffer::DFLT_SIZE);

	return s;
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
 *	< <0	: fail, error at opening file.
 * @desc	:
 *	the generic method to open file with specific mode and permission.
 */
int File::_open(const char *path, const int mode, const int perm)
{
	register int s;

	if (!path) {
		return -1;
	}
	if (!_v) {
		s = File::init();
		if (s < 0)
			return s;
	}

	_d = ::open(path, mode, perm);
	if (_d < 0) {
		_d = 0;
		return -1;
	}

	s = _name.copy_raw(path, 0);
	if (s < 0)
		return s;

	_status = (mode & (O_RDONLY | O_WRONLY | O_RDWR));

	return 0;

}

/**
 * @method	: File::open
 * @param	:
 *	> path	: path to a file name.
 * @return	:
 *	< 0	: success, or 'path' is nil.
 *	< <0	: fail, error at opening file.
 * @desc	:
 *	open file for read and write, create a file if it is not exist.
 */
int File::open(const char *path)
{
	return _open(path, FILE_OPEN_RW);
}

/**
 * @method	: File::open_ro
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, error at opening file.
 * @desc	: open file for read only.
 */
int File::open_ro(const char *path)
{
	return _open(path, FILE_OPEN_R);
}

/**
 * @method	: File::open_wo
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, error at opening file.
 * @desc	: open file for write only.
 */
int File::open_wo(const char *path)
{
	return _open(path, FILE_OPEN_W);
}

/**
 * @method	: File::open_wa
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: open file for write and append.
 */
int File::open_wa(const char *path)
{
	return _open(path, FILE_OPEN_WA);
}

/**
 * @method	: File::read
 * @return	:
 *	< >0	: success, return number of bytes readed.
 *	< 0	: EOF
 *	< <0	: fail, error at reading descriptor.
 * @desc	: read contents of file and saved it to buffer.
 */
int File::read()
{
	if (_status == O_WRONLY)
		return 0;

	_i = ::read(_d, &_v[0], _l);
	if (_i < 0)
		return -1;

	_p	= 0;
	_v[_i]	= '\0';

	return _i;
}

/**
 * @method	: File::readn
 * @param	:
 *	> n	: number of bytes to be read from descriptor.
 * @return	:
 *	< >0	: success, return number of bytes readed.
 *	< 0	: EOF, or file is not open.
 *	< <0	: fail.
 * @desc	:
 *	read n bytes of characters from file, automatically increase buffer if n
 *	is greater than File buffer size.
 */
int File::readn(int n)
{
	register int s;

	if (_status == O_WRONLY)
		return 0;

	if (n > _l) {
		s = resize(n);
		if (s != 0)
			return s; 
	}
	_i = 0;
	while (n > 0) {
		s = ::read(_d, &_v[_i], n);
		if (s < 0) {
			if (s == EAGAIN || s == EWOULDBLOCK)
				break;
			return -1;
		}
		if (s == 0)
			break;
		_i += s;
		n -= s;
	}

	_p	= 0;
	_v[_i]	= '\0';

	return _i;
}

/**
 * @method	: File::get_line
 * @param	:
 *	> line	: out, pointer to Buffer object.
 * @return	:
 *	< 1	: success, one line readed.
 *	< 0	: success, EOF.
 *	< <0	: fail.
 * @desc	: get one line at a time from buffer.
 *
 *	- this operation will change contents of file buffer.
 *	- this operation return a string without a new line character.
 */
int File::get_line(Buffer **line)
{
	register int s;
	register int start;
	register int len;

	if (_i == 0) {
		s = File::read();
		if (s <= 0)
			return s;
	}

	start = _p;
	while (_v[_p] != GET_EOL_CHR(_eol)) {
		if (_p >= _i) {
			_p = _p - start;
			memmove(&_v[0], &_v[start], _p);

			len = _l - _p;
			if (len == 0) {
				len = _l;
				resize(_l * 2);
			}

			_i = _p;
			while (len > 0) {
				s = ::read(_d, &_v[_i], len);
				if (s < 0) {
					return -1;
				}
				if (s == 0)
					break;
				_i	+= s;
				len	-= s;
			}

			start	= 0;
			_v[_i]	= '\0';
			if (_i == 0)
				break;
		} else {
			++_p;
		}
	}

	len = _p - start;
	if (len == 0)
		return 0;

	if (!(*line)) {
		s = Buffer::INIT_SIZE(line, len + 1);
		if (s < 0)
			return s;
	}

	s = (*line)->copy_raw(&_v[start], len);
	if (s < 0)
		return s;

	_p++;

	return 1;
}

/**
 * @method	: File::write
 * @param	:
 *	> bfr	: Buffer object to be write to file.
 * @return	:
 *	< >=0	: success, number of bytes appended to File buffer.
 *	< <0	: fail, error at writing to descriptor.
 * @desc	: append buffer 'bfr' to File buffer for writing.
 */
int File::write(const Buffer *bfr)
{
	if (!bfr)
		return 0;

	return write_raw(bfr->_v, bfr->_i);
}

/**
 * @method	: File::write_raw
 * @param	:
 *	> bfr	: pointer to raw buffer.
 *	> len	: length of 'bfr' to write, default to zero.
 * @return	:
 *	< >=0	: success, number of bytes appended to File buffer.
 *	< <0	: fail, error at writing to descriptor.
 * @desc	: append buffer 'bfr' to File buffer for writing.
 */
int File::write_raw(const char *bfr, int len)
{
	register int x = 0;
	register int s;

	if (_status == O_RDONLY || ! bfr) {
		return 0;
	}
	if (!bfr) {
		return 0;
	}
	if (!len) {
		len = strlen(bfr);
		if (!len)
			return 0;
	}

	/* direct write */
	if (len >= _l) {
		s = flush();
		if (s < 0)
			return s;

		while (len > 0) {
			s = ::write(_d, &bfr[x], len);
			if (s < 0)
				return -1;

			x	+= s;
			len	-= s;
		}
		len = x;
	} else {
		if (_l < (_i + len)) {
			s = flush();
			if (s < 0)
				return s;
		}
		s = append_raw(bfr, len);
		if (s < 0)
			return s;
	}

	return len;
}

/**
 * @method	: File::writef
 * @param	:
 *	> fmt	: formatted string.
 *	> args	: list of arguments for formatted string.
 * @return	:
 *	< >=0	: success, return number of bytes written to file.
 *	< 0	: success, file is not open.
 *	< <0	: fail.
 * @desc	: write buffer of formatted string to file.
 */
int File::writef(const char *fmt, va_list args)
{
	register int	s;
	Buffer		b;

	if (_status == O_RDONLY || ! fmt)
		return 0;

	s = b.vprint(fmt, args);
	if (s < 0)
		return s;

	return write_raw(b._v, b._i);
}

/**
 * @method	: File::writes
 * @param	:
 *	> fmt	: formatted string.
 *	> ...	: any arguments for value in formatted string.
 * @return	:
 *	< >=0	: success, return number of bytes written to file.
 *	< <0	: fail.
 * @desc	: write buffer of formatted string to file.
 *
 *	NOTE: arguments must end with NULL.
 */
int File::writes(const char *fmt, ...)
{
	register int	s;
	Buffer		b;
	va_list		al;

	va_start(al, fmt);
	s = b.vprint(fmt, al);
	va_end(al);

	return write_raw(b._v, b._i);
}

/**
 * @method	: File::writec
 * @param	:
 *	> c	: a character to be appended to file.
 * @return	:
 *	< 1	: success.
 *	< <0	: fail.
 * @desc	: write one character to file.
 */
int File::writec(const char c)
{
	register int s;

	if (_i + 1 >= _l) {
		s = flush();
		if (s < 0)
			return s;
	}

	_v[_i++] = c;

	return 1;
}

/**
 * @method	: File::flush
 * @return	:
 *	< >=0	: success, size of buffer flushed to the system, in bytes.
 *	< <0	: fail, error at writing to descriptor.
 * @desc	: flush buffer cache; write all File buffer to disk.
 */
int File::flush()
{
	register int x = 0;
	register int s;

	if (_status == O_RDONLY)
		return 0;

	while (_i > 0) {
		s = ::write(_d, &_v[x], _i);
		if (s < 0)
			return -1;

		x	+= s;
		_i	-= s;
	}

	reset();

	return x;
}

/**
 * @method	: File::close
 * @desc	: close file descriptor.
 */
void File::close()
{
	flush();
	_name.reset();
	if (_d && _d != STDOUT_FILENO && _d != STDERR_FILENO)
		::close(_d);
	_status = FILE_OPEN_NO;
	_d	= 0;
}

/**
 * @method	: File::dump
 * @desc	: dump content of buffer to standard output.
 */
void File::dump()
{
	printf("file descriptor: %d\n", _d);
	printf("file name      : %s\n", _name._v);
	printf("file contents  :\n>> %s\n", _v);
}

/**
 * @method	: File::get_size
 * @return	:
 *	< >0	: size of file.
 *	< 0	: if file is empty.
 *	< <0	: fail, error at seek.
 * @desc	: get current size of file.
 */
int File::get_size()
{
	register off_t s;
	register off_t cur;
	register off_t size;

	cur = lseek(_d, 0, SEEK_CUR);
	if (cur < 0)
		return -1;

	s = lseek(_d, 0, SEEK_SET);
	if (s < 0)
		return -1;

	size = lseek(_d, 0, SEEK_END);
	if (size < 0)
		return -1;

	cur = lseek(_d, cur, SEEK_SET);
	if (cur < 0)
		return -1;

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
	if (mode == FILE_EOL_NIX || mode == FILE_EOL_DOS)
		_eol = mode;
}

/**
 * @method	: File::GET_SIZE
 * @param	:
 *	> path	: path to a file.
 * @return	:
 *	< >0	: size of file in path.
 *	< 0	: if file is empty, or file is not exist.
 * @desc	: get the size of file.
 */
int File::GET_SIZE(const char *path)
{
	register int fd;
	register int size;

	if (!path)
		return 0;

	fd = ::open(path, O_RDONLY);
	if (fd < 0)
		return 0;

	size = ::lseek(fd, 0, SEEK_END);
	if (size < 0)
		size = 0;

	::close(fd);

	return size;
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
int File::IS_EXIST(const char *path, int acc_mode)
{
	register int fd;

	if (!path)
		return 0;

	fd = ::open(path, acc_mode);
	if (fd < 0)
		return 0;

	::close(fd);

	return 1;
}

/**
 * @method		: File::BASENAME
 * @param		:
 *	> name		: return value, the last directory of 'path'.
 *	> path		: a path to directory or file.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: get the basename, last directory, of path.
 */
int File::BASENAME(Buffer *name, const char *path)
{
	register int	s;
	register int	len;
	register int	p;

	if (!name)
		return -1;

	if (!path) {
		s = name->appendc('.');
		if (s < 0)
			return s;
	} else {
		if (path[0] != '/') {
			s = name->copy_raw(path, 0);
			if (s < 0)
				return s;
		} else {
			len = strlen(path);
			if (path[0] == '/' && len == 1) {
				s = name->copy_raw(path, 0);
				if (s < 0)
					return s;
			} else {
				p = len - 1;
				if (path[p] == '/')
					--len;

				do {
					--p;
				} while (path[p] != '/');

				++p;
				s = name->copy_raw(&path[p], len - p);
				if (s < 0)
					return s;
			}
		}
	}
	return 0;
}

} /* namespace::vos */
