/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FILE_HPP
#define	_LIBVOS_FILE_HPP	1

#include <fcntl.h>
#include <unistd.h>
#include "Buffer.hpp"

using vos::Buffer;

namespace vos {

enum _file_eol_type {
	FILE_EOL_NIX	= 0,
	FILE_EOL_DOS	= 1,
	N_FILE_EOL_TYPE
};
extern const char *__eol[N_FILE_EOL_TYPE];

#define	GET_EOL_CHR(t)	__eol[t][0]
#define	GET_EOL_STR(t)	__eol[t]

enum _file_open_type {
	FILE_OPEN_NO	= -1,
	FILE_OPEN_R	= O_RDONLY,
	FILE_OPEN_W	= O_WRONLY | O_CREAT | O_TRUNC,
	FILE_OPEN_WA	= O_WRONLY | O_CREAT | O_APPEND,
	FILE_OPEN_RW	= O_RDWR | O_CREAT | O_APPEND,
};

/**
 * @class		: File
 * @attr		:
 *	- _d		: file descriptor.
 *	- _p		: file iterator.
 *	- _status	: status of open file.
 *	- _eol		: end of line character that used on this File.
 *	- _name		:
 *		file name, with or without path, depends on how user
 *		called at opening it.
 * @desc		:
 *
 *	_p is used to mark the end of line when used to read one line, using
 *	get_line() method, from buffer.
 */
class File : public Buffer {
public:

	File();
	~File();

	int init(const int bfr_size = DFLT_BUFFER_SIZE);

	int _open(const char *path, const int mode,
			const int perm = S_IRUSR | S_IWUSR);
	int open(const char *path);
	int open_ro(const char *path);
	int open_wo(const char *path);
	int open_wa(const char *path);
	int read();
	int readn(int n);
	int write(const Buffer *bfr);
	int write_raw(const char *bfr, int len = 0);
	int writef(const char *fmt, va_list args);
	int writes(const char *fmt, ...);
	int writec(const char c);
	int flush();
	void close();
	void dump();

	int get_size();
	int get_line(Buffer **line);

	void set_eol(const int mode);

	static int GET_SIZE(const char *path);
	static int IS_EXIST(const char *path, int acc_mode = O_RDWR);
	static Buffer* BASENAME(const char *path);

	static unsigned int DFLT_BUFFER_SIZE;

	int		_d;
	int		_p;
	int		_status;
	int		_eol;
	Buffer		_name;
private:
	File(const File&);
	void operator=(const File&);
};

} /* namespace::vos */

#endif
