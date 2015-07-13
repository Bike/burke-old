#include "alloc_boehm.h"
#include "types.h"
#include "mtag.h"

lisptag next_tag = LT_MTAG + 1; // boehm doesn't require any more objs

// boehm is not a moving collector
inline int eqp(lispobj* a, lispobj* b) {
  return a == b;
}
