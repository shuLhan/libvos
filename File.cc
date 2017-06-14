//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "File.hh"

namespace vos {

Error ErrFileEmpty("File: empty");
Error ErrFileExist("File: exist");
Error ErrFileNameEmpty("File: name is empty");
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
		return ErrFileNotFound;
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
		*size = 0;
	}

	::close(fd);

	return NULL;
}

/**
 * Method IS_EXIST(path,access_mode) will check if file referenced by `path`
 * exist in system and user have permission to access the file with
 * `access_mode` (default to read-write).
 *
 * It will return 1 if only if,
 * - file exist and access mode is read-only (O_RDONLY) and user can be read it,
 * - file exist and access mode is write-only (O_WRONLY) and user can write into
 *   it,
 * - file exist and access mode is read-write (O_RDWR) and user can read-write
 *   into it.
 *
 * On fail it will return 0.
 */
int File::IS_EXIST(const char* path, int access_mode)
{
	if (!path) {
		return 0;
	}

	int fd = 0;

	fd = ::open(path, access_mode);
	if (fd < 0) {
		return 0;
	}

	::close(fd);

	return 1;
}

/**
 * Method BASENAME will get the basename of `path`, it could be a file or
 * directory.
 *
 * On success it will return NULL and save the base name into `name` parameter.
 *
 * On fail it will return error ErrOutOfMemory.
 */
Error File::BASENAME(Buffer* name, const char* path)
{
	if (!name) {
		return NULL;
	}

	Error err;

	name->reset();

	if (!path) {
		err = name->appendc('.');
		if (err != NULL) {
			return err;
		}

		return NULL;
	}

	size_t len = strlen(path);
	if (len == 1) {
		err = name->appendc(path[0]);
		if (err != NULL) {
			return err;
		}

		return NULL;
	}

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
		return err;
	}

	return NULL;
}

/**
 * Method COPY(src,dst) will copy file 'src' to 'dst', create a new file if
 * 'dst' is not exist, or overwrite 'dst' if already exist.
 *
 * NOTE: use 'rename()' system call for easy and fast move.
 *
 * On success it will return NULL, otherwise it will return error:
 *
 * - ErrFileNameEmpty if both or either src or dst is NULL
 * - ErrFileNotFound if src file can not be opened
 */
Error File::COPY(const char* src, const char* dst)
{
	if (!src || !dst) {
		return ErrFileNameEmpty;
	}

	Error err;
	File from;
	File to;

	err = from.open_ro(src);
	if (err != NULL) {
		return err;
	}

	err = to.open_wo(dst);
	if (err != NULL) {
		return err;
	}

	ssize_t s = from.read();
	while (s > 0) {
		err = to.write(&from);
		if (err != NULL) {
			return err;
		}

		s = from.read();
	}

	return NULL;
}

/**
 * Method TOUCH(filename,mode,perm) will create `filename` if its not exist or
 * update access and modification time if its exist.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::TOUCH(const char* filename, int mode, int perm)
{
	if (!filename) {
		return ErrFileNameEmpty;
	}

	int s = utime(filename, NULL);
	if (s == 0) {
		return NULL;
	}

	if (errno != ENOENT) {
		return Error::SYS();
	}

	s = ::open(filename, mode, perm);
	if (s < 0) {
		return Error::SYS();
	}

	::close(s);

	return NULL;
}

/**
 * Method WRITE_PID(file) will write current process ID to `file`.
 *
 * This function usually used by process daemon.
 *
 * When daemon started, it will write its process id to file, to :
 * - make user know that daemon is already running, so
 * - no other daemon running, just one, or
 * - if daemon is not running but PID file exist, that mean is last daemon is
 *   exit in abnormal state; so see log file for further information.
 *
 * On success it will return NULL, otherwise it will return Error object.
 */
Error File::WRITE_PID(const char* file)
{
	File f;

	Error err = f.open_wx(file);
	if (err != NULL) {
		return err;
	}

	err = f.appendi(getpid());
	if (err != NULL) {
		return err;
	}

	return NULL;
}

/**
 * Method File(bfr_size) will create new File object with initial buffer size
 * set to `bfr_size`.
 */
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
 * Method `as_stdout()` will close previous file and set default write to
 * standard output.
 */
void File::as_stdout()
{
	close();
	_d = STDERR_FILENO;
	_status = O_WRONLY;
}

/**
 * Method open(path,mode,perm) will open file `path` with specific `mode` and
 * permission `perm`.
 *
 * On success it will return NULL.
 * On fail it will return error.
 */
Error File::open(const char* path, const int mode, const int perm)
{
	if (!path) {
		return ErrFileNameEmpty;
	}

	Error err;

	if (!_v) {
		err = resize(DFLT_SIZE);
		if (err != NULL) {
			return err;
		}
	}

	_d = ::open(path, mode, perm);
	if (_d < 0) {
		_d = 0;
		switch (errno) {
		case ENOENT:
			return ErrFileNotFound;
		case EEXIST:
			return ErrFileExist.with(path, strlen(path));
		default:
			return Error::SYS();
		}
	}

	err = _name.copy_raw(path);
	if (err != NULL) {
		return err;
	}

	_size = get_size();
	_status = (mode & (O_RDONLY | O_WRONLY | O_RDWR));
	_perm = perm;

	return NULL;
}

/**
 * Method open(path) will open file referenced by `path` for reading and
 * writing. File will be created if its not exist.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::open(const char* path)
{
	return open(path, FILE_OPEN_RW);
}

/**
 * Method open_ro(path) will open file referenced by `path` with read only mode.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::open_ro(const char* path)
{
	return open(path, FILE_OPEN_R);
}

/**
 * Method open_wo(path) will open file referenced by `path` with write only
 * mode. It will create file if its not exist.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::open_wo(const char* path)
{
	return open(path, FILE_OPEN_W);
}

/**
 * Method open_wx(path) will open file referenced by `path` with write only
 * mode. If file exist it will return an error.
 *
 * On success it will return NULL, otherwise it will return errpr.
 */
Error File::open_wx(const char* path)
{
	return open(path, FILE_OPEN_WX);
}

/**
 * Method open_wa(path) will open file referenced by `path` for writing and with
 * append mode. If file exist the content will not be truncated.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::open_wa(const char* path)
{
	return open(path, FILE_OPEN_WA);
}

/**
 * Method truncate(flush_mode) will reset file content and size to zero.
 *
 * if flush_mode is FLUSH_NO, buffer will not be flushed to file
 * before or after truncated.
 *
 * If flush_mode is FLUSH_FIRST, buffer will be reset first and then
 * file will be truncated.
 *
 * If flush_mode is FLUSH_LAST (default) then buffer will be flushed
 * after file was truncated.
 */
Error File::truncate(enum flush_mode flush_mode)
{
	Error err;

	if (flush_mode & FLUSH_FIRST) {
		reset();
	}

	// Try to close file.
	if (_d && (_d != STDOUT_FILENO && _d != STDERR_FILENO)) {
		ssize_t s = ::close(_d);
		if (s != 0) {
			return Error::SYS();
		}
	}

	// Reopen file by truncate.
	err = open(_name.v(), _status | O_CREAT | O_TRUNC, _perm);
	if (err != NULL) {
		return err;
	}

	if (flush_mode & FLUSH_LAST) {
		err = flush ();
		if (err != NULL) {
			return err;
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
Error File::write(const Buffer* bfr)
{
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
Error File::write_raw(const char* bfr, size_t len)
{
	if (_status == O_RDONLY || !bfr) {
		return NULL;
	}
	if (!len) {
		len = strlen(bfr);
		if (!len) {
			return NULL;
		}
	}

	Error err;

	// direct write
	if (len >= _l) {
		err = flush();
		if (err != NULL) {
			return err;
		}

		size_t x = 0;

		while (len > 0) {
			ssize_t s = ::write(_d, &bfr[x], len);
			if (s < 0) {
				return Error::SYS();
			}
			x	+= size_t(s);
			len	-= size_t(s);
		}
		len = x;
		_size += len;
	} else {
		if (_l < (_i + len)) {
			err = flush();
			if (err != NULL) {
				return err;
			}
		}

		err = append_raw(bfr, len);
		if (err != NULL) {
			return err;
		}
		if (_status & O_SYNC) {
			flush();
		}
	}

	return NULL;
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
Error File::writef(const char* fmt, va_list args)
{
	if (_status == O_RDONLY || !fmt) {
		return 0;
	}

	Buffer b;

	Error err = b.vappend_fmt(fmt, args);
	if (err != NULL) {
		return err;
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
Error File::writes(const char* fmt, ...)
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
		return err;
	}

	return write_raw(b.v(), b.len());
}

/**
 * Method writec(c) will write one character to file buffer.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::writec(const char c)
{
	if (_status == O_RDONLY) {
		return NULL;
	}

	Error err;

	if (_i + 1 >= _l) {
		err = flush();
		if (err != NULL) {
			return err;
		}
	}

	_v[_i++] = c;

	if (_status & O_SYNC) {
		err = flush();
		if (err != NULL) {
			return err;
		}
	}

	return NULL;
}

/**
 * Method is_readable(read_set,all_set) will check and return 1 if file
 * descriptor is ready to read from set `read_fds`.
 *
 * If file is ready to read and `all_fds` is not NULL, then it will clear the
 * descriptor from `all_fds`.
 */
int File::is_readable(fd_set* read_fds, fd_set* all_fds)
{
	int s = FD_ISSET(_d, read_fds);

	if (s && all_fds) {
		set_clear(all_fds);
	}

	return s;
}

/**
 * Method set_add(fds,maxfd) will add current file descriptor to the set of file
 * descriptor `fds` and change the `maxfds` value to d + 1 if current descriptor
 * is greater than `maxfd`.
 */
void File::set_add(fd_set* fds, int* maxfd)
{
	FD_SET(_d, fds);

	if ((*maxfd) && _d >= (*maxfd)) {
		(*maxfd) = _d + 1;
	}
}

/**
 * Method set_clear(fds) will remove current file descriptor from the set `fds`.
 */
void File::set_clear(fd_set* fds)
{
	FD_CLR(_d, fds);
}

/**
 * Method flush() will write all file's buffer to disk only if file open status
 * is write or read-write.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::flush()
{
	if (_status == O_RDONLY) {
		reset();
		return NULL;
	}

	size_t x = 0;

	while (_i > 0) {
		ssize_t s = ::write(_d, &_v[x], _i);
		if (s < 0) {
			return Error::SYS();
		}
		x	+= size_t(s);
		_i	-= size_t(s);
	}
	_i = x;
	_size += _i;

	reset();

	return NULL;
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

/**
 * Method fd() will return current file descriptor.
 */
int File::fd()
{
	return _d;
}

/**
 * Method name() will return the string representation of the File object,
 * which is the name of file.
 */
const char* File::name()
{
	return _name.v();
}

} // namespace::vos

// vi: ts=8 sw=8 tw=80:
