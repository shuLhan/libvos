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

OBJECT_OBJS=	$(LIBVOS_BLD_D)/libvos.oo	\
		$(LIBVOS_BLD_D)/Object.oo

BNODE_OBJS=	$(OBJECT_OBJS)			\
		$(LIBVOS_BLD_D)/BNode.oo

BUFFER_OBJS=	$(BNODE_OBJS)			\
		$(LIBVOS_BLD_D)/List.oo		\
		$(LIBVOS_BLD_D)/Buffer.oo


DLOGGER_OBJS=	$(BUFFER_OBJS)			\
		$(LIBVOS_BLD_D)/File.oo		\
		$(LIBVOS_BLD_D)/Dlogger.oo

SSVREADER_OBJS=		$(BUFFER_OBJS)			\
			$(LIBVOS_BLD_D)/File.oo		\
			$(LIBVOS_BLD_D)/Record.oo	\
			$(LIBVOS_BLD_D)/SSVReader.oo

DNSQUERY_OBJS=		$(SSVREADER_OBJS)		\
			$(LIBVOS_BLD_D)/DNS_rr.oo	\
			$(LIBVOS_BLD_D)/DNSQuery.oo


TARGET=	$(BLD_D)/Object.test		\
	$(BLD_D)/Buffer.test		\
	$(BLD_D)/BNode.test		\
	$(BLD_D)/List.test		\
	$(BLD_D)/Dlogger.test		\
	$(BLD_D)/SSVReader.test		\
	$(BLD_D)/host_to_dnsquery.test

.PHONY: all clean

all: libvos-debug
	@for test in ${TARGET}; do	\
		echo "[T] $$test";	\
		$$test || (echo "[T] FAIL: $$test" && exit 1); \
	done

clean: libvos-clean
	@$(call do_rm,$(TARGET))

$(BLD_D)/Object.test: Object.test.cc $(OBJECT_OBJS)
	@$(do_build)

$(BLD_D)/Buffer.test: Buffer.test.cc $(BUFFER_OBJS)
	@$(do_build)

$(BLD_D)/BNode.test: BNode.test.cc $(BNODE_OBJS)
	@$(do_build)

$(BLD_D)/List.test: List.test.cc $(BUFFER_OBJS)
	@$(do_build)

$(BLD_D)/Dlogger.test: Dlogger.test.cc $(DLOGGER_OBJS)
	@$(do_build)

$(BLD_D)/SSVReader.test: SSVReader.test.cc $(SSVREADER_OBJS)
	@$(do_build)

$(BLD_D)/host_to_dnsquery.test: host_to_dnsquery.test.cc $(DNSQUERY_OBJS)
	@$(do_build)

## vi: ts=8 sw=8 tw=78:
