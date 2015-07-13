#ifndef BURKE_TYPES_H
#define BURKE_TYPES_H

#include <stddef.h>
#include <stdio.h> /* for ports */

/* LISP TYPES */

typedef int lisptag;
#define TAG_MAX INT_MAX
#define TAG_CONVERSION_SPEC "%d"

typedef struct lispobj {
  lisptag tag;
  char data[];
} lispobj;

#define SIZEOF_LOD(databytes) (offsetof(lispobj, data) + databytes)
#define SIZEOF_LO(LTYPE) (SIZEOF_LOD(sizeof(LTYPE)))
#define LO_TAG(LO) ((LO).tag)
#define LO_DATA(LO) ((LO).data)
#define LO_GET(TYPE, LO) ((TYPE*)((LO).data))

extern lisptag next_tag; // defined in alloc

#define lispobj_tagp(obj, tag) (LO_TAG(*(obj)) == tag)
/*
inline int lispobj_tagp(lispobj* obj, lisptag tag) {
  return LO_TAG(*obj) == tag;
}
*/

// defined in alloc
// TODO: Rearrange to have this inlined.
int eqp(lispobj*, lispobj*);

#endif // guard
