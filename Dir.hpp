/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_DIR_HPP
#define	_LIBVOS_DIR_HPP	1

#include <dirent.h>
#include "DirNode.hpp"

namespace vos {

#define	DEF_CREATE_MODE	(S_IRWXU | S_IRGRP | S_IXGRP)

/**
 * @class		: Dir
 * @attr		:
 *	- _i		: number of DirNode object in '_ls'.
 *	- _l		: lenght of '_ls' array.
 *	- _depth	: maximum depth of child directory to scan for.
 *	- _name		: the first name of directory to scan.
 *	- _ls		: array of DirNode objects.
 * @desc		:
 * a module for handling task involving directory (listing,
 * creating, and removing directory).
 */
class Dir {
public:
	Dir();
	~Dir();

	int open(const char *path, int depth = 1);
	int get_parent_path(Buffer *path, DirNode *ls, int depth = 1);
	int get_list(DirNode* list, const char *path);
	int get_symlink(DirNode* list);
	DirNode* get_node(Buffer* path, const char* root, int root_len);
	void dump();

	static int CREATE(const char *path, mode_t mode = DEF_CREATE_MODE);
	static int CREATES(const char* path, mode_t mode = DEF_CREATE_MODE);

	int	_depth;
	Buffer	_name;
	DirNode	*_ls;
private:
	Dir(const Dir&);
	void operator=(const Dir&);
};

} /* namespace::vos */

#endif
