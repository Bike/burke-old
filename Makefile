# let's give this a shot
CC = gcc
CFLAGS = -Wall

# I = "independent", as in for all targets
ISOURCES = error.c lisp.c main.c package.c read.c write.c
IOBJECTS = $(ISOURCES:.c=.o)

EXECUTABLE = lisp

all: boehm

boehm: $(IOBJECTS) alloc_boehm.o layout_malloc.o
	$(CC) $(LDFLAGS) $(IOBJECTS) alloc_boehm.o layout_malloc.o -lgc -o $(EXECUTABLE)

malloc: $(IOBJECTS) alloc_malloc.o layout_malloc.o
	$(CC) $(LDFLAGS) $(IOBJECTS) alloc_malloc.o layout_malloc.o -o $(EXECUTABLE)

clean:
	rm lisp *.o
