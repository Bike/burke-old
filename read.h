#ifndef BURKE_READ_H
#define BURKE_READ_H

#include <stdio.h>
#include "types.h"
#include "package.h"

lispobj* read_lisp(FILE*, lisp_package*);
lispobj* read_delimited_list(FILE*, lisp_package*, char);
lispobj* read_symbol(FILE*, lisp_package*);
lispobj* read_sharp(FILE*);
lispobj* read_integer(FILE*);
lispobj* read_string(FILE*);

#endif /* guard */
