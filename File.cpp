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

int File::DFLT_BUFFER_SIZE = 8192;

File::File() : Buffer(File::DFLT_BUFFER_SIZE),
	_d(0),
	_p(0),
	_status(FILE_OPEN_NO),
	_eol(_file_eol[FILE_EOL_NIX][0]),
	_name(),
	_line(0),
	__eol(_file_eol[FILE_EOL_NIX])
{}

/**
 * @param:
 *	> size : initial size for buffer.
 */
File::File(const int bfr_size) : Buffer(bfr_size),
	_d(0),
	_p(0),
	_status(FILE_OPEN_NO),
	_eol(_file_eol[FILE_EOL_NIX][0]),
	_name(),
	_line(0),
	__eol(_file_eol[FILE_EOL_NIX])
{}

/**
 * @param:
 *	> path		: path to a file.
 *	> bfr_size	: initial size for file buffer.
 */
File::File(const char *path, const int bfr_size) : Buffer(bfr_size),
	_d(0),
	_p(0),
	_status(FILE_OPEN_NO),
	_eol(_file_eol[FILE_EOL_NIX][0]),
	_name(),
	_line(0),
	__eol(_file_eol[FILE_EOL_NIX])
{
	open(path);
}

File::~File()
{
	close();
}

/**
 * @desc:
 *	open file for read and write, create a file if it's not exist.
 *
 * @param:
 *	> path : path to a file.
 *
 * @return:
 *	< 0		: success.
 *	< E_FILE_OPEN	: fail, error at opening file.
 */
int File::open(const char *path)
{
	if (! path)
		return 0;

	_d = ::open(path, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (_d < 0)
		return E_FILE_OPEN;

	_name.copy(path, 0);
	_status = FILE_OPEN_RW;

	return 0;
}

/**
 * @desc: open file for read only.
 *
 * @param:
 *	> path : path to a file.
 *
 * @return:
 *	< 0		: success.
 *	< E_FILE_OPEN	: fail, error at opening file.
 */
int File::open_ro(const char *path)
{
	if (! path)
		return 0;

	_d = ::open(path, O_RDONLY);
	if (_d < 0)
		return E_FILE_OPEN;

	_name.copy(path, 0);
	_status = FILE_OPEN_R;

	return 0;
}

/**
 * @desc: open file for write only.
 *
 * @param:
 *	> path : path to a file.
 *
 * @return:
 *	< 0		: success.
 *	< E_FILE_OPEN	: fail, error at opening file.
 */
int File::open_wo(const char *path)
{
	if (! path)
		return 0;

	_d = ::open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (_d < 0)
		return E_FILE_OPEN;

	_line.resize(Buffer::DFLT_SIZE);

	_name.copy(path, 0);
	_status = FILE_OPEN_W;

	return 0;
}

/**
 * @desc: open file for write and append.
 *
 * @param:
 *	> path : path to a file.
 *
 * @return:
 *	< 0		: success.
 *	< E_FILE_OPEN	: fail, error at opening file.
 */
int File::open_wa(const char *path)
{
	if (! path)
		return 0;

	_d = ::open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (_d < 0)
		return E_FILE_OPEN;

	_name.copy(path, 0);
	_status = FILE_OPEN_W;

	return 0;
}

/**
 * @return:
 * 	< >0 : success, return number of bytes read.
 * 	< 0  : EOF
 *
 * @exception:
 *	< E_FILE_READ : fail, error at reading.
 */
int File::read()
{
	if (! (_status & FILE_OPEN_R))
		return 0;

	_i = ::read(_d, _v, _l);
	if (_i < 0) {
		throw Error(E_FILE_READ, _name._v);
	}

	_p	= 0;
	_v[_i]	= '\0';

	return _i;
}


/**
 * @desc: get one line at a time from buffer.
 *
 * NOTE:
 *	- this operation will change contents of file buffer.
 *	- this operation return a string without a new line character.
 *
 * @return:
 *	< _line	: a line in buffer.
 *	< NULL	: fail, EOF.
 *
 * @exception:
 *	< E_FILE_READ	: Error, at reading file.
 */
Buffer *File::get_line()
{
	int start;

	if (_i == 0) {
		if (read() == 0)
			return NULL;
	}

	if (_v[_p] == '\0') {
		_v[_p] = '\n';
		++_p;
	}

	start = _p;
	do {
		while (_p < _i && _v[_p] != _eol) {
			++_p;
		}
		if (_p >= _i && _v[_p] != _eol) {
			_p = _p - start;
			memmove(&_v[0], &_v[start], _p);

			_i = ::read(_d, &_v[_p], _l - _p);
			if (_i < 0) {
				throw Error(E_FILE_READ, _name._v);
			}
			if (_i == 0) {
				return NULL;
			}

			start	= 0;
			_i	+= _p;
			_v[_i]	= '\0';
		}
	} while (_v[_p] != _eol);
	
	_v[_p]		= '\0';
	_line._v	= &_v[start];
	_line._i	= _p - start;

	return &_line;
}

/**
 * @param:
 *	> bfr : buffer to be write to file.
 */
void File::write(const Buffer *bfr)
{
	if (! bfr)
		return;

	write(bfr->_v, bfr->_i);
}

void File::write(const char *bfr, int len)
{
	int x = 0;
	int s;

	if (! (_status & FILE_OPEN_W) || ! bfr)
		return;

	if (! len) {
		len = strlen(bfr);
		if (! len)
			return;
	}

	/* direct write */
	if (len >= File::DFLT_BUFFER_SIZE) {
		flush();

		while (len > 0) {
			s = ::write(_d, &bfr[x], len);
			if (s < 0)
				throw Error(E_FILE_WRITE, _name._v);

			x	+= s;
			len	-= s;
		}
	} else {
		if (_l < (_i + len))
			flush();
		append(bfr, len);
	}
}

void File::write(const char *fmt, va_list args)
{
	if (! (_status & FILE_OPEN_W) || ! fmt)
		return;

	Buffer b;
	b.vprint(fmt, args);
	write(b._v, b._i);
}

void File::writes(const char *fmt ...)
{
	Buffer	b;
	va_list al;

	va_start(al, fmt);
	b.vprint(fmt, al);
	va_end(al);

	write(b._v, b._i);
}

void File::write(const char c)
{
	if (_i + 1 >= _l) 
		flush();

	_v[_i++] = c;
}

void File::flush()
{
	int x = 0;
	int s;

	if (! (_status & FILE_OPEN_W))
		return;

	while (_i > 0) {
		s = ::write(_d, &_v[x], _i);
		if (s < 0)
			throw Error(E_FILE_WRITE, _name._v);

		x	+= s;
		_i	-= s;
	}

	reset();
}

void File::close()
{
	flush();
	_name.reset();
	_status = FILE_OPEN_NO;
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
 * @desc: get current size of file.
 *
 * @return:
 *	< >0	: size of file.
 *	< 0	: if file is empty, or error at seek.
 */
int File::get_size()
{
	off_t cur;
	off_t size;

	cur = lseek(_d, 0, SEEK_CUR);
	if (cur < 0)
		throw Error(E_FILE_SEEK, _name._v);

	lseek(_d, 0, SEEK_SET);

	size = lseek(_d, 0, SEEK_END);
	if (size < 0)
		throw Error(E_FILE_SEEK, _name._v);

	cur = lseek(_d, cur, SEEK_SET);
	if (cur < 0)
		throw Error(E_FILE_SEEK, _name._v);

	return size;
}


/**
 * @desc: set end of line character for file based on known mode.
 *
 * @param:
 *	> mode : mode of end of line: Unix or DOS.
 */
void File::set_eol(const int mode)
{
	if (mode < FILE_EOL_NIX || mode >= N_FILE_EOL_TYPE)
		return;

	_eol	= _file_eol[mode][0];
	__eol	= _file_eol[mode];
}

/**
 * @desc: set custom end of line character for file.
 *
 * @param:
 *	> eol : a character to indicate eol.
 */
void File::set_eol(const char *eol)
{
	if (! eol)
		return;

	_eol	= eol[0];
	__eol	= eol;
}

/**
 * @desc: get the size of file.
 *
 * @param:
 *	> path : path to a file.
 *
 * @return:
 *	< >0	: size of file in path.
 *	< 0	: if file is empty, or file is not exist.
 */
int File::GET_SIZE(const char *path)
{
	int fd;
	int size;

	if (! path)
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
 * @desc: check if 'path' is exist in file system.
 *
 * @param:
 *	> path		: a path to directory or file.
 *	> acc_mode	: access mode; read only, write only, or read-write.
 *
 * @return:
 *	< 1		: if 'path' is exist.
 *	< 0		: if 'path' does not exist.
 */
int File::IS_EXIST(const char *path, int acc_mode)
{
	int fd;

	if (! path)
		return 0;

	switch (acc_mode) {
	case FILE_OPEN_R:
		acc_mode = O_RDONLY;
		break;
	case FILE_OPEN_W:
		acc_mode = O_WRONLY;
		break;
	case FILE_OPEN_RW:
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
 * @desc: get the last directory on path.
 *
 * @param:
 *	> path : a path to directory or file.
 *
 * @return:
 *	< Buffer * : buffer contain last directory on path.
 */
Buffer * File::BASENAME(const char *path)
{
	int	len;
	int	p;
	Buffer	*name = new Buffer();

	if (! path) {
		name->copy(".", 0);
	} else {
		if (path[0] != '/') {
			name->copy(path, 0);
		} else {
			len = strlen(path);
			if (path[0] == '/' && len == 1) {
				name->copy(path, 0);
			} else {
				p = len - 1;
				if (path[p] == '/')
					--len;

				do {
					--p;
				} while (path[p] != '/');

				++p;
				name->copy(&path[p], len - p);
			}
		}
	}

	return name;
}

} /* namespace::vos */
