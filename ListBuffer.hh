//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_LISTBUFFER_HH
#define _LIBVOS_LISTBUFFER_HH 1

#include "Buffer.hh"
#include "List.hh"

namespace vos {

List* SPLIT_BY_CHAR(Buffer *b, const char sep, int trim = 0);
List* SPLIT_BY_WHITESPACE(Buffer *b);

} // namespace::vos

#endif //
