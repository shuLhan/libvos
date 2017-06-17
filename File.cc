//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "File.hh"

namespace vos {

Error ErrFileEmpty("File: empty");
Error ErrFileEnd("File: end of file");
Error ErrFileExist("File: exist");
Error ErrFileNameEmpty("File: name is empty");
Error ErrFileNotFound("File: path is empty or invalid");
Error ErrFileReadOnly("File: read only");
Error ErrFileWriteOnly("File: write only");

const char* File::__CNAME = "File";

uint16_t File::DFLT_SIZE = 4096;

const char* __eol[N_FILE_EOL_MODE] = {
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
		return Error::SYS();
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

	err = to.open_wt(dst);
	if (err != NULL) {
		return err;
	}

	do {
		err = from.read();
		if (err != NULL) {
			if (err == ErrFileEnd) {
				err = NULL;
				break;
			}
			return err;
		}

		err = to.write(&from);
	} while(err == NULL);

	return err;
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
,	_eol(__eol[FILE_EOL_NIX])
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
	_status = FILE_OPEN_WO;
}

/**
 * Method open(path,mode,perm) will open file `path` with specific `mode` and
 * permission `perm`.
 *
 * On success it will return NULL.
 * On fail it will return error,
 * - ErrFileNotFound, if path is invalid
 * - ErrFileExist, if mode is FILE_OPEN_WOCX
 */
Error File::open(const char* path, const enum file_open_mode mode
	, const int perm)
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
	return open(path, FILE_OPEN_RWCA);
}

/**
 * Method open_ro(path) will open file referenced by `path` with read only mode.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::open_ro(const char* path)
{
	return open(path, FILE_OPEN_RO);
}

/**
 * Method open_wo(path) will open file referenced by `path` with write only
 * mode. It will create file if its not exist.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::open_wo(const char* path)
{
	return open(path, FILE_OPEN_WOCA);
}

/**
 * Method open_wt(path) will open file referenced by `path` with write only
 * and truncated mode. It will create file if its not exist.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::open_wt(const char* path)
{
	return open(path, FILE_OPEN_WOCT);
}

/**
 * Method open_wx(path) will open file referenced by `path` with write only
 * mode. If file exist it will return an error.
 *
 * On success it will return NULL, otherwise it will return errpr.
 */
Error File::open_wx(const char* path)
{
	return open(path, FILE_OPEN_WOCX);
}

/**
 * Method truncate(flush_mode) will reset file content and size to zero only if
 * file opened with write mode.
 *
 * if flush_mode is FLUSH_NO, buffer will not be flushed to file
 * before or after truncated.
 *
 * If flush_mode is FLUSH_FIRST, buffer will be reset first and then
 * file will be truncated.
 *
 * If flush_mode is FLUSH_LAST (default) then buffer will be flushed
 * after file was truncated.
 *
 * On success it will return NULL.
 *
 * On fail it will return error,
 * - ErrFileReadOnly, if file opened with read only mode.
 */
Error File::truncate(enum flush_mode flush_mode)
{
	if (_status == O_RDONLY) {
		return ErrFileReadOnly;
	}
	if (_d == STDOUT_FILENO || _d == STDERR_FILENO) {
		return flush();
	}

	Error err;

	if (flush_mode & FLUSH_FIRST) {
		err = flush();
		if (err != NULL) {
			return err;
		}
	}

	int s = ftruncate(_d, 0);
	if (s < 0) {
		return Error::SYS();
	}

	_size = 0;

	if (flush_mode & FLUSH_LAST) {
		err = flush();
	}

	return err;
}

/**
 * Method `is_open()` will return 1 if file descriptor is open,
 * or `0` if its closed.
 */
int File::is_open()
{
	return (_d > 0);
}

/**
 * Method get_size() will get the current file size.
 * On success it will return value equal or greater than 0.
 * On fail it will return -1.
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
 * Method set_eol(mode) will set the end of line mode for this current File
 * object. There are two modes: EOL_NIX ("\n") or EOL_DOS ("\r\n").
 * If mode is unknown, it will set default to EOL_NIX.
 */
void File::set_eol(enum file_eol_mode mode)
{
	if (mode != FILE_EOL_NIX && mode != FILE_EOL_DOS) {
		mode = FILE_EOL_NIX;
	}

	_eol = __eol[mode];
}

/**
 * Method read(n) will read `n` bytes of characters from file, automatically
 * increase the buffer if `n` is greater than current File buffer size.
 *
 * If `n` is less or equal than zero, then it will set to current buffer size.
 *
 * On success it will return NULL, otherwise it will return error:
 * - ErrFileWriteOnly: if file is cannot be read because its opened as
 *   read-only.
 */
Error File::read(size_t n)
{
	if (_status == O_WRONLY) {
		return ErrFileWriteOnly;
	}

	Error err;

	if (n == 0) {
		n = _l;
	}
	if (n > _l) {
		err = resize(n);
		if (err != NULL) {
			return err;
		}
	}

	_i = 0;
	_p = 0;
	_v[_i] = '\0';

	while (n > 0) {
		ssize_t s = ::read(_d, &_v[_i], n);
		if (s < 0) {
			if (s == EAGAIN || s == EWOULDBLOCK) {
				break;
			}
			return Error::SYS();
		}
		if (s == 0) {
			if (_i > 0) {
				_v[_i] = '\0';
				return NULL;
			}
			return ErrFileEnd;
		}

		_i += size_t(s);
		n = n - size_t(s);
	}
	_v[_i] = '\0';

	return NULL;
}

/**
 * Method refill(read_min) will refill buffer with new data.
 * read_min contains minimum length of buffer to fill, default to '0'.
 *
 * All data from position of '_p' until '_i' will be moved to the beginning of
 * buffer and will not be replaced, new data will be filled in position after
 * (_i - _p).
 *
 * On success it will return NULL.
 *
 * On failure it will return,
 * - ErrFileWriteOnly, if trying to read on write-only file.
 * - Other system error
 */
Error File::refill(size_t read_min)
{
	ssize_t s = 0;
	size_t move_len = 0;
	size_t len = 0;

	move_len = _i - _p;

	if (move_len > 0 && _p > 0) {
		memmove(&_v[0], &_v[_p], move_len);
	}

	len = move_len + read_min;
	if (len > _l) {
		resize(len);
		len -= move_len;
	} else {
		len = _l - move_len;
		if (len == 0) {
			len = _l * 2;
			resize(len);
			len -= move_len;
		}
	}

	s = ::read(_d, &_v[move_len], len);
	if (s < 0) {
		return Error::SYS();
	}
	if (s == 0) {
		return ErrFileEnd;
	}

	_i = move_len + size_t(s);
	_p = 0;
	_v[_i] = '\0';

	return NULL;
}

/**
 * Method get_line(line) will read one line from file at save it into `line`
 * buffer without end-of-line character(s).
 *
 * This operation will change contents of file buffer.
 *
 * On success it will return NULL.
 * On fail it will return,
 * - ErrFileWriteOnly if file open mode is write only.
 * - ErrFileEnd if no more line in file.
 */
Error File::get_line(Buffer* line)
{
	if (_status == O_WRONLY) {
		return ErrFileWriteOnly;
	}
	if (!line) {
		return NULL;
	}

	line->reset();

	size_t x = 0;
	Error err;

	// If file buffer empty, fill it.
	if (_i == 0) {
		err = File::read();
		if (err != NULL) {
			return err;
		}
	}

	x = _p;
	while (_v[x] != LF) {
		if (x < _i) {
			x++;
			continue;
		}

		// End-of-line is not found since the beginning of buffer.

		x = x - _p;

		if (_p == 0) {
			err = refill(_l);
		} else {
			err = refill();
		}
		if (err != NULL) {
			if (err != ErrFileEnd) {
				return err;
			}
			break;
		}
	}

	if (x == 0) {
		return ErrFileEnd;
	}

	size_t len = x - _p;

	// Is it end-of-line or empty line?
	if (len == 0) {
		// Empty line.
		if (_v[x]) {
			goto empty;
		}
		return ErrFileEnd;
	}

	// Do not copy '\r' from end of line.
	if (x > 0 && _v[x - 1] == CR) {
		--len;
	}

	if (len > 0) {
		err = line->copy_raw(&_v[_p], len);
		if (err != NULL) {
			return err;
		}

	}

empty:
	// Set _p to the next character after EOL.
	_p = x + 1;

	return NULL;
}

/**
 * Method write(bfr) will append content of buffer `bfr` to file.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::write(const Buffer* bfr)
{
	if (!bfr) {
		return NULL;
	}
	return write_raw(bfr->v(), bfr->len());
}

/**
 * Method write_raw(bfr,len) will append string `bfr` with length `len` to file.
 *
 * (1) If length is not defined (zero by default), it will be computed
 * automatically.
 *
 * (2) If length is greater than file buffer, then it will be written directly
 * to file descriptor instead of to buffer.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::write_raw(const char* bfr, size_t len)
{
	if (_status == O_RDONLY) {
		return ErrFileReadOnly;
	}
	if (!bfr) {
		return NULL;
	}
	if (!len) {
		len = strlen(bfr);
		if (!len) {
			return NULL;
		}
	}

	Error err;

	// (2) direct write
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
		_size += x;
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
 * Method writef(fmt,...) will append formatted string to file.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error File::writef(const char* fmt, ...)
{
	if (_status == O_RDONLY) {
		return ErrFileReadOnly;
	}
	if (!fmt) {
		return NULL;
	}

	Buffer b;
	va_list al;

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
		return ErrFileReadOnly;
	}

	Error err;

	if (_i + 1 >= _l) {
		err = flush();
		if (err != NULL) {
			return err;
		}
	}

	_v[_i] = c;
	_i++;
	_v[_i] = '\0';


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

	if (maxfd != NULL && _d >= (*maxfd)) {
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
 * Method close() will write the remaining file buffer to disk, close the file
 * descriptor, and reset all attributes.
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

/**
 * Method size() will return the size of current opened file.
 * If file is not open, it will return 0.
 */
off_t File::size()
{
	if (_size > 0) {
		return _size;
	}
	if (is_open()) {
		_size = get_size();
	}
	return _size;
}

/**
 * Method status() will return the status of opened file, its either O_RDONLY,
 * O_WRONLY, or O_RDWR.
 *
 * If file is not open, it will return FILE_OPEN_NO.
 */
int File::status()
{
	return _status;
}

/**
 * Method `eol()` will return current file end of line as string.
 */
const char* File::eol()
{
	return _eol;
}

} // namespace::vos

// vi: ts=8 sw=8 tw=80:
