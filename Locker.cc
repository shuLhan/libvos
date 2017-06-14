//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Locker.hh"

namespace vos {

const char* Locker::__CNAME = "Locker";

/**
 * Method `Locker()` will initialize locker mutex.
 */
Locker::Locker(int type)
:	Object()
,	_attr_type(type)
,	_attr()
,	_lock()
{
	int s = 0;

	do {
		s = pthread_mutexattr_init(&_attr);
		if (s) {
			perror(__CNAME);
		}
	} while(s == ENOMEM);

	do {
		s = pthread_mutexattr_settype(&_attr, type);
		if (s) {
			perror(__CNAME);
			type = PTHREAD_MUTEX_ERRORCHECK;
		}
	} while(s == EINVAL);

	do {
		s = pthread_mutex_init(&_lock, &_attr);
		if (s) {
			perror(__CNAME);
		}
	} while(s == EAGAIN || s == ENOMEM);
}

/**
 * Method `~Locker()` will destory locker mutex.
 */
Locker::~Locker()
{
	pthread_mutexattr_destroy(&_attr);
	pthread_mutex_destroy(&_lock);
}

const char* Locker::chars()
{
	return __str;
}

/**
 * Method `lock()` will try to lock the mutex.
 */
void Locker::lock()
{
	int s = 0;
	struct timespec interval;

	do {
		s = pthread_mutex_lock(&_lock);

		if (s == 0) {
			break;
		}

		switch(s) {
		case EAGAIN:
		case EBUSY:
			interval.tv_sec = 0;
			interval.tv_nsec = 100;
			nanosleep(&interval, NULL);
			break;
		case EDEADLK:
		case EOWNERDEAD:
			perror(__CNAME);
			unlock();
			break;
		}
	} while (s);
}

/**
 * Method is_locked() will return 1 if mutex is already locked, or 0 if its not
 * locked.
 */
int Locker::is_locked()
{
	if (_attr_type == PTHREAD_MUTEX_ERRORCHECK) {
		int s = pthread_mutex_trylock(&_lock);
		if (s == 0) {
			return 0;
		}
		if (s == EBUSY) {
			return 1;
		}
	}
	return 1;
}

/**
 * Method `unlock()` will try to unlock the mutex.
 */
void Locker::unlock()
{
	pthread_mutex_unlock(&_lock);
}

} // namespace vos
// vi: ts=8 sw=8 tw=80:
