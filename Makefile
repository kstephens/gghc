
all:

YACC = bison
LEX = flex
MV = mv
GGHC_LIB_DIR=`pwd`/
CFLAGS = $(OTHER_CFLAGS) -Wall -g '-DGGHC_LIB_DIR="'$(GGHC_LIB_DIR)'"' -DYYDEBUG # -DLEXDEBUG2 -DLEXDEBUG 
YACCFLAGS = # -v
LEXFLAGS = -f

PRODUCT = ./gghc

YFILES = cy.y
LFILES = cl.l

CFILES = \
  malloc_debug.c \
  mm_buf.c \
  gghc.c \
  gghc_sym.c \
  gghc_o.c

LIBS = # -lMallocDebug

###################################################################

.SUFFIXES: .y .l .c .h .o

.c.o :
	$(CC) -I. $(CFLAGS) -c $*.c -o $*.o
.y.c .y.h :
	@rm -f $*.c $*.h $*.y.dot $*.y.dot.svg
	$(YACC) $(YACCFLAGS) --graph=$*.y.dot -d $*.y && $(MV) $*.tab.c $*.c && $(MV) $*.tab.h $*.h
	tool/yy_action $*.c
.l.c :
	$(LEX) -t $*.l | sed 's?debug = 0;?debug = 1;?' > $*.c

$(YFILES:.l=.o) :: $(YFILES)

DERIVED_CFILES = $(YFILES:.y=.c) $(LFILES:.l=.c)
DERIVED_HFILES = $(YFILES:.y=.h)
OFILES = $(CFILES:.c=.o) $(DERIVED_CFILES:.c=.o)
DERIVED_FILES = $(DERIVED_CFILES) $(DERIVED_HFILES) $(OFILES) $(PRODUCT)

$(PRODUCT) : $(DERIVED_HFILES) $(DERIVED_CFILES) $(OFILES)
	$(CC) -o $(PRODUCT) $(OFILES) $(LIBS)

ggraph : cy.y.dot.svg

cy.y.dot.svg : all
	dot -Tsvg -o cy.y.dot.svg cy.y.dot
	open cy.y.dot.svg

clean :
	rm -f $(DERIVED_FILES) cy.output gdbinit *.dot *.dot.svg

all : $(OFILE_DIR) $(PRODUCT)

test : all
	$(PRODUCT) -debug -v -g test.c

test-deep : all test
	$(PRODUCT) stdlib.h
	$(PRODUCT) stdio.h
	$(PRODUCT) mm_buf.h
	$(PRODUCT) malloc_debug.h
	$(PRODUCT) gghc.h
	$(PRODUCT) gghc_o.h
	$(PRODUCT) gghc_t.h
	$(PRODUCT) gghc_sym.h

debug : all
	lldb -f $(PRODUCT) -- -v -g test.c

$(GGHC_OFILES) : $(PRODUCT)

