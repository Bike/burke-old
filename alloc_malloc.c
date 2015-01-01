/* Really stupid implementation of allocations.
   Use C alloc, with no free.
   Stupid. */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h> // for make_port
#include "alloc.h"
#include "error.h"
#include "types.h"
#include "layout_malloc.h"

/* shitty fucking malloc macro */
#define SMALLOC(S) if (!(ret = malloc(S))) return lerror("OOM\n");

lispobj* make_symbol(const char* str, size_t len) {
  lisp_symbol *ret;

  ++len; // null termination

  /* sizeof(char) == 1, but i think this is clearer, honestly. */
  SMALLOC(offsetof(lisp_symbol, name) + len*sizeof(char));
  ret->tag = LT_SYMBOL;
  strncpy(ret->name, str, len-1);
  ret->name[len-1] = '\0';
  return (lispobj*)ret;
}

lispobj* make_pair(lispobj *car, lispobj *cdr) {
  lisp_pair *ret;

  SMALLOC(sizeof(lisp_pair));
  ret->tag = LT_PAIR;
  ret->car = car; ret->cdr = cdr;
  return (lispobj*)ret;
}

lispobj* make_smallenv(lispobj* parent) {
  lisp_smallenv *ret;
  
  SMALLOC(sizeof(lisp_smallenv));
  ret->tag = LT_SMALLENV;
  ret->parent = parent;
  return (lispobj*)ret;
}

lispobj* make_nenv(lispobj* parent, size_t size) {
  lisp_nenv *ret;

  SMALLOC(sizeof(lisp_nenv));
  ret->tag = LT_NENV;
  ret->parent = parent;
  ret->length = size;
  ret->fillptr = 0;
  if (!(ret->names = malloc(size*sizeof(lispobj*))))
    return lerror("OOM\n");
  if (!(ret->values = malloc(size*sizeof(lispobj*))))
    return lerror("OOM\n");
  return (lispobj*)ret;
}

lispobj* make_wrapped(lisptag tag, lispobj *underlying) {
  lisp_wrapped *ret;

  SMALLOC(sizeof(lisp_wrapped));
  ret->tag = tag;
  ret->underlying = underlying;
  return (lispobj*)ret;
}

lispobj* make_port(FILE *stream) {
  lisp_port *ret;

  SMALLOC(sizeof(lisp_port));
  ret->tag = LT_PORT;
  ret->stream = stream;
  return (lispobj*)ret;
}

lispobj* make_fsubr(fsubr_funptr fun) {
  lisp_fsubr *ret;

  SMALLOC(sizeof(lisp_fsubr));
  ret->tag = LT_FSUBR;
  ret->fun = fun;
  return (lispobj*)ret;
}

lispobj* make_singleton(unsigned short id) {
  lisp_singleton *ret;

  SMALLOC(sizeof(lisp_singleton));
  ret->tag = LT_SINGLETON;
  ret->id = id;
  return (lispobj*)ret;
}

lispobj* make_fexpr(lispobj *arg, lispobj *earg, lispobj *env, lispobj *body) {
  lisp_fexpr *ret;

  SMALLOC(sizeof(lisp_fexpr));
  ret->tag = LT_FEXPR;
  ret->arg = arg; ret->earg = earg; ret->env = env; ret->body = body;
  return (lispobj*)ret;
}

lispobj* make_vector(fixnum len) {
  lisp_vector *ret;

  SMALLOC(offsetof(lisp_vector, data) + len*sizeof(lispobj*));
  ret->tag = LT_VECTOR;
  ret->length = len;
  /* could remove the following For Speed, possibly */
  memset(ret->data, 0, len*sizeof(lispobj*));
  return (lispobj*)ret;
}

lispobj* make_fixnum(fixnum num) {
  lisp_fixnum *ret;

  SMALLOC(sizeof(lisp_fixnum));
  ret->tag = LT_FIXNUM;
  ret->num = num;
  return (lispobj*)ret;
}

lispobj* make_mtag(lisptag tag) {
  lisp_mtag *ret;

  SMALLOC(sizeof(lisp_mtag));
  ret->tag = LT_MTAG;
  ret->mtag = tag;
  return (lispobj*)ret;
}
