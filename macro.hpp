/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_MACRO_HPP
#define	_LIBVOS_MACRO_HPP	1

namespace vos {
/**
 * @ref:
 * http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
 *
 * A macro to disallow the copy constructor and operator= functions
 * This should be used in the private: declarations for a class
 */
#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
	TypeName(const TypeName&);		\
	void operator=(const TypeName&)

} /* namespace::vos */

#endif
