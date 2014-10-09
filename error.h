#ifndef BURKE_ERROR_H
#define BURKE_ERROR_H

/* FIXME: FIXME */

#include <stdarg.h>
#include <assert.h>
#include "types.h" // lispobj_typep

#ifdef __GNUC__
void error(const char*, ...) __attribute__ ((format(printf,1,2), noreturn));
#else
void error(const char*, ...);
#endif /* printf attrs */

#define assert_type(LISPOBJ, TYPE)		\
  do {						\
    assert((LISPOBJ));				\
    assert(lispobj_typep((LISPOBJ), (TYPE)));	\
  } while(0)
    
#endif
