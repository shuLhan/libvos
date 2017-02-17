/**
 * Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef	_LIBVOS_THREAD_HH
#define	_LIBVOS_THREAD_HH	1

#include "Locker.hh"

namespace vos {

class Thread : public Locker {
public:
	Thread(void* (*fn)(void* arg) = NULL);
	~Thread();

	void* _arg;

	virtual void* run(void* arg = NULL);

	int is_running();
	pthread_t get_id();

	virtual int start(void* arg = NULL);
	void stop();
	int wait();
	int wakeup();
	int join(void** res = NULL);

	static const char* __cname;

	static void* RUN(void *arg);
private:
	Thread(const Thread&);
	void operator=(const Thread&);

	int _init;
	uint8_t _running;
	uint16_t _waiting;
	pthread_t _id;
	pthread_attr_t _attr;
	pthread_cond_t _cond;
	void *(*_fn)(void* arg);
};

}

#endif
