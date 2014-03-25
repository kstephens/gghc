
all:

YACC = bison
LEX = flex
MV = mv
CFLAGS = $(OTHER_CFLAGS) -Wall -g -DYYDEBUG # -DLEXDEBUG2 -DLEXDEBUG 
YACCFLAGS = # -v
LEXFLAGS = -f

PRODUCT = ./gghc

YFILES = cy.y
LFILES = cl.l

CFILES = \
  malloc_debug.c \
  malloc_zone.c \
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

code-stats :
	find * -type f | sort | egrep -e '\.[chyl]$$' | egrep -v -e 'c[yl].[ch]' | xargs wc -l

clean :
	rm -f $(DERIVED_FILES) cy.output gdbinit *.dot *.dot.svg

all : $(OFILE_DIR) $(PRODUCT)

test : all
	$(PRODUCT) $(CC) -debug -v -g t/test.c
	$(PRODUCT) gcc   -debug -v -g t/test.c
	$(PRODUCT) clang -debug -v -g t/test.c
	$(RM) -rf /tmp/gghc_*_*.*

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
	lldb -f $(PRODUCT) -- -v -g t/test.c

$(GGHC_OFILES) : $(PRODUCT)

