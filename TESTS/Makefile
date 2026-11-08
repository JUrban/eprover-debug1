# Unit tests for EPROVER BASICS data structures.
# Build: make  ; Run: make check

CC = gcc
CFLAGS = -O2 -g -I ../BASICS -I ../include
LIBS = -lm

BASICS_SRCS = ../BASICS/clb_pstacks.c ../BASICS/clb_memory.c \
              ../BASICS/clb_error.c ../BASICS/clb_verbose.c \
              ../BASICS/clb_simple_stuff.c ../BASICS/clb_min_heap.c

TESTS = test_pstack_binsearch

all: $(TESTS)

%: %.c $(BASICS_SRCS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

check: all
	./test_pstack_binsearch

clean:
	rm -f $(TESTS)

.PHONY: all check clean
