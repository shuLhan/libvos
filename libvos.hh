//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef	_LIBVOS_MACRO_HH
#define	_LIBVOS_MACRO_HH	1

#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif

#define _DEFAULT_SOURCE 1
#define _XOPEN_SOURCE 700

#ifdef __USE_GNU
#undef __USE_GNU
#endif

#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace vos {

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

extern int LIBVOS_DEBUG;

} // namespace::vos

#endif

// vi: ts=8 sw=8 tw=80:
