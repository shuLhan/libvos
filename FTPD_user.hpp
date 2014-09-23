/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_FTP_USER_HPP
#define	_LIBVOS_FTP_USER_HPP	1

#include "Buffer.hpp"

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
