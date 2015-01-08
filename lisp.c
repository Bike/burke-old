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

lispobj *nil = NULL, *inert = NULL, *ignore = NULL, *sharp_t = NULL, *sharp_f = NULL;
lispobj *user_evals = NULL, *user_combines = NULL, *user_lookups = NULL;
lispobj *user_writes = NULL, *user_defines = NULL;
lispobj *empty_environment = NULL;

/* MORE COMPLICATED LISPY/MISC FUNCTIONS IN C */

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
    set_pair_cdr(tail, make_pair(va_arg(args, lispobj*), nil));
    tail = pair_cdr(tail);
  }

  va_end(args);
  return result;
}

/* "blank" because "empty list" would confuse me w/nil */
lispobj* blank_list(fixnum count) {
  lispobj* result;

  for(result = nil; count > 0; --count, result = make_pair(result, nil));
  return result;
}

inline int nullp(lispobj* obj) {return obj == nil;}
inline int ignorep(lispobj* obj) {return obj == ignore;}
inline int undefinedp(lispobj* obj) {return obj == NULL;}

fixnum list_length(lispobj* list) {
  fixnum result = 0;

  for(result = 0; !nullp(list); ++result, list = pair_cdr(list));
  return result;
}

/* BASIC EVALUATOR */

lispobj* eval(lispobj *obj, lispobj *env) {
  lisptag objtag = tagof_lispobj(obj);
  lispobj* user_eval = vref(user_evals, objtag);

  if (undefinedp(user_eval))
    // return lerror("No user evaluator for object tag %d\n", objtag);
    return obj; // self-evaluate by default - bad idea maybe?
  else
    return combine(user_eval, list(2, obj, env), empty_environment);
}

lispobj* combine(lispobj *combiner, lispobj *combinand, lispobj *env) {
  lisptag combinertag = tagof_lispobj(combiner);
  lispobj* user_combine = vref(user_combines, combinertag);

  if (lispobj_tagp(combiner, LT_FSUBR)) // break metacircle
    return standard_fsubr_combine(combiner, combinand, env);

  if (undefinedp(user_combine))
    return lerror("No user combiner for combiner tag %d\n", combinertag);
  else
    return combine(user_combine, list(3, combiner, combinand, env), empty_environment);
}

lispobj* lookup(lispobj *name, lispobj* env) {
  lisptag envtag = tagof_lispobj(env);
  lispobj* user_lookup = vref(user_lookups, envtag);

  if (undefinedp(user_lookup))
    return lerror("No user lookup for env tag %d\n", envtag);
  else
    return combine(user_lookup, list(2, name, env), empty_environment);
}

void define(lispobj *name, lispobj *value, lispobj *env) {
  lisptag envtag = tagof_lispobj(env);
  lispobj* user_define = vref(user_defines, envtag);

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
  return combine(eval(pair_car(pair), env), pair_cdr(pair), env);
}

lispobj* standard_fsubr_combine(lispobj* combiner, lispobj* combinand, lispobj* env) {
  check_tag(combiner, LT_FSUBR);
  return (fsubr_fun(combiner))(combinand, env);
}

lispobj* standard_fexpr_combine(lispobj* combiner, lispobj* combinand, lispobj* env) {
  check_tag(combiner, LT_FEXPR);

  lispobj* arg = fexpr_arg(combiner);
  lispobj* earg = fexpr_earg(combiner);
  lispobj* fenv = make_smallenv(fexpr_env(combiner));
  lispobj* body = fexpr_body(combiner);

  /* non-symbols should be understood at fexpr creation time */
  if (lispobj_tagp(arg, LT_SYMBOL)) {
    set_smallenv_bind1_name(fenv, arg);
    set_smallenv_bind1_value(fenv, combinand);
  }
  if (lispobj_tagp(earg, LT_SYMBOL)) {
    set_smallenv_bind2_name(fenv, earg);
    set_smallenv_bind2_value(fenv, env);
  }

  return eval(body, fenv);
}

lispobj* standard_applicative_combine(lispobj* combiner,
				      lispobj* combinand, lispobj* env) {
  check_tag(combiner, LT_APPLICATIVE);

  lispobj* underlying = applicative_underlying(combiner);
  lispobj *evaled = nil;

  /* this is essentially map1. move out if it's needed elsewhere
     the difference, of course, is that C has no closures (well, the new one does)
     and i don't want to bother with the void** callback shit if i can avoid it. */
  if (!nullp(combinand)) {
    check_tag(combinand, LT_PAIR);

    lispobj *cur;

    evaled = cur = make_pair(eval(pair_car(combinand), env), nil);
    for(;;) { // could be non-vacuous
      combinand = pair_cdr(combinand);
      if (nullp(combinand)) break;
      check_tag(combinand, LT_PAIR); // these asserts are getting silly
      set_pair_cdr(cur, make_pair(eval(pair_car(combinand), env), nil));
      cur = pair_cdr(cur);
    }
  }

  return combine(underlying, evaled, env);
}

lispobj* standard_smallenv_lookup(lispobj* name, lispobj* smallenv) {
  check_tag(smallenv, LT_SMALLENV);

  if (eqp(name, smallenv_bind1_name(smallenv)))
    return smallenv_bind1_value(smallenv);
  else if (eqp(name, smallenv_bind2_name(smallenv)))
    return smallenv_bind2_value(smallenv);

  /* smallenvs should always have a parent due to how they're used
     but this is nontrivial to assert... */
  lispobj* parent = smallenv_parent(smallenv);
  assert(!(parent == NULL)); // FIXME
  return lookup(name, parent);
}

lispobj* standard_nenv_lookup(lispobj* name, lispobj* nenv) {
  check_tag(nenv, LT_NENV);

  lispobj **names = nenv_names(nenv), **values = nenv_values(nenv);
  lispobj *parent = nenv_parent(nenv);
  size_t i, fillptr = nenv_fillptr(nenv);

  for(i = 0; i < fillptr; ++i) {
    if (eqp(name, names[i]))
      return values[i];
  }
  if (parent == NULL)
    return lerror("unbound: %s\n", symbol_name(name)); // FIXME
  else
    return lookup(name, parent);
}

/* since these are accessible, they gotta return something */
lispobj* standard_smallenv_define(lispobj *name, lispobj *value, lispobj *smallenv) {
  check_tag(smallenv, LT_SMALLENV);

  if (eqp(name, smallenv_bind1_name(smallenv)))
    set_smallenv_bind1_name(smallenv, value);
  else if (eqp(name, smallenv_bind2_name(smallenv)))
    set_smallenv_bind2_name(smallenv, value);
  else
    return lerror("can't create new bindings in a smallenv\n");

  return inert;
}

lispobj* standard_nenv_define(lispobj *name, lispobj *value, lispobj *nenv) {
  check_tag(nenv, LT_NENV);

  lispobj **names = nenv_names(nenv), **values = nenv_values(nenv);
  size_t fillptr = nenv_fillptr(nenv);
  size_t i;

  for(i = 0; i < fillptr; ++i) {
    if (eqp(name, names[fillptr])) {
      values[fillptr] = value;
      break;
    }
  }

  /* here we have mutability, in new definitions appearing */
  if (fillptr > nenv_length(nenv))
    return lerror("No space left in nenv!\n"); // FIXME goddamn
  names[fillptr] = name;
  values[fillptr] = value;
  set_nenv_fillptr(nenv, fillptr + 1);
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

  LDEF("package", (lispobj*)p); // FIXME

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
  set_vref(user_evals, LT_SYMBOL, make_fsubr(standard_symbol_eval_fsubr));
  set_vref(user_evals, LT_PAIR, make_fsubr(standard_pair_eval_fsubr));
}

void populate_combines(void) {
  set_vref(user_combines, LT_FSUBR, make_fsubr(standard_fsubr_combine_fsubr));
  set_vref(user_combines, LT_FEXPR, make_fsubr(standard_fexpr_combine_fsubr));
  set_vref(user_combines, LT_APPLICATIVE,
	   make_fsubr(standard_applicative_combine_fsubr));
}

void populate_lookups(void) {
  set_vref(user_lookups, LT_NENV, make_fsubr(standard_nenv_lookup_fsubr));
  set_vref(user_lookups, LT_SMALLENV, make_fsubr(standard_smallenv_lookup_fsubr));
}

void populate_defines(void) {
  set_vref(user_defines, LT_NENV, make_fsubr(standard_nenv_define_fsubr));
  set_vref(user_defines, LT_SMALLENV, make_fsubr(standard_smallenv_define_fsubr));
}

void populate_writes(void) {
  set_vref(user_writes, LT_PAIR, make_fsubr(standard_pair_write_fsubr));
  set_vref(user_writes, LT_FIXNUM, make_fsubr(standard_fixnum_write_fsubr));
  set_vref(user_writes, LT_SYMBOL, make_fsubr(standard_symbol_write_fsubr));
  set_vref(user_writes, LT_VECTOR, make_fsubr(standard_vector_write_fsubr));
  set_vref(user_writes, LT_FSUBR, make_fsubr(standard_fsubr_write_fsubr));
  set_vref(user_writes, LT_SINGLETON, make_fsubr(standard_singleton_write_fsubr));
  set_vref(user_writes, LT_MTAG, make_fsubr(standard_mtag_write_fsubr));
}

void initialize_globals(void) {

  empty_environment = make_nenv(NULL, 0);

  user_evals = make_vector(next_tag); populate_evals();
  user_combines = make_vector(next_tag); populate_combines();
  user_lookups = make_vector(next_tag); populate_lookups();
  user_defines = make_vector(next_tag); populate_defines();
  user_writes = make_vector(next_tag); populate_writes();

  nil = make_singleton(0);
  inert = make_singleton(1);
  ignore = make_singleton(2);
  sharp_t = make_singleton(3);
  sharp_f = make_singleton(4);

}

/* STANDARD FSUBRS */

#define FSUBR_ARG(EXPR, VAR)			\
  lispobj *VAR;					\
  if (lispobj_tagp((EXPR), LT_PAIR))		\
    VAR = pair_car((EXPR));			\
  else						\
    return lerror("Not enough arguments\n");

#define FSUBR_END(EXPR) if (!nullp(EXPR)) return lerror("too many arguments\n");

#define FSUBR_AUX1(VAR1)			\
  FSUBR_ARG(combinand, VAR1);			\
  FSUBR_END(pair_cdr(combinand));

#define FSUBR_AUX2(VAR1, VAR2)				\
  FSUBR_ARG(combinand, VAR1);				\
  FSUBR_ARG(pair_cdr(combinand), VAR2);			\
  FSUBR_END(pair_cdr(pair_cdr(combinand)));
/*
  lispobj *VAR1, *VAR2;					\
  check_tag(combinand, LT_PAIR);			\
  VAR1 = pair_car(combinand);				\
  check_tag(pair_cdr(combinand), LT_PAIR);		\
  VAR2 = pair_car(pair_cdr(combinand));			\
  assert(nullp(pair_cdr(pair_cdr(combinand))));
*/

#define FSUBR_AUX3(VAR1, VAR2, VAR3)			\
  FSUBR_ARG(combinand, VAR1);				\
  FSUBR_ARG(pair_cdr(combinand), VAR2);			\
  FSUBR_ARG(pair_cdr(pair_cdr(combinand)), VAR3);	\
  FSUBR_END(pair_cdr(pair_cdr(pair_cdr(combinand))));

#define SIMPLE_FSUBR1(LNAME, CNAME, VAR1)			\
  lispobj* LNAME##_fsubr(lispobj *combinand, lispobj *env) {	\
    UNUSED(env);						\
    FSUBR_AUX1(VAR1);						\
    return CNAME(VAR1);						\
  }

#define SIMPLE_FSUBR2(LNAME, CNAME, VAR1, VAR2)			\
  lispobj* LNAME##_fsubr(lispobj *combinand, lispobj *env) {	\
    UNUSED(env);						\
    FSUBR_AUX2(VAR1, VAR2);					\
    return CNAME(VAR1, VAR2);					\
  }

#define SIMPLE_FSUBR3(LNAME, CNAME, VAR1, VAR2, VAR3)		\
  lispobj* LNAME##_fsubr(lispobj *combinand, lispobj *env) {	\
    UNUSED(env);						\
    FSUBR_AUX3(VAR1, VAR2, VAR3);				\
    return CNAME(VAR1, VAR2, VAR3);				\
  }

SIMPLE_FSUBR1(car, pair_car, cons);
SIMPLE_FSUBR1(cdr, pair_cdr, cons);
SIMPLE_FSUBR3(combine, combine, combiner, comb, cenv);
SIMPLE_FSUBR2(cons, make_pair, car, cdr);

/*
lispobj* define_fsubr(lispobj *combinand, lispobj *env) {
  FSUBR_AUX3(name, value, denv);
  define(name, eval(value, env), eval(denv, env));
  return inert;
}
*/
lispobj* define_fsubr(lispobj *combinand, lispobj *env) {
  FSUBR_AUX3(name, value, denv);
  define(name, value, denv);
  return inert;
}

lispobj* eqp_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(a, b);
  return untruth(eqp(a,b));
}

SIMPLE_FSUBR2(eval, eval, form, eenv);

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

SIMPLE_FSUBR2(lookup, lookup, name, lenv);

SIMPLE_FSUBR2(standard_symbol_eval, standard_symbol_eval, name, lenv);
SIMPLE_FSUBR2(standard_pair_eval, standard_pair_eval, pair, lenv);
SIMPLE_FSUBR3(standard_fsubr_combine, standard_fsubr_combine, op, comb, denv);
SIMPLE_FSUBR3(standard_fexpr_combine, standard_fexpr_combine, op, comb, denv);
SIMPLE_FSUBR3(standard_applicative_combine, standard_applicative_combine,
	      combiner, comb, denv);
SIMPLE_FSUBR2(standard_smallenv_lookup, standard_smallenv_lookup, name, lenv);
SIMPLE_FSUBR2(standard_nenv_lookup, standard_nenv_lookup, name, lenv);
SIMPLE_FSUBR3(standard_smallenv_define, standard_smallenv_define, name, value, denv);
SIMPLE_FSUBR3(standard_nenv_define, standard_nenv_define, name, value, denv);

lispobj* newtag_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  assert(nullp(combinand));
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
  return read_lisp(port_stream(port), (lisp_package*)package);
}

lispobj* tag_of_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX1(obj);
  return make_mtag(tagof_lispobj(obj));
}

lispobj* tag_equal_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(o1, o2);
  check_tag(o1, LT_MTAG); check_tag(o2, LT_MTAG); // FIXME shouldn't be fatal errors
  return untruth(mtag_mtag(o1) == mtag_mtag(o2));
}

lispobj* write_lisp_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(obj, port);
  write_lisp(obj, port);
  return inert;
}

SIMPLE_FSUBR1(unwrap, unwrap, applicative);
lispobj* wrap_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(tag, obj);
  check_tag(tag, LT_MTAG);
  return make_wrapped(mtag_mtag(tag), obj);
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

WRITE_FSUBR_AUX(pair);
WRITE_FSUBR_AUX(fixnum);
WRITE_FSUBR_AUX(symbol);
WRITE_FSUBR_AUX(vector);
WRITE_FSUBR_AUX(fsubr);
WRITE_FSUBR_AUX(singleton);
WRITE_FSUBR_AUX(mtag);
