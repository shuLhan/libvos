/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "File.hpp"

namespace vos {

const char* __eol[N_EOL_MODE] = {
	"\n",
	"\r\n"
};

unsigned int File::DFLT_SIZE = 8192;

File::File(const unsigned int bfr_size) : Buffer(),
	_d(0),
	_p(0),
	_status(FILE_OPEN_NO),
	_eol(__eol[EOL_NIX][0]),
	_eols(__eol[EOL_NIX]),
	_name()
{
	Buffer::resize(bfr_size);
}

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
 *	< <0	: fail, error at opening file.
 * @desc	:
 *	the generic method to open file with specific mode and permission.
 */
int File::_open(const char* path, const int mode, const int perm)
{
	register int s;

	if (!path) {
		return -1;
	}
	if (!_v) {
		s = resize(DFLT_SIZE);
		if (s < 0) {
			return s;
		}
	}

	_d = ::open(path, mode, perm);
	if (_d < 0) {
		_d = 0;
		return -1;
	}

	s = _name.copy_raw(path);
	if (s < 0) {
		return s;
	}

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
 *	< <0	: fail, error at opening file.
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
 *	< <0	: fail, error at opening file.
 * @desc	: open file for write only.
 */
int File::open_wo(const char* path)
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
int File::open_wa(const char* path)
{
	return _open(path, FILE_OPEN_WA);
}

/**
 * @method	: File::get_size
 * @return	:
 *	< >0	: size of file.
 *	< 0	: if file is empty.
 *	< <0	: fail, error at seek.
 * @desc	: get current size of file.
 */
off_t File::get_size()
{
	register off_t s;
	register off_t cur;
	register off_t size;

	cur = lseek(_d, 0, SEEK_CUR);
	if (cur < 0) {
		return -1;
	}
	s = lseek(_d, 0, SEEK_SET);
	if (s < 0) {
		return -1;
	}
	size = lseek(_d, 0, SEEK_END);
	if (size < 0) {
		return -1;
	}
	cur = lseek(_d, cur, SEEK_SET);
	if (cur < 0) {
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
 *	< >0	: success, return number of bytes readed.
 *	< 0	: EOF
 *	< <0	: fail, error at reading descriptor.
 * @desc	: read contents of file and saved it to buffer.
 */
int File::read()
{
	if (_status == O_WRONLY) {
		return 0;
	}

	_i = (int) ::read(_d, &_v[0], _l);
	if (_i < 0) {
		return -1;
	}

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
	if (_status == O_WRONLY) {
		return 0;
	}

	register int s;

	if (n > _l) {
		s = resize(n);
		if (s != 0) {
			return s; 
		}
	}
	_i = 0;
	while (n > 0) {
		s = (int) ::read(_d, &_v[_i], n);
		if (s < 0) {
			if (s == EAGAIN || s == EWOULDBLOCK) {
				break;
			}
			return -1;
		}
		if (s == 0) {
			break;
		}
		_i += s;
		n -= s;
	}

	_p	= 0;
	_v[_i]	= '\0';

	return _i;
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
int File::refill(int read_min)
{
	if (_status == O_WRONLY) {
		return 0;
	}

	register int move_len	= 0;
	register int len	= 0;

	move_len = _i - _p;
	if (move_len > 0 && _p > 0) {
		memmove(&_v[0], &_v[_p], move_len);
	} else { /* move_len <= 0 || _p <= 0 */
		_p = 0;
		return 0;
	}

	len = move_len + read_min;
	if (len > _l) {
		if (LIBVOS_DEBUG) {
			printf("[VOS::FILE] read resize: from %d to %d\n"
				, _l, len);
		}
		resize(len);
		len -= move_len;
	} else {
		len = _l - move_len;
		if (len <= 0) {
			len = _l * 2;
			if (LIBVOS_DEBUG) {
				printf("[VOS::FILE] read resize: from %d to %d\n"
					, _l, len);
			}
			resize(len);
			len -= move_len;
		}
	}

	_i = (int) ::read(_d, &_v[move_len], len);
	if (_i < 0) {
		return -1;
	}

	_i	+= move_len;
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
int File::get_line(Buffer* line)
{
	if (_status == O_WRONLY || !line) {
		return 0;
	}

	register int s;
	register int start;
	register int len;

	if (_i == 0) {
		s = File::read();
		if (s <= 0) {
			return s;
		}
	}

	start = _p;
	while (_v[_p] != _eol) {
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
				s = (int) ::read(_d, &_v[_i], len);
				if (s < 0) {
					return -1;
				}
				if (s == 0) {
					break;
				}
				_i	+= s;
				len	-= s;
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

	len = _p - start;
	if (len == 0) {
		if (_p < _i) {
			line->reset();
			_p++;
			return 1;
		}
		return 0;
	}

	s = line->copy_raw(&_v[start], len);
	if (s < 0) {
		return s;
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
 *	< <0	: fail, error at writing to descriptor.
 * @desc	: append buffer 'bfr' to File buffer for writing.
 */
int File::write(const Buffer* bfr)
{
	if (_status == O_RDONLY || !bfr) {
		return 0;
	}
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
int File::write_raw(const char* bfr, int len)
{
	if (_status == O_RDONLY || !bfr) {
		return 0;
	}
	if (!len) {
		len = (int) strlen(bfr);
		if (!len) {
			return 0;
		}
	}

	register int x = 0;
	register int s;

	/* direct write */
	if (len >= _l) {
		s = flush();
		if (s < 0) {
			return s;
		}
		while (len > 0) {
			s = (int) ::write(_d, &bfr[x], len);
			if (s < 0) {
				return -1;
			}
			x	+= s;
			len	-= s;
		}
		len = x;
	} else {
		if (_l < (_i + len)) {
			s = flush();
			if (s < 0) {
				return s;
			}
		}
		s = append_raw(bfr, len);
		if (s < 0) {
			return s;
		}
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
int File::writef(const char* fmt, va_list args)
{
	if (_status == O_RDONLY || !fmt) {
		return 0;
	}

	register int	s;
	Buffer		b;

	s = b.vprint(fmt, args);
	if (s < 0) {
		return s;
	}

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
 */
int File::writes(const char* fmt, ...)
{
	if (_status == O_RDONLY || !fmt) {
		return 0;
	}

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
	if (_status == O_RDONLY) {
		return 0;
	}

	register int s;

	if (_i + 1 >= _l) {
		s = flush();
		if (s < 0) {
			return s;
		}
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
	if (_status == O_RDONLY) {
		reset();
		return 0;
	}

	register int x = 0;
	register int s;

	while (_i > 0) {
		s = (int) ::write(_d, &_v[x], _i);
		if (s < 0) {
			return -1;
		}
		x	+= s;
		_i	-= s;
	}
	_i = x;

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

	if (_d && (_d != STDOUT_FILENO && _d != STDERR_FILENO)) {
		::close(_d);
	}

	_status = FILE_OPEN_NO;
	_d	= 0;
}

/**
 * @method	: File::dump
 * @desc	: dump content of buffer to standard output.
 */
void File::dump()
{
	printf("[FILE-%d]\n", _d);
	printf("\t name     : %s\n", _name.v());
	printf("\t contents :\n"
		"\t>>\n%s\n<<\n", v());
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
off_t File::GET_SIZE(const char* path)
{
	if (!path) {
		return 0;
	}

	register int fd;
	register off_t size;

	fd = ::open(path, O_RDONLY);
	if (fd < 0) {
		return 0;
	}

	size = ::lseek(fd, 0, SEEK_END);
	if (size < 0) {
		size = 0;
	}

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
int File::IS_EXIST(const char* path, int acc_mode)
{
	if (!path) {
		return 0;
	}

	register int fd;

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
 *	< <0		: fail.
 * @desc		:
 * get the basename, last node, of path, it could be a file or directory.
 */
int File::BASENAME(Buffer* name, const char* path)
{
	if (!name) {
		return -1;
	}

	register int	s;
	register int	len;
	register int	p;

	name->reset();

	if (!path) {
		s = name->appendc('.');
		if (s < 0) {
			return s;
		}
	} else {
		len = (int) strlen(path);
		if (path[0] == '/' && len == 1) {
			s = name->appendc('/');
			if (s < 0) {
				return s;
			}
		} else {
			p = len - 1;
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
			s = name->copy_raw(&path[p], len - p);
			if (s < 0) {
				return s;
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

	register int	s;
	File		from;
	File		to;

	s = from.open_ro(src);
	if (s < 0) {
		return s;
	}

	s = to.open_wo(dst);
	if (s < 0) {
		return s;
	}

	s = from.read();
	while (s > 0) {
		s = to.write(&from);
		if (s < 0) {
			return s;
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
				return s;
			}
			::close(s);
			s = 0;
		}
	}

	return s;
}

} /* namespace::vos */
