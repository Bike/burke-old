#ifndef BURKE_FIXNUM_H
#define BURKE_FIXNUM_H

#include "types.h"

#define LT_FIXNUM 0

typedef long fixnum;
#define FIXNUM_CONVERSION_SPEC "%ld"

lispobj* make_fixnum(fixnum);

#endif /* guard */
