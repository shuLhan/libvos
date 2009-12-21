/**
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
extern const char *_file_eol[N_FILE_EOL_TYPE];

enum _file_open_type {
	FILE_OPEN_NO	= 0,
	FILE_OPEN_R,
	FILE_OPEN_W,
	FILE_OPEN_RW
};

class File : public Buffer {
public:

	File();
	~File();

	int init(const int bfr_size);

	int open(const char *path);
	int open_ro(const char *path);
	int open_wo(const char *path);
	int open_wa(const char *path);
	int read();
	int write(const Buffer *bfr);
	int write_raw(const char *bfr, int len);
	int writef(const char *fmt, va_list args);
	int writes(const char *fmt ...);
	int writec(const char c);
	int flush();
	void close();
	void dump();

	int get_size();
	int get_line(Buffer **line);

	void set_eol_mode(const int mode);
	void set_eol(const char *eol);

	static int GET_SIZE(const char *path);
	static int IS_EXIST(const char *path, int acc_mode);
	static Buffer* BASENAME(const char *path);

	static unsigned int DFLT_BUFFER_SIZE;

	int		_d;		/* file descriptor */
	int		_p;		/* file iterator */
	int		_status;	/* file status */
	char		_eol;		/* file end of line character */
	Buffer		_name;		/* file name */
	const char	*__eol;		/* file end of line string */
private:
	DISALLOW_COPY_AND_ASSIGN(File);
};

} /* namespace::vos */

#endif
