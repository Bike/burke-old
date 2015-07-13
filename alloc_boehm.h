#ifndef BURKE_ALLOC_BOEHM_H
#define BURKE_ALLOC_BOEHM_H

#include "error.h"
#include <gc/gc.h>

#define LALLOC_RESERVE(addr, size)		\
  do {						\
  addr = GC_MALLOC(size);			\
  if (!addr) return lerror("OOM");		\
  } while (0)

#define LALLOC_RESERVE_LEAF(addr, size)		\
  do {						\
  addr = GC_MALLOC_ATOMIC(size);		\
  if (!addr) return lerror("OOM");		\
  } while (0)

#define LALLOC_FAILED(addr, size) 0

#endif /* guard */
