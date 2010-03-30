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

	int get_stat(const char *realpath, const char *path = NULL);
	int get_linkname();
	int is_dir();
	int is_link();
	virtual void dump();

	static void ADD(DirNode **head, DirNode *node);

	int		_mode;
	int		_uid;
	int		_gid;
	long		_size;
	long		_mtime;
	Buffer		_name;
	Buffer		*_linkname;
	DirNode		*_parent;
	DirNode		*_child;
	DirNode		*_next;
	DirNode		*_last;
private:
	DirNode(const DirNode&);
	void operator=(const DirNode &);
};

} /* namespace::vos */

#endif
