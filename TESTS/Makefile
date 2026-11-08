# Unit tests for EPROVER BASICS data structures.
# Build: make  ;  Run: make check

CC = gcc
CFLAGS = -O2 -g -I ../BASICS -I ../include
LIBS = -lm

BASICS_SRCS = ../BASICS/clb_pstacks.c ../BASICS/clb_memory.c \
              ../BASICS/clb_error.c ../BASICS/clb_verbose.c \
              ../BASICS/clb_simple_stuff.c ../BASICS/clb_min_heap.c

TESTS := $(patsubst test_%.c,%,$(wildcard test_*.c))

all: $(TESTS)

%: test_%.c $(BASICS_SRCS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

check: all
	@ok=0; fail=0; \
	for t in $(TESTS); do \
	  echo "== $$t"; \
	  if ./$$t > /dev/null; then ok=$$((ok+1)); echo "   passed"; else fail=$$((fail+1)); echo "   FAILED"; fi; \
	done; \
	echo "$$ok passed, $$fail failed"; \
	test $$fail -eq 0

clean:
	rm -f $(TESTS)

.PHONY: all check clean
