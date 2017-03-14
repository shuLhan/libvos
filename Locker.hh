//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
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
	explicit Locker(int type = PTHREAD_MUTEX_NORMAL);
	virtual ~Locker();
	const char* chars();

	void lock();
	void unlock();

	int _attr_type;
	pthread_mutexattr_t _attr;
	pthread_mutex_t _lock;

	static const char* __cname;
private:
	Locker(const Locker&);
	void operator=(const Locker&);
};

} // namespace vos
#endif
// vi: ts=8 sw=8 tw=78:
