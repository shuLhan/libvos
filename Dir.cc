//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Dir.hh"

namespace vos {

const char* Dir::__cname = "Dir";

Dir::Dir()
:	Object()
,	_depth(0)
,	_name()
,	_ls(NULL)
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
	char	rpath[PATH_MAX + 1];

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
	s = get_list(_ls, rpath, depth);
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
 * @method	: Dir::close
 * @desc	: reset and clear all attributes.
 */
void Dir::close()
{
	_depth = 0;
	_name.reset();
	if (_ls) {
		delete _ls;
		_ls = NULL;
	}
}

/**
 * @method	: Dir::find
 * @param	:
 *	> dir	: directory to be search for.
 *	> name	: name of file.
 *	> depth	: maximum number of child directory to looking for 'name'.
 * @return	:
 *	< NULL	: not found.
 *	< !NULL	: found.
 * @desc	: search file with name is 'name' recursively in list of node.
 */
DirNode* Dir::find(DirNode* dir, const char* name, int depth)
{
	int		s;
	DirNode*	ls = NULL;
	DirNode*	node = NULL;

	if (depth == 0) {
		return NULL;
	}

	if (LIBVOS_DEBUG) {
		printf("[%s] find: scanning directory '%s'\n", __cname
			, dir->_name.chars());
	}

	ls = dir->_child;

	while (ls) {
		s = ls->_name.cmp_raw(name);
		if (s == 0) {
			return ls;
		}
		if (ls->is_dir()) {
			node = find(ls, name, depth - 1);
			if (node) {
				return node;
			}
		}
		ls = ls->_next;
	}

	return NULL;
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
	||  (path->_i > 0 && path->char_at(path->_i - 1) != '/')) {
		path->appendc('/');
	}

	if (ls->_name.cmp_raw("/") != 0) {
		path->append(&ls->_name);

		if (ls->is_dir() && path->char_at(path->_i - 1) != '/') {
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
int Dir::get_list(DirNode* list, const char* path, int depth)
{
	int		s	= 0;
	int		n	= 0;
	Buffer		rpath;
	DIR*		dir	= NULL;
	struct dirent*	dent	= NULL;
	DirNode*	node	= NULL;

	if (depth == 0) {
		return 0;
	}

	if (LIBVOS_DEBUG) {
		printf("[%s] get_list: scanning '%s' ...\n", __cname, path);
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
			printf("[%s] get_list: checking '%s'\n", __cname
				, dent->d_name);
		}

		rpath.reset();
		rpath.append_raw(path);

		if (rpath.char_at(rpath._i - 1) != '/') {
			rpath.appendc('/');
		}

		rpath.append_raw(dent->d_name);

		s = DirNode::INIT(&node, rpath.v(), dent->d_name);
		if (s < 0) {
			break;
		}

		if (node->is_dir()) {
			if (node->_linkname.is_empty()) {
				s = get_list(node, rpath.v(), depth - 1);
				if (s < 0) {
					return s;
				}
			} else {
				s = strncmp(node->_linkname.v(), _name.v()
						, _name._i);
				if (s != 0) {
					s = get_list(node, node->_linkname.v()
						, depth - 1);
					if (s < 0) {
						return s;
					}
				}
			}
		}

		node->_parent = list;
		DirNode::INSERT(&list->_child, node);
		node = NULL;
		n++;
	} while (dent);

	closedir(dir);

	return n;
}

/**
 * @method	: Dir::get_symlink
 * @param	:
 *	> list	: list of DirNode objects to search and set the link.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	:
 * set '_link' pointer in DirNode object to point to the real DirNode object.
 */
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
				return -1;
			}
		} else if (list->_linkname.is_empty()) {
			continue;
		} else {
			list->_link = get_node(&list->_linkname, _name.v()
						, _name._i);
			if (!list->_link) {
				fprintf(stderr
, "[%s] get_symlink: cannot get link to '%s'\n", __cname
					, list->_linkname.chars());
				return -1;
			}
		}
	}
	return 0;
}

/**
 * @method		: Dir::get_node
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
 * 'path' is a symbolic link to 'x', so, instead of creating list of directory
 * 'path' again, we just point the _link of 'path' node to the same value of
 * child pointer in directory 'x'.
 *
 * This function also can be used to get node index of any 'path', as long as
 * they were in the same 'root'.
 */
DirNode* Dir::get_node(Buffer* path, const char* root, size_t root_len)
{
	int		s;
	size_t i = 0;
	Buffer		node;
	size_t len = path->_i;
	const char*	name	= path->v();
	DirNode*	p	= _ls;
	DirNode*	c	= NULL;

	if (!path || !root) {
		return NULL;
	}
	if (root_len <= 0) {
		root_len = strlen(root);
	}
	s = strncmp(path->v(), root, root_len);
	if (s != 0) {
		return NULL;
	}
	if (LIBVOS_DEBUG) {
		printf("[%s] get_node: get link child %s\n", __cname, name);
	}

	for (i = root_len; i <= len; i++) {
		if (i < len && name[i] != '/') {
			node.appendc(name[i]);
			continue;
		}
		if (node.is_empty()) {
			continue;
		}
		if (node.like_raw(".") == 0) {
			node.reset();
			continue;
		}
		if (node.like_raw("..") == 0) {
			node.reset();
			p = p->_parent;
			continue;
		}
		for (c = p->_child; c; c = c->_next) {
			if (node.cmp(&c->_name) == 0) {
				node.reset();
				p = c;
				break;
			}
		}
		if (node.is_empty()) {
			continue;
		}

		if (LIBVOS_DEBUG) {
			fprintf(stderr, "[%s] get_node:\n"
					"  invalid path: %s\n"
					"  node name   : %s\n"
				, __cname, name, node.chars());
		}
		return NULL;
	}

	return p;
}

/**
 * @method	: Dir:refresh_by_path
 * @param	:
 *	> path	: path to be refreshed, path must in the same root.
 * @return	:
 *	< >0	: number of change (new node, mod node) in the path.
 *	< -1	: fail, path is not in the same root.
 *	< -2	: fail, system error.
 * @desc	:
 *	check for a new, deleted or modified node in the 'path'.
 *	Use '_name' path if 'path' value is NULL.
 */
int Dir::refresh_by_path(Buffer* path)
{
	int		n = 0;
	int		s;
	Buffer		rpath;
	DIR*		dir	= NULL;
	struct dirent*	dent	= NULL;
	DirNode*	list	= NULL;
	DirNode*	cnode	= NULL;
	DirNode*	childs	= NULL;

	if (!path) {
		path = &_name;
	}

	s = rpath.resize(PATH_MAX + 1);
	if (s < 0) {
		return -2;
	}

	rpath.set_at(0, realpath(path->v(), NULL), 0);

	list = get_node(&rpath, _name.v(), _name._i);
	if (!list) {
		return -1;
	}

	if (LIBVOS_DEBUG) {
		printf("[%s] refresh_by_path: refreshing '%s' ...\n"
			, __cname, path->chars());
	}

	s = list->update_attr(list, rpath.v());
	if (s <= 0 || !list->is_dir()) {
		return s;
	}

	dir = opendir(rpath.v());
	if (!dir) {
		if (errno == EACCES) {
			return 0;
		}
		return -2;
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
			printf("[%s] refresh_by_path: checking '%s'\n"
				, __cname, dent->d_name);
		}

		rpath.reset();
		rpath.append(path);

		if (rpath.char_at(rpath._i - 1) != '/') {
			rpath.appendc('/');
		}

		rpath.append_raw(dent->d_name);

		s = list->update_child_attr(&cnode, rpath.v(), dent->d_name);
		if (s >= 0) {
			if (s == 1) {
				n++;
			}
			if (!cnode) {
				fprintf(stderr
, "[%s] refresh_by_path: child node empty!", __cname);
				return -2;
			}

			DirNode::UNLINK(&list->_child, cnode);
			DirNode::INSERT(&childs, cnode);
		} else if (s == -1) {
			s = DirNode::INIT(&cnode, rpath.v(), dent->d_name);
			if (s < 0) {
				break;
			}

			if (cnode->is_dir()) {
				if (cnode->_linkname.is_empty()) {
					s = get_list(cnode, rpath.v());
					if (s < 0) {
						return s;
					}
				} else {
					s = strncmp(cnode->_linkname.v()
							, _name.v(), _name._i);
					if (s != 0) {
						s = get_list(cnode
							, cnode->_linkname.v());
						if (s < 0) {
							return s;
						}
					}
				}
			}

			cnode->_parent = list;
			DirNode::INSERT(&childs, cnode);
			cnode = NULL;
			n++;
		} else if (s == -2) {
			return s;
		}
		cnode = NULL;
	} while (dent);

	if (list->_child) {
		if (LIBVOS_DEBUG) {
			list->_child->dump();
		}
		delete list->_child;
	}
	list->_child = childs;

	closedir(dir);

	if (LIBVOS_DEBUG) {
		printf(
"[%s] refresh_by_path: numbers of node changed '%d'\n", __cname, n);
	}

	return n;
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
 * @method	: Dir::CREATE
 * @param	:
 *	> path	: a path to directory.
 *	> perm	: the file permission bits of the new directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create a new directory 'path'.
 */
int Dir::CREATE(const char *path, mode_t perm)
{
	register int s = 0;

	if (LIBVOS_DEBUG) {
		printf("[%s] CREATE: %s\n", __cname, path);
	}

	s = mkdir(path, perm);

	return s;
}

/**
 * @method	: Dir::CREATES
 * @param	:
 *	> path	: a path to directory name.
 *	> perm	: the file permission bits of the new directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	:
 *	create 'path' directory recursively.
 *	if 'path' is 'a/b/c', then create directory 'a', 'a/b', and then
 *	'a/b/c'.
 */
int Dir::CREATES(const char* path, mode_t perm)
{
	if (!path) {
		return 0;
	}
	if (LIBVOS_DEBUG) {
		printf("[%s] CREATES: %s\n", __cname, path);
	}

	int s = 0;
	int i = 0;
	int len = (int) strlen(path);
	Buffer d;

	while (i < len) {
		if (path[i] == '/') {
			d.appendc(path[i]);

			s = CREATE(d.v(), perm);
			if (s < 0 && errno != EEXIST) {
				goto err;
			}

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
		s = CREATE(d.v(), perm);
		if (s < 0 && errno != EEXIST) {
			goto err;
		}
	}

	s = 0;
err:
	return s;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
