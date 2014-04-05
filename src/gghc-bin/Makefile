ROOT=../..
include $(ROOT)/mk/common.mk

BIN = gghc
CLIBS += -lgghc -lggrt -lm # -lffi

include $(ROOT)/mk/target.mk

test-local: gghc
	tool/run-test "./gghc -- $(CC) --" t/test-*.in

