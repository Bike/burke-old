#ifndef BURKE_FSUBR_H
#define BURKE_FSUBR_H

#include "types.h"

#define LT_FSUBR 4

/* lisp_fsubr is the type of pointers to
 * functions taking two lispobj* and returning one.
 */
typedef lispobj*(*lisp_fsubr)(lispobj*,lispobj*);

//TODO inline
lispobj* fsubr_combine(lisp_fsubr* fsubr, lispobj* arg, lispobj* env);

lispobj* make_fsubr(lisp_fsubr);

#endif /* guard */
