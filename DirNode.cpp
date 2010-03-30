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
	_linkname(NULL),
	_parent(NULL),
	_child(NULL),
	_next(NULL),
	_last(NULL)
{}

DirNode::~DirNode()
{
	if (_linkname) {
		delete _linkname;
	}
	if (_child) {
		delete _child;
	}
	if (_next) {
		delete _next;
	}
}

/**
 * @method	: DirNode::get_stat
 * @param	:
 *	> path	: path to directory.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: get 'path' attribute.
 */
int DirNode::get_stat(const char *realpath, const char *path)
{
	if (!path) {
		return 0;
	}

	register int	s;
	struct stat	st;

	if (path) {
		_name.copy_raw(path);
	} else {
		_name.copy_raw(realpath);
	}

	s = lstat(realpath, &st);
	if (s < 0) {
		return -1;
	}

	if (S_ISLNK(st.st_mode)) {
		s = get_linkname();
		if (s < 0) {
			return -1;
		}

		memset(&st, 0, sizeof(struct stat));
		s = stat(_linkname->_v, &st);
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

int DirNode::get_linkname()
{
	int s = 0;

	if (!_linkname) {
		s = Buffer::INIT_SIZE(&_linkname, 255);
		if (s < 0) {
			return -1;
		}
	}

	while (1) {
		s = readlink(_name._v, _linkname->_v, _linkname->_l);
		if (s < 0) {
			return -1;
		}
		if (s < _linkname->_l) {
			break;
		}

		s = _linkname->resize(_linkname->_l * 2);
		if (s < 0) {
			return -1;
		}
	}

	return 0;
}

int DirNode::is_dir()
{
	return S_ISDIR(_mode);
}

int DirNode::is_link()
{
	return S_ISLNK(_mode);
}

void DirNode::dump()
{
	if (is_dir()) {
		printf("d ");
	} else {
		printf("- ");
	}
	printf("|%d|%d|%d|%ld|%ld|%s\n",
		_mode, _uid, _gid, _size, _mtime, _name._v);

	if (_next) {
		_next->dump();
	}
	if (_child) {
		_child->dump();
	}
}

/**
 * @method	: DirNode::ADD
 * @param	:
 *	> head	: head of list.
 *	> node	: pointer to DirNode object to be added to list.
 * @desc	: add 'node' to the list of 'head'.
 */
void DirNode::ADD(DirNode **head, DirNode *node)
{
	if (!(*head)) {
		(*head) = node;
	} else {
		(*head)->_last->_next	= node;
		node->_parent		= (*head)->_parent;
	}
	(*head)->_last = node;
}

} /* namespace::vos */
