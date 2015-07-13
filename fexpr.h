#ifndef BURKE_FEXPR_H
#define BURKE_FEXPR_H

#include "types.h"

#define LT_FEXPR 5

typedef struct lisp_fexpr {
  lispobj* arg;
  lispobj* earg;
  lispobj* env;
  lispobj* body;
} lisp_fexpr;

//TODO inline
lispobj* fexpr_arg(lisp_fexpr*);
lispobj* fexpr_earg(lisp_fexpr*);
lispobj* fexpr_env(lisp_fexpr*);
lispobj* fexpr_body(lisp_fexpr*);

lispobj* fexpr_combine(lisp_fexpr*, lispobj*, lispobj*);

lispobj* make_fexpr(lispobj*,lispobj*,lispobj*,lispobj*);

#endif /* guard */
