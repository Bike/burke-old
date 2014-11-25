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
lispobj *lstdin = NULL, *lstdout = NULL, *lstderr = NULL;
lispobj *user_evals = NULL, *user_combines = NULL, *user_lookups = NULL;
lispobj *user_writes = NULL, *user_defines = NULL;
lispobj *ground_environment = NULL, *empty_environment = NULL;

/* MORE COMPLICATED LISPY/MISC FUNCTIONS IN C */

int truth(lispobj *val) {
  assert_type(val, LT_SINGLETON);
  if (val == sharp_t) return 1;
  if (val == sharp_f) return 0;
  error("truth of a non boolean\n");
  return 0; // this is a bad idea
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
  lisptype objtype = typeof_lispobj(obj);
  lispobj* user_eval = vref(user_evals, objtype);

  if (undefinedp(user_eval))
    // return error("No user evaluator for object type %d\n", objtype);
    return obj; // self-evaluate by default - bad idea maybe?
  else
    return combine(user_eval, list(2, obj, env), empty_environment);
}

lispobj* combine(lispobj *combiner, lispobj *combinand, lispobj *env) {
  lisptype combinertype = typeof_lispobj(combiner);
  lispobj* user_combine = vref(user_combines, combinertype);

  if (lispobj_typep(combiner, LT_FSUBR)) // break metacircle
    return standard_fsubr_combine(combiner, combinand, env);

  if (undefinedp(user_combine))
    return error("No user combiner for combiner type %d\n", combinertype);
  else
    return combine(user_combine, list(3, combiner, combinand, env), empty_environment);
}

lispobj* lookup(lispobj *name, lispobj* env) {
  lisptype envtype = typeof_lispobj(env);
  lispobj* user_lookup = vref(user_lookups, envtype);

  if (undefinedp(user_lookup))
    return error("No user lookup for env type %d\n", envtype);
  else
    return combine(user_lookup, list(2, name, env), empty_environment);
}

void define(lispobj *name, lispobj *value, lispobj *env) {
  lisptype envtype = typeof_lispobj(env);
  lispobj* user_define = vref(user_defines, envtype);

  if (undefinedp(user_define))
    error("No user define for env type %d\n", envtype);
  else
    combine(user_define, list(3, name, value, env), empty_environment);
}

/* STANDARD EVALUATION/ETC METHODS */

lispobj* standard_symbol_eval(lispobj* symbol, lispobj* env) {
  assert_type(symbol, LT_SYMBOL);
  return lookup(symbol, env);
}

lispobj* standard_pair_eval(lispobj* pair, lispobj* env) {
  assert_type(pair, LT_PAIR);
  return combine(eval(pair_car(pair), env), pair_cdr(pair), env);
}

lispobj* standard_fsubr_combine(lispobj* combiner, lispobj* combinand, lispobj* env) {
  assert_type(combiner, LT_FSUBR);
  return (fsubr_fun(combiner))(combinand, env);
}

lispobj* standard_fexpr_combine(lispobj* combiner, lispobj* combinand, lispobj* env) {
  assert_type(combiner, LT_FEXPR);

  lispobj* arg = fexpr_arg(combiner);
  lispobj* earg = fexpr_earg(combiner);
  lispobj* fenv = make_smallenv(fexpr_env(combiner));
  lispobj* body = fexpr_body(combiner);

  /* non-symbols should be understood at fexpr creation time */
  if (lispobj_typep(arg, LT_SYMBOL)) {
    set_smallenv_bind1_name(fenv, arg);
    set_smallenv_bind1_value(fenv, combinand);
  }
  if (lispobj_typep(earg, LT_SYMBOL)) {
    set_smallenv_bind2_name(fenv, earg);
    set_smallenv_bind2_value(fenv, env);
  }

  return eval(body, fenv);
}

lispobj* standard_applicative_combine(lispobj* combiner,
				      lispobj* combinand, lispobj* env) {
  assert_type(combiner, LT_APPLICATIVE);

  lispobj* underlying = applicative_underlying(combiner);
  lispobj *evaled = nil;

  /* this is essentially map1. move out if it's needed elsewhere
     the difference, of course, is that C has no closures (well, the new one does)
     and i don't want to bother with the void** callback shit if i can avoid it. */
  if (!nullp(combinand)) {
    assert_type(combinand, LT_PAIR);

    lispobj *cur;

    evaled = cur = make_pair(eval(pair_car(combinand), env), nil);
    for(;;) { // could be non-vacuous
      combinand = pair_cdr(combinand);
      if (nullp(combinand)) break;
      assert_type(combinand, LT_PAIR); // these asserts are getting silly
      set_pair_cdr(cur, make_pair(eval(pair_car(combinand), env), nil));
      cur = pair_cdr(cur);
    }
  }

  return combine(underlying, evaled, env);
}

lispobj* standard_smallenv_lookup(lispobj* name, lispobj* smallenv) {
  assert_type(smallenv, LT_SMALLENV);

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
  assert_type(nenv, LT_NENV);

  lispobj **names = nenv_names(nenv), **values = nenv_values(nenv);
  lispobj *parent = nenv_parent(nenv);
  size_t i, fillptr = nenv_fillptr(nenv);

  for(i = 0; i < fillptr; ++i) {
    if (eqp(name, names[i]))
      return values[i];
  }
  if (parent == NULL)
    return error("unbound\n"); // FIXME
  else
    return lookup(name, parent);
}

/* since these are accessible, they gotta return something */
lispobj* standard_smallenv_define(lispobj *name, lispobj *value, lispobj *smallenv) {
  assert_type(smallenv, LT_SMALLENV);

  if (eqp(name, smallenv_bind1_name(smallenv)))
    set_smallenv_bind1_name(smallenv, value);
  else if (eqp(name, smallenv_bind2_name(smallenv)))
    set_smallenv_bind2_name(smallenv, value);
  else
    return error("can't create new bindings in a smallenv\n");

  return inert;
}

lispobj* standard_nenv_define(lispobj *name, lispobj *value, lispobj *nenv) {
  assert_type(nenv, LT_NENV);

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
    return error("No space left in nenv!\n"); // FIXME goddamn
  names[fillptr] = name;
  values[fillptr] = value;
  set_nenv_fillptr(nenv, fillptr + 1);
  return inert;
}

/* INITIALIZATION mostly globals oh no */

/* we have to use standard_nenv_define rather than define since we are,
   after all, defining the ability to define, among other things.
   double metacircles across the sky! */
#define DEFGROUND(STR, VALUE)					\
  standard_nenv_define(find_or_intern(STR), VALUE, ground_environment);
#define DEFGROUNDV(STR, NAME) DEFGROUND(STR, make_fsubr(NAME));
#define DEFGROUNDA(STR, NAME) DEFGROUND(STR, make_applicative(make_fsubr(NAME)));

void populate_ground(void) {
  DEFGROUND("combinators", user_combines);
  DEFGROUND("defines", user_defines);
  DEFGROUND("evaluators", user_evals);
  DEFGROUND("lookupers", user_lookups);

  DEFGROUNDA("car", car_fsubr);
  DEFGROUNDA("cdr", cdr_fsubr);
  DEFGROUNDA("combine", combine_fsubr);
  DEFGROUNDA("cons", cons_fsubr);
  DEFGROUNDA("define!", define_fsubr);
  DEFGROUNDA("eq?", eqp_fsubr);
  DEFGROUNDA("eval", eval_fsubr);
  DEFGROUNDV("$fexpr", fexpr_fsubr);
  DEFGROUNDV("$if", if_fsubr);
  DEFGROUNDA("lookup", lookup_fsubr);
  DEFGROUNDA("read", read_lisp_fsubr);
  DEFGROUNDA("write", write_lisp_fsubr);
  DEFGROUNDA("unwrap", unwrap_fsubr);
  DEFGROUNDA("wrap", wrap_fsubr);
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
}

void initialize_globals(void) {
  /* all these constants are guesses on my part, not too important */
  initialize_package();

  lstdin = make_port(stdin);
  lstdout = make_port(stdout);
  lstderr = make_port(stderr);

  empty_environment = make_nenv(NULL, 0);
  ground_environment = make_nenv(NULL, 271);

  user_evals = make_vector(20); populate_evals();
  user_combines = make_vector(20); populate_combines();
  user_lookups = make_vector(20); populate_lookups();
  user_defines = make_vector(20); populate_defines();
  user_writes = make_vector(20); populate_writes();

  nil = make_singleton(0);
  inert = make_singleton(1);
  ignore = make_singleton(2);
  sharp_t = make_singleton(3);
  sharp_f = make_singleton(4);

  populate_ground();
}

/* STANDARD FSUBRS */

#define FSUBR_ARG(EXPR, VAR)			\
  lispobj *VAR;					\
  if (lispobj_typep((EXPR), LT_PAIR))		\
    VAR = pair_car((EXPR));			\
  else						\
    return error("Not enough arguments\n");

#define FSUBR_END(EXPR) if (!nullp(EXPR)) return error("too many arguments\n");

#define FSUBR_AUX1(VAR1)			\
  FSUBR_ARG(combinand, VAR1);			\
  FSUBR_END(pair_cdr(combinand));

#define FSUBR_AUX2(VAR1, VAR2)				\
  FSUBR_ARG(combinand, VAR1);				\
  FSUBR_ARG(pair_cdr(combinand), VAR2);			\
  FSUBR_END(pair_cdr(pair_cdr(combinand)));
/*
  lispobj *VAR1, *VAR2;					\
  assert_type(combinand, LT_PAIR);			\
  VAR1 = pair_car(combinand);				\
  assert_type(pair_cdr(combinand), LT_PAIR);		\
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
  return eqp(a,b) ? sharp_t : sharp_f;
}

SIMPLE_FSUBR2(eval, eval, form, eenv);

lispobj* fexpr_fsubr(lispobj *combinand, lispobj *env) {
  FSUBR_AUX3(arg, earg, body);

  if (!ignorep(arg)) assert_type(arg, LT_SYMBOL);
  if (!ignorep(earg)) assert_type(earg, LT_SYMBOL);

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

lispobj* read_lisp_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX1(port);
  assert_type(port, LT_PORT);
  return read_lisp(port_stream(port));
}

lispobj* write_lisp_fsubr(lispobj *combinand, lispobj *env) {
  UNUSED(env);
  FSUBR_AUX2(obj, port);
  write_lisp(obj, port);
  return inert;
}

SIMPLE_FSUBR1(unwrap, applicative_underlying, applicative);
SIMPLE_FSUBR1(wrap, make_applicative, combiner);

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
