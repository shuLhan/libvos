//
// Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "ListSockAddr.hh"

namespace vos {

/**
 * Method `LISTSOCKADDR_CREATE(list, sep, def_port)` will parse list of
 * address in `str` where each address is separated by `sep` (default to
 * comma).
 *
 * On success it will return List of SockAddr, otherwise it will return NULL.
 */
List* LISTSOCKADDR_CREATE(const char *str, const char sep,
	const uint16_t def_port)
{
	if (str == NULL) {
		return NULL;
	}

	size_t str_len = strlen(str);
	if (str_len <= 0) {
		return NULL;
	}

	Buffer b(str, str_len);

	List *addrs = SPLIT_BY_CHAR(&b, sep, 1);
	if (addrs == NULL) {
		return NULL;
	}

	int s;
	long int port;
	SockAddr* saddr = NULL;
	List *list_address = new List();

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
			list_address->push_tail(saddr);
		}

		delete addr_port;
	}

	delete addrs;

	if (list_address->size() == 0) {
		delete list_address;
		return NULL;
	}

	return list_address;
}

} // namespace::vos
