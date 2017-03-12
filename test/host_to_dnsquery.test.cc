//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../DNSQuery.hh"
#include "../List.hh"
#include "../SSVReader.hh"

int main()
{
	const char* exp_ips[] = {
			"127.0.0.1"
		,	"::1"
		,	"127.0.0.1"
		,	"127.0.0.1"
		};
	const char* exp_addr[] = {
			"localhost.localdomain"
		,	"localhost"
		,	"localhost.localdomain"
		,	"localhost"
		,	"bubu"
		,	"local.jquery.com"
		,	"local.api.jquery.com"
		,	"local.blog.jquery.com"
		};

	register int s;
	int x = 0;
	int y = 0;
	int exp_addr_idx = 0;
	int d;
	vos::SSVReader reader;
	vos::Buffer* ip = NULL;
	vos::List* row = NULL;
	vos::Buffer* c = NULL;
	vos::DNSQuery qanswer;

	reader._comment_c = '#';

	s = reader.load ("./hosts");
	if (s) {
		return 1;
	}

	for (; x < reader._rows->size(); x++) {
		row = (vos::List*) reader._rows->at(x);
		ip = (vos::Buffer*) row->at(0);

		expectString(exp_ips[x], ip->chars(), 0);

		for (y = 1; y < row->size(); y++) {
			c = (vos::Buffer*) row->at(y);

			expectString(exp_addr[exp_addr_idx++], c->chars(), 0);

			s = inet_pton (AF_INET, ip->chars(), &d);

			if (s == 1) {
				qanswer.create_answer (c->chars()
					, vos::QUERY_T_ADDRESS
					, vos::QUERY_C_IN
					, UINT_MAX
					, (uint16_t) ip->_i, ip->chars());

				qanswer.extract (vos::DNSQ_EXTRACT_RR_AUTH);
				//qanswer.dump ();
			}
		}
	}

	return 0;
}

// vi: ts=8 sw=8 tw=78:
