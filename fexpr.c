#include "fexpr.h"
#include "alloc.h"
#include "smallenv.h"
#include "lisp.h"
#include "symbol.h"

lispobj* fexpr_arg(lisp_fexpr* fexpr)
{ return fexpr->arg; }
lispobj* fexpr_earg(lisp_fexpr* fexpr)
{ return fexpr->earg; }
lispobj* fexpr_env(lisp_fexpr* fexpr)
{ return fexpr->env; }
lispobj* fexpr_body(lisp_fexpr* fexpr)
{ return fexpr->body; }

lispobj* fexpr_combine(lisp_fexpr* fexpr,
		       lispobj* combinand, lispobj* env) {
  lispobj* senv = make_smallenv(fexpr->env);
  lisp_smallenv* fenv = LO_GET(lisp_smallenv, *senv);

  /* non-symbols should be understood at fexpr creation time */
  if (lispobj_tagp(fexpr->arg, LT_SYMBOL)) {
    set_smallenv_bind1_name(fenv, fexpr->arg);
    set_smallenv_bind1_value(fenv, combinand);
  }
  if (lispobj_tagp(fexpr->earg, LT_SYMBOL)) {
    set_smallenv_bind2_name(fenv, fexpr->earg);
    set_smallenv_bind2_value(fenv, env);
  }

  return eval(fexpr->body, senv);
}

lispobj* make_fexpr(lispobj* arg, lispobj* earg,
		    lispobj* env, lispobj* body) {
  lispobj* ret;
  size_t size = ALIGN_LO(lisp_fexpr);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_FEXPR;
    LO_GET(lisp_fexpr, *ret)->arg = arg;
    LO_GET(lisp_fexpr, *ret)->earg = earg;
    LO_GET(lisp_fexpr, *ret)->env = env;
    LO_GET(lisp_fexpr, *ret)->body = body;
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
