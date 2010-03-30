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

/**
 * @class	: Dir
 * @attr	:
 * @decs	:
 * a module for handling task involving directory (listing,
 * creating, and removing directory).
 */
class Dir {
public:
	Dir();
	~Dir();

	int open(const char *path);
	void dump();
	void close();

	long	_n_ls;
	Buffer	_name;
	DIR	*_d;
	DirNode	*_ls;
private:
	Dir(const Dir&);
	void operator=(const Dir&);

	int get_list();
};

} /* namespace::vos */

#endif
