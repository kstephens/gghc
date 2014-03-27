ROOT=.
include $(ROOT)/mk/common.mk

SUBDIRS = src/*
include $(ROOT)/mk/target.mk

##############################

TEST_INPUTS := \
  t/test.c \
  $(shell ls src/*/*.h) \
  stdlib.h \
  stdio.h

PROG=bin/gghc

test-local :
	$(PROG) $(CC) -debug -v -g t/test.c
	$(PROG) gcc   -debug -v -g t/test.c
	$(PROG) clang -debug -v -g t/test.c
	rm -rf /tmp/gghc-*-*.*

test-deep : all test
	@set -ex; for f in $(TEST_INPUTS) ;\
	do
	  $(PROG) $(CC) $$f ;\
	done

debug : all
	lldb -f $(PROG) -- $(CC) -v -g t/test.c

