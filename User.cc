//
// Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "User.hh"

namespace vos {

const char* User::__cname = "User";

/**
 * Method User() will set the user name, UID, GID, EUID, EGID to current user
 * who run the program.
 */
User::User()
:	Object()
,	name(getlogin())
,	id(getuid())
,	gid(getgid())
,	ename()
,	eid(geteuid())
,	egid(getegid())
{}

User::~User()
{}

/**
 * Method chars() will return the current User object to JSON string format.
 */
const char* User::chars()
{
	if (__str) {
		free(__str);
		__str = NULL;
	}

	Buffer b;
	b.aprint("{ " K(name) ": " K(%s) ", " K(id) ": %d, " K(gid) ": %d, " K(eid) ": %d, " K(egid) ": %d }"
		, name.chars(), id, gid, eid, egid);

	__str = b.detach();

	return __str;
}

/**
 * Method get_name() will return current user name.
 */
const char* User::get_name()
{
	return name.chars();
}

/**
 * Method get_id() will return current user ID.
 */
uid_t User::get_id()
{
	return id;
}

/**
 * Method get_gid() will return current user group ID.
 */
gid_t User::get_gid()
{
	return gid;
}

/**
 * Method set_effective_name() will set the process effective user name, EUID,
 * and EGID to user `name`.
 *
 * It will return,
 *
 * `0` on success, or
 * `-1` if out of memory, or
 * `-2` if user `name` is not exist in the system.
 */
int User::set_effective_name(const char* name)
{
	if (name) {
		ename.copy_raw(name);
	}

	int s = 0;
	char* buf = NULL;
	ssize_t buflen = 0;
	struct passwd user_raw;
	struct passwd* user_raw_p;

	buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (buflen == -1) {
		buflen = 16384;
	}

	buf = (char*) calloc(size_t(buflen), sizeof(char));
	if (!buf) {
		s = -1;
		goto out;
	}
	
	s = getpwnam_r(ename.chars(), &user_raw, buf, size_t(buflen)
		, &user_raw_p);
	if (s) {
		s = -2;
		goto out;
	}

	eid = user_raw.pw_uid;
	egid = user_raw.pw_gid;

out:
	if (s) {
		perror(__cname);
	}
	if (buf) {
		free(buf);
		buf = NULL;
	}

	return s;
}

/**
 * Method get_effective_name() will return current effective name.
 */
const char* User::get_effective_name()
{
	return ename.chars();
}

/**
 * Method get_effective_id() will return current effective ID.
 */
uid_t User::get_effective_id()
{
	return eid;
}

/**
 * Method get_effective_gid() will return current effective group ID.
 */
gid_t User::get_effective_gid()
{
	return egid;
}

/**
 * Method switch_to_effective() will set the effective user ID and group ID to
 * user ID and group ID of system user identified by `name`.
 *
 * It will return,
 *
 * `0` on success, or
 * `-1` if fail at getting information about user `name` from system,
 * `-2` if fail setting EGID
 * `-3` if fail setting EUID
 */
int User::switch_to_effective(const char* name)
{
	int s = 0;

	if (name) {
		s = set_effective_name(name);
		if (s) {
			return -1;
		}
	}

	s = setegid(egid);
	if (s) {
		s = -2;
		goto out;
	}

	s = seteuid(eid);
	if (s) {
		s = -3;
		goto out;
	}

out:
	if (s) {
		perror(__cname);
	}

	return s;
}

/**
 * Method switch_to_real() will set the effective user back to real user ID.
 *
 * It will return,
 *
 * `0` on success,
 * `-1` if fail when setting EGID, or
 * `-2` if fail when setting EUID back to real UID.
 */
int User::switch_to_real()
{
	int s = 0;

	s = setegid(gid);
	if (s) {
		s = -1;
		goto out;
	}

	s = seteuid(id);
	if (s) {
		s = -2;
		goto out;
	}

	eid = id;
	egid = gid;
	ename.copy(&name);

out:
	if (s) {
		perror(__cname);
	}

	return s;
}

}
