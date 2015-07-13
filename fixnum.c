#include "fixnum.h"
#include "types.h"
#include "alloc.h"

lispobj* make_fixnum(fixnum n) {
  lispobj *ret;
  size_t size = ALIGN_LO(fixnum);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_FIXNUM;
    *LO_GET(fixnum, *ret) = n;
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
