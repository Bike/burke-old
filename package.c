#include <string.h> // strlen, strcmp
#include "alloc.h"
#include "package.h"
#include "error.h"
#include "lisp.h"

lispobj *package = NULL;

/* fuck's sake (not in lisp for damn sure) */
#define PACKAGE_MAX 1000
int package_fillptr;

lispobj* intern(const char* name) {
  lispobj *symbol;

  if (package_fillptr > PACKAGE_MAX)
    error("out of symbol space!");

  symbol = make_symbol(name, strlen(name));
  set_vref(package, package_fillptr++, symbol);

  return symbol;
}

lispobj* find_symbol(const char *name) {
  int i;
  for(i = 0; i < package_fillptr; ++i) {
    lispobj* symbol = vref(package, i);
    if (!strcmp(name, symbol_name(symbol)))
      return symbol;
  }
  return NULL;
}

lispobj* find_or_intern(const char *name) {
  lispobj *found = find_symbol(name);
  if (found == NULL)
    return intern(name);
  else
    return found;
}

void initialize_package(void) {
  package = make_vector(PACKAGE_MAX);
}
