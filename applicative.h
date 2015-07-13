#ifndef BURKE_APPLICATIVE_H
#define BURKE_APPLICATIVE_H

#include "types.h"
#include "wrapped.h"

#define LT_APPLICATIVE 8

// no actual struct, since it's just a wrapped

lispobj* applicative_combine(lisp_wrapped*, lispobj*, lispobj*);

#endif /* guard */
