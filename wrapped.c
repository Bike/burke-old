#include "wrapped.h"
#include "alloc.h"

lispobj* unwrap(lisp_wrapped* wrapped) {
  return wrapped->underlying;
}

lispobj* make_wrapped(lisptag tag, lispobj* underlying) {
  lispobj* ret;
  size_t size = ALIGN_LO(lisp_wrapped);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = tag;
    LO_GET(lisp_wrapped, *ret)->underlying = underlying;
  } while (LALLOC_FAILED(addr, size));

  return ret;
}
