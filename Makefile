# let's give this a shot
CC = gcc
CFLAGS = -std=c99 -Wall -Wpedantic -fpic

# I = "independent", as in for all targets
ITYPESRC = vector.c string.c port.c mtag.c nenv.c singleton.c applicative.c fexpr.c fsubr.c package.c wrapped.c fixnum.c amap.c pair.c smallenv.c symbol.c
ITYPEOBJ = $(ITYPESRC:.c=.o)
ISOURCES = error.c lisp.c read.c write.c
IOBJECTS = $(ISOURCES:.c=.o)

LIBRARY = liblisp.so

EXECUTABLE = repl

all: boehm repl

debug: CFLAGS += -g
debug: all

boehm: CFLAGS += -D LALLOC_BOEHM
boehm: $(IOBJECTS) $(ITYPEOBJ) alloc_boehm.o
	$(CC) $(CFLAGS) -shared $(IOBJECTS) $(ITYPEOBJ) alloc_boehm.o -lgc -o $(LIBRARY) $(LDFLAGS)

malloc: CFLAGS += -D LALLOC_MALLOC
malloc: $(IOBJECTS) $(ITYPEOBJ) alloc_malloc.o
	$(CC) $(CFLAGS) -shared $(IOBJECTS) $(ITYPEOBJ) alloc_malloc.o -o $(LIBRARY) $(LDFLAGS)

repl: repl.o
	$(CC) $(CFLAGS) -L. -o $(EXECUTABLE) repl.o -llisp

clean:
	rm -f $(EXECUTABLE) $(LIBRARY) *.o
