/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_DIRNODE_HPP
#define	_LIBVOS_DIRNODE_HPP	1

#include <sys/stat.h>
#include <unistd.h>
#include "Buffer.hpp"

namespace vos {

#define	LINKNAME_INIT_SIZE	3

class DirNode {
public:
	DirNode(long id = 0);
	virtual ~DirNode();

	int get_stat(const char *rpath, const char *path = NULL);
	int is_dir();
	int is_link();
	virtual void dump();

	static int INIT(DirNode** node, const char* rpath
			, const char* path, long id);
	static int GET_LINK_NAME(Buffer* linkname, const char* path);

	long		_id;
	long		_pid;
	long		_cid;
	int		_mode;
	int		_uid;
	int		_gid;
	long		_size;
	long		_mtime;
	Buffer		_name;
	Buffer		_linkname;
private:
	DirNode(const DirNode&);
	void operator=(const DirNode &);
};

} /* namespace::vos */

#endif
