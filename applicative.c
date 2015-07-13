#include "applicative.h"
#include "lisp.h"
#include "wrapped.h"

// Poor man's closure
// FIXME: Move this to lisp.*
lispobj* reval(lispobj* form, void* env) {
  return eval(form, env);
}

lispobj* applicative_combine(lisp_wrapped* app,
			     lispobj* combinand, lispobj* env) {
  return combine(unwrap(app),
		 map1(reval, combinand, env),
		 env);
}
