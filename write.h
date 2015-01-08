#ifndef BURKE_WRITE_H
#define BURKE_WRITE_H

#include "types.h"
#include <stdio.h>

/* this is for when i change types because i'm dumb */
#define DECWRITE(NAME) void write_##NAME(lispobj*,lispobj*);

void write_lisp(lispobj*,lispobj*);
DECWRITE(pair);
DECWRITE(fixnum);
DECWRITE(symbol);
DECWRITE(vector);
DECWRITE(fsubr);
DECWRITE(singleton);
DECWRITE(string);
DECWRITE(mtag);

#endif /* guard */
