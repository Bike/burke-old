#include "fsubr.h"
#include "alloc.h"

lispobj* fsubr_combine(lisp_fsubr* fsubr,
		       lispobj* arg, lispobj* env) {
  /* Note about the depths of the C:
   * Compiling everything gets warnings like this
   *  types.h:22:27: warning: ISO C forbids conversion of object pointer
   *   to function pointer type [-Wpedantic]
   *  #define LO_GET(TYPE, LO) ((TYPE*)((LO).data))
   * if lisp_fsubr is a function type instead of a function pointer
   * type (i.e. typedef lispobj*(lisp_fsubr)(lispobj*,lispobj*);)
   *
   * But with lisp_fsubr as funptr,
   * lisp_fsubr* is a _double_ indirected function, which apparently
   * counts as an object pointer, so there are no warnings now.
   * I was probably going to end up casting through void*, so yay.
   *
   * That's why things are doubly indirect and why we need *fsubr. */
  return (*fsubr)(arg, env);
}

lispobj* make_fsubr(lisp_fsubr fsubr) {
  lispobj* ret;
  size_t size = ALIGN_LO(lisp_fsubr);
  void* addr;
  do {
    LALLOC_RESERVE_LEAF(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_FSUBR;
    *LO_GET(lisp_fsubr, *ret) = fsubr;
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
