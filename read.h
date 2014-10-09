#ifndef BURKE_READ_H
#define BURKE_READ_H

#include <stdio.h>
#include "types.h"

lispobj* read_lisp(FILE*);
lispobj* read_delimited_list(FILE*, char);
lispobj* read_symbol(FILE*);
lispobj* read_sharp(FILE*);
lispobj* read_integer(FILE*);

#endif /* guard */
