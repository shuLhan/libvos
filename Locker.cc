//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Locker.hh"

namespace vos {

const char* Locker::__cname = "Locker";

//
// `Locker()` constructor will initialize locker mutex.
//
Locker::Locker() : Object()
,	_lock()
{
	pthread_mutex_init(&_lock, NULL);
}

//
// `~Locker()` destructor will destory locker mutex.
//
Locker::~Locker()
{
	pthread_mutex_destroy(&_lock);
}

const char* Locker::chars()
{
	return __str;
}

//
// `lock()` will try to lock the mutex.
//
void Locker::lock()
{
	while (pthread_mutex_trylock(&_lock) != 0)
		;
}

//
// `unlock()` will try to unlock the mutex.
//
void Locker::unlock()
{
	while (pthread_mutex_unlock(&_lock) != 0)
		;
}

} // namespace vos
// vi: ts=8 sw=8 tw=78:
