UNAME_S:=$(shell uname -s 2>/dev/null)#
CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
CPPFLAGS += $(CINCS)
CFLAGS += $(CPPFLAGS)
LDLIBS += $(CLIBS)
CPP = $(CC) $(CPPFLAGS) -E

CPPFLAGS += -DYYDEBUG # -DLEXDEBUG2 -DLEXDEBUG 

YACC = bison
LEX = flex
YACCFLAGS = # -v
LEXFLAGS = -f

PRODUCT = ./gghc

YFILES := $(shell ls *.y)
LFILES := $(shell ls *.l)
CFILES := $(shell ls *.c)
HFILES := $(shell ls *.h)

CINCS += -I.
CINCS += -Igen
CINCS += src

CLIBS += -Llib
CLIBS += -lggrt

GEN_CFILES = $(YFILES:%.y=gen/%.c) $(LFILES:%.l=gen/%.c)
GEN_HFILES = $(YFILES:%.y=gen/%.h)
OFILES = $(CFILES:.c=.o) $(GEN_CFILES:.c=.o)
GEN_FILES = $(GEN_CFILES) $(GEN_HFILES) $(OFILES) $(PRODUCT)

all: src-libs $(PRODUCT)

src-libs :
	$(MAKE) -C src/ggrt all

###################################################################

.SUFFIXES: .y .l

gen/cy.c : cy.y
#	@rm -f $*.c $*.h $*.y.dot $*.y.dot.svg
	$(YACC) $(YACCFLAGS) --graph=$@.y.dot -d cy.y
	mv cy.tab.c $@
	mv cy.tab.h gen/cy.h
	mv cy.output gen/
	tool/yy_action $@

gen/cy.h : gen/cy.c

gen/cl.c : cl.l
	$(LEX) -t cl.l | sed 's?debug = 0;?debug = 1;?' > $@

$(YFILES:.y=.o) :: $(YFILES)
$(YFILES:.l=.o) :: $(LFILES)

$(PRODUCT) : $(GEN_HFILES) $(GEN_CFILES) $(OFILES)
	$(CC) -o $(PRODUCT) $(OFILES) $(LDFLAGS) $(LDLIBS)

ggraph : cy.y.dot.svg

cy.y.dot.svg : all
	dot -Tsvg -o gen/cy.y.dot.svg gen/cy.y.dot
	open gen/cy.y.dot.svg

code-stats :
	find * -type f | sort | egrep -e '\.[chyl]$$' | egrep -v -e 'c[yl].[ch]' | xargs wc -l

clean :
	rm -f $(GEN_FILES) gen/cy.output gdbinit *.dot *.dot.svg

all : $(PRODUCT)

TEST_INPUTS := \
  t/test.c \
  $(shell ls *.h) \
  stdlib.h \
  stdio.h

test : all
	$(PRODUCT) $(CC) -debug -v -g t/test.c
	$(PRODUCT) gcc   -debug -v -g t/test.c
	$(PRODUCT) clang -debug -v -g t/test.c
	$(RM) -rf /tmp/gghc-*-*.*

test-deep : all test
	$(PRODUCT) $(CC) stdlib.h
	$(PRODUCT) $(CC) stdio.h
	$(PRODUCT) $(CC) mm_buf.h
	$(PRODUCT) $(CC) malloc_debug.h
	$(PRODUCT) $(CC) gghc.h
	$(PRODUCT) $(CC) gghc_o.h
	$(PRODUCT) $(CC) gghc_t.h
	$(PRODUCT) $(CC) gghc_sym.h

debug : all
	lldb -f $(PRODUCT) -- $(CC) -v -g t/test.c

