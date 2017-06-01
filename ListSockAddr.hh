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

class ListSockAddr : public List {
public:
	// `__cname` contain canonical name of this object.
	static const char* __cname;

	static int NEW(ListSockAddr **lsa, const char *str, const char sep,
		const uint16_t def_port);

	ListSockAddr();
	~ListSockAddr();

	void reset();
	SockAddr* rotate();
	SockAddr* get();
private:
	ListSockAddr(const ListSockAddr&);
	void operator=(const ListSockAddr&);

	// _p_current is a pointer to current (active) address in list of
	// addresses.
	BNode *_p_current;

	// _p_current_Sa is a pointer to current SockAddr object.
	SockAddr *_p_current_sa;
};

} // namespace::vos
#endif
