//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "FTPD_user.hh"

namespace vos {

const char* FTPD_user::__cname = "FTPD_user";

FTPD_user::FTPD_user() :
	_name()
,	_pass()
{}

FTPD_user::~FTPD_user()
{}

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

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
