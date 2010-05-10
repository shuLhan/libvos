/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Dir.hpp"

namespace vos {

int Dir::DEF_LS_SIZE = 64;

Dir::Dir() :
	_i(0),
	_l(0),
	_depth(0),
	_name(),
	_ls(NULL)
{}

Dir::~Dir()
{
	reset();
	if (_ls) {
		free(_ls);
	}
}

/**
 * @method	: Dir::init
 * @param	:
 *	> path	: path to the root directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: initialize Dir object.
 */
int Dir::init()
{
	register int s = 0;

	if (_ls) {
		reset();
	} else {
		s = resize();
	}

	return s;
}

/**
 * @method	: Dir::resize
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: resize array of DirNode object.
 */
int Dir::resize()
{
	_l = _l + DEF_LS_SIZE;

	_ls = (DirNode **) realloc(_ls, _l * sizeof(DirNode *));
	if (!_ls) {
		_l = _l - DEF_LS_SIZE;
		return -1;
	}

	memset(_ls + _i, 0, DEF_LS_SIZE * sizeof(DirNode *));

	return 0;
}

/**
 * @method	: Dir::reset
 * @desc	: release all DirNode object.
 */
void Dir::reset()
{
	if (_ls) {
		for (; _i >= 0; _i--) {
			if (_ls[_i] != NULL) {
				delete _ls[_i];
				_ls[_i] = NULL;
			}
		}
	}
	_i = 0;
}

/**
 * @method	: Dir::open
 * @param	:
 *	> path	: path to directory.
 *	> depth	: depth of directory, to get list of (default to 1).
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: open directory 'path' and list of content directory.
 *
 *	set 'depth' to -1 for getting all list of directory recursively.
 */
int Dir::open(const char *path, int depth)
{
	int	s;
	int	c;
	Buffer	ppath;

	if (!path) {
		return 0;
	}

	s = init();
	if (s < 0) {
		return -1;
	}

	s = DirNode::INIT(&_ls[_i], path, path, 0);
	if (s < 0) {
		return -1;
	}
	_i++;

	c	= 1;
	_depth	= depth;

	s = get_list(path, 0);
	if (s < 0) {
		return s;
	}

	for (; c < _i; c++) {
		if (_ls[c]->is_dir()) {
			ppath.reset();

			if (_ls[c]->is_link()) {
				s = _ls[c]->get_linkname();
				if (s < 0) {
					return -1;
				}

				s = ppath.copy(&_ls[c]->_linkname);
			} else {
				ppath.append_raw(path);
				s = get_parent_path(&ppath, _ls[c]);
				if (s < 0) {
					goto err;
				} else if (s > 0) {
					break;
				}
			}
			if (s < 0) {
				break;
			}

			_ls[c]->_cid = _i;

			s = get_list(ppath._v, c);
			if (s < 0) {
				break;
			}
		}
	}

	if (LIBVOS_DEBUG) {
		dump();
	}

	s = _name.copy_raw(path);
err:
	return s;
}

/**
 * @method	: Dir::get_parent_path
 * @param	:
 *	< path	: return value, a string of path.
 *	< ls	: array of directory nodes.
 *	< depth	: a counter for the limit of traversing a child directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: get a parent path of specific node.
 *	i.e.: if 'node' is a random node in directory list, we want to get a
 *	real path of 'node' from the first directory. It could be "./node" or
 *	"../node" or "../dir/dir/node".
 */
int Dir::get_parent_path(Buffer *path, DirNode *ls, int depth)
{
	register int s = 0;

	if (depth == _depth) {
		return depth;
	}

	if (ls && ls->_pid) {
		s = get_parent_path(path, _ls[ls->_pid], depth + 1);
	}
	if (s < 0) {
		return -1;
	}

	s = path->append(&ls->_name);
	if (s < 0) {
		return -1;
	}

	s = path->appendc('/');

	return s;
}

/**
 * @method	: Dir::get_list
 * @param	:
 *	> path	: a directory path to scan for list of node.
 *	> pid	: parent id of 'path' in array of node.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: get list of all files in directory.
 */
int Dir::get_list(const char *path, long pid)
{
	int		s	= 0;
	Buffer		realpath;
	DIR		*dir	= NULL;
	struct dirent	*dent	= NULL;

	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::DIR] scanning '%s' ...\n", path);
	}

	dir = opendir(path);
	if (!dir) {
		if (errno == EACCES) {
			return 0;
		}
		return -1;
	}

	do {
		dent = readdir(dir);
		if (!dent) {
			break;
		}

		s = strcmp(dent->d_name, ".");
		if (s == 0) {
			continue;
		}
		s = strcmp(dent->d_name, "..");
		if (s == 0) {
			continue;
		}

		realpath.reset();
		realpath.append_raw(path);
		realpath.appendc('/');
		realpath.append_raw(dent->d_name);

		s = DirNode::INIT(&_ls[_i], realpath._v, dent->d_name, _i);
		if (s < 0) {
			break;
		}

		_ls[_i]->_pid = pid;
		_i++;
		if (_i >= _l) {
			resize();
		}
	} while (dent);

	closedir(dir);

	return s;
}

/**
 * @method	: Dir::insert
 * @param	:
 *	> node	: pointer to DirNode object.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: insert 'node' to the array of DirNode '_ls'.
 */
int Dir::insert(DirNode *node)
{
	register int s = 0;

	_ls[_i] = node;
	_i++;
	if (_i >= _l) {
		s = resize();
	}

	return s;
}

/**
 * @method	: Dir::dump
 * @desc	: dump content of Dir object.
 */
void Dir::dump()
{
	for (int c = 0; c < _i; c++) {
		_ls[c]->dump();
	}
}

/**
 * @method	: Dir::CREATE
 * @param	:
 *	> path	: a path to directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create a new directory 'path'.
 */
int Dir::CREATE(const char *path, mode_t mode)
{
	register int s = 0;

	s = mkdir(path, mode);

	return s;
}

} /* namespace::vos */
