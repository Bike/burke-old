#ifndef BURKE_SMALLENV_H
#define BURKE_SMALLENV_H

#include "types.h"

#define LT_SMALLENV 6

typedef struct lisp_smallenv {
  lispobj* parent;
  lispobj* bind1_name;
  lispobj* bind1_value;
  lispobj* bind2_name;
  lispobj* bind2_value;
} lisp_smallenv;

// TODO inline
lispobj* smallenv_parent(lisp_smallenv*);
lispobj* smallenv_bind1_name(lisp_smallenv*);
lispobj* smallenv_bind1_value(lisp_smallenv*);
lispobj* smallenv_bind2_name(lisp_smallenv*);
lispobj* smallenv_bind2_value(lisp_smallenv*);
void set_smallenv_bind1_name(lisp_smallenv*,lispobj*);
void set_smallenv_bind1_value(lisp_smallenv*,lispobj*);
void set_smallenv_bind2_name(lisp_smallenv*,lispobj*);
void set_smallenv_bind2_value(lisp_smallenv*,lispobj*);

lispobj* smallenv_lookup(lisp_smallenv*, lispobj*);
void smallenv_define(lisp_smallenv*, lispobj*, lispobj*);

lispobj* make_smallenv(lispobj*);

#endif /* guard */
