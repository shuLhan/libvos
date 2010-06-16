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
	_ctime(0),
	_name(),
	_linkname(),
	_next(NULL),
	_prev(NULL),
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
	_prev	= NULL;
	_link	= NULL;
	_parent	= NULL;
}

/**
 * @method		: DirNode::get_attr
 * @param		:
 *	> rpath		: a full path, from current directory.
 *	> name		: the basename of 'rpath'.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: get 'path' attribute.
 */
int DirNode::get_attr(const char *rpath, const char *name)
{
	if (!rpath && !name) {
		return 0;
	}

	register int	s;
	struct stat	st;

	if (name) {
		_name.copy_raw(name);
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
	_ctime	= st.st_ctime;

	return 0;
}

/**
 * @method	: DirNode::is_file
 * @return	:
 *	< 1	: true, DirNode object is a regular file.
 *	< 0	: false, DirNode object is NOT a regular file.
 * @desc	: check if DirNode object is a regular file or not.
 */
int DirNode::is_file()
{
	return S_ISREG(_mode);
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
 * @method	: DirNode::update_attr
 * @param	:
 *	> node	: pointer to DirNode object that will be updated.
 *	> rpath	: a path that of 'node' in file system.
 * @return	:
 *	< 3	: success, _mtime and _ctime changed.
 *	< 2	: success, _ctime changed
 *	< 1	: success, _mtime changed.
 *	< 0	: success, stat does not change.
 *	< -1	: fail.
 * @desc	: update node attributes.
 */
int DirNode::update_attr(DirNode* node, const char* rpath)
{
	register int	s = 0;
	struct stat	st;

	if (node->_linkname.is_empty()) {
		s = lstat(rpath, &st);
	} else {
		s = lstat(node->_linkname._v, &st);
	}
	if (s < 0) {
		return -1;
	}

	if (st.st_mtime != node->_mtime) {
		s		|= _MTIME_CHANGED;
		node->_size	= st.st_size;
		node->_mtime	= st.st_mtime;
	}
	if (st.st_ctime != node->_ctime) {
		s		|= _CTIME_CHANGED;
		node->_mode	= st.st_mode;
		node->_uid	= st.st_uid;
		node->_gid	= st.st_gid;
		node->_ctime	= st.st_ctime;
	}

	return s;
}

/**
 * @method	: DirNode::update_child_attr
 * @param	:
 *	> node	: return value, pointer to child node.
 *	> rpath	: a full path name that need to be updated.
 *	> name	: name of the last node, basename, of 'rpath'.
 * @return	:
 *	< 1	: success, child stat change.
 *	< 0	: success, but child stat does not change.
 *	< -1	: fail. child with _name is 'name' not found.
 *	< -2	: fail, system error.
 * @desc	: update attributes of child node with 'name', if change.
 */
int DirNode::update_child_attr(DirNode** node, const char* rpath
				, const char* name)
{
	register int	s;
	DirNode*	p = _child;

	while (p) {
		s = p->_name.cmp_raw(name);
		if (s == 0) {
			break;
		}
		p = p->_next;
	}
	if (!p) {
		return -1;
	}

	(*node)	= p;
	s	= update_attr(p, rpath);
	if (s < 0) {
		return -2;
	}

	return s;
}

/**
 * @method	: DirNode::dump
 * @param	:
 *	> space	: formating, number of space before printing content of object.
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
	printf("|%5d|%5d|%5d|%12ld|%ld|%ld|%s|%s|%s", _mode, _uid, _gid, _size
		, _mtime, _ctime, _name._v ? _name._v : ""
		, _linkname._v ? _linkname._v : "", ctime(&_mtime));
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

	return (*node)->get_attr(rpath, path);
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
		printf("[LIBVOS::DirNode_] get link name: '%s'\n", path);
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
 * @desc		:
 * insert 'node' to the list of DirNode, sort by node name.
 */
void DirNode::INSERT(DirNode** list, DirNode* node)
{
	if (!node) {
		return;
	}
	if (!(*list)) {
		(*list) = node;
		return;
	}

	int		s;
	DirNode*	last	= NULL;
	DirNode*	p	= (*list);

	while (p) {
		s = p->_name.like(&node->_name);
		if (s > 0) {
			break;
		}
		last	= p;
		p	= p->_next;
	}
	if (!last) {
		node->_next	= (*list);
		(*list)->_prev	= node;
		(*list)		= node;
	} else {
		if (p) {
			p->_prev = node;
		}
		node->_next = p;
		node->_prev = last;
		last->_next = node;
	}
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
	node->_parent = list;
	INSERT(&list->_child, node);

	return 0;
}

void DirNode::UNLINK(DirNode** list, DirNode* node)
{
	if (!node) {
		return;
	}

	DirNode* next = node->_next;
	DirNode* prev = node->_prev;

	if ((*list) == node) {
		(*list) = next;
	} else if (prev) {
		prev->_next = next;
	}
	if (next) {
		next->_prev = prev;
	}

	node->_next = NULL;
	node->_prev = NULL;
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
	DirNode* p = list->_child;

	while (p) {
		if (p->_name.cmp_raw(name) == 0) {
			UNLINK(&list->_child, p);
			delete p;
			return 0;
		}

		p = p->_next;
	}
	return -1;
}

} /* namespace::vos */
