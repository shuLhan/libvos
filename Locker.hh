//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_LOCKER_HH
#define _LIBVOS_LOCKER_HH 1

#include <pthread.h>
#include "Object.hh"

namespace vos {

class Locker : public Object {
public:
	static const char* __CNAME;

	explicit Locker(int type = PTHREAD_MUTEX_ERRORCHECK);
	virtual ~Locker();
	const char* chars();

	void lock();
	int is_locked();
	void unlock();

protected:
	int _attr_type;
	pthread_mutexattr_t _attr;
	pthread_mutex_t _lock;

private:
	Locker(const Locker&);
	void operator=(const Locker&);
};

} // namespace vos
#endif
// vi: ts=8 sw=8 tw=80:
