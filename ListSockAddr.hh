//
// Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_LIST_SOCK_ADDR_HH
#define _LIBVOS_LIST_SOCK_ADDR_HH 1

#include "Buffer.hh"
#include "List.hh"
#include "ListBuffer.hh"
#include "SockAddr.hh"

namespace vos {

#define DEF_ADDR_PORT_SEP ':'

List* LISTSOCKADDR_CREATE(const char *list, const char sep,
	const uint16_t def_port);

} // namespace::vos
#endif
