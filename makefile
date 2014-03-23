
all:

YACC = bison
LEX = flex
KSHC_LIB_DIR=`pwd`/
CFLAGS = $(OTHER_CFLAGS) -Wall -g '-DKSHC_LIB_DIR="'$(KSHC_LIB_DIR)'"' -DYYDEBUG # -DLEXDEBUG2 -DLEXDEBUG 
YACCFLAGS = # -v
LEXFLAGS = -f

PRODUCT = kshc

YFILES = cy.y
LFILES = cl.l

CFILES = \
  kshc.c \
  kshc_sym.c \
  kshc_o.c

LIBS = # -lMallocDebug

###################################################################


.SUFFIXES: .y .l .c .cc .h .o

.c.o :
	$(CC) -I. -I$(OFILE_DIR) $(CFLAGS) -c $*.c -o $(OFILE_DIR)/$*.o
.cc.o :
	$(CC) -I. -I$(OFILE_DIR) $(CFLAGS) -ObjC++ -c $*.cc -o $(OFILE_DIR)/$*.o
.h.o :
	$(PRODUCT) -C++ $< -o $(OFILE_DIR)/$*.o
.y.h :
	$(YACC) $(YACCFLAGS) -d $*.y && $(MV) $*.tab.c $(OFILE_DIR)/$*.c && $(MV) $*.tab.h $(OFILE_DIR)/$*.h	
.y.c :
	$(YACC) $(YACCFLAGS) -d $*.y && $(MV) $*.tab.c $(OFILE_DIR)/$*.c && $(MV) $*.tab.h $(OFILE_DIR)/$*.h
.l.c :
	$(LEX) -t $*.l | sed 's?debug = 0;?debug = 1;?' > $(OFILE_DIR)/$*.c  


OFILE_DIR=obj
VPATH=$(OFILE_DIR)

$(YFILES:.l=.o) :: $(YFILES)

DERIVED_CFILES = $(YFILES:.y=.c) $(LFILES:.l=.c)
DERIVED_HFILES = $(YFILES:.y=.h)

OFILES = $(CFILES:.c=.o)

$(OFILE_DIR) :
	mkdir -p $(OFILE_DIR)

$(PRODUCT) : $(DERIVED_HFILES) $(OFILES) $(DERIVED_CFILES) $(DERIVED_CFILES:.c=.o) 
	$(CC) -o $(PRODUCT) $(OFILES) $(DERIVED_CFILES:.c=.o) $(LIBS)

clean :
	rm -rf $(OFILE_DIR) $(PRODUCT)


USRINCLUDE=/usr/include/ansi
KSHC_OFILES = \
  stdlib.o \
  stdio.o

all : $(OFILE_DIR) $(PRODUCT) kshc_i.o $(KSHC_OFILES)

debug : all
	@(echo "directory $(OFILE_DIR)" ;\
	echo "view" ;\
	echo "set args -I.. test.c"; \
	echo "run" ;\
	) > gdbinit
	gdb $(PRODUCT) -x gdbinit

kshc_i.o : kshc_i.cc kshc_i.h
	$(CC) -I. -I$(OFILE_DIR) $(CFLAGS) -ObjC++ -c $*.cc -o $(OFILE_DIR)/$*.o

$(KSHC_OFILES) : $(PRODUCT)

stdlib.o: mod/stdlib.h
	$(PRODUCT) -v -C++ -Wall -g mod/stdlib.h -o $(OFILE_DIR)/stdlib.o

stdio.o: mod/stdio.h
	$(PRODUCT) -v -C++ -Wall -g mod/stdio.h -o $(OFILE_DIR)/stdio.o
