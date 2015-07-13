#ifndef BURKE_PAIR_H
#define BURKE_PAIR_H

#include "types.h"

#define LT_PAIR 1

typedef struct lisp_pair {
  lispobj *car, *cdr;
} lisp_pair;

// TODO inline
lispobj* pair_car(lisp_pair*);
lispobj* pair_cdr(lisp_pair*);
void set_pair_car(lisp_pair*, lispobj*);
void set_pair_cdr(lisp_pair*, lispobj*);

lispobj* make_pair(lispobj*, lispobj*);

#endif /* guard */
