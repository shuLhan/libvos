SRC_D	=.
BLD_D	=.
LIBVOS_D=..

include $(LIBVOS_D)/Makefile

TARGET_OBJS=	$(LIBVOS_BLD_D)/libvos.oo

BUFFER_OBJS=	$(TARGET_OBJS)			\
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

TARGET=$(BLD_D)/SSVReader.test		\
       $(BLD_D)/host_to_dnsquery.test	\
       $(BLD_D)/Dlogger.test		\
       $(BLD_D)/Buffer.test

.PHONY: all debug clean

all: libvos-all

debug: libvos-debug

clean: libvos-clean
	@$(call do_rm,$(TARGET))

$(BLD_D)/Buffer.test: Buffer.test.cpp $(BUFFER_OBJS)
	@$(do_build)

$(BLD_D)/Dlogger.test: Dlogger.test.cpp $(DLOGGER_OBJS)
	@$(do_build)

$(BLD_D)/SSVReader.test: SSVReader.test.cpp $(SSVREADER_OBJS)
	@$(do_build)

$(BLD_D)/host_to_dnsquery.test: host_to_dnsquery.test.cpp $(DNSQUERY_OBJS)
	@$(do_build)
