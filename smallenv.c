#include "smallenv.h"
#include "lisp.h"
#include "alloc.h" // alloc, but also eqp, eventually, maybe

lispobj* smallenv_parent(lisp_smallenv* s)
{ return s->parent; }
lispobj* smallenv_bind1_name(lisp_smallenv* s)
{ return s->bind1_name; }
lispobj* smallenv_bind1_value(lisp_smallenv* s)
{ return s->bind1_value; }
lispobj* smallenv_bind2_name(lisp_smallenv* s)
{ return s->bind2_name; }
lispobj* smallenv_bind2_value(lisp_smallenv* s)
{ return s->bind2_value; }
void set_smallenv_bind1_name(lisp_smallenv* s, lispobj* v)
{ s->bind1_name = v; }
void set_smallenv_bind1_value(lisp_smallenv* s, lispobj* v)
{ s->bind1_value = v; }
void set_smallenv_bind2_name(lisp_smallenv* s, lispobj* v)
{ s->bind2_name = v; }
void set_smallenv_bind2_value(lisp_smallenv* s, lispobj* v)
{ s->bind2_value = v; }

lispobj* smallenv_lookup(lisp_smallenv* smallenv, lispobj* name) {
  if (eqp(name, smallenv->bind1_name))
    return smallenv->bind1_value;
  else if (eqp(name, smallenv->bind2_name))
    return smallenv->bind2_value;

  /* smallenvs should always have a parent due to how they're used */
  assert(smallenv->parent);
  return lookup(name, smallenv->parent);
}
void smallenv_define(lisp_smallenv* smallenv,
		     lispobj* name, lispobj* value) {
  if (eqp(name, smallenv->bind1_name))
    smallenv->bind1_value = value;
  else if (eqp(name, smallenv->bind2_name))
    smallenv->bind2_value = value;

  lerror("can't create new bindings in a smallenv");
}

lispobj* make_smallenv(lispobj* parent) {
  lispobj* ret;
  size_t size = ALIGN_LO(lisp_smallenv);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_SMALLENV;
    LO_GET(lisp_smallenv, *ret)->parent = parent;
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
