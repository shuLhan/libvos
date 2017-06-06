//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_FILE_HH
#define _LIBVOS_FILE_HH 1

#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include "Buffer.hh"

using vos::Buffer;

namespace vos {

extern Error ErrFileEmpty;
extern Error ErrFileNotFound;

enum _file_eol_mode {
	EOL_NIX	= 0,
	EOL_DOS	= 1,
	N_EOL_MODE
};
extern const char* __eol[N_EOL_MODE];

enum _file_open_type {
	FILE_OPEN_NO	= -1
,	FILE_OPEN_R	= O_RDONLY
,	FILE_OPEN_W	= O_WRONLY | O_CREAT | O_TRUNC
,	FILE_OPEN_WA	= O_WRONLY | O_CREAT | O_APPEND
,	FILE_OPEN_RW	= O_RDWR | O_CREAT | O_APPEND
,	FILE_OPEN_WX	= O_WRONLY | O_CREAT | O_EXCL
,	FILE_OPEN_SYNC	= O_SYNC
};

enum _file_truncate_mode {
	FILE_TRUNC_FLUSH_NO	= 0
,	FILE_TRUNC_FLUSH_FIRST	= 1
,	FILE_TRUNC_FLUSH_LAST	= 2
};

/**
 * Class File represent file in system as object.
 *
 * To walk through buffer use '_p' or for marking the position that you
 * have already checked/processed in buffer. So, '_p' <= '_i' <= '_l'.
 *
 * '_p' is also used to mark the end of line when used to read one line,
 * using get_line() method, from buffer; or, used as pointer, iterating
 * through buffer '_v' from 0 until '_i'.
 *
 * Field DFLT_SIZE define default buffer size for file.
 *
 * Field _d contains file descriptor.
 * Field _p contains file iterator.
 * Field _status contains status of open file.
 * Field _eol contains end of line as a character.
 * Field _eols contains end of line as a string.
 * Field _name contains file name, with or without path, depends on how user
 * called at opening it.
 */
class File : public Buffer {
public:
	static const char* __CNAME;
	static uint16_t DFLT_SIZE;

	static Error GET_SIZE(const char* path, off_t* size);
	static int IS_EXIST(const char* path, int access_mode = O_RDWR);
	static int BASENAME(Buffer* name, const char* path);
	static int COPY(const char* src, const char* dst);
	static int TOUCH(const char* file, int mode = FILE_OPEN_WA
		, int perm = S_IRUSR | S_IWUSR);
	static int WRITE_PID(const char* file);

	int		_d;
	size_t		_p;
	int		_status;
	int		_perm;
	off_t		_size;
	int		_eol;
	const char*	_eols;
	Buffer		_name;

	explicit File(const size_t bfr_size = File::DFLT_SIZE);
	~File();

	int _open(const char* path, const int mode,
			const int perm = S_IRUSR | S_IWUSR);
	int open(const char* path);
	int open_ro(const char* path);
	int open_wo(const char* path);
	int open_wx(const char* path);
	int open_wa(const char* path);
	int truncate (uint8_t flush_mode = FILE_TRUNC_FLUSH_LAST);

	int is_open();

	off_t get_size();
	void set_eol(const int mode);

	ssize_t read();
	ssize_t readn(size_t n);
	ssize_t refill(size_t read_min = 0);
	int get_line(Buffer* line);

	ssize_t write(const Buffer* bfr);
	ssize_t write_raw(const char* bfr, size_t len = 0);
	ssize_t writef(const char* fmt, va_list args);
	ssize_t writes(const char* fmt, ...);
	int writec(const char c);

	ssize_t flush();
	void close();
	void dump();

private:
	File(const File&);
	void operator=(const File&);
};

} // namespace::vos
#endif
// vi: ts=8 sw=8 tw=80:
