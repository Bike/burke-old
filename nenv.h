#ifndef BURKE_NENV_H
#define BURKE_NENV_H

#include "types.h"

#define LT_NENV 7

typedef struct lisp_nenv {
  lispobj* parent;
  lispobj* map;
} lisp_nenv;

lispobj* nenv_lookup(lisp_nenv*, lispobj*);
void nenv_define(lisp_nenv*, lispobj*, lispobj*);

lispobj* make_nenv(lispobj*, size_t);

#endif /* guard */
