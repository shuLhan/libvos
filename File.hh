//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
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
 * @class		: File
 * @attr		:
 *	- _d		: file descriptor.
 *	- _p		: file iterator.
 *	- _status	: status of open file.
 *	- _eol		: end of line as a character.
 *	- _eols		: end of line as a string.
 *	- _name		:
 *		file name, with or without path, depends on how user
 *		called at opening it.
 * @desc		:
 *	Just like Buffer, '_l' in File represent the length of buffer '_v',
 *	and '_i' represent the length of data in that buffer, so '_i' <= '_l'.
 *
 *	To walk through buffer use '_p' or for marking the position that you
 *	have already checked/processed in buffer. So, '_p' <= '_i' <= '_l'.
 *
 *	'_p' is also used to mark the end of line when used to read one line,
 *	using get_line() method, from buffer; or, used as pointer, iterating
 *	through buffer '_v' from 0 until '_i'.
 */
class File : public Buffer {
public:
	File(const unsigned int bfr_size = File::DFLT_SIZE);
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

	int read();
	int readn(int n);
	int refill(int read_min = 0);
	int get_line(Buffer* line);

	int write(const Buffer* bfr);
	int write_raw(const char* bfr, int len = 0);
	int writef(const char* fmt, va_list args);
	int writes(const char* fmt, ...);
	int writec(const char c);

	int flush();
	void close();
	void dump();

	static off_t GET_SIZE(const char* path);
	static int IS_EXIST(const char* path, int acc_mode = O_RDWR);
	static int BASENAME(Buffer* name, const char* path);
	static int COPY(const char* src, const char* dst);
	static int TOUCH(const char* file);
	static int WRITE_PID(const char* file);

	static unsigned int DFLT_SIZE;

	int		_d;
	int		_p;
	int		_status;
	int		_perm;
	off_t		_size;
	int		_eol;
	const char*	_eols;
	Buffer		_name;
private:
	File(const File&);
	void operator=(const File&);
};

} /* namespace::vos */
#endif
// vi: ts=8 sw=8 tw=78:
