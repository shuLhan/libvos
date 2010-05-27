/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "DirNode.hpp"

namespace vos {

DirNode::DirNode() :
	_mode(0),
	_uid(0),
	_gid(0),
	_size(0),
	_mtime(0),
	_name(),
	_linkname(),
	_next(NULL),
	_child(NULL),
	_link(NULL),
	_parent(this)
{}

DirNode::~DirNode()
{
	if (_next) {
		delete _next;
		_next = NULL;
	}
	if (_child) {
		delete _child;
		_child = NULL;
	}
	_link	= NULL;
	_parent	= NULL;
}

/**
 * @method		: DirNode::get_stat
 * @param		:
 *	> rpath		: a full path, from current directory.
 *	> path		: the basename of 'rpath'.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: get 'path' attribute.
 */
int DirNode::get_stat(const char *rpath, const char *path)
{
	if (!rpath && !path) {
		return 0;
	}

	register int	s;
	struct stat	st;

	if (path) {
		_name.copy_raw(path);
	} else {
		_name.copy_raw(rpath);
	}

	s = lstat(rpath, &st);
	if (s < 0) {
		return -1;
	}

	if (S_ISLNK(st.st_mode)) {
		s = GET_LINK_NAME(&_linkname, rpath);
		if (s < 0) {
			return -1;
		}

		memset(&st, 0, sizeof(struct stat));

		s = lstat(_linkname._v, &st);
		if (s < 0) {
			return -1;
		}
	}

	_mode	= st.st_mode;
	_uid	= st.st_uid;
	_gid	= st.st_gid;
	_size	= st.st_size;
	_mtime	= st.st_mtime;

	return 0;
}

/**
 * @method	: DirNode::is_dir
 * @return	:
 *	< 1	: true, DirNode object is a directory.
 *	< 0	: false, DirNode object is not a directory.
 * @desc	: check if DirNode object is directory or not.
 */
int DirNode::is_dir()
{
	return S_ISDIR(_mode);
}

/**
 * @method	: DirNode::is_link
 * @return	:
 *	< 1	: true, DirNode object is a symbolic link.
 *	< 0	: false, DirNode object is not a symbolick link.
 * @desc	: check if DirNode object is a symbolic link or not.
 */
int DirNode::is_link()
{
	return S_ISLNK(_mode);
}

/**
 * @method	: DirNode::dump
 * @param	:
 *	> space	: number of space before print content of object.
 * @desc	: dump content of DirNode object.
 */
void DirNode::dump(int space)
{
	for (int i = 0; i < space; i++) {
		putchar(' ');
	}
	if (is_dir()) {
		printf("> d ");
	} else {
		printf("> - ");
	}
	printf("|%5d|%5d|%5d|%12ld|%ld|%s|%s\n", _mode, _uid, _gid, _size
		, _mtime , _name._v ? _name._v : ""
		, _linkname._v ? _linkname._v : "");
	if (_link) {
		printf(" => %s\n", _link->_name._v);
	}
	if (_child) {
		_child->dump(space + 2);
	}
	if (_next) {
		_next->dump(space);
	}
}

/**
 * @method		: DirNode::INIT
 * @param		:
 *	> node		: return value, pointer to a DirNode object.
 *	> rpath	: a full path of node from current directory.
 *	> path		: a name of path in current directory in file system.
 *	> id		: id for a new DirNode object.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	create and initialize a new DirNode object based on 'rpath'.
 *	'rpath' is a full path of node, i.e.: '../../node', when 'path' is
 *	only contain 'node'.
 */
int DirNode::INIT(DirNode **node, const char *rpath, const char *path)
{
	(*node) = new DirNode();
	if (!(*node)) {
		return -1;
	}

	return (*node)->get_stat(rpath, path);
}

/**
 * @method		: DirNode::GET_LINK_NAME
 * @param		:
 *	> linkname	: pointer to Buffer object, where name of link will be
 *			saved.
 *	> rpath		: path to node to check for link name.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: get a real path to symbolic link 'path'.
 */
int DirNode::GET_LINK_NAME(Buffer* linkname, const char* path)
{
	if (!linkname || !path) {
		return -1;
	}
	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::DIR] get link name: '%s'\n", path);
	}
	if (! linkname->is_empty()) {
		free(linkname->_v);
	}

	linkname->_v = realpath(path, NULL);
	if (! linkname->_v) {
		return -1;
	}

	linkname->_i = strlen(linkname->_v);
	linkname->_l = linkname->_i;

	return 0;
}

/**
 * @method		: Dir::insert
 * @param		:
 *	> list		: head of list.
 *	> node		: a new node.
 * @return		:
 *	< DirNode*	: a new head of list.
 * @desc		:
 * insert 'node' to the list of DirNode, sort by node name.
 */
DirNode* DirNode::INSERT(DirNode* list, DirNode* node)
{
	if (!node) {
		return list;
	}
	if (!list) {
		return node;
	}

	int		s;
	DirNode*	last	= NULL;
	DirNode*	p	= list;

	while (p) {
		s = p->_name.like(&node->_name);
		if (s > 0) {
			if (!last) {
				node->_next = list;
				return node;
			}
			node->_next = last->_next;
			last->_next = node;
			return list;
		}
		last = p;
		p = p->_next;
	}
	if (!last) {
		list = node;
	} else {
		last->_next = node;
	}

	return list;
}

/**
 * @method	: DirNode::INSERT_CHILD
 * @param	:
 *	> list	: pointer to DirNode object.
 *	> rpath	: real path of node from root diretory.
 *	> name	: name of node.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create and insert a new node into child of 'list'.
 */
int DirNode::INSERT_CHILD(DirNode* list, const char* rpath, const char* name)
{
	int		s;
	DirNode*	node = NULL;

	s = INIT(&node, rpath, name);
	if (s < 0) {
		return -1;
	}
	node->_parent	= list;
	list->_child	= INSERT(list->_child, node);

	return 0;
}

/**
 * @method	: DirNode::REMOVE_CHILD_BY_NAME
 * @param	:
 *	> list	: pointer to DirNode object, which child will be searched and
 *		removed.
 *	> name	: name of DirNode object that will be removed.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail, DirNode object with name is 'name' is not found.
 * @desc	:
 * remove child node with name is 'name' from child node of 'list'.
 */
int DirNode::REMOVE_CHILD_BY_NAME(DirNode* list, const char* name)
{
	DirNode* last	= NULL;
	DirNode* p	= list->_child;

	while (p) {
		if (p->_name.like_raw(name) == 0) {
			if (!last) {
				list->_child = p->_next;
			} else {
				last->_next = p->_next;
			}

			p->_next = NULL;
			delete p;

			return 0;
		}

		last	= p;
		p	= p->_next;
	}

	return -1;
}

} /* namespace::vos */
