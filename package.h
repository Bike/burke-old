#ifndef BURKE_PACKAGE_H
#define BURKE_PACKAGE_H

#include "types.h"

extern lispobj *package;

lispobj* intern(const char*);
lispobj* find_symbol(const char*);
lispobj* find_or_intern(const char*);

void initialize_package(void);

#endif /* guard */
