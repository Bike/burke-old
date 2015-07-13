#ifndef BURKE_ALLOC_MALLOC_H
#define BURKE_ALLOC_MALLOC_H

#include "error.h"
#include <stdlib.h>

#define LALLOC_RESERVE(addr, size)		\
  do {						\
  addr = malloc(size);				\
  if (!addr) return lerror("OOM");		\
  } while (0)

#define LALLOC_RESERVE_LEAF(addr, size) LALLOC_RESERVE(addr, size)

#define LALLOC_FAILED(addr, size) 0

#endif /* guard */
