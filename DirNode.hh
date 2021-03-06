//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_DIRNODE_HH
#define _LIBVOS_DIRNODE_HH 1

#include <sys/stat.h>
#include <time.h>
#include "Buffer.hh"

namespace vos {

enum _DirNode_upstat {
	_MTIME_CHANGED	= 1
,	_CTIME_CHANGED	= 2
,	_MCTIME_CHANGED	= 3
};

/**
 * @class		: DirNode
 * @attr		:
 *	- _mode		: attributes (type and permission) of node.
 *	- _uid		: user id of node in file system.
 *	- _gid		: groud id of node in file system.
 *	- _size		: size of node.
 *	- _mtime	: last modification to the contents of the file.
 *	- _ctime	: last modification to the attributes of the file.
 *	- _name		: name of node.
 *	- _linkname	: a real path if node is symlink.
 *	- _next		: pointer to the next node in the same directory.
 *	- _prev		: pointer to the previous node in the same directory.
 *	- _child	: pointer to child node if this node is directory.
 *	- _link		: pointer to the real node object if this node is
 *			  symbolic link to directory.
 *	- _parent	: pointer to parent directory.
 * @desc		:
 *
 * This class handling attributes and link of each node (regular file
 * or directory) in directory.
 */
class DirNode : public Object {
public:
	DirNode();
	virtual ~DirNode();

	int get_attr(const char* rpath, const char* name = NULL);
	int update_attr(DirNode* node, const char* rpath);
	int update_child_attr(DirNode** node, const char* rpath
				, const char* name);
	void dump(int space = 0);

	inline int is_dir()
	{
		return S_ISDIR(_mode);
	}
	inline int is_link()
	{
		return S_ISLNK(_mode);
	}
	inline int is_file()
	{
		return S_ISREG(_mode);
	}

	static int INIT(DirNode** node, const char* rpath, const char* path);
	static int GET_LINK_NAME(Buffer* linkname, const char* path);

	static void INSERT(DirNode** list, DirNode* node);
	static int INSERT_CHILD(DirNode* list, const char* rpath
				, const char* name);
	static void UNLINK(DirNode** list, DirNode* node);
	static int REMOVE_CHILD_BY_NAME(DirNode* list, const char* name);

	mode_t _mode;
	uid_t		_uid;
	gid_t		_gid;
	off_t		_size;
	long		_mtime;
	long		_ctime;
	Buffer		_name;
	Buffer		_linkname;
	DirNode*	_next;
	DirNode*	_prev;
	DirNode*	_child;
	DirNode*	_link;
	DirNode*	_parent;

	static const char* __cname;
private:
	DirNode(const DirNode&);
	void operator=(const DirNode &);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
