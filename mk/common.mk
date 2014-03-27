UNAME_S:=$(shell uname -s 2>/dev/null)#
CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
CPPFLAGS += $(CINCS)
CFLAGS += $(CPPFLAGS)
LDLIBS += $(CLIBS)
CPP = $(CC) $(CPPFLAGS) -E

YACC = bison
LEX = flex
YACCFLAGS = # -v
LEXFLAGS = -f

CINCS += -I.
CLIBS += -L$(ROOT)/lib

CLIBS += $(LIB)

YFILES := $(shell ls *.y 2>/dev/null)
LFILES := $(shell ls *.l 2>/dev/null)
CFILES := $(shell ls *.c 2>/dev/null)
HFILES := $(shell ls *.h 2>/dev/null)

GEN_CFILES = $(YFILES:%.y=gen/%.c) $(LFILES:%.l=gen/%.c)
GEN_HFILES = $(YFILES:%.y=gen/%.h)
OFILES = $(CFILES:.c=.o) $(GEN_CFILES:.c=.o)
GEN_FILES = $(GEN_CFILES) $(GEN_HFILES) $(OFILES) $(PRODUCTS)

TCFILES := $(shell ls t/*.t.c 2>/dev/null)
TFILES = $(TCFILES:%.c=%)

PRODUCTS += $(LIB) $(BIN)

