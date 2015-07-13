#include "mtag.h"
#include "alloc.h"

// dumb (dumb (dumb (dumb)))
lisptag mtag_mtag(lisptag* tag) {
  return *tag;
}

lispobj* make_mtag(lisptag tag) {
  lispobj* ret;
  size_t size = ALIGN_LO(lisptag);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_MTAG;
    *LO_GET(lisptag, *ret) = tag;
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
