/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Dir.hpp"

namespace vos {

Dir::Dir() :
	_n_ls(0),
	_name(),
	_d(NULL),
	_ls(NULL)
{}

Dir::~Dir()
{
	close();
	if (_ls) {
		delete _ls;
	}
}

/**
 * @method	: Dir::open
 * @param	:
 *	> path	: a path to directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: open a directory 'path' and get list of all node in there.
 */
int Dir::open(const char *path)
{
	if (!path) {
		return -1;
	}

	_d = opendir(path);
	if (!_d) {
		return -1;
	}

	register int s = 0;

	s = get_list();
	if (s < 0) {
		return s;
	}

	s = _name.copy_raw(path);
	if (s < 0) {
		return s;
	}

	return s;
}

/**
 * @method	: Dir::get_list
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: get list of all files in directory.
 */
int Dir::get_list()
{
	if (!_d) {
		return 0;
	}

	register int	s	= 0;
	DirNode		*node	= NULL;
	struct dirent	*de	= NULL;

	if (_ls) {
		delete _ls;
		_ls = NULL;
	}

	do {
		de = readdir(_d);
		if (!de) {
			break;
		}

		s = strcmp(de->d_name, ".");
		if (s == 0) {
			continue;
		}
		s = strcmp(de->d_name, "..");
		if (s == 0) {
			continue;
		}

		node = new DirNode();
		if (!node) {
			return -1;
		}

		node->get_stat(de->d_name);

		DirNode::ADD(&_ls, node);
		node = NULL;
	} while (de);

	return s;
}

/**
 * @method	: Dir::close
 * @desc	: close file descriptor on opened directory.
 */
void Dir::close()
{
	if (_d) {
		closedir(_d);
		_d = NULL;

		_name.reset();
	}
}

} /* namespace::vos */
