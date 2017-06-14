//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "../Locker.hh"

using vos::Locker;

class TestLocker : public Locker {
public:
	TestLocker();
	~TestLocker();

	void inc();
	void dec();
	int data;
private:
	TestLocker(const TestLocker&);
	void operator=(const TestLocker&);
};
