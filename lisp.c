#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "lisp.h"
#include "types.h"
#include "error.h"
#include "alloc.h"
#include "read.h"
#include "write.h"
#include "package.h"
#include "singleton.h"
#include "pair.h"
#include "vector.h"
#include "fsubr.h"
#include "symbol.h"
#include "fexpr.h"
#include "applicative.h"
#include "wrapped.h"
#include "smallenv.h"
#include "nenv.h"
#include "string.h"
#include "mtag.h"
#include "port.h"

lispobj *user_evals = NULL, *user_combines = NULL, *user_lookups = NULL;
lispobj *user_writes = NULL, *user_defines = NULL;
lispobj *empty_environment = NULL;

/* MORE COMPLICATED LISPY/MISC FUNCTIONS IN C */

// these are not good
inline int nullp(lispobj* obj) {return obj == nil;}
inline int ignorep(lispobj* obj) {return obj == ignore;}
inline int undefinedp(lispobj* obj) {return obj == NULL;}

int truth(lispobj *val) {
  assert(val);
  if (val == sharp_t) return 1;
  if (val == sharp_f) return 0;
  // If it isn't a boolean, prompt the error handler for a boolean.
  // Sure hope the compiler has tail calls.
  return truth(lerror("truth of a non-boolean\n"));
}

inline lispobj* untruth(int bool) {
  if (bool)
    return sharp_t;
  else
    return sharp_f;
}

lispobj* list(int count, ...) {
  /* Return a lisp list of COUNT elements. */
  int i;
  lispobj* result = nil;
  lispobj* tail;
  va_list args;

  if (count == 0) return result;

  va_start(args, count);

  result = tail = make_pair(va_arg(args, lispobj*), nil);

  for (i = 1; i < count; ++i) {
    set_pair_cdr(LO_GET(lisp_pair, *tail),
		 make_pair(va_arg(args, lispobj*), nil));
    tail = pair_cdr(LO_GET(lisp_pair, *tail));
  }

  va_end(args);
  return result;
}

lispobj* map1(lispobj*(*fun)(lispobj*,void*),
	      lispobj* list, void* data) {
  /* Calls FUN on each element of LIST and returns a new LIST with
   * the result. FUN receives the DATA as a second argument. */
  lispobj* result = nil;
  lispobj* cur;

  if (!nullp(list)) {
    check_tag(list, LT_PAIR);
    result = cur = make_pair(fun(pair_car(LO_GET(lisp_pair, *list)),
				 data),
			     nil);
    for(;;) { // could be non-vacuous but don't fix what ain't broke
      list = pair_cdr(LO_GET(lisp_pair, *list));
      if (nullp(list)) break;
      check_tag(list, LT_PAIR); // these asserts are getting silly
      set_pair_cdr(LO_GET(lisp_pair, *cur),
		   make_pair(fun(pair_car(LO_GET(lisp_pair, *list)),
				 data),
			     nil));
      cur = pair_cdr(LO_GET(lisp_pair, *cur));
    }
  }

  return result;
}

fixnum list_length(lispobj* list) {
  fixnum result = 0;

  for(result = 0; !nullp(list);
      ++result, list = pair_cdr(LO_GET(lisp_pair, *list)));
  return result;
}

/* BASIC EVALUATOR */

lispobj* eval(lispobj *obj, lispobj *env) {
  lisptag objtag = LO_TAG(*obj);
  lispobj* user_eval = vref(LO_GET(lisp_vector, *user_evals), objtag);

  if (undefinedp(user_eval))
    // return lerror("No user evaluator for object tag %d\n", objtag);
    return obj; // self-evaluate by default - bad idea maybe?
  else
    return combine(user_eval, list(2, obj, env), empty_environment);
}

lispobj* combine(lispobj *combiner, lispobj *combinand, lispobj *env) {
  lisptag combinertag = LO_TAG(*combiner);
  lispobj* user_combine = vref(LO_GET(lisp_vector, *user_combines),
			       combinertag);

  if (lispobj_tagp(combiner, LT_FSUBR)) // break metacircle
    return fsubr_combine(LO_GET(lisp_fsubr, *combiner),
			 combinand, env);

  if (undefinedp(user_combine))
    return lerror("No user combiner for combiner tag %d\n", combinertag);
  else
    return combine(user_combine,
		   list(3, combiner, combinand, env),
		   empty_environment);
}

lispobj* lookup(lispobj *name, lispobj* env) {
  lisptag envtag = LO_TAG(*env);
  lispobj* user_lookup = vref(LO_GET(lisp_vector, *user_lookups),
			      envtag);

  if (undefinedp(user_lookup))
    return lerror("No user lookup for env tag %d\n", envtag);
  else
    return combine(user_lookup, list(2, name, env), empty_environment);
}

void define(lispobj *name, lispobj *value, lispobj *env) {
  lisptag envtag = LO_TAG(*env);
  lispobj* user_define = vref(LO_GET(lisp_vector, *user_defines),
			      envtag);

  if (undefinedp(user_define))
    lerror("No user define for env tag %d\n", envtag);
  else
    combine(user_define, list(3, name, value, env), empty_environment);
}

/* STANDARD EVALUATION/ETC METHODS */

lispobj* standard_symbol_eval(lispobj* symbol, lispobj* env) {
  check_tag(symbol, LT_SYMBOL);
  return lookup(symbol, env);
}

lispobj* standard_pair_eval(lispobj* pair, lispobj* env) {
  check_tag(pair, LT_PAIR);
  return combine(eval(pair_car(LO_GET(lisp_pair, *pair)), env),
		 pair_cdr(LO_GET(lisp_pair, *pair)), env);
}

lispobj* standard_fsubr_combine(lispobj* combiner,
				lispobj* combinand, lispobj* env) {
  check_tag(combiner, LT_FSUBR);
  // duplicate of in combine...
  return fsubr_combine(LO_GET(lisp_fsubr, *combiner),
		       combinand, env);
		       
}

lispobj* standard_fexpr_combine(lispobj* combiner,
				lispobj* combinand, lispobj* env) {
  check_tag(combiner, LT_FEXPR);
  return fexpr_combine(LO_GET(lisp_fexpr, *combiner),
		       combinand, env);
}

lispobj* standard_applicative_combine(lispobj* combiner,
				      lispobj* combinand, lispobj* env) {
  check_tag(combiner, LT_APPLICATIVE);
  return applicative_combine(LO_GET(lisp_wrapped, *combiner),
			     combinand, env);
}

lispobj* standard_smallenv_lookup(lispobj* name, lispobj* smallenv) {
  check_tag(smallenv, LT_SMALLENV);
  return smallenv_lookup(LO_GET(lisp_smallenv, *smallenv), name);
}

lispobj* standard_nenv_lookup(lispobj* name, lispobj* nenv) {
  check_tag(nenv, LT_NENV);
  return nenv_lookup(LO_GET(lisp_nenv, *nenv), name);
}

/* since these are accessible, they gotta return something */
lispobj* standard_smallenv_define(lispobj *name, lispobj *value,
				  lispobj *smallenv) {
  check_tag(smallenv, LT_SMALLENV);
  smallenv_define(LO_GET(lisp_smallenv, *smallenv), name, value);
  return inert;
}

lispobj* standard_nenv_define(lispobj *name, lispobj *value, lispobj *nenv) {
  check_tag(nenv, LT_NENV);
  nenv_define(LO_GET(lisp_nenv, *nenv), name, value);
  return inert;
}

/* INITIALIZATION mostly globals oh no */

/* we have to use standard_nenv_define rather than define since we are,
   after all, defining the ability to define, among other things.
   double metacircles across the sky! */

lispobj* make_ground(lisp_package* p) {

#define LDEF(STR, VALUE)					\
  standard_nenv_define(find_or_intern(STR, p), VALUE, ret);
#define LDEFV(STR, NAME) LDEF(STR, make_fsubr(NAME));
#define LDEFA(STR, NAME)					\
  LDEF(STR, make_wrapped(LT_APPLICATIVE, make_fsubr(NAME)));

  lispobj* ret = make_nenv(NULL, 271);
  
  LDEF("combinators", user_combines);
  LDEF("defines", user_defines);
  LDEF("evaluators", user_evals);
  LDEF("lookupers", user_lookups);

  //LDEF("package", (lispobj*)p); // FIXME

  LDEFA("car", car_fsubr);
  LDEFA("cdr", cdr_fsubr);
  LDEFA("combine", combine_fsubr);
  LDEFA("cons", cons_fsubr);
  LDEFA("define!", define_fsubr);
  LDEFA("eq?", eqp_fsubr);
  LDEFA("eval", eval_fsubr);
  LDEFV("$fexpr", fexpr_fsubr);
  LDEFV("$if", if_fsubr);
  LDEFA("lookup", lookup_fsubr);
  LDEFA("newtag", newtag_fsubr);
  LDEFV("$quote", quote_fsubr);
  LDEFA("read", read_lisp_fsubr);
  LDEFA("tag-of", tag_of_fsubr);
  LDEFA("tag=?", tag_equal_fsubr);
  LDEFA("write", write_lisp_fsubr);
  LDEFA("unwrap", unwrap_fsubr);
  LDEFA("wrap", wrap_fsubr);
  LDEFA("app", app_fsubr);

  return ret;

  /* probably not strictly necessary */
#undef LDEF
#undef LDEFV
#undef LDEFV
}

void populate_evals(void) {
  lisp_vector* vuser_evals = LO_GET(lisp_vector, *user_evals);
  set_vref(vuser_evals, LT_SYMBOL, make_fsubr(standard_symbol_eval_fsubr));
  set_vref(vuser_evals, LT_PAIR, make_fsubr(standard_pair_eval_fsubr));
}

void populate_combines(void) {
  lisp_vector* vuser_combines = LO_GET(lisp_vector, *user_combines);
  set_vref(vuser_combines, LT_FSUBR, make_fsubr(standard_fsubr_combine_fsubr));
  set_vref(vuser_combines, LT_FEXPR, make_fsubr(standard_fexpr_combine_fsubr));
  set_vref(vuser_combines, LT_APPLICATIVE,
	   make_fsubr(standard_applicative_combine_fsubr));
}

void populate_lookups(void) {
  lisp_vector* vuser_lookups = LO_GET(lisp_vector, *user_lookups);
  set_vref(vuser_lookups, LT_NENV, make_fsubr(standard_nenv_lookup_fsubr));
  set_vref(vuser_lookups, LT_SMALLENV, make_fsubr(standard_smallenv_lookup_fsubr));
}

void populate_defines(void) {
  lisp_vector* vuser_defines = LO_GET(lisp_vector, *user_defines);
  set_vref(vuser_defines, LT_NENV, make_fsubr(standard_nenv_define_fsubr));
  set_vref(vuser_defines, LT_SMALLENV, make_fsubr(standard_smallenv_define_fsubr));
}

void populate_writes(void) {
  lisp_vector* vuser_writes = LO_GET(lisp_vector, *user_writes);
  set_vref(vuser_writes, LT_PAIR, make_fsubr(standard_pair_write_fsubr));
  set_vref(vuser_writes, LT_FIXNUM, make_fsubr(standard_fixnum_write_fsubr));
  set_vref(vuser_writes, LT_SYMBOL, make_fsubr(standard_symbol_write_fsubr));
  set_vref(vuser_writes, LT_VECTOR, make_fsubr(standard_vector_write_fsubr));
  set_vref(vuser_writes, LT_FSUBR, make_fsubr(standard_fsubr_write_fsubr));
  // FIXME?
  set_vref(vuser_writes, LT_NIL, make_fsubr(standard_singleton_write_fsubr));
  set_vref(vuser_writes, LT_IGNORE, make_fsubr(standard_singleton_write_fsubr));
  set_vref(vuser_writes, LT_INERT, make_fsubr(standard_singleton_write_fsubr));
  set_vref(vuser_writes, LT_TRUE, make_fsubr(standard_singleton_write_fsubr));
  set_vref(vuser_writes, LT_FALSE, make_fsubr(standard_singleton_write_fsubr));
  set_vref(vuser_writes, LT_STRING, make_fsubr(standard_string_write_fsubr));
  set_vref(vuser_writes, LT_MTAG, make_fsubr(standard_mtag_write_fsubr));
}

void initialize_globals(void) {

  empty_environment = make_nenv(NULL, 0);

  user_evals = make_vector(next_tag); populate_evals();
  user_combines = make_vector(next_tag); populate_combines();
  user_lookups = make_vector(next_tag); populate_lookups();
  user_defines = make_vector(next_tag); populate_defines();
  user_writes = make_vector(next_tag); populate_writes();

  // see singleton.c for whining
  *nil = ((lispobj) {.tag = LT_NIL});
  *ignore = ((lispobj) {.tag = LT_IGNORE});
  *inert = ((lispobj) {.tag = LT_INERT});
  *sharp_f = ((lispobj) {.tag = LT_FALSE});
  *sharp_t = ((lispobj) {.tag = LT_TRUE});
}

/* STANDARD FSUBRS */

#define LCDR(EXPR) (pair_cdr(LO_GET(lisp_pair, *(EXPR))))

#define FSUBR_ARG(EXPR, VAR)			\
  lispobj *VAR;					\
  if (lispobj_tagp((EXPR), LT_PAIR))		\
    VAR = pair_car(LO_GET(lisp_pair, *(EXPR)));	\
  else						\
    return lerror("Not enough arguments");

#define FSUBR_END(EXPR)				\
  if (!nullp(EXPR))				\
    return lerror("too many arguments");

#define FSUBR_AUX0() FSUBR_END(combinand)

#define FSUBR_AUX1(VAR1)			\
  FSUBR_ARG(combinand, VAR1);			\
  FSUBR_END(LCDR(combinand));

#define FSUBR_AUX2(VAR1, VAR2)			\
  FSUBR_ARG(combinand, VAR1);			\
  FSUBR_ARG(LCDR(combinand), VAR2);		\
  FSUBR_END(LCDR(LCDR(combinand)));

#define FSUBR_AUX3(VAR1, VAR2, VAR3)		\
  FSUBR_ARG(combinand, VAR1);			\
  FSUBR_ARG(LCDR(combinand), VAR2);		\
  FSUBR_ARG(LCDR(LCDR(combinand)), VAR3);	\
  FSUBR_END(LCDR(LCDR(LCDR(combinand))));

#define SIMPLE_FSUBR1(LNAME, CNAME)				\
  lispobj* LNAME##_fsubr(lispobj *combinand, lispobj *env) {	\
    UNUSED(env);						\
    FSUBR_AUX1(v1);						\
    return CNAME(v1);						\
  }

#define TYPED_FSUBR1(LNAME, LTYPE, CNAME)			\
  lispobj* LNAME##_fsubr(lispobj* combinand, lispobj* env) {	\
    UNUSED(env);						\
    FSUBR_AUX1(v1);						\
    return CNAME(LO_GET(LTYPE, *v1));				\
  }

#define SIMPLE_FSUBR2(LNAME, CNAME)				\
  lispobj* LNAME##_fsubr(lispobj *combinand, lispobj *env) {	\
    UNUSED(env);						\
    FSUBR_AUX2(v1, v2);						\
    return CNAME(v1, v2);					\
  }

#define SIMPLE_FSUBR3(LNAME, CNAME)				\
  lispobj* LNAME##_fsubr(lispobj *combinand, lispobj *env) {	\
    UNUSED(env);						\
    FSUBR_AUX3(v1, v2, v3);					\
    return CNAME(v1, v2, v3);					\
  }

TYPED_FSUBR1(car, lisp_pair, pair_car)
TYPED_FSUBR1(cdr, lisp_pair, pair_cdr)
SIMPLE_FSUBR3(combine, combine)
SIMPLE_FSUBR2(cons, make_pair)

lispobj* define_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX3(name, value, denv);
  define(name, value, denv);
  return inert;
}

lispobj* eqp_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(a, b);
  return untruth(eqp(a,b));
}

SIMPLE_FSUBR2(eval, eval)

lispobj* fexpr_fsubr(lispobj *combinand, lispobj *env) {
  FSUBR_AUX3(arg, earg, body);

  if (!ignorep(arg)) check_tag(arg, LT_SYMBOL);
  if (!ignorep(earg)) check_tag(earg, LT_SYMBOL);

  return make_fexpr(arg, earg, env, body);
}

lispobj* if_fsubr(lispobj *combinand, lispobj *env) {
  FSUBR_AUX3(condition, consequent, alternate);

  condition = eval(condition, env);
  if (truth(condition))
    return eval(consequent, env);
  else
    return eval(alternate, env);
}

SIMPLE_FSUBR2(lookup, lookup)

SIMPLE_FSUBR2(standard_symbol_eval, standard_symbol_eval)
SIMPLE_FSUBR2(standard_pair_eval, standard_pair_eval)
SIMPLE_FSUBR3(standard_fsubr_combine, standard_fsubr_combine)
SIMPLE_FSUBR3(standard_fexpr_combine, standard_fexpr_combine)
SIMPLE_FSUBR3(standard_applicative_combine, standard_applicative_combine)
SIMPLE_FSUBR2(standard_smallenv_lookup, standard_smallenv_lookup)
SIMPLE_FSUBR2(standard_nenv_lookup, standard_nenv_lookup)
SIMPLE_FSUBR3(standard_smallenv_define, standard_smallenv_define)
SIMPLE_FSUBR3(standard_nenv_define, standard_nenv_define)

lispobj* newtag_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX0();
  return make_mtag(next_tag++);
}

lispobj* quote_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX1(obj);
  return obj;
}

lispobj* read_lisp_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(port, package);
  check_tag(port, LT_PORT);
  check_tag(package, LT_PACKAGE);
  return read_lisp(port_stream(LO_GET(lisp_port, *port)),
		   LO_GET(lisp_package, *package));
}

lispobj* tag_of_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX1(obj);
  return make_mtag(LO_TAG(*obj));
}

lispobj* tag_equal_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(o1, o2);
  check_tag(o1, LT_MTAG); check_tag(o2, LT_MTAG);
  return untruth(mtag_mtag(LO_GET(lisptag, *o1))
		 == mtag_mtag(LO_GET(lisptag, *o2)));
}

lispobj* write_lisp_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(obj, port);
  write_lisp(obj, port);
  return inert;
}

TYPED_FSUBR1(unwrap, lisp_wrapped, unwrap)
lispobj* wrap_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(tag, obj);
  check_tag(tag, LT_MTAG);
  return make_wrapped(mtag_mtag(LO_GET(lisptag, *tag)), obj);
}
lispobj* app_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX1(combiner);
  return make_wrapped(LT_APPLICATIVE, combiner);
}

#define WRITE_FSUBR_AUX(NAME)					\
  lispobj* standard_##NAME##_write_fsubr(lispobj *combinand,	\
					 lispobj *env) {	\
    UNUSED(env);						\
    FSUBR_AUX2(obj, port);					\
    write_##NAME(obj, port);					\
      return inert;						\
  }

WRITE_FSUBR_AUX(pair)
WRITE_FSUBR_AUX(fixnum)
WRITE_FSUBR_AUX(symbol)
WRITE_FSUBR_AUX(vector)
WRITE_FSUBR_AUX(fsubr)
WRITE_FSUBR_AUX(singleton)
WRITE_FSUBR_AUX(string)
WRITE_FSUBR_AUX(mtag)
