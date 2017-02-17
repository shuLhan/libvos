/**
 * Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "test.hh"
#include "../Thread.hh"

using vos::Thread;

int data = 0;

void* run(void* arg)
{
	Thread* T = (Thread*) arg;

	data++;

	printf("Thread data: %d\n", data);

	T->wait();

	return 0;
}

class Run : public Thread {
public:
	inline Run() : Thread(NULL) {};
	inline ~Run() {};

	inline void* run(void* arg)
	{
		Run* me = (Run*) arg;
		data++;

		printf("Thread data: %d\n", data);

		me->wait();

		return 0;
	}
};

int main()
{
	Thread T1(&run);
	Thread T2(&run);
	Thread T3(&run);
	Thread T4(&run);
	Run me;

	T1.start(&T1);
	T2.start(&T2);
	T3.start(&T3);
	T4.start(&T4);
	me.start(&me);

	sleep(1);

	T1.join();
	T2.join();
	T3.join();
	T4.join();
	me.join();
}
