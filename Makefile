# let's give this a shot
CC = gcc
CFLAGS = -Wall -fpic

# I = "independent", as in for all targets
ISOURCES = error.c lisp.c package.c read.c write.c
IOBJECTS = $(ISOURCES:.c=.o)

LIBRARY = liblisp.so

EXECUTABLE = repl

all: boehm repl

debug: CFLAGS += -g
debug: all

boehm: $(IOBJECTS) alloc_boehm.o layout_malloc.o
	$(CC) $(CFLAGS) -shared $(IOBJECTS) alloc_boehm.o layout_malloc.o -lgc -o $(LIBRARY) $(LDFLAGS)

malloc: $(IOBJECTS) alloc_malloc.o layout_malloc.o
	$(CC) $(CFLAGS) -shared $(IOBJECTS) alloc_malloc.o layout_malloc.o -o $(LIBRARY) $(LDFLAGS)

repl: repl.o
	$(CC) $(CFLAGS) -L. -o $(EXECUTABLE) repl.o -llisp

clean:
	rm -f $(EXECUTABLE) $(LIBRARY) *.o
