UNAME_S:=$(shell uname -s 2>/dev/null)#
CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
CPPFLAGS += $(CINCS)
CFLAGS += $(CPPFLAGS)
LDLIBS += $(CLIBS)
CPP = $(CC) $(CPPFLAGS) -E

CINCS += -I..

ifeq "$(UNAME_S)" "Darwin"
CINCS += -I/opt/local/include
CINCS += -I/opt/local/lib/libffi-3.0.13/include
CINCS += -fmacro-backtrace-limit=0
CLIBS += -L/opt/local/lib
CLIBS += -L/opt/local/lib/x86_64
endif
ifeq "$(UNAME_S)" "Linux"
CINCS += -I/usr/include/x86_64-linux-gnu
CLIBS += -L/usr/lib/x86_64-linux-gnu
endif

CLIBS += $(LIB)
CLIBS += -lffi
CLIBS += -lm

CFILES := $(shell ls *.c)
HFILES := $(shell ls *.h)
OFILES = $(CFILES:%.c=%.o)

LIB = libggrt.a

TCFILES := $(shell ls t/*.t.c)
TFILES = $(TCFILES:%.c=%)

all : show-uname $(LIB) $(TFILES)

$(LIB) : $(OFILES)
	ar -r $@ $(OFILES)
	cp -p $@ ../../lib

show-uname :
	@echo "  UNAME_S=$(UNAME_S)"

clean :
	rm -f $(LIB) $(OFILES) $(TFILES)

test : all $(TFILES)
	@for t in $(TFILES); \
	do \
	  echo "  Running $$t:" ;\
	  ./$$t ;\
	done

