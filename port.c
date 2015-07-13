#include "port.h"
#include "alloc.h"

/* On first glance you may think this is stupid.
 * On second glance you'll know it's stupid */
FILE* port_stream(lisp_port* port) {
  return *port;
}

lispobj* make_port(FILE* file) {
  lispobj* ret;
  size_t size = ALIGN_LO(lisp_port);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_PORT;
    *LO_GET(lisp_port, *ret) = file;
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
