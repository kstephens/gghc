
all:

YACC = bison
LEX = flex
MV = mv
GGHC_LIB_DIR=`pwd`/
CFLAGS = $(OTHER_CFLAGS) -Wall -g '-DGGHC_LIB_DIR="'$(GGHC_LIB_DIR)'"' -DYYDEBUG # -DLEXDEBUG2 -DLEXDEBUG 
YACCFLAGS = # -v
LEXFLAGS = -f

PRODUCT = gghc

YFILES = cy.y
LFILES = cl.l

CFILES = \
  malloc_debug.c \
  gghc.c \
  gghc_sym.c \
  gghc_o.c

LIBS = # -lMallocDebug

###################################################################

.SUFFIXES: .y .l .c .cc .h .o

.c.o :
	$(CC) -I. $(CFLAGS) -c $*.c -o $*.o
.cc.o :
	$(CC) -I. $(CFLAGS) -ObjC++ -c $*.cc -o $*.o
.h.o :
	$(PRODUCT) -C++ $< -o $*.o
.y.h :
	$(YACC) $(YACCFLAGS) -d $*.y && $(MV) $*.tab.c $*.c && $(MV) $*.tab.h $*.h
.y.c :
	$(YACC) $(YACCFLAGS) -d $*.y && $(MV) $*.tab.c $*.c && $(MV) $*.tab.h $*.h
.l.c :
	$(LEX) -t $*.l | sed 's?debug = 0;?debug = 1;?' > $*.c

$(YFILES:.l=.o) :: $(YFILES)

DERIVED_CFILES = $(YFILES:.y=.c) $(LFILES:.l=.c)
DERIVED_HFILES = $(YFILES:.y=.h)
OFILES = $(CFILES:.c=.o) $(DERIVED_CFILES:.c=.o)
DERIVED_FILES = $(DERIVED_CFILES) $(DERIVED_HFILES) $(OFILES) $(PRODUCT)

$(PRODUCT) : $(DERIVED_HFILES) $(DERIVED_CFILES) $(OFILES)
	$(CC) -o $(PRODUCT) $(OFILES) $(LIBS)

clean :
	rm -f $(DERIVED_FILES)


USRINCLUDE=/usr/include/ansi
GGHC_OFILES = \
  stdlib.o \
  stdio.o

all : $(OFILE_DIR) $(PRODUCT)

debug : all
	@(\
	echo "view" ;\
	echo "set args -I.. test.c"; \
	echo "run" ;\
	) > gdbinit
	gdb $(PRODUCT) -x gdbinit

gghc_i.o : gghc_i.cc gghc_i.h
	$(CC) -I. $(CFLAGS) -c $*.cc -o $*.o

$(GGHC_OFILES) : $(PRODUCT)

stdlib.o: mod/stdlib.h
	./$(PRODUCT) -v -C++ -Wall -g mod/stdlib.h -o stdlib.o

stdio.o: mod/stdio.h
	./$(PRODUCT) -v -C++ -Wall -g mod/stdio.h -o stdio.o
