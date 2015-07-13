#include "pair.h"
#include "alloc.h"

lispobj* pair_car(lisp_pair* pair) { return pair->car; }
lispobj* pair_cdr(lisp_pair* pair) { return pair->cdr; }
void set_pair_car(lisp_pair* pair, lispobj* obj) {
  pair->car = obj;
}
void set_pair_cdr(lisp_pair* pair, lispobj *obj) {
  pair->cdr = obj;
}

lispobj* make_pair(lispobj* car, lispobj* cdr) {
  lispobj* ret;
  size_t size = ALIGN_LO(lisp_pair);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_PAIR;
    LO_GET(lisp_pair, *ret)->car = car;
    LO_GET(lisp_pair, *ret)->cdr = cdr;
  } while (LALLOC_FAILED(addr, size));

  return ret;
}
