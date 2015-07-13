#ifndef BURKE_SINGLETON_H
#define BURKE_SINGLETON_H

#include "types.h"

/* A singleton object. They used to be ID'd but now I'll just give
 * them all their own tag. #YOLO */

#define LT_NIL 9
#define LT_IGNORE 10
#define LT_INERT 11
#define LT_TRUE 14
#define LT_FALSE 15

// defined in singleton.c
extern lispobj *nil, *ignore, *inert, *sharp_f, *sharp_t;

#endif /* guard */
