#include <string.h> // strlen, strcmp
#include <stdlib.h> // alloc (see note)
#include "alloc.h"
#include "package.h"
#include "error.h"
#include "lisp.h"

/* In the future, packages should be lisp accessible objects.
 * (Well, after I wrote this comment I scrambled to make them so.)
 * That's why I don't feel too bad about using malloc here.
 * Note for the future that packages' symbols are fairly static
 *  so using manual memory management for them makes sense.
 * (In CL it's intern and unintern)
 * We also use realloc, which entails separate storage
 *  for the symbols (i.e. lispobj** instead of lispobj*[])
 */

lispobj* intern(const char* name, lisp_package* p) {
  lispobj *symbol;

  if (p->fill > p->size)
    p->symbols = realloc(p->symbols, p->size * 2);

  symbol = make_symbol(name, strlen(name));
  p->symbols[p->fill++] = symbol;

  return symbol;
}

lispobj* find_symbol(const char *name, lisp_package* p) {
  size_t i;
  for(i = 0; i < p->fill; ++i) {
    lispobj* symbol = p->symbols[i];
    if (!strcmp(name, symbol_name(symbol)))
      return symbol;
  }
  return NULL;
}

lispobj* find_or_intern(const char *name, lisp_package *p) {
  lispobj *found = find_symbol(name, p);
  if (found == NULL)
    return intern(name, p);
  else
    return found;
}

lisp_package* make_package(size_t size) {
  lisp_package* ret = malloc(sizeof(lisp_package));
  ret->tag = LT_PACKAGE;
  ret->size = size;
  ret->fill = 0;
  ret->symbols = malloc(size*sizeof(lispobj*));
  return ret;
}
