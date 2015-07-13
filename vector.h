#ifndef BURKE_VECTOR_H
#define BURKE_VECTOR_H

#include "types.h"
#include "fixnum.h"

// Unspecialized vectors, i.e., of lispobjs.

#define LT_VECTOR 3

typedef struct lisp_vector {
  fixnum length;
  lispobj* data[];
} lisp_vector;

//TODO inline
fixnum vector_length(lisp_vector*);

lispobj* vref(lisp_vector*, fixnum);
void set_vref(lisp_vector*, fixnum, lispobj*);

lispobj* make_vector(fixnum);

#endif /* guard */
