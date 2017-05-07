##
## Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
## Use of this source code is governed by a BSD-style license that can be
## found in the LICENSE file.
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
			$(LIBVOS_BLD_D)/Object.oo		\
			$(LIBVOS_BLD_D)/Locker.oo		\
			$(LIBVOS_BLD_D)/BNode.oo		\
			$(LIBVOS_BLD_D)/Buffer.oo		\
			$(LIBVOS_BLD_D)/List.oo			\
			$(LIBVOS_BLD_D)/ListBuffer.oo		\
			$(LIBVOS_BLD_D)/File.oo			\
			$(LIBVOS_BLD_D)/Dlogger.oo		\
			$(LIBVOS_BLD_D)/Config.oo		\
			$(LIBVOS_BLD_D)/ConfigData.oo		\
			$(LIBVOS_BLD_D)/DSVRecordMD.oo		\
			$(LIBVOS_BLD_D)/DSVRecord.oo		\
			$(LIBVOS_BLD_D)/DSVReader.oo		\
			$(LIBVOS_BLD_D)/DSVWriter.oo		\
			$(LIBVOS_BLD_D)/Dir.oo			\
			$(LIBVOS_BLD_D)/DirNode.oo		\
			$(LIBVOS_BLD_D)/SockAddr.oo		\
			$(LIBVOS_BLD_D)/Socket.oo		\
			$(LIBVOS_BLD_D)/SockServer.oo		\
			$(LIBVOS_BLD_D)/DNSRecordType.oo	\
			$(LIBVOS_BLD_D)/DNS_rr.oo		\
			$(LIBVOS_BLD_D)/DNSQuery.oo		\
			$(LIBVOS_BLD_D)/Resolver.oo		\
			$(LIBVOS_BLD_D)/FTP_cmd.oo		\
			$(LIBVOS_BLD_D)/FTP.oo			\
			$(LIBVOS_BLD_D)/FTPD_cmd.oo		\
			$(LIBVOS_BLD_D)/FTPD_client.oo		\
			$(LIBVOS_BLD_D)/FTPD_user.oo		\
			$(LIBVOS_BLD_D)/FTPD.oo			\
			$(LIBVOS_BLD_D)/Rowset.oo		\
			$(LIBVOS_BLD_D)/SSVReader.oo		\
			$(LIBVOS_BLD_D)/TreeNode.oo		\
			$(LIBVOS_BLD_D)/RBT.oo

#
# with OCI library
#
ifdef ORACLE_HOME
TARGET_OBJS	+=	$(LIBVOS_BLD_D)/OCIValue.oo	\
			$(LIBVOS_BLD_D)/OCI.oo

CXXFLAGS_ADD	+=	-I$(ORACLE_HOME)/include -I$(ORACLE_HOME)/rdbms/public
LDFLAGS_ADD	+=	-L$(ORACLE_HOME)/lib -lclntsh

$(LIBVOS_BLD_D)/OCI.oo: $(LIBVOS_BLD_D)/Locker.oo
endif

#
# library needed for FTP and OCI module on Solaris system.
#
ifeq ($(SYS),SunOS)
LDFLAGS_ADD	+= -lsocket -lnsl
endif

all:

.PHONY: libvos
.PHONY: libvos-opts EMPTY_OPTS NO_DEFAULT_LIBS
.PHONY: libvos-all libvos-all-32 libvos-all-64
.PHONY: libvos-debug libvos-debug-32 libvos-debug-64
.PHONY: cppcheck clangcheck check

libvos: $$(PRE_TARGET) $$(TARGET_OBJS) $$(TARGET)

libvos-opts: LIBVOS_OPTS+=EMPTY_OPTS
libvos-opts: $$(LIBVOS_OPTS)

EMPTY_OPTS: libvos

NO_DEFAULT_LIBS: CXXFLAGS_ADD+=$(NO_DEFAULT_LIBS)
NO_DEFAULT_LIBS: LINKER=$(LIBVOS_LINKER)
NO_DEFAULT_LIBS: libvos

libvos-all: CXXFLAGS+=$(CXXFLAGS_ADD)
libvos-all: LDFLAGS+=$(LDFLAGS_ADD)
libvos-all: libvos-opts

libvos-all-32: CXXFLAGS_ADD+=-m32
libvos-all-32: libvos-all

libvos-all-64: CXXFLAGS_ADD+=-m64
libvos-all-64: libvos-all

libvos-debug: CXXFLAGS=$(CXXFLAGS_DEBUG) $(CXXFLAGS_ADD)
libvos-debug: LDFLAGS+=$(LDFLAGS_ADD)
libvos-debug: libvos-opts

libvos-debug-32: CXXFLAGS_ADD+=$(CXXFLAGS_DEBUG) -m32
libvos-debug-32: libvos-debug

libvos-debug-64: CXXFLAGS_ADD+=$(CXXFLAGS_DEBUG) -m64
libvos-debug-64: libvos-debug

libvos-clean:
	@$(call do_rmdir,$(LIBVOS_BLD_D))

$(LIBVOS_BLD_D):
	@$(call do_mkdir,$@)

$(LIBVOS_BLD_D)/Object.oo	: $(LIBVOS_BLD_D)/libvos.oo

$(LIBVOS_BLD_D)/DNSRecordType.oo\
$(LIBVOS_BLD_D)/Locker.oo	\
$(LIBVOS_BLD_D)/BNode.oo	\
$(LIBVOS_BLD_D)/Buffer.oo	: $(LIBVOS_BLD_D)/Object.oo

$(LIBVOS_BLD_D)/TreeNode.oo	\
$(LIBVOS_BLD_D)/List.oo		: $(LIBVOS_BLD_D)/BNode.oo

$(LIBVOS_BLD_D)/ListBuffer.oo	\
$(LIBVOS_BLD_D)/Rowset.oo	: $(LIBVOS_BLD_D)/List.oo

$(LIBVOS_BLD_D)/ListBuffer.oo	\
$(LIBVOS_BLD_D)/SockAddr.oo	\
$(LIBVOS_BLD_D)/List.oo		\
$(LIBVOS_BLD_D)/FTPClient.oo	\
$(LIBVOS_BLD_D)/Dir.oo		\
$(LIBVOS_BLD_D)/DirNode.oo	\
$(LIBVOS_BLD_D)/DSVRecord.oo	\
$(LIBVOS_BLD_D)/DNS_rr.oo	\
$(LIBVOS_BLD_D)/ConfigData.oo	\
$(LIBVOS_BLD_D)/File.oo		: $(LIBVOS_BLD_D)/Buffer.oo

$(LIBVOS_BLD_D)/Dir.oo		: $(LIBVOS_BLD_D)/DirNode.oo

$(LIBVOS_BLD_D)/Config.oo	: $(LIBVOS_BLD_D)/ConfigData.oo

$(LIBVOS_BLD_D)/SSVReader.oo	\
$(LIBVOS_BLD_D)/DSVRecordMD.oo	\
$(LIBVOS_BLD_D)/Config.oo	\
$(LIBVOS_BLD_D)/Dlogger.oo	\
$(LIBVOS_BLD_D)/Socket.oo	: $(LIBVOS_BLD_D)/File.oo

$(LIBVOS_BLD_D)/RBT.oo		: $(LIBVOS_BLD_D)/TreeNode.oo

$(LIBVOS_BLD_D)/RBT.oo		\
$(LIBVOS_BLD_D)/List.oo		\
$(LIBVOS_BLD_D)/Dlogger.oo	\
$(LIBVOS_BLD_D)/SockServer.oo	: $(LIBVOS_BLD_D)/Locker.oo

$(LIBVOS_BLD_D)/Socket.oo	: $(LIBVOS_BLD_D)/SockAddr.oo

$(LIBVOS_BLD_D)/SockServer.oo	: $(LIBVOS_BLD_D)/Socket.oo

$(LIBVOS_BLD_D)/DNS_rr.oo	: $(LIBVOS_BLD_D)/DNSRecordType.oo

$(LIBVOS_BLD_D)/DNSQuery.oo	: $(LIBVOS_BLD_D)/DNS_rr.oo

$(LIBVOS_BLD_D)/FTPCmd.oo	\
$(LIBVOS_BLD_D)/FTPD.oo		\
$(LIBVOS_BLD_D)/FTP.oo		\
$(LIBVOS_BLD_D)/Resolver.oo	: $(LIBVOS_BLD_D)/SockServer.oo

$(LIBVOS_BLD_D)/SSVReader.oo	\
$(LIBVOS_BLD_D)/Resolver.oo	: $(LIBVOS_BLD_D)/ListBuffer.oo

$(LIBVOS_BLD_D)/DSVReader.oo	\
$(LIBVOS_BLD_D)/DSVWriter.oo	: $(LIBVOS_BLD_D)/DSVRecordMD.oo

$(LIBVOS_BLD_D)/SSVReader.oo	\
$(LIBVOS_BLD_D)/DSVReader.oo	\
$(LIBVOS_BLD_D)/DSVWriter.oo	: $(LIBVOS_BLD_D)/DSVRecord.oo

$(LIBVOS_BLD_D)/FTPUser.oo	: $(LIBVOS_BLD_D)/Dir.oo

$(LIBVOS_BLD_D)/%.oo: $(LIBVOS_SRC_D)/%.cc $(LIBVOS_SRC_D)/%.hh
	@$(do_compile)

cppcheck:
	@cppcheck --enable=all \
		--suppressions-list=$(LIBVOS_SRC_D)/libvos.cppcheck.suppress \
		$(LIBVOS_SRC_D)

clangcheck:
	@find $(LIBVOS_SRC_D) -name '*.cc' | xargs clang-check

check: cppcheck clangcheck

# vi: ts=8 sw=8 tw=78:
