/*
 * Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE file.
 */

#include "libvos.hh"

#if (NO_DEFAULT_LIBS)

extern "C" void* __my_cpp_new(size_t len) { \
	void *p = calloc(1, len);
	if (!p) {
		(void)!write(2, "out of memory\n", 14);
		abort();
	}
	return p;
}

extern "C" void __my_cpp_delete(void* p) {
	if (p) {
		free(p);
		p = NULL;
	}
}

void* operator new(size_t len)		__attribute__((alias("__my_cpp_new")));
void* operator new[](size_t len)	__attribute__((alias("__my_cpp_new")));
void  operator delete(void* p)		__attribute__((alias("__my_cpp_delete")));
void  operator delete[](void* p)	__attribute__((alias("__my_cpp_delete")));
void  operator delete(void* p, unsigned long)	\
	__attribute__((alias("__my_cpp_delete")));
void  operator delete[](void* p, unsigned long)	\
	__attribute__((alias("__my_cpp_delete")));

void* __cxa_pure_virtual = 0;
void* __gxx_personality_v0 = 0;

#endif	/* NO_DEFAULT_LIBS */

namespace vos {

int LIBVOS_DEBUG = getenv("LIBVOS_DEBUG") == NULL
			? 0
			: atoi(getenv("LIBVOS_DEBUG"));

} /* namespace::vos */
// vi: ts=8 sw=8:
