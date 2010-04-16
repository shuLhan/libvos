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

CXXFLAGS_ADD	+= -I${LIBVOS_SRC_D}

# link needed for FTP and OCI on Solaris system.
ifeq (${SYS},SunOS)
LDFLAGS		+= -lsocket -lnsl
endif

LIBVOS_BLD_D	= ${LIBVOS_SRC_D}/build

TARGET_OBJS	=						\
			${LIBVOS_BLD_D}/libvos.oo		\
			${LIBVOS_BLD_D}/Buffer.oo		\
			${LIBVOS_BLD_D}/ConfigData.oo		\
			${LIBVOS_BLD_D}/Config.oo		\
			${LIBVOS_BLD_D}/DirNode.oo		\
			${LIBVOS_BLD_D}/Dir.oo			\
			${LIBVOS_BLD_D}/Dlogger.oo		\
			${LIBVOS_BLD_D}/DNS_rr.oo		\
			${LIBVOS_BLD_D}/DNSQuery.oo		\
			${LIBVOS_BLD_D}/File.oo			\
			${LIBVOS_BLD_D}/FTP.oo			\
			${LIBVOS_BLD_D}/Reader.oo		\
			${LIBVOS_BLD_D}/RecordMD.oo		\
			${LIBVOS_BLD_D}/Record.oo		\
			${LIBVOS_BLD_D}/Resolver.oo		\
			${LIBVOS_BLD_D}/SockAddr.oo		\
			${LIBVOS_BLD_D}/Socket.oo		\
			${LIBVOS_BLD_D}/Writer.oo

#
# with OCI library
#
ifdef ORACLE_HOME
TARGET_OBJS	+=	${LIBVOS_BLD_D}/OCIValue.oo	\
			${LIBVOS_BLD_D}/OCI.oo

CXXFLAGS_ADD	+=	-I${ORACLE_HOME}/include
LDFLAGS		+=	-L${ORACLE_HOME}/lib -lclntsh -lnnz10
endif

PRE_TARGET	= ${LIBVOS_BLD_D}
TARGET		= ${TARGET_OBJS}

.PHONY: all debug install clean

all:: CXXFLAGS+=${CXXFLAGS_ADD}
all:: ${PRE_TARGET} ${TARGET}

all-32:: CXXFLAGS_ADD+=-m32
all-32:: LDFLAGS+=-m32
all-32:: all

debug:: CXXFLAGS=${CXXFLAGS_DEBUG} ${CXXFLAGS_ADD}
debug:: ${PRE_TARGET} ${TARGET}

${LIBVOS_BLD_D}:
	@mkdir -p $@

${LIBVOS_BLD_D}/Buffer.oo	: ${LIBVOS_BLD_D}/libvos.oo

${LIBVOS_BLD_D}/Dir.oo		\
${LIBVOS_BLD_D}/DirNode.oo	\
${LIBVOS_BLD_D}/Record.oo	\
${LIBVOS_BLD_D}/DNS_rr.oo	\
${LIBVOS_BLD_D}/ConfigData.oo	\
${LIBVOS_BLD_D}/File.oo		: ${LIBVOS_BLD_D}/Buffer.oo

${LIBVOS_BLD_D}/Dir.oo		: ${LIBVOS_BLD_D}/DirNode.oo

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
	@${do_compile}

clean::
	@echo "[R] ${LIBVOS_BLD_D}";	\
		rm -rf ${LIBVOS_BLD_D}
