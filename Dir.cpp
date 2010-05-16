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
			if (_ls[_i]) {
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
	int	rpath_len;
	char	rpath[PATH_MAX];
	Buffer	ppath;

	if (!path) {
		return 0;
	}

	_depth = depth;
	realpath(path, rpath);

	rpath_len = strlen(rpath);
	if (rpath_len <= 0) {
		return -1;
	}

	s = init();
	if (s < 0) {
		return -1;
	}

	s = DirNode::INIT(&_ls[_i], rpath, rpath, 0);
	if (s < 0) {
		return -1;
	}
	_i++;

	s = get_list(rpath, 0);
	if (s < 0) {
		return s;
	}

	/* 1st pass: scan normal directory or any symlink of directory that 
	 * does not have the same root path
	 */
	for (c = 1; c < _i; c++) {
		if (!_ls[c]->is_dir()) {
			continue;
		}
		if (!_ls[c]->_linkname.is_empty()) {
			s = strncmp(rpath, _ls[c]->_linkname._v, rpath_len);
			if (s == 0) {
				/* check in second pass */
				continue;
			}
		}

		ppath.reset();
		ppath.append_raw(rpath);

		s = get_parent_path(&ppath, _ls[c]);
		if (s < 0) {
			goto err;
		} else if (s > 0) {
			break;
		}

		_ls[c]->_cid = _i;

		s = get_list(ppath._v, c);
		if (s < 0) {
			break;
		}
		if (s == 0) {
			_ls[c]->_cid = 0;
		}
	}
	/* 2nd pass: scan symlink of directory that has the same root path,
	 * and set the child-index.
	 */
	for (c = 1; c < _i; c++) {
		if (!_ls[c]->is_dir()) {
			continue;
		}
		if (_ls[c]->_cid != 0) {
			continue;
		}
		if (!_ls[c]->_linkname.is_empty()) {
			s = get_link_child(&_ls[c]->_linkname, rpath
						, rpath_len);
			if (s < 0) {
				goto err;
			}
			_ls[c]->_cid = s;
		}
	}

	if (LIBVOS_DEBUG) {
		dump();
	}

	s = _name.copy_raw(rpath);
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
		if (s < 0) {
			return -1;
		}
	}

	if ((path->_i <= 0)
	||  (path->_i > 0 && path->_v[path->_i - 1] != '/')) {
		path->appendc('/');
	}

	if (ls->_name.cmp_raw("/") != 0) {
		path->append(&ls->_name);

		if (path->_v[path->_i - 1] != '/') {
			path->appendc('/');
		}
	}

	return 0;
}

/**
 * @method	: Dir::get_list
 * @param	:
 *	> path	: a directory path to scan for list of node.
 *	> pid	: parent id of 'path' in array of node.
 * @return	:
 *	< >=0	: success, number of child in this node.
 *	< -1	: fail.
 * @desc	: get list of all files in directory.
 */
int Dir::get_list(const char *path, long pid)
{
	int		s	= 0;
	int		n	= 0;
	Buffer		rpath;
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
		if (LIBVOS_DEBUG) {
			printf("[LIBVOS::DIR] checking '%s'\n", dent->d_name);
		}

		rpath.reset();
		rpath.append_raw(path);

		if (rpath._v[rpath._i - 1] != '/') {
			rpath.appendc('/');
		}

		rpath.append_raw(dent->d_name);

		s = DirNode::INIT(&_ls[_i], rpath._v, dent->d_name, _i);
		if (s < 0) {
			break;
		}

		_ls[_i]->_pid = pid;
		_i++;
		n++;
		if (_i >= _l) {
			resize();
		}
	} while (dent);

	closedir(dir);

	return n;
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
 * @method		: Dir::get_link_child
 * @param		:
 *	> path		: path to directory.
 *	> root		: the root directory of Dir object.
 *	> root_len	: length of 'root'.
 * @return		:
 *	> >=0		: success.
 *	< -1		: fail.
 * @desc		:
 *
 * case example,
 *
 *	0 | /root/dir
 *	x | /root/dir/real/path/to/dir
 *	...
 *	node | linkname | /root/dir/real/path/to/dir
 *
 * 'path' is a symbolic link to 'x', so, instead of create list of directory
 * 'path' again, we just point the child-index of 'path' to the same value of
 * child-index in directory 'x'.
 *
 * This function also can be used to get child node of any 'path', as long as
 * they were in the same 'root'.
 */
int Dir::get_link_child(Buffer* path, const char* root, int root_len)
{
	int		s;
	int		i;
	int		cur_id;
	Buffer		dir;
	int		len	= path->_i;
	const char*	name	= path->_v;

	if (!root) {
		return 0;
	}
	if (root_len <= 0) {
		root_len = strlen(root);
	}
	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::DIR] get link child : %s\n", name);
	}

	cur_id	= 0;
	i	= len - root_len;

	for (i = root_len; i <= len; i++) {
		if (i < len && name[i] != '/') {
			dir.appendc(name[i]);
			continue;
		}
		if (dir.is_empty()) {
			continue;
		}
		if (dir.like_raw(".") == 0) {
			dir.reset();
			continue;
		}
		if (dir.like_raw("..") == 0) {
			dir.reset();
			cur_id = _ls[cur_id]->_pid;
			continue;
		}
		for (s = _ls[cur_id]->_cid; cur_id == _ls[s]->_pid; s++) {
			if (!_ls[s]->is_dir()) {
				continue;
			}
			if (dir.cmp(&_ls[s]->_name) == 0) {
				dir.reset();
				cur_id = s;
				break;
			}
		}
		if (dir.is_empty()) {
			continue;
		}

		fprintf(stderr, "[LIBVOS::DIR] invalid path : %s\n" , name);
		return -1;
	}

	return _ls[cur_id]->_cid;
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
