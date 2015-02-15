#ifndef BURKE_PACKAGE_H
#define BURKE_PACKAGE_H

#include <stddef.h> // size_t
#include "types.h"
#include "layout_malloc.h" // ugh. for package def

lispobj* intern(const char*, lisp_package*);
lispobj* find_symbol(const char*, lisp_package*);
lispobj* find_or_intern(const char*, lisp_package*);

lisp_package* make_package(size_t);

#endif /* guard */
