#!/usr/bin/make
##
# Copyright (C) 2009 kilabit.org
# Author:
#	- m.shulhan (ms@kilabit.org)
##

#
# LIBVOS_D variable is defined by user who want to use this library.
#
ifdef LIBVOS_D
LIBVOS_SRC_D	= ${LIBVOS_D}
else
LIBVOS_SRC_D	= .
endif

include ${LIBVOS_SRC_D}/Makefile.common

CPPFLAGS	+= -I${LIBVOS_SRC_D}

# link needed for FTP and OCI on Solaris system.
ifeq (${SYS},SunOS)
LDFLAGS		+= -lsocket -lnsl
endif

LIBVOS_BLD_D	= ${LIBVOS_SRC_D}/build

LIBVOS_OBJS	=						\
			${LIBVOS_BLD_D}/Error.oo		\
			${LIBVOS_BLD_D}/Buffer.oo		\
			${LIBVOS_BLD_D}/ConfigData.oo		\
			${LIBVOS_BLD_D}/Config.oo		\
			${LIBVOS_BLD_D}/Dlogger.oo		\
			${LIBVOS_BLD_D}/DNSQuery.oo		\
			${LIBVOS_BLD_D}/DNS_rr.oo		\
			${LIBVOS_BLD_D}/File.oo			\
			${LIBVOS_BLD_D}/FTP.oo			\
			${LIBVOS_BLD_D}/Reader.oo		\
			${LIBVOS_BLD_D}/RecordMD.oo		\
			${LIBVOS_BLD_D}/Record.oo		\
			${LIBVOS_BLD_D}/Resolver.oo		\
			${LIBVOS_BLD_D}/Socket.oo		\
			${LIBVOS_BLD_D}/Writer.oo

#
# with OCI library
#
ifneq (${ORACLE_HOME},)
LIBVOS_OBJS	+=	${LIBVOS_BLD_D}/OCIValue.oo	\
			${LIBVOS_BLD_D}/OCI.oo

CPPFLAGS	+=	-I${ORACLE_HOME}/include
LDFLAGS		+=	-L${ORACLE_HOME}/lib -lclntsh
endif


.PHONY: all clean

all:: ${LIBVOS_OBJS}

${LIBVOS_BLD_D}/Buffer.oo	: ${LIBVOS_BLD_D}/Error.oo

${LIBVOS_BLD_D}/Record.oo	\
${LIBVOS_BLD_D}/DNS_rr.oo	\
${LIBVOS_BLD_D}/ConfigData.oo	\
${LIBVOS_BLD_D}/File.oo		: ${LIBVOS_BLD_D}/Buffer.oo

${LIBVOS_BLD_D}/Config.oo	: ${LIBVOS_BLD_D}/ConfigData.oo

${LIBVOS_BLD_D}/RecordMD.oo	\
${LIBVOS_BLD_D}/Config.oo	\
${LIBVOS_BLD_D}/Dlogger.oo	\
${LIBVOS_BLD_D}/Socket.oo	: ${LIBVOS_BLD_D}/File.oo

${LIBVOS_BLD_D}/DNSQuery.oo	: ${LIBVOS_BLD_D}/DNS_rr.oo

${LIBVOS_BLD_D}/FTP.oo		\
${LIBVOS_BLD_D}/Resolver.oo	: ${LIBVOS_BLD_D}/Socket.oo

${LIBVOS_BLD_D}/Reader.oo	\
${LIBVOS_BLD_D}/Writer.oo	: ${LIBVOS_BLD_D}/RecordMD.oo

${LIBVOS_BLD_D}/Reader.oo	\
${LIBVOS_BLD_D}/Writer.oo	: ${LIBVOS_BLD_D}/Record.oo


${LIBVOS_BLD_D}/%.oo: ${LIBVOS_SRC_D}/%.cpp ${LIBVOS_SRC_D}/%.hpp
	@mkdir -p ${LIBVOS_BLD_D}
	@${do_compile}

clean::
	@rm -rf ${LIBVOS_BLD_D}