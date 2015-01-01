#ifndef BURKE_ERROR_H
#define BURKE_ERROR_H

/* FIXME: FIXME */

#include <stdarg.h>
#include <assert.h>
#include "types.h" // lispobj_typep, lispobj

// should take a lispobj in the future
typedef lispobj*(*error_handler)(const char* format, ...);

#ifndef _Thread_local
// some impls support threads but not C11, because they suck.
#ifdef __GNUC__
#define _Thread_local __thread
#endif
#endif

extern _Thread_local error_handler lerror __attribute__ ((format(printf,1,2)));

#define assert_tag(LISPOBJ, TAG)		\
  do {						\
    assert((LISPOBJ));				\
    assert(lispobj_tagp((LISPOBJ), (TAG)));	\
  } while(0)
    
#endif
