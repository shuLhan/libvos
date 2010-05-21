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

class DirNode {
public:
	DirNode();
	virtual ~DirNode();

	int get_stat(const char *rpath, const char *path = NULL);
	int is_dir();
	int is_link();
	virtual void dump(int space = 0);

	static int INIT(DirNode** node, const char* rpath
			, const char* path);
	static int GET_LINK_NAME(Buffer* linkname, const char* path);
	static DirNode* INSERT(DirNode* list, DirNode* node);

	int		_mode;
	int		_uid;
	int		_gid;
	long		_size;
	long		_mtime;
	Buffer		_name;
	Buffer		_linkname;
	DirNode		*_next;
	DirNode		*_child;
	DirNode		*_link;
	DirNode		*_parent;
private:
	DirNode(const DirNode&);
	void operator=(const DirNode &);
};

} /* namespace::vos */

#endif
