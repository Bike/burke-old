#ifndef BURKE_MTAG_H
#define BURKE_MTAG_H

#include "types.h"

#define LT_MTAG 17

//TODO inline
lisptag mtag_mtag(lisptag* tag);

lispobj* make_mtag(lisptag);

#endif /* guard */
