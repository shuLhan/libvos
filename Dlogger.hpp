/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_DLOGGER_HPP
#define	_LIBVOS_DLOGGER_HPP	1

#include <pthread.h>
#include "File.hpp"

namespace vos {

class Dlogger : public File {
public:
	Dlogger();
	~Dlogger();

	int open(const char *logfile);
	void close();

	int er(const char *fmt, ...);
	int it(const char *fmt, ...);
private:
	pthread_mutex_t	_lock;
	DISALLOW_COPY_AND_ASSIGN(Dlogger);
};

} /* namespace::vos */

#endif
