/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "FTPUser.hpp"

namespace vos {

FTPUser::FTPUser() :
	_name()
,	_pass()
,	_next(NULL)
{}

FTPUser::~FTPUser()
{
	if (_next) {
		delete _next;
		_next = NULL;
	}
}

/**
 * @method	: FTPUser::INIT
 * @param	:
 *	> user	: pointer to a new FTPUser object.
 *	> name	: user-name, string identified as user when login.
 *	> pass	: string for authenticated user when login.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create a new FTPUser object.
 */
int FTPUser::INIT(FTPUser** user, const char* name, const char* pass)
{
	(*user) = new FTPUser();
	if (! (*user)) {
		return -1;
	}

	(*user)->_name.copy_raw(name);
	(*user)->_pass.copy_raw(pass);

	return 0;
}

/**
 * @method	: FTPUser::ADD
 * @param	:
 *	> list	: pointer to list of FTPUser object.
 *	> user	: pointer to FTPUser object to be added to the 'list'.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail, user name already exist.
 * @desc	: add 'user' to 'list' of FTPUser.
 */
int FTPUser::ADD(FTPUser** list, FTPUser* user)
{
	int		s;
	FTPUser*	p = (*list);

	if (! (*list)) {
		(*list) = user;
	} else {
		while (p->_next) {
			s = user->_name.cmp(&p->_name);
			if (s == 0) {
				return -1;
			}
			p = p->_next;
		}
		s = user->_name.cmp(&p->_name);
		if (s == 0) {
			return -1;
		}
		p->_next = user;
	}
	return 0;
}

/**
 * @method	: FTPUser::IS_USER_EXIST
 * @param	:
 *	> users	: list of FTPUser object to search for 'name'.
 *	> name	: name of user to be search on 'list'.
 * @return	:
 *	< 0	: false, user 'name' is NOT exist in 'list' of user.
 *	< 1	: true, user 'name' exist in 'list' of user.
 * @desc	: check if user 'name' exist in 'list' of user.
 */
int FTPUser::IS_NAME_EXIST(FTPUser* users, Buffer* name)
{
	if (!users || !name) {
		return 0;
	}

	int s;
	FTPUser* p = users;

	while (p) {
		s = name->cmp(&p->_name);
		if (s == 0) {
			return 1;
		}
		p = p->_next;
	}
	return 0;
}

/**
 * @method	: FTPUser::IS_EXIST
 * @param	:
 *	> users	: list of user.
 *	> name	: name of user to search for.
 *	> pass	: identification for user 'name'.
 * @return	:
 *	< 1	: true, user with 'name' and 'pass' found.
 *	< 0	: false, user with 'name' and 'pass' NOT found.
 * @desc	: search list of user 'users' for user with the name is 'name'
 * and identified by 'pass'.
 */
int FTPUser::IS_EXIST(FTPUser* users, Buffer* name, Buffer* pass)
{
	if (!users || !name || !pass) {
		return 0;
	}

	int		s;
	FTPUser*	p = users;

	while (p) {
		s = name->cmp(&p->_name);
		if (s == 0) {
			s = pass->cmp(&p->_pass);
			if (s == 0) {
				return 1;
			}
			return 0;
		}
		p = p->_next;
	}
	return 0;
}

} /* namespace::vos */
