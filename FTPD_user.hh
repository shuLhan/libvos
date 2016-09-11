//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_FTP_USER_HH
#define _LIBVOS_FTP_USER_HH 1

#include "Buffer.hh"

namespace vos {

/**
 * @class	: FTPD_user
 * @attr	:
 *	- _name	: name of user.
 *	- _pass	: string to identified user name at login.
 *	- _next	: pointer to the next FTPD_user in the list.
 * @desc	: This class handling each user account on FTP server.
 */
class FTPD_user {
public:
	FTPD_user();
	~FTPD_user();

	Buffer		_name;
	Buffer		_pass;
	FTPD_user*	_next;

	static int INIT(FTPD_user** user, const char* name, const char* pass);
	static int ADD(FTPD_user** list, FTPD_user* account);
	static int IS_EXIST(FTPD_user* users, Buffer* name, Buffer* pass = NULL);
private:
	FTPD_user(const FTPD_user&);
	void operator=(const FTPD_user&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
