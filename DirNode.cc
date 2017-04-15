//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "DirNode.hh"

namespace vos {

const char* DirNode::__cname = "DirNode";

DirNode::DirNode()
:	Object()
,	_mode(0)
,	_uid(0)
,	_gid(0)
,	_size(0)
,	_mtime(0)
,	_ctime(0)
,	_name()
,	_linkname()
,	_next(NULL)
,	_prev(NULL)
,	_child(NULL)
,	_link(NULL)
,	_parent(this)
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
 *	> name		: the basename of 'rpath', default to NULL.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: get 'path' attribute.
 */
int DirNode::get_attr(const char* rpath, const char* name)
{
	if (!rpath) {
		return 0;
	}

	if (name) {
		_name.copy_raw(name);
	} else {
		_name.copy_raw(rpath);
	}

	struct stat st;

	int s = lstat(rpath, &st);
	if (s < 0) {
		return -1;
	}

	_mode = st.st_mode;

	if (S_ISLNK(st.st_mode)) {
		s = GET_LINK_NAME(&_linkname, rpath);
		if (s < 0) {
			return -1;
		}

		memset(&st, 0, sizeof(struct stat));

		s = lstat(_linkname.v(), &st);
		if (s < 0) {
			return -1;
		}
	}

	_uid	= st.st_uid;
	_gid	= st.st_gid;
	_size	= st.st_size;
	_mtime	= st.st_mtime;
	_ctime	= st.st_ctime;

	return 0;
}

/**
 * @method	: DirNode::update_attr
 * @param	:
 *	> node	: pointer to DirNode object that will be updated.
 *	> rpath	: a path to the 'node' in file system.
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
	int s = 0;
	struct stat st;

	if (node->_linkname.is_empty()) {
		s = lstat(rpath, &st);
	} else {
		s = lstat(node->_linkname.v(), &st);
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
	DirNode*	p = _child;

	while (p) {
		int s = p->_name.cmp_raw(name);
		if (s == 0) {
			(*node)	= p;
			s	= update_attr(p, rpath);
			if (s < 0) {
				return -2;
			}
			return 0;
		}
		p = p->_next;
	}
	return -1;
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
	} else if (is_file()) {
		printf("> f ");
	} else if (is_link()) {
		printf("> l ");
	} else {
		printf("> - ");
	}
	printf("|%5d|%5d|%5d|%12ld|%ld|%ld|%s|%s|%s", _mode, _uid, _gid, _size
		, _mtime, _ctime, _name.chars(), _linkname.chars()
		, ctime(&_mtime));
	if (_link) {
		printf(" => %s\n", _link->_name.chars());
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
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	create and initialize a new DirNode object based on 'rpath'.
 *	'rpath' is a full path of node, i.e.: '../../node', when 'path' is
 *	only contain 'node'.
 */
int DirNode::INIT(DirNode** node, const char* rpath, const char* path)
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
		printf("[%s] GET_LINK_NAME: '%s'\n", __cname, path);
	}

	char *actual_path;

	actual_path = realpath(path, NULL);
	if (!actual_path) {
		perror(__cname);
		return -1;
	}

	linkname->copy_raw(actual_path);

	free(actual_path);

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

	DirNode*	last	= NULL;
	DirNode*	p	= (*list);

	while (p) {
		int s = p->_name.like(&node->_name);
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
 *	> rpath	: real path of node from root directory.
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

/**
 * @method	: DirNode::UNLINK
 * @param	:
 *	> list	: pointer to the list of DirNode object.
 *	> node	: node to be removed from list.
 * @desc	: Unlink or remove node from list.
 */
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
// vi: ts=8 sw=8 tw=78:
