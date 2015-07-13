#ifndef BURKE_WRAPPED_H
#define BURKE_WRAPPED_H

/* A wrapped object, basically any other object but with a tag that
 * makes it mean something else. In some circumstances it might be
 * possible to do this by just mutating the tag, but I'll figure that
 * out at some other time.
 *
 * The most obvious example of a wrapped type is applicatives.
 */

#include "types.h"

// No specific LT

typedef struct lisp_wrapped {
  lispobj* underlying;
} lisp_wrapped;

//TODO inline
lispobj* unwrap(lisp_wrapped*);

lispobj* make_wrapped(lisptag, lispobj*);

#endif /* guard */
