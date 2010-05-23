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

/**
 * @class		: DirNode
 * @attr		:
 *	- _mode		: attributes (type and permission) of node.
 *	- _uid		: user id of node in file system.
 *	- _gid		: groud id of node in file system.
 *	- _size		: size of node.
 *	- _mtime	: modification time of node.
 *	- _name		: name of node.
 *	- _linkname	: a real path if node is symlink.
 *	- _next		: pointer to the next node in the same directory.
 *	- _child	: pointer to child node if this node is directory.
 *	- _link		: pointer to the real node object if this node is
 *			  symbolic link to directory.
 *	- _parent	: pointer to parent directory.
 * @desc		:
 *
 * This class handling attributes and link of each node (regular file
 * or directory) in directory.
 */
class DirNode {
public:
	DirNode();
	virtual ~DirNode();

	int get_stat(const char *rpath, const char *path = NULL);
	int is_dir();
	int is_link();
	void dump(int space = 0);

	static int INIT(DirNode** node, const char* rpath
			, const char* path);
	static int GET_LINK_NAME(Buffer* linkname, const char* path);

	static DirNode* INSERT(DirNode* list, DirNode* node);
	static int REMOVE_CHILD_BY_NAME(DirNode* list, const char* name);

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
