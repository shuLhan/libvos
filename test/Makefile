SRC_D	=.
BLD_D	=.
LIBVOS_D=..

include $(LIBVOS_D)/Makefile

TARGET_OBJS=	$(LIBVOS_BLD_D)/libvos.oo

SSVREADER_OBJS=		$(LIBVOS_BLD_D)/SSVReader.oo	\
			$(LIBVOS_BLD_D)/Record.oo	\
			$(LIBVOS_BLD_D)/File.oo		\
			$(LIBVOS_BLD_D)/Buffer.oo	\
			$(LIBVOS_BLD_D)/libvos.oo

TARGET=$(BLD_D)/SSVReader.test

.PHONY: all debug clean

all: libvos-all

debug: libvos-debug

clean: libvos-clean
	@$(call do_rm,$(TARGET))

$(BLD_D)/%.test: %.test.cpp $(SSVREADER_OBJS)
	@$(do_build)
