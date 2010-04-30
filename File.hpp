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

enum _eol_mode {
	EOL_NIX	= 0,
	EOL_DOS	= 1,
	N_EOL_MODE
};
extern const char *__eol[N_EOL_MODE];

enum _open_type {
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
	int refill(int read_min = 0);
	int write(const Buffer *bfr);
	int write_raw(const char *bfr, int len = 0);
	int writef(const char *fmt, va_list args);
	int writes(const char *fmt, ...);
	int writec(const char c);
	int flush();
	void close();
	void dump();

	off_t get_size();
	int get_line(Buffer **line);

	void set_eol(const int mode);

	static off_t GET_SIZE(const char *path);
	static int IS_EXIST(const char *path, int acc_mode = O_RDWR);
	static int BASENAME(Buffer *name, const char *path);
	static int COPY(const char *src, const char *dst);

	static unsigned int DFLT_BUFFER_SIZE;

	int		_d;
	int		_p;
	int		_status;
	int		_eol;
	const char	*_eols;
	Buffer		_name;
private:
	File(const File&);
	void operator=(const File&);
};

} /* namespace::vos */

#endif
