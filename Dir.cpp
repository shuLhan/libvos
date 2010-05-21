/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Dir.hpp"

namespace vos {

Dir::Dir() :
	_depth(0),
	_name(),
	_ls(NULL)
{}

Dir::~Dir()
{
	if (_ls) {
		delete _ls;
		_ls = NULL;
	}
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
	char	rpath[PATH_MAX];

	if (!path) {
		return 0;
	}

	_depth = depth;
	realpath(path, rpath);

	s = _name.copy_raw(rpath);
	if (s < 0) {
		return 0;
	}

	s = DirNode::INIT(&_ls, rpath, rpath);
	if (s < 0) {
		return -1;
	}

	/* 1st pass: scan normal directory or any symlink of directory that 
	 * does not have the same root path */
	s = get_list(_ls, rpath);
	if (s < 0) {
		return s;
	}

	/* 2nd pass: scan symlink of directory that has the same root path */
	s = get_symlink(_ls->_child);
	if (s < 0) {
		return s;
	}

	if (LIBVOS_DEBUG) {
		dump();
	}

	return 0;
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
	if (ls && ls->_parent != _ls) {
		s = get_parent_path(path, ls->_parent, depth + 1);
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
 *	> list	: parent node, pointer to DirNode object.
 *	> path	: a directory path to scan for list of node.
 * @return	:
 *	< >=0	: success, number of child in this node.
 *	< -1	: fail.
 * @desc	: get list of all files in directory.
 */
int Dir::get_list(DirNode* list, const char* path)
{
	int		s	= 0;
	int		n	= 0;
	Buffer		rpath;
	DIR*		dir	= NULL;
	struct dirent*	dent	= NULL;
	DirNode*	node	= NULL;

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
		s = strncmp(dent->d_name, ".", 1);
		if (s == 0) {
			continue;
		}
		s = strncmp(dent->d_name, "..", 2);
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

		s = DirNode::INIT(&node, rpath._v, dent->d_name);
		if (s < 0) {
			break;
		}

		if (node->is_dir()) {
			if (node->_linkname.is_empty()) {
				s = get_list(node, rpath._v);
				if (s < 0) {
					return s;
				}
			} else {
				s = strncmp(node->_linkname._v, _name._v
						, _name._i);
				if (s != 0) {
					s = get_list(node, node->_linkname._v);
					if (s < 0) {
						return s;
					}
				}
			}
		}

		node->_parent	= list;
		list->_child	= DirNode::INSERT(list->_child, node);
		node		= NULL;
		n++;
	} while (dent);

	closedir(dir);

	return n;
}

int Dir::get_symlink(DirNode *list)
{
	int s;

	for (; list; list = list->_next) {
		if (!list->is_dir()) {
			continue;
		}
		if (list->_child) {
			s = get_symlink(list->_child);
			if (s < 0) {
				return s;
			}
		} else if (list->_linkname.is_empty()) {
			continue;
		} else {
			list->_link = get_node(&list->_linkname, _name._v
						, _name._i);
			if (!list->_link) {
				printf("[DIR] cannot get link to '%s'\n"
					, list->_linkname._v);
				return -1;
			}
		}
	}
	return 0;
}

/**
 * @method	: Dir::dump
 * @desc	: dump content of Dir object.
 */
void Dir::dump()
{
	if (_ls) {
		_ls->dump();
	}
}

/**
 * @method		: Dir::get_link_child
 * @param		:
 *	> path		: path to directory.
 *	> root		: the root directory of Dir object.
 *	> root_len	: length of 'root'.
 * @return		:
 *	< DirNode*	: pointer to DirNode object of 'path'.
 *	< NULL		: fail.
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
 * This function also can be used to get node index of any 'path', as long as
 * they were in the same 'root'.
 */
DirNode* Dir::get_node(Buffer* path, const char* root, int root_len)
{
	int		s;
	int		i;
	Buffer		dir;
	int		len	= path->_i;
	const char*	name	= path->_v;
	DirNode*	p	= _ls;
	DirNode*	c	= NULL;

	if (!path || !root) {
		return NULL;
	}
	if (root_len <= 0) {
		root_len = strlen(root);
	}
	s = strncmp(path->_v, root, root_len);
	if (s != 0) {
		return NULL;
	}
	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::DIR] get link child : %s\n", name);
	}

	i = len - root_len;

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
			p = p->_parent;
			continue;
		}
		for (c = p->_child; c; c = c->_next) {
			if (!c->is_dir()) {
				continue;
			}
			if (dir.cmp(&c->_name) == 0) {
				dir.reset();
				p = c;
				break;
			}
		}
		if (dir.is_empty()) {
			continue;
		}

		fprintf(stderr, "[LIBVOS::DIR] invalid path : %s\n" , name);
		return NULL;
	}

	return p;
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

	if (LIBVOS_DEBUG) {
		printf("[DIR] create: %s\n", path);
	}

	s = mkdir(path, mode);

	return s;
}

/**
 * @method	: Dir::CREATES
 * @param	:
 *	> path	: a path to directory name.
 *	> mode	: the file permission bits of the new directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	:
 *	create 'path' directory recursively.
 *	if 'path' is 'a/b/c', then create directory 'a', 'a/b', and then
 *	'a/b/c'.
 */
int Dir::CREATES(const char* path, mode_t mode)
{
	if (!path) {
		return 0;
	}
	if (LIBVOS_DEBUG) {
		printf("[DIR] create recursive: %s\n", path);
	}

	int s = 0;
	int i = 0;
	int len = strlen(path);
	Buffer d;

	while (path[i] == '/') {
		i++;
	}
	while (i < len) {
		if (path[i] == '/') {
			s = CREATE(d._v, mode);
			if (s < 0 && errno != EEXIST) {
				goto err;
			}

			d.appendc(path[i]);

			do {
				i++;
			} while (path[i] == '/');
		} else {
			d.appendc(path[i]);
			i++;
		}
	}
	i--;
	if (path[i] != '/' && path[i] != '.') {
		s = CREATE(d._v, mode);
		if (s < 0 && errno != EEXIST) {
			goto err;
		}
	}

	s = 0;
err:
	return s;
}

} /* namespace::vos */
