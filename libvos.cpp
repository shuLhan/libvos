/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#include "libvos.hpp"

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
	}
}

void* operator new(size_t len)		__attribute__((alias("__my_cpp_new")));
void* operator new[](size_t len)	__attribute__((alias("__my_cpp_new")));
void  operator delete(void* p)		__attribute__((alias("__my_cpp_delete")));
void  operator delete[](void* p)	__attribute__((alias("__my_cpp_delete")));

void* __cxa_pure_virtual = 0;

#endif	/* NO_DEFAULT_LIBS */

namespace vos {

int LIBVOS_DEBUG = getenv("LIBVOS_DEBUG") == NULL
			? 0
			: atoi(getenv("LIBVOS_DEBUG"));

} /* namespace::vos */
