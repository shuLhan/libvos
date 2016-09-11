//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "../DNSQuery.hpp"
#include "../DNS_rr.hpp"
#include "../SSVReader.hpp"

int main (int argc, char** argv)
{
	register int s;
	int d;
	vos::SSVReader reader;
	vos::Record* ip= NULL;
	vos::Record* r = NULL;
	vos::Record* c = NULL;
	vos::DNSQuery qanswer;

	reader._comment_c = '#';

	s = reader.load ("./hosts");

	r = reader._rows;
	while (r) {
		ip = r;

		//printf ("IP : %s\n", ip->_v);

		c = ip->_next_col;
		while (c) {
			//printf ("\t %s\n", c->_v);
			s = inet_pton (AF_INET, ip->_v, &d);

			if (s == 1) {
				qanswer.create_answer (c->_v
					, vos::QUERY_T_ADDRESS
					, vos::QUERY_C_IN
					, UINT_MAX
					, ip->_i, ip->_v);

				qanswer.extract (vos::DNSQ_EXTRACT_RR_AUTH);
				//qanswer.dump ();

				c = c->_next_col;
			} else {
				c = NULL;
			}
		}

		r = r->_next_row;
	}

	return 0;
}

// vi: ts=8 sw=8 tw=78:
