#ifndef BURKE_PACKAGE_H
#define BURKE_PACKAGE_H

#include <stddef.h> // size_t
#include "types.h"

#define LT_PACKAGE 13

typedef struct lisp_package {
  size_t size;
  size_t fill;
  lispobj** symbols; // can't resize with a flexible w/o more indirect
} lisp_package;

lispobj* intern(char*, lisp_package*);
lispobj* find_symbol(char*, lisp_package*);
lispobj* find_or_intern(char*, lisp_package*);

lispobj* make_package(size_t);

#endif /* guard */
