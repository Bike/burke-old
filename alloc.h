#ifndef BURKE_ALLOC_H
#define BURKE_ALLOC_H

#include <stdio.h> // for make_port
#include "types.h"

lispobj* make_symbol(const char*, size_t);
lispobj* make_pair(lispobj*, lispobj*);
lispobj* make_smallenv(lispobj*);
lispobj* make_nenv(lispobj*, size_t);
lispobj* make_wrapped(lisptag, lispobj*);
lispobj* make_port(FILE*);
lispobj* make_fsubr(fsubr_funptr);
lispobj* make_singleton(unsigned short);
lispobj* make_fexpr(lispobj*,lispobj*,lispobj*,lispobj*);
lispobj* make_vector(fixnum);
lispobj* make_fixnum(fixnum);
lispobj* make_mtag(lisptag);

#endif // guard
