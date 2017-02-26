//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef _LIBVOS_USER_HH
#define _LIBVOS_USER_HH 1

#include <pwd.h>
#include "Buffer.hh"

namespace vos {

/**
 * Class User is used to manage user process, getting UID, GID, effective ID
 * and GID, setting effective UID, and/or switching back to real UID.
 */
class User : public Object {
public:
	User();
	~User();
	const char* chars();

	const char* get_name();
	uid_t get_id();
	gid_t get_gid();

	int set_effective_name(const char* name);
	const char* get_effective_name();
	uid_t get_effective_id();
	gid_t get_effective_gid();

	int switch_to_effective(const char* name = NULL);
	int switch_to_real();

	static const char* __cname;

	static User* INIT(const char* name);
private:
	User(const User&);
	void operator=(const User&);

	Buffer name;
	uid_t id;
	gid_t gid;

	Buffer ename;
	uid_t eid;
	gid_t egid;
};

}

#endif
