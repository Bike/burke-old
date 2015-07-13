#include "singleton.h"

/* let's try to get these in static space.
 * Whoops this is a pain in the ass who saw that coming */

//#define STATIC_INIT(ltag) ((lispobj) {.tag = ltag})
//#define STATIC_INIT(ltag) ((lispobj)(char[])ltag)

static lispobj o_nil;// = STATIC_INIT(LT_NIL);
static lispobj o_ignore;// = STATIC_INIT(LT_IGNORE);
static lispobj o_inert;// = STATIC_INIT(LT_INERT);
static lispobj o_sharp_f;// = STATIC_INIT(LT_FALSE);
static lispobj o_sharp_t;// = STATIC_INIT(LT_TRUE);

//#undef STATIC_INIT

lispobj* nil = &o_nil;
lispobj* ignore = &o_ignore;
lispobj* inert = &o_inert;
lispobj* sharp_f = &o_sharp_f;
lispobj* sharp_t = &o_sharp_t;
