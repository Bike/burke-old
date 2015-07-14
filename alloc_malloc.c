#include "alloc_malloc.h"
#include "types.h"
#include "mtag.h"

lisptag next_tag = LT_MTAG + 1;

inline int eqp(lispobj* a, lispobj* b) { return a == b; }
