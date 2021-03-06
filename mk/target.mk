
CINCS += -Igen
CINCS += -Iboot
CINCS += -I$(ROOT)/src
CLIBS += -L$(ROOT)/lib

ifeq "$(UNAME_S)" "Darwin"
CINCS += -I/opt/local/include
CINCS := $(CINCS) -I$(shell ls -d /opt/local/lib/libffi-3.*/include | sort | tail -1)
#CINCS += -fmacro-backtrace-limit=0
CLIBS += -L/opt/local/lib
CLIBS += -L/opt/local/lib/x86_64
endif
ifeq "$(UNAME_S)" "Linux"
CINCS += -I/usr/include/x86_64-linux-gnu
CLIBS += -L/usr/lib/x86_64-linux-gnu
endif

ROOT_LIBS := $(shell ls $(ROOT)/lib/lib*.a 2>/dev/null)

all : show-uname all-subdirs $(PRODUCTS) $(TARGETS)

v :
	@echo v="'"'$($(v))'"'"

show-uname :
	@echo "  UNAME_S=$(UNAME_S)"

code-stats :
	find * -type f | sort | egrep -e '(Makefile|\.[chyl])$$' | egrep -v -e 'gen/|lab/' | sort | xargs wc -l

.SUFFIXES: .y .l

gen/cy.c : cy.y
#	@rm -f $*.c $*.h $*.y.dot $*.y.dot.svg
	$(YACC) $(YACCFLAGS) --graph=gen/cy.y.dot --xml=gen/cy.y.xml --output=gen/cy.c --defines=gen/cy.h cy.y
	tool/yy_action $@

gen/cy.h : gen/cy.c

gen/cl.c : cl.l
	$(LEX) --align --reentrant --prefix=gghc_yy -t cl.l | sed 's?debug = 0;?debug = 1;?' > $@

$(YFILES:.y=.o) :: $(YFILES)
$(YFILES:.l=.o) :: $(LFILES)

$(LIB) : $(GEN_HFILES) $(GEN_CFILES) $(OFILES)
	ar -r $(LIB) $(OFILES)
	cp -p $(LIB) $(ROOT)/lib

$(BIN) : $(GEN_HFILES) $(GEN_CFILES) $(OFILES) $(ROOT_LIBS)
	$(CC) $(CFLAGS) -o $(BIN) $(OFILES) $(LDFLAGS) $(LDLIBS)
	cp -p $(BIN) $(ROOT)/bin

$(TFILES) : $(ROOT_LIBS)

all-subdirs :
	@set -e; for d in $(SUBDIRS) ;\
	do  \
	  $(MAKE) -C "$$d" all test ;\
	done

ggraph : gen/cy.y.dot.svg

gen/cy.y.dot.svg : all
	dot -Tsvg -o gen/cy.y.dot.svg gen/cy.y.dot
	open gen/cy.y.dot.svg

clean : clean-subdirs
	rm -f  $(BIN) $(LIB) $(GEN_FILES) $(TFILES)
	rm -rf gen/* $(GARBAGE)

clean-subdirs :
	@set -e; for d in $(SUBDIRS) ;\
	do  \
	  $(MAKE) -C "$$d" clean ;\
	done

test : all test-subdirs test-local test-t

test-subdirs : all
	@set -e; for d in $(SUBDIRS) ;\
	do  \
	  $(MAKE) -C "$$d" test ;\
	done

test-local :

test-t : all $(TFILES)
	@for t in $(TFILES); \
	do \
	  echo "  Running $$t:" ;\
	  ./$$t ;\
	done

