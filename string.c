#include "string.h"
#include "alloc.h"
#include <string.h>

char* string_string(lisp_string* string) {
  return *string;
}

lispobj* make_string(char *str) {
  lispobj* ret;
  size_t len = strlen(str);
  size_t size = ALIGN_LOD(len);
  void* addr;
  do {
    LALLOC_RESERVE_LEAF(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_STRING;
    strcpy(*LO_GET(lisp_string, *ret), str);
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
