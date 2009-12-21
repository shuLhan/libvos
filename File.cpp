/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "File.hpp"

namespace vos {

const char *_file_eol[N_FILE_EOL_TYPE] = {
	"\n",
	"\r\n"
};

unsigned int File::DFLT_BUFFER_SIZE = 8192;

File::File() : Buffer(),
	_d(0),
	_p(0),
	_status(vos::FILE_OPEN_NO),
	_eol(_file_eol[FILE_EOL_NIX][0]),
	_name(),
	__eol(_file_eol[FILE_EOL_NIX])
{}

File::~File()
{
	close();
}

/**
 * @desc		: initialize File object.
 *
 * @param		:
 *	> bfr_size	: size of File buffer, default to DFLT_BUFFER_SIZE.
 *
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 */
int File::init(const int bfr_size)
{
	int s;

	s = Buffer::init_size(bfr_size);
	if (s < 0)
		return s;

	s = _name.init(NULL);

	return s;
}

/**
 * @desc	: open file for read and write, or create a file if it
 *		is not exist.
 *
 * @param	:
 *	> path	: path to a file name.
 *
 * @return	:
 *	< 0	: success, or 'path' is nil.
 *	< <0	: fail, error at opening file.
 */
int File::open(const char *path)
{
	int s;

	if (!path) {
		return 0;
	}
	if (!_v) {
		s = File::init(DFLT_BUFFER_SIZE);
		if (s < 0)
			return s;
	}

	_d = ::open(path, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (_d < 0) {
		_d = 0;
		return -E_FILE_OPEN;
	}

	s = _name.copy_raw(path, 0);
	if (s < 0)
		return s;

	_status = vos::FILE_OPEN_RW;

	return 0;
}

/**
 * @desc	: open file for read only.
 *
 * @param	:
 *	> path	: path to a file.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail, error at opening file.
 */
int File::open_ro(const char *path)
{
	int s;

	if (!path) {
		return 0;
	}
	if (!_v) {
		s = File::init(DFLT_BUFFER_SIZE);
		if (s < 0)
			return s;
	}

	_d = ::open(path, O_RDONLY);
	if (_d < 0) {
		_d = 0;
		return -E_FILE_OPEN;
	}

	s = _name.copy_raw(path, 0);
	if (s < 0)
		return s;

	_status = vos::FILE_OPEN_R;

	return 0;
}

/**
 * @desc	: open file for write only.
 *
 * @param	:
 *	> path	: path to a file.
 *
 * @return:
 *	< 0	: success.
 *	< <0	: fail, error at opening file.
 */
int File::open_wo(const char *path)
{
	int s;

	if (!path) {
		return 0;
	}
	if (!_v) {
		s = File::init(DFLT_BUFFER_SIZE);
		if (s < 0)
			return s;
	}

	_d = ::open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (_d < 0) {
		_d = 0;
		return -E_FILE_OPEN;
	}

	s = _name.copy_raw(path, 0);
	if (s < 0)
		return s;

	_status = vos::FILE_OPEN_W;

	return 0;
}

/**
 * @desc	: open file for write and append.
 *
 * @param	:
 *	> path	: path to a file.
 *
 * @return:
 *	< 0	: success.
 *	< !0	: fail, error at opening file.
 */
int File::open_wa(const char *path)
{
	int s;

	if (!path) {
		return 0;
	}
	if (!_v) {
		s = File::init(DFLT_BUFFER_SIZE);
		if (s < 0)
			return s;
	}

	_d = ::open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (_d < 0) {
		_d = 0;
		return -E_FILE_OPEN;
	}

	s = _name.copy_raw(path, 0);
	if (s < 0)
		return s;

	_status = vos::FILE_OPEN_W;

	return 0;
}

/**
 * @desc	: read contents of file to buffer.
 *
 * @return	:
 * 	< >0	: success, return number of bytes readed.
 * 	< 0	: EOF
 *	< <0	: fail, error at reading descriptor.
 */
int File::read()
{
	if (! (_status & vos::FILE_OPEN_R))
		return 0;

	_i = ::read(_d, _v, _l);
	if (_i < 0) {
		return -E_FILE_READ;
	}

	_p	= 0;
	_v[_i]	= '\0';

	return _i;
}


/**
 * @desc	: get one line at a time from buffer.
 *
 *	- this operation will change contents of file buffer.
 *	- this operation return a string without a new line character.
 *
 * @param	:
 *	> line	: out, pointer to Buffer object.
 *
 * @return	:
 *	< 1	: success, one line readed.
 *	< 0	: success, EOF.
 *	< <0	: fail.
 */
int File::get_line(Buffer **line)
{
	int	s;
	int	start;
	int	len	= 0;

	if (_i == 0) {
		s = File::read();
		if (s <= 0)
			return s;
	}

	start = _p;
	while (_v[_p] != _eol) {
		if (_p >= _i && _v[_p] != _eol) {
			_p = _p - start;
			memmove(&_v[0], &_v[start], _p);

			_i = ::read(_d, &_v[_p], _l - _p);
			if (_i < 0) {
				return -E_FILE_READ;
			}

			start	= 0;
			_i	+= _p;
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
 * @desc	: append buffer 'bfr' to File buffer for writing.
 *
 * @param	:
 *	> bfr	: Buffer object to be write to file.
 *
 * @return	:
 *	< >=0	: success, number of bytes appended to File buffer.
 *	< <0	: fail, error at writing to descriptor.
 */
int File::write(const Buffer *bfr)
{
	if (!bfr)
		return 0;

	return write_raw(bfr->_v, bfr->_i);
}

/**
 * @desc	: append buffer 'bfr' to File buffer for writing.
 *
 * @param	:
 *	> bfr	: pointer to raw buffer.
 *	> len	: length of 'bfr' to write.
 *
 * @return:
 *	< >=0	: success, number of bytes appended to File buffer.
 *	< <0	: fail, error at writing to descriptor.
 */
int File::write_raw(const char *bfr, int len)
{
	int x = 0;
	int s;

	if (! (_status & vos::FILE_OPEN_W) || ! bfr) {
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
				return -E_FILE_WRITE;

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
 * @desc	: write buffer of formatted string to file.
 *
 * @param	:
 *	> fmt	: formatted string.
 *	> args	: list of arguments for formatted string.
 *
 * @return	:
 *	< >=0	: success, return number of bytes written to file.
 *	< 0	: success, file is not open.
 *	< <0	: fail.
 */
int File::writef(const char *fmt, va_list args)
{
	int	s;
	Buffer	b;

	if (! (_status & vos::FILE_OPEN_W) || ! fmt)
		return 0;

	s = b.vprint(fmt, args);
	if (s < 0)
		return s;

	return write_raw(b._v, b._i);
}

/**
 * @desc	: write buffer of formatted string to file.
 *
 *	NOTE: arguments must end with NULL.
 *
 * @param	:
 *	> fmt	: formatted string.
 *
 * @return	:
 *	< >=0	: success, return number of bytes written to file.
 *	< <0	: fail.
 */
int File::writes(const char *fmt, ...)
{
	int	s;
	Buffer	b;
	va_list al;

	va_start(al, fmt);
	s = b.vprint(fmt, al);
	va_end(al);

	if (s < 0)
		return s;

	return write_raw(b._v, b._i);
}

/**
 * @desc	: write one character to file.
 *
 * @param	:
 *	> c	: character.
 *
 * @return	:
 *	< 1	: success.
 *	< <0	: fail.
 */
int File::writec(const char c)
{
	int s;

	if (_i + 1 >= _l) {
		s = flush();
		if (s < 0)
			return s;
	}

	_v[_i++] = c;

	return 1;
}

/**
 * @desc		: flush buffer cache.
 *
 * @return		:
 *	< >=0		: success, size of buffer flushed to the system, in
 *			bytes.
 *	< -E_FILE_WRITE	: fail, error at writing to descriptor.
 */
int File::flush()
{
	int x = 0;
	int s;

	if (! (_status & vos::FILE_OPEN_W))
		return 0;

	while (_i > 0) {
		s = ::write(_d, &_v[x], _i);
		if (s < 0)
			return -E_FILE_WRITE;

		x	+= s;
		_i	-= s;
	}

	reset();

	return x;
}

/**
 * @desc	: close file descriptor.
 */
void File::close()
{
	flush();
	_name.reset();
	_status = vos::FILE_OPEN_NO;
	if (_d && _d != STDOUT_FILENO && _d != STDERR_FILENO)
		::close(_d);
	_d = 0;
}

void File::dump()
{
	printf("file descriptor: %d\n", _d);
	printf("file name      : %s\n", _name._v);
	printf("file contents  :\n>> %s\n", _v);
}

/**
 * @desc	: get current size of file.
 *
 * @return	:
 *	< >0	: size of file.
 *	< 0	: if file is empty.
 *	< <0	: fail, error at seek.
 */
int File::get_size()
{
	off_t s;
	off_t cur;
	off_t size;

	cur = lseek(_d, 0, SEEK_CUR);
	if (cur < 0)
		return -E_FILE_SEEK;

	s = lseek(_d, 0, SEEK_SET);
	if (s < 0)
		return -E_FILE_SEEK;

	size = lseek(_d, 0, SEEK_END);
	if (size < 0)
		return -E_FILE_SEEK;

	cur = lseek(_d, cur, SEEK_SET);
	if (cur < 0)
		return -E_FILE_SEEK;

	return size;
}


/**
 * @desc	: set end of line character for file based on known mode.
 *
 * @param	:
 *	> mode	: mode of end of line: Unix or DOS.
 */
void File::set_eol_mode(const int mode)
{
	if (mode < FILE_EOL_NIX || mode >= N_FILE_EOL_TYPE)
		return;

	_eol	= _file_eol[mode][0];
	__eol	= _file_eol[mode];
}

/**
 * @desc	: set a custom end of line character.
 *
 * @param	:
 *	> eol	: a character to indicate eol.
 */
void File::set_eol(const char *eol)
{
	if (!eol)
		return;

	_eol	= eol[0];
	__eol	= eol;
}

/**
 * @desc	: get the size of file.
 *
 * @param	:
 *	> path	: path to a file.
 *
 * @return	:
 *	< >0	: size of file in path.
 *	< 0	: if file is empty, or file is not exist.
 */
int File::GET_SIZE(const char *path)
{
	int fd;
	int size;

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
 * @desc		: check if 'path' is exist in file system.
 *
 * @param		:
 *	> path		: a path to directory or file.
 *	> acc_mode	: access mode; read only, write only, or read-write.
 *
 * @return		:
 *	< 1		: if 'path' is exist.
 *	< 0		: if 'path' does not exist.
 */
int File::IS_EXIST(const char *path, int acc_mode)
{
	int fd;

	if (!path)
		return 0;

	switch (acc_mode) {
	case vos::FILE_OPEN_R:
		acc_mode = O_RDONLY;
		break;
	case vos::FILE_OPEN_W:
		acc_mode = O_WRONLY;
		break;
	case vos::FILE_OPEN_RW:
		acc_mode = O_RDWR;
		break;
	default:
		printf("[FILE] unknown access mode %d\n", acc_mode);
		return 0;
	}

	fd = ::open(path, acc_mode);
	if (fd < 0)
		return 0;

	::close(fd);

	return 1;
}

/**
 * @desc		: get the last directory on path.
 *
 * @param		:
 *	> path		: a path to directory or file.
 *
 * @return		:
 *	< Buffer*	: buffer contain last directory on path.
 */
Buffer* File::BASENAME(const char *path)
{
	int	s;
	int	len;
	int	p;
	Buffer	*name = NULL;

	s = Buffer::INIT(&name, NULL);
	if (s < 0)
		return NULL;

	if (!path) {
		s = name->appendc('.');
		if (s < 0)
			goto err;
	} else {
		if (path[0] != '/') {
			s = name->copy_raw(path, 0);
			if (s < 0)
				goto err;
		} else {
			len = strlen(path);
			if (path[0] == '/' && len == 1) {
				s = name->copy_raw(path, 0);
				if (s < 0)
					goto err;
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
					goto err;
			}
		}
	}
	return name;
err:
	delete name;
	return NULL;
}

} /* namespace::vos */
