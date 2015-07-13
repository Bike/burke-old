#ifndef BURKE_SYMBOL_H
#define BURKE_SYMBOL_H

#include "types.h"

#define LT_SYMBOL 2

 // I think this is correct? lisp_symbol = char[]
typedef char lisp_symbol[];

// TODO inline
char* symbol_name(lisp_symbol*);

lispobj* make_symbol(char*);

#endif /* guard */
