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

#define DEFREADER(OTYPE, VTYPE, OTYPEID, REF)		   \
  inline VTYPE OTYPE##_##REF(lispobj* obj) {		   \
    check_tag(obj, OTYPEID);				   \
    return obj->OTYPE.REF;				   \
  }
#define DEFWRITER(OTYPE, VTYPE, OTYPEID, REF)			\
  inline void set_##OTYPE##_##REF(lispobj* obj, VTYPE value) {	\
    check_tag(obj, OTYPEID);					\
    obj->OTYPE.REF = value;					\
  }
#define DEFACCESS(OTYPE, VTYPE, OTYPEID, REF)		\
  DEFREADER(OTYPE, VTYPE, OTYPEID, REF);		\
  DEFWRITER(OTYPE, VTYPE, OTYPEID, REF);

//DEFREADER(symbol, LT_SYMBOL, name);
DEFREADER(symbol, char*, LT_SYMBOL, name);
DEFREADER(fsubr, fsubr_funptr, LT_FSUBR, fun);

DEFREADER(fixnum, fixnum, LT_FIXNUM, num);

DEFREADER(fexpr, lispobj*, LT_FEXPR, arg);
DEFREADER(fexpr, lispobj*, LT_FEXPR, earg);
DEFREADER(fexpr, lispobj*, LT_FEXPR, env);
DEFREADER(fexpr, lispobj*, LT_FEXPR, body);

DEFREADER(smallenv, lispobj*, LT_SMALLENV, parent);
DEFACCESS(smallenv, lispobj*, LT_SMALLENV, bind1_name);
DEFACCESS(smallenv, lispobj*, LT_SMALLENV, bind1_value);
DEFACCESS(smallenv, lispobj*, LT_SMALLENV, bind2_name);
DEFACCESS(smallenv, lispobj*, LT_SMALLENV, bind2_value);

DEFREADER(nenv, lispobj*, LT_NENV, parent);
DEFREADER(nenv, size_t, LT_NENV, length);
DEFACCESS(nenv, size_t, LT_NENV, fillptr);
DEFREADER(nenv, lispobj**, LT_NENV, names);
DEFREADER(nenv, lispobj**, LT_NENV, values);

inline lispobj* wrapped_underlying(lispobj* obj) {
  assert(obj); // can't assert tag if there's multiple
  return obj->wrapped.underlying;
}

DEFACCESS(pair, lispobj*, LT_PAIR, car);
DEFACCESS(pair, lispobj*, LT_PAIR, cdr);

DEFREADER(port, FILE*, LT_PORT, stream);
DEFREADER(string, char*, LT_STRING, string);
DEFREADER(mtag, lisptag, LT_MTAG, mtag);
DEFREADER(vector, fixnum, LT_VECTOR, length);

lispobj* vref(lispobj* vector, fixnum index) {
  check_tag(vector, LT_VECTOR);
  if ((index >= vector_length(vector)) || (index < 0)) {
    // write_vector(vector, lstderr);
    fputc('\n', stderr);
    return lerror("out of bounds: index %ld\n", index); // FIXME: better error handling
  } else
    return (vector->vector.data)[index];
}

void set_vref(lispobj *vector, fixnum index, lispobj *value) {
  check_tag(vector, LT_VECTOR);
  if ((index > vector_length(vector)) || (index < 0))
    lerror("out of bounds\n");
  else
    (vector->vector.data)[index] = value;
}

int eqp(lispobj* a, lispobj* b) {
  return a == b;
}
