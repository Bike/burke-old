#include <string.h> // strlen, strcmp
#include <stdlib.h> // alloc (see note)
#include "alloc.h"
#include "package.h"
#include "error.h"
#include "lisp.h"
#include "symbol.h"

/* In the future, packages should be lisp accessible objects.
 * (Well, after I wrote this comment I scrambled to make them so.)
 * That's why I don't feel too bad about using malloc here.
 * Note for the future that packages' symbols are fairly static
 *  so using manual memory management for them makes sense.
 * (In CL it's intern and unintern)
 * We also use realloc, which entails separate storage
 *  for the symbols (i.e. lispobj** instead of lispobj*[])
 */

lispobj* intern(char* name, lisp_package* p) {
  lispobj *symbol;

  if (p->fill > p->size)
    p->symbols = realloc(p->symbols, p->size * 2);

  symbol = make_symbol(name);
  p->symbols[p->fill++] = symbol;

  return symbol;
}

lispobj* find_symbol(char *name, lisp_package* p) {
  size_t i;
  for(i = 0; i < p->fill; ++i) {
    lispobj* symbol = p->symbols[i];
    if (!strcmp(name, symbol_name(LO_GET(lisp_symbol, *symbol))))
      return symbol;
  }
  return NULL;
}

lispobj* find_or_intern(char *name, lisp_package *p) {
  lispobj *found = find_symbol(name, p);
  if (found == NULL)
    return intern(name, p);
  else
    return found;
}

/*
// this could be rearranged to take the symbols as client data
// makes it closure-safe (unimportant in boehm, probably mps)
void finalize_package(lispobj *package, void *ignore) {
  (void)ignore;
  free(package->symbols);
}
*/

lispobj* make_package(size_t initial_syms) {
  lispobj *ret;
  size_t size = ALIGN_LO(lisp_package);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_PACKAGE;
    LO_GET(lisp_package, *ret)->size = initial_syms;
    LO_GET(lisp_package, *ret)->fill = 0;
    LO_GET(lisp_package, *ret)->symbols = 
      malloc(initial_syms*sizeof(lispobj*));
  } while (LALLOC_FAILED(addr, size));
  //GC_register_finalizer(ret, finalize_package, NULL, NULL, NULL);
  return ret;
}
