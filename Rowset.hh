//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_ROWSET_HH
#define _LIBVOS_ROWSET_HH 1

#include "List.hh"

namespace vos {

//
// class Rowset define a list of row.
// Each row may contain different number of record.
//
class Rowset : public List {
public:
	Rowset();
	virtual ~Rowset();

	const char* chars();

	// `__cname` contain canonical name of this object.
	static const char* __cname;
private:

	Rowset(const Rowset&);
	void operator=(const Rowset&);
};

} // namespace vos
#endif
// vi: ts=8 sw=8 tw=78:
