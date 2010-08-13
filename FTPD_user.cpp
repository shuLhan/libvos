/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "FTPD_user.hpp"

namespace vos {

FTPD_user::FTPD_user() :
	_name()
,	_pass()
,	_next(NULL)
{}

FTPD_user::~FTPD_user()
{
	if (_next) {
		delete _next;
		_next = NULL;
	}
}

/**
 * @method	: FTPD_user::INIT
 * @param	:
 *	> user	: pointer to a new FTPD_user object.
 *	> name	: user-name, string identified as user when login.
 *	> pass	: string for authenticated user when login.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create a new FTPD_user object.
 */
int FTPD_user::INIT(FTPD_user** user, const char* name, const char* pass)
{
	if (!name) {
		return -1;
	}

	(*user) = new FTPD_user();
	if (! (*user)) {
		return -1;
	}

	(*user)->_name.copy_raw(name);
	(*user)->_pass.copy_raw(pass);

	return 0;
}

/**
 * @method	: FTPD_user::ADD
 * @param	:
 *	> list	: pointer to list of FTPD_user object.
 *	> user	: pointer to FTPD_user object to be added to the 'list'.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail, user name already exist.
 * @desc	: add 'user' to 'list' of FTPD_user.
 */
int FTPD_user::ADD(FTPD_user** list, FTPD_user* account)
{
	if (!account) {
		return 0;
	}

	int		s;
	FTPD_user*	p = (*list);

	if (! (*list)) {
		(*list) = account;
	} else {
		while (p->_next) {
			s = account->_name.like(&p->_name);
			if (s == 0) {
				return -1;
			}
			p = p->_next;
		}
		s = account->_name.like(&p->_name);
		if (s == 0) {
			return -1;
		}
		p->_next = account;
	}
	return 0;
}

/**
 * @method	: FTPD_user::IS_EXIST
 * @param	:
 *	> users	: list of user.
 *	> name	: name of user to search for.
 *	> pass	: identification for user 'name'.
 * @return	:
 *	< 1	: true, user with 'name' and 'pass' found.
 *	< 0	: false, user with 'name' and 'pass' NOT found.
 * @desc	: search list of user 'users' for user with the name is 'name'
 * and identified by 'pass'. If 'pass' is nil, only check for the 'name'.
 */
int FTPD_user::IS_EXIST(FTPD_user* users, Buffer* name, Buffer* pass)
{
	if (!users || !name) {
		return 0;
	}

	int		s;
	FTPD_user*	p = users;

	while (p) {
		s = name->like(&p->_name);
		if (s == 0) {
			if (pass) {
				s = pass->like(&p->_pass);
				if (s == 0) {
					return 1;
				}
			}
			return 0;
		}
		p = p->_next;
	}
	return 0;
}

} /* namespace::vos */
