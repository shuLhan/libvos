/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "DirNode.hpp"

namespace vos {

DirNode::DirNode(long id) :
	_id(id),
	_pid(0),
	_cid(0),
	_mode(0),
	_uid(0),
	_gid(0),
	_size(0),
	_mtime(0),
	_name(),
	_linkname()
{}

DirNode::~DirNode()
{}

/**
 * @method		: DirNode::get_stat
 * @param		:
 *	> realpath	: a real name for path, if 'path' is symbolic link.
 *	> path		: path to directory.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: get 'path' attribute.
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
 * @method	: DirNode::get_linkname
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: get a realpath to symbolic link.
 */
int DirNode::get_linkname()
{
	int s = 0;

	if (_linkname._i == 0) {
		s = _linkname.init_size(LINKNAME_INIT_SIZE);
		if (s < 0) {
			return -1;
		}
	}

	while (1) {
		s = readlink(_name._v, _linkname._v, _linkname._l);
		if (s < 0) {
			return -1;
		}
		if (s < _linkname._l) {
			break;
		}

		s = _linkname.resize(_linkname._l * 2);
		if (s < 0) {
			return -1;
		}
	}

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
 * @desc	: dump content of DirNode object.
 */
void DirNode::dump()
{
	if (is_dir()) {
		printf("d ");
	} else {
		printf("- ");
	}
	printf("|%8ld|%8ld|%8ld|%d|%d|%d|%12ld|%ld|%s\n",
		_id, _pid, _cid, _mode, _uid, _gid, _size, _mtime, _name._v);
}

/**
 * @method		: DirNode::INIT
 * @param		:
 *	> node		: return value, pointer to a DirNode object.
 *	> realpath	: a full path of node from current directory.
 *	> path		: a name of path in current directory in file system.
 *	> id		: id for a new DirNode object.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	create and initialize a new DirNode object based on 'realpath'.
 *	'realpath' is a full path of node, i.e.: '../../node', when 'path' is
 *	only contain 'node'.
 */
int DirNode::INIT(DirNode **node, const char *realpath, const char *path,
			long id)
{
	(*node) = new DirNode(id);
	if (!(*node)) {
		return -1;
	}

	return (*node)->get_stat(realpath, path);
}

} /* namespace::vos */
