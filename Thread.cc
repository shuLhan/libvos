/**
 * Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Thread.hh"

namespace vos {

const char* Thread::__cname = "Thread";

Thread::Thread(void* (*fn)(void* arg))
:	Locker()
,	_arg(NULL)
,	_init(0)
,	_running(0)
,	_waiting(0)
,	_id(0)
,	_attr()
,	_cond()
,	_fn(fn)
{
	int s = 0;

	if (!fn) {
		_fn = &Thread::RUN;
	}

	s = pthread_attr_init(&_attr);
	if (s) {
		_init = s;
		perror(__cname);
	}

	s = pthread_cond_init(&_cond, NULL);
	if (s) {
		_init = s;
		perror(__cname);
	}
}

Thread::~Thread()
{
	lock();
	pthread_attr_destroy(&_attr);
	pthread_cond_destroy(&_cond);
	unlock();
}

void* Thread::run(void* arg)
{
	int* p = (int*) arg;
	return p;
}

int Thread::is_running()
{
	return _running;
}

pthread_t Thread::get_id()
{
	return _id;
}

int Thread::start(void* arg)
{
	int s = 0;

	_running = 1;

	_arg = arg;
	if (!_arg) {
		_arg = this;
	}

	s = pthread_create(&_id, &_attr, _fn, _arg);

	return s;
}

void Thread::stop()
{
	lock();
	_running = 0;
	unlock();
}

int Thread::wait()
{
	int s = 0;

	lock();
	_waiting = 1;
	s = pthread_cond_wait(&_cond, &_lock);
	_waiting = 0;
	unlock();

	return s;
}

int Thread::wakeup()
{
	int s = 0;

	lock();
	if (_waiting > 0) {
		s = pthread_cond_signal(&_cond);
	}
	unlock();

	return s;
}

int Thread::join(void** res)
{
	int s = 0;

	_init = 0;

	wakeup();

	if (_id) {
		s = pthread_join(_id, res);
	}

	return s;
}

void* Thread::RUN(void* arg)
{
	Thread* T = (Thread*) arg;

	return T->run(T->_arg);
}

}
