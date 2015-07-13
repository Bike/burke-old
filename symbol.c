#include "symbol.h"
#include "alloc.h"
#include <string.h>

char* symbol_name(lisp_symbol* symbol) {
  return *symbol;
}

lispobj* make_symbol(char *name) {
  lispobj* ret;
  size_t len = strlen(name);
  size_t size = ALIGN_LOD(len);
  void* addr;
  do {
    LALLOC_RESERVE_LEAF(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_SYMBOL;
    strcpy(*LO_GET(lisp_symbol, *ret), name);
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
