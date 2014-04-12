ROOT=.
include $(ROOT)/mk/common.mk

SUBDIRS = src/ggrt src/gghc src/gghc-bin
include $(ROOT)/mk/target.mk

##############################

TEST_INPUTS := \
  t/test.c \
  $(shell ls src/*/*.h) \
  stdlib.h \
  stdio.h

GARBAGE += bin/* lib/*
PROG=bin/gghc

test-local-2 : test-local
	$(PROG) --debug --yydebug --yydebug -v -o gen/test.c.gghc.ss       -- $(CC) -- t/test.c
	$(PROG) --debug -v -o gen/test.c.gghc.gcc.ss   -- gcc   -- t/test.c
	$(PROG) --debug -v -o gen/test.c.gghc.clang.ss -- clang -- t/test.c
	rm -rf /tmp/gghc-*-*.*

TEST_IN := $(shell ls t/test-*.in)

test-local:
	tool/run-test '$(PROG) --cpp-output $$i.i -- $(CC) --' $(TEST_IN)
	tool/run-test '$(PROG) -- gcc   --' $(TEST_IN)
	tool/run-test '$(PROG) -- clang --' $(TEST_IN)

test-deep : all test
	@set -ex; for f in $(TEST_INPUTS) ;\
	do
	  $(PROG) $(CC) $$f ;\
	done

debug : all
	lldb -f $(PROG) -- --yydebug --yydebug -- $(CC) -v -g -- t/test.c

