#ifndef BURKE_ALLOC_H
#define BURKE_ALLOC_H

#include "types.h"
#include "error.h"

// from mps examples scheme.c
#define ALIGNMENT (sizeof(void*))
/* Align size upwards to the next multiple of the word size. */
#define ALIGN_WORD(size)			\
  (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
/* Align size upwards to the next multiple of the word size, and
 * additionally ensure that it's big enough to store a forwarding
 * pointer. Evaluates its argument twice. */
/*
#define ALIGN_OBJ(size)                                \
  (ALIGN_WORD(size) >= ALIGN_WORD(sizeof(fwd_s))       \
   ? ALIGN_WORD(size)                                  \
   : ALIGN_WORD(sizeof(fwd_s)))
*/
#define ALIGN_OBJ(size) ALIGN_WORD(size)
#define ALIGN_LO(ltype) ALIGN_OBJ(SIZEOF_LO(ltype))
#define ALIGN_LOD(size) ALIGN_OBJ(SIZEOF_LOD(size))

#ifdef LALLOC_MPS // do not use this!
#include "alloc_mps.h"
#elif defined LALLOC_BOEHM
#include "alloc_boehm.h"
#elif defined LALLOC_MALLOC
#include "alloc_malloc.h"
#endif

#endif /* guard */
