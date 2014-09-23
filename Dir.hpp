/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_DIR_HPP
#define	_LIBVOS_DIR_HPP	1

#include <dirent.h>
#include "DirNode.hpp"

namespace vos {

#define	DEF_DIR_PERM	(S_IRWXU | S_IRGRP | S_IXGRP)

/**
 * @class		: Dir
 * @attr		:
 *	- _depth	: maximum depth of child directory to scan.
 *	- _name		: the first name of directory to scan.
 *	- _ls		: linked list of DirNode objects.
 * @desc		:
 * a module for handling task involving directory (listing,
 * creating, and removing directory).
 */
class Dir {
public:
	Dir();
	~Dir();

	int open(const char *path, int depth = 1);
	void close();

	DirNode* find(DirNode* dir, const char* name, int depth = 1);

	int get_parent_path(Buffer *path, DirNode *ls, int depth = 1);
	int get_list(DirNode* list, const char *path);
	int get_symlink(DirNode* list);
	DirNode* get_node(Buffer* path, const char* root, int root_len);

	int refresh_by_path(Buffer* path);
	void dump();

	static int CREATE(const char *path, mode_t perm = DEF_DIR_PERM);
	static int CREATES(const char* path, mode_t perm = DEF_DIR_PERM);

	int		_depth;
	Buffer		_name;
	DirNode*	_ls;
private:
	Dir(const Dir&);
	void operator=(const Dir&);
};

} /* namespace::vos */

#endif
