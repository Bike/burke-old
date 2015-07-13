#ifndef BURKE_AMAP_H
#define BURKE_AMAP_H

/* An Associative MAP.
 * This implementation is really stupid and lazy.
 */

#include "types.h"

#define LT_AMAP 8

typedef struct lisp_amap {
  size_t length;
  size_t fillptr;
  lispobj* kv[];
} lisp_amap;

int amap_assoc(lisp_amap*, lispobj*, lispobj**);
int set_amap_assoc(lisp_amap*, lispobj*, lispobj*);

lispobj* make_amap(size_t);
lispobj* amap_expand(lisp_amap*);

#endif /* guard */
