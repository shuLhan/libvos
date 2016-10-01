##
## Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
## Use of this source code is governed by a BSD-style license that can be found
## in the LICENSE file.
##

SRC_D	=.
BLD_D	=.
LIBVOS_D=..

include $(LIBVOS_D)/Makefile

LIBVOS_OPTS     +=NO_DEFAULT_LIBS

Object_OBJS=	$(LIBVOS_BLD_D)/libvos.oo	\
		$(LIBVOS_BLD_D)/Object.oo

COMMON_OBJS=	$(Object_OBJS)			\
		$(LIBVOS_BLD_D)/Buffer.oo

BNode_OBJS=	$(COMMON_OBJS)			\
		$(LIBVOS_BLD_D)/BNode.oo	\
		$(LIBVOS_BLD_D)/List.oo		\
		$(LIBVOS_BLD_D)/Locker.oo

Buffer_OBJS=	$(BNode_OBJS)

List_OBJS=	$(Buffer_OBJS)

Dlogger_OBJS=	$(Buffer_OBJS)			\
		$(LIBVOS_BLD_D)/Locker.oo	\
		$(LIBVOS_BLD_D)/File.oo		\
		$(LIBVOS_BLD_D)/Dlogger.oo

SSVReader_OBJS=		$(Buffer_OBJS)			\
			$(LIBVOS_BLD_D)/File.oo		\
			$(LIBVOS_BLD_D)/Record.oo	\
			$(LIBVOS_BLD_D)/Rowset.oo	\
			$(LIBVOS_BLD_D)/SSVReader.oo


SockAddr_OBJS=	$(Buffer_OBJS)			\
		$(LIBVOS_BLD_D)/SockAddr.oo

DNSQuery_OBJS=		$(SSVReader_OBJS)		\
			$(LIBVOS_BLD_D)/DNS_rr.oo	\
			$(LIBVOS_BLD_D)/DNSQuery.oo

host_to_dnsquery_OBJS=	$(SSVReader_OBJS)	\
			$(DNSQuery_OBJS)

Rowset_OBJS=	$(List_OBJS)			\
		$(LIBVOS_BLD_D)/Rowset.oo

Resolver_OBJS=	\
		$(List_OBJS)			\
		$(DNSQuery_OBJS)		\
		$(SockAddr_OBJS)		\
		$(LIBVOS_BLD_D)/Socket.oo	\
		$(LIBVOS_BLD_D)/Resolver.oo

Locker_OBJS=	$(Object_OBJS)			\
		$(BLD_D)/TestLocker.oo		\
		$(LIBVOS_BLD_D)/Locker.oo

TARGET=	$(BLD_D)/Object.test		\
	$(BLD_D)/Buffer.test		\
	$(BLD_D)/BNode.test		\
	$(BLD_D)/List.test		\
	$(BLD_D)/Dlogger.test		\
	$(BLD_D)/SSVReader.test		\
	$(BLD_D)/SockAddr.test		\
	$(BLD_D)/host_to_dnsquery.test	\
	$(BLD_D)/Resolver.test		\
	$(BLD_D)/Rowset.test		\
	$(BLD_D)/Locker.test

.PHONY: all clean

all: libvos-debug
	@for test in ${TARGET}; do	\
		echo "[T] $$test";	\
		$$test || (echo "[T] FAIL: $$test" && exit 1); \
	done

clean: libvos-clean
	@$(call do_rm,$(TARGET))

$(BLD_D)/%.oo: $(SRC_D)/%.cc $(SRC_D)/%.hh
	@$(do_compile)

$(BLD_D)/%.test: %.test.cc $$(%_OBJS)
	@$(do_build)

## vi: ts=8 sw=8 tw=78:
