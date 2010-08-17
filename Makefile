#!/usr/bin/make
##
# Copyright (C) 2010 kilabit.org
# Author:
#	- m.shulhan (ms@kilabit.org)
##

.SECONDEXPANSION:

#
# LIBVOS_D variable is defined by user who want to use this library.
#
ifdef LIBVOS_D
LIBVOS_SRC_D	= $(LIBVOS_D)
else
LIBVOS_SRC_D	= .
endif

include $(LIBVOS_SRC_D)/Makefile.common

CXXFLAGS_ADD	+= -I$(LIBVOS_SRC_D)
LIBVOS_BLD_D	= $(LIBVOS_SRC_D)/build
PRE_TARGET	= $(LIBVOS_BLD_D)
TARGET		=
TARGET_OBJS	=						\
			$(LIBVOS_BLD_D)/libvos.oo		\
			$(LIBVOS_BLD_D)/Buffer.oo		\
			$(LIBVOS_BLD_D)/File.oo			\
			$(LIBVOS_BLD_D)/Dlogger.oo		\
			$(LIBVOS_BLD_D)/Config.oo		\
			$(LIBVOS_BLD_D)/ConfigData.oo		\
			$(LIBVOS_BLD_D)/RecordMD.oo		\
			$(LIBVOS_BLD_D)/Record.oo		\
			$(LIBVOS_BLD_D)/Reader.oo		\
			$(LIBVOS_BLD_D)/Writer.oo		\
			$(LIBVOS_BLD_D)/Dir.oo			\
			$(LIBVOS_BLD_D)/DirNode.oo		\
			$(LIBVOS_BLD_D)/SockAddr.oo		\
			$(LIBVOS_BLD_D)/Socket.oo		\
			$(LIBVOS_BLD_D)/SockServer.oo		\
			$(LIBVOS_BLD_D)/DNS_rr.oo		\
			$(LIBVOS_BLD_D)/DNSQuery.oo		\
			$(LIBVOS_BLD_D)/Resolver.oo		\
			$(LIBVOS_BLD_D)/FTP_cmd.oo		\
			$(LIBVOS_BLD_D)/FTP.oo			\
			$(LIBVOS_BLD_D)/FTPD_cmd.oo		\
			$(LIBVOS_BLD_D)/FTPD_client.oo		\
			$(LIBVOS_BLD_D)/FTPD_user.oo		\
			$(LIBVOS_BLD_D)/FTPD.oo

#
# with OCI library
#
ifdef ORACLE_HOME
TARGET_OBJS	+=	$(LIBVOS_BLD_D)/OCIValue.oo	\
			$(LIBVOS_BLD_D)/OCI.oo

CXXFLAGS_ADD	+=	-I$(ORACLE_HOME)/include -I$(ORACLE_HOME)/rdbms/public
LDFLAGS_ADD	+=	-L$(ORACLE_HOME)/lib -lclntsh
endif

# link needed for FTP and OCI on Solaris system.
ifeq ($(SYS),SunOS)
LDFLAGS_ADD	+= -lsocket -lnsl
endif


.PHONY: all libvos-all libvos-all-32 libvos-all-64 libvos-debug libvos-clean

libvos-all: CXXFLAGS+=$(CXXFLAGS_ADD)
libvos-all: LDFLAGS+=$(LDFLAGS_ADD)
libvos-all: $$(PRE_TARGET) $$(TARGET_OBJS) $$(TARGET)

libvos-all-32: CXXFLAGS_ADD+=-m32
libvos-all-32: libvos-all

libvos-all-64: CXXFLAGS_ADD+=-m64
libvos-all-64: libvos-all

libvos-debug: CXXFLAGS=$(CXXFLAGS_DEBUG) $(CXXFLAGS_ADD)
libvos-debug: LDFLAGS+=$(LDFLAGS_ADD)
libvos-debug: $$(PRE_TARGET) $$(TARGET_OBJS) $$(TARGET)

libvos-debug-32: CXXFLAGS_ADD+=$(CXXFLAGS_DEBUG) -m32
libvos-debug-32: libvos-debug

libvos-debug-64: CXXFLAGS_ADD+=$(CXXFLAGS_DEBUG) -m64
libvos-debug-64: libvos-debug

libvos-clean:
	@$(call do_rmdir,$(LIBVOS_BLD_D))

$(LIBVOS_BLD_D):
	@$(call do_mkdir,$@)

$(LIBVOS_BLD_D)/Buffer.oo	: $(LIBVOS_BLD_D)/libvos.oo

$(LIBVOS_BLD_D)/FTPClient.oo	\
$(LIBVOS_BLD_D)/Dir.oo		\
$(LIBVOS_BLD_D)/DirNode.oo	\
$(LIBVOS_BLD_D)/Record.oo	\
$(LIBVOS_BLD_D)/DNS_rr.oo	\
$(LIBVOS_BLD_D)/ConfigData.oo	\
$(LIBVOS_BLD_D)/File.oo		: $(LIBVOS_BLD_D)/Buffer.oo

$(LIBVOS_BLD_D)/Dir.oo		: $(LIBVOS_BLD_D)/DirNode.oo

$(LIBVOS_BLD_D)/Config.oo	: $(LIBVOS_BLD_D)/ConfigData.oo

$(LIBVOS_BLD_D)/SockAddr.oo	: $(LIBVOS_BLD_D)/Buffer.oo

$(LIBVOS_BLD_D)/RecordMD.oo	\
$(LIBVOS_BLD_D)/Config.oo	\
$(LIBVOS_BLD_D)/Dlogger.oo	\
$(LIBVOS_BLD_D)/Socket.oo	: $(LIBVOS_BLD_D)/File.oo

$(LIBVOS_BLD_D)/Socket.oo	: $(LIBVOS_BLD_D)/SockAddr.oo

$(LIBVOS_BLD_D)/SockServer.oo	: $(LIBVOS_BLD_D)/Socket.oo

$(LIBVOS_BLD_D)/DNSQuery.oo	: $(LIBVOS_BLD_D)/DNS_rr.oo

$(LIBVOS_BLD_D)/FTPCmd.oo	\
$(LIBVOS_BLD_D)/FTPD.oo		\
$(LIBVOS_BLD_D)/FTP.oo		\
$(LIBVOS_BLD_D)/Resolver.oo	: $(LIBVOS_BLD_D)/SockServer.oo

$(LIBVOS_BLD_D)/Reader.oo	\
$(LIBVOS_BLD_D)/Writer.oo	: $(LIBVOS_BLD_D)/RecordMD.oo

$(LIBVOS_BLD_D)/Reader.oo	\
$(LIBVOS_BLD_D)/Writer.oo	: $(LIBVOS_BLD_D)/Record.oo

$(LIBVOS_BLD_D)/FTPUser.oo	: $(LIBVOS_BLD_D)/Dir.oo

$(LIBVOS_BLD_D)/%.oo: $(LIBVOS_SRC_D)/%.cpp $(LIBVOS_SRC_D)/%.hpp
	@$(do_compile)
