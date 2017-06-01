//
// Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "ListSockAddr.hh"

namespace vos {

const char* ListSockAddr::__cname = "ListSockAddr";

/**
 * Method `NEW(lsa, str, sep, def_port)` will create a list of SockAddr by
 * parsing list of address in `str`, where each address is separated by `sep`
 * (default to comma).
 */
int ListSockAddr::NEW(ListSockAddr **lsa, const char *str, const char sep
	, const uint16_t def_port)
{
	if (str == NULL) {
		return -1;
	}

	size_t str_len = strlen(str);
	if (str_len <= 0) {
		return -1;
	}

	Buffer b(str, str_len);

	List *addrs = SPLIT_BY_CHAR(&b, sep, 1);
	if (addrs == NULL) {
		return -1;
	}

	int s;
	long int port;
	SockAddr* saddr = NULL;
	ListSockAddr* list_sa = new ListSockAddr();

	for (int x = 0; x < addrs->size(); x++) {
		Buffer *b_addr = (Buffer*) addrs->at(x);

		List *addr_port = SPLIT_BY_CHAR(b_addr, DEF_ADDR_PORT_SEP);

		Buffer *addr = (Buffer*) addr_port->at(0);

		if (addr_port->size() == 2) {
			Buffer *b_port = (Buffer*) addr_port->at(1);

			if (b_port->len() == 0) {
				port = def_port;
			} else {
				s = b_port->to_lint(&port);
				if (s != 0 || port < 0 || port > 65535) {
					port = def_port;
				}
			}
		} else {
			port = def_port;
		}

		s = SockAddr::INIT(&saddr, AF_INET, addr->v(), (uint16_t) port);
		if (s == 0) {
			list_sa->push_tail(saddr);
		}

		delete addr_port;
	}

	delete addrs;

	if (list_sa->size() == 0) {
		delete list_sa;
		return -1;
	}

	*lsa = list_sa;

	return 0;
}

ListSockAddr::ListSockAddr()
: List()
, _p_current(NULL)
, _p_current_sa(NULL)
{}

ListSockAddr::~ListSockAddr()
{}

/**
 * Method `reset()` will clear all addresses.
 */
void ListSockAddr::reset()
{
	_p_current_sa = (SockAddr*) pop_head();
	while (_p_current_sa) {
		delete _p_current_sa;
		_p_current_sa = (SockAddr*) pop_head();
	}
}

/**
 * Method `rotate()` will switch pointer of current socket address to another
 * one in the list and return it.
 */
SockAddr* ListSockAddr::rotate()
{
	if (!_p_current) {
		_p_current = _head;
		if (_p_current) {
			_p_current_sa = (SockAddr*) _p_current->_item;
		}
	} else {
		_p_current = _p_current->_right;
		_p_current_sa = (SockAddr*) _p_current->_item;
	}

	if (LIBVOS_DEBUG && _p_current_sa) {
		fprintf(stderr, "[%s] Switch to '%s'\n", __cname
			, _p_current_sa->chars());
	}

	return _p_current_sa;
}

/**
 * Method `get()` will return the current socket address.
 */
SockAddr* ListSockAddr::get()
{
	return _p_current_sa;
}

} // namespace::vos
