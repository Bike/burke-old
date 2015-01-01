#include "types.h"
#include "layout_malloc.h"
#include "error.h"
#include "write.h" // FIXME
#include "lisp.h" // maybe FIXME? for lstd{in,out,err} at least
#include <stdio.h> // FIXME JESUS

/* LISP FROM C ACCESSORS AND SUCH
   This should be the only place using -> or . on lispobjs.
   Other than the allocation? maybe? */

lisptag next_tag = LT_MTAG+1;

inline lisptag tagof_lispobj(lispobj* obj) {
  return obj->tag.tag;
}
inline int lispobj_tagp(lispobj* obj, lisptag tag) {
  return tagof_lispobj(obj) == tag;
}

#define DEFREADER(TYPENAME, TYPEID, REF)	   \
  inline lispobj* TYPENAME##_##REF(lispobj* obj) { \
    assert_tag(obj, TYPEID);			   \
    return obj->TYPENAME.REF;			   \
  }
#define DEFWRITER(TYPENAME, TYPEID, REF)				\
  inline void set_##TYPENAME##_##REF(lispobj* obj, lispobj* value) {	\
    assert_tag(obj, TYPEID);						\
    obj->TYPENAME.REF = value;						\
  }
#define DEFACCESS(TYPENAME, TYPEID, REF)	\
  DEFREADER(TYPENAME, TYPEID, REF);		\
  DEFWRITER(TYPENAME, TYPEID, REF);

//DEFREADER(symbol, LT_SYMBOL, name);
inline char* symbol_name(lispobj *obj) {
  assert_tag(obj, LT_SYMBOL);
  return obj->symbol.name;
}

inline fsubr_funptr fsubr_fun(lispobj *obj) {
  assert_tag(obj, LT_FSUBR);
  return obj->fsubr.fun;
}

inline fixnum fixnum_num(lispobj *obj) {
  assert_tag(obj, LT_FIXNUM);
  return obj->fixnum.num;
}

DEFREADER(fexpr, LT_FEXPR, arg);
DEFREADER(fexpr, LT_FEXPR, earg);
DEFREADER(fexpr, LT_FEXPR, env);
DEFREADER(fexpr, LT_FEXPR, body);

DEFREADER(smallenv, LT_SMALLENV, parent);
DEFACCESS(smallenv, LT_SMALLENV, bind1_name);
DEFACCESS(smallenv, LT_SMALLENV, bind1_value);
DEFACCESS(smallenv, LT_SMALLENV, bind2_name);
DEFACCESS(smallenv, LT_SMALLENV, bind2_value);

DEFREADER(nenv, LT_NENV, parent);
//DEFREADER(nenv, LT_NENV, length);
inline size_t nenv_length(lispobj* obj) {
  assert_tag(obj, LT_NENV);
  return obj->nenv.length;
}
inline size_t nenv_fillptr(lispobj* obj) {
  assert_tag(obj, LT_NENV);
  return obj->nenv.fillptr;
}
inline void set_nenv_fillptr(lispobj* obj, size_t value) {
  assert_tag(obj, LT_NENV);
  obj->nenv.fillptr = value;
}
inline lispobj** nenv_names(lispobj* obj) {
  assert_tag(obj, LT_NENV);
  return obj->nenv.names;
}
inline lispobj** nenv_values(lispobj* obj) {
  assert_tag(obj, LT_NENV);
  return obj->nenv.values;
}

inline lispobj* unwrap(lispobj* obj) {
  assert(obj); // can't assert tag if there's multiple
  return obj->wrapped.underlying;
}
inline lispobj* applicative_underlying(lispobj* obj) {
  assert_tag(obj, LT_APPLICATIVE);
  return obj->wrapped.underlying;
}

DEFACCESS(pair, LT_PAIR, car);
DEFACCESS(pair, LT_PAIR, cdr);

inline FILE* port_stream(lispobj* obj) {
  assert_tag(obj, LT_PORT);
  return obj->port.stream;
}

inline lisptag mtag_mtag(lispobj* obj) {
  assert_tag(obj, LT_MTAG);
  return obj->mtag.mtag;
}

inline fixnum vlength(lispobj* vector) {
  assert_tag(vector, LT_VECTOR);
  return vector->vector.length;
}

lispobj* vref(lispobj* vector, fixnum index) {
  assert_tag(vector, LT_VECTOR);
  if ((index >= vlength(vector)) || (index < 0)) {
    // write_vector(vector, lstderr);
    fputc('\n', stderr);
    return lerror("out of bounds: index %ld\n", index); // FIXME: better error handling
  } else
    return (vector->vector.data)[index];
}

void set_vref(lispobj *vector, fixnum index, lispobj *value) {
  assert_tag(vector, LT_VECTOR);
  if ((index > vlength(vector)) || (index < 0))
    lerror("out of bounds\n");
  else
    (vector->vector.data)[index] = value;
}

int eqp(lispobj* a, lispobj* b) {
  return a == b;
}
