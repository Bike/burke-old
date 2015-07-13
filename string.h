#ifndef BURKE_STRING_H
#define BURKE_STRING_H

#include "types.h"

/* This is pretty much identical to symbols. */

#define LT_STRING 16

// see symbol.h for fuckedness
typedef char lisp_string[];

//TODO inline
char* string_string(lisp_string*);

lispobj* make_string(char*);

#endif /* guard */
