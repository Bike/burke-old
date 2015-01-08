#ifndef BURKE_LISP_H
#define BURKE_LISP_H

#include <stdarg.h>
#include "types.h"
#include "package.h" // lisp_package*

#define UNUSED(x) ((void)x)

/* LISP CONSTANTS C NEEDS */
extern lispobj *nil, *inert, *ignore, *sharp_t, *sharp_f;
extern lispobj *lstdin, *lstdout, *lstderr;
extern lispobj *user_writes, *user_evals, *user_combines, *user_lookups, *user_defines;
extern lispobj *empty_environment;

int truth(lispobj*);
lispobj* untruth(int);

lispobj* list(int, ...);
lispobj* blank_list(fixnum);

inline int nullp(lispobj*);
inline int ignorep(lispobj*);
inline int undefinedp(lispobj*);

fixnum list_length(lispobj*);

lispobj* eval(lispobj*, lispobj*);
lispobj* combine(lispobj*, lispobj*, lispobj*);
lispobj* lookup(lispobj*, lispobj*);
void define(lispobj*, lispobj*, lispobj*);

lispobj* standard_symbol_eval(lispobj*, lispobj*);
lispobj* standard_pair_eval(lispobj*, lispobj*);

lispobj* standard_fsubr_combine(lispobj*, lispobj*, lispobj*);
lispobj* standard_fexpr_combine(lispobj*, lispobj*, lispobj*);
lispobj* standard_applicative_combine(lispobj*, lispobj*, lispobj*);

lispobj* standard_smallenv_lookup(lispobj*, lispobj*);
lispobj* standard_nenv_lookup(lispobj*, lispobj*);

lispobj* standard_smallenv_define(lispobj*, lispobj*, lispobj*);
lispobj* standard_nenv_define(lispobj*, lispobj*, lispobj*);

lispobj* make_ground(lisp_package*);
void populate_combines(void);
void populate_lookups(void);
void populate_definers(void);
void initialize_globals(void);

#define DECLARE_FSUBR(NAME) lispobj* NAME##_fsubr(lispobj*,lispobj*);

DECLARE_FSUBR(car);
DECLARE_FSUBR(cdr);
DECLARE_FSUBR(combine);
DECLARE_FSUBR(cons);
DECLARE_FSUBR(define);
DECLARE_FSUBR(eqp);
DECLARE_FSUBR(eval);
DECLARE_FSUBR(fexpr);
DECLARE_FSUBR(if);
DECLARE_FSUBR(lookup);
DECLARE_FSUBR(newtag);
DECLARE_FSUBR(quote);
DECLARE_FSUBR(read_lisp);
DECLARE_FSUBR(standard_symbol_eval);
DECLARE_FSUBR(standard_pair_eval);
DECLARE_FSUBR(standard_fsubr_combine);
DECLARE_FSUBR(standard_fexpr_combine);
DECLARE_FSUBR(standard_applicative_combine);
DECLARE_FSUBR(standard_smallenv_lookup);
DECLARE_FSUBR(standard_nenv_lookup);
DECLARE_FSUBR(standard_smallenv_define);
DECLARE_FSUBR(standard_nenv_define);
DECLARE_FSUBR(standard_pair_write);
DECLARE_FSUBR(standard_fixnum_write);
DECLARE_FSUBR(standard_symbol_write);
DECLARE_FSUBR(standard_vector_write);
DECLARE_FSUBR(standard_fsubr_write);
DECLARE_FSUBR(standard_singleton_write);
DECLARE_FSUBR(standard_mtag_write);
DECLARE_FSUBR(tag_of);
DECLARE_FSUBR(tag_equal);
DECLARE_FSUBR(unwrap);
DECLARE_FSUBR(wrap);
DECLARE_FSUBR(app);
DECLARE_FSUBR(write_lisp);

#endif /* guard */
