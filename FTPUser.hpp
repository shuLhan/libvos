/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FTP_USER_HPP
#define	_LIBVOS_FTP_USER_HPP	1

#include "Buffer.hpp"

namespace vos {

class FTPUser {
public:
	FTPUser();
	~FTPUser();

	Buffer		_name;
	Buffer		_pass;
	FTPUser*	_next;

	static int INIT(FTPUser** user, const char* name, const char* pass);
	static int ADD(FTPUser** list, FTPUser* user);
	static int IS_NAME_EXIST(FTPUser* users, Buffer* name);
	static int IS_EXIST(FTPUser* users, Buffer* name, Buffer* pass);
private:
	FTPUser(const FTPUser&);
	void operator=(const FTPUser&);
};

} /* namespace::vos */

#endif
