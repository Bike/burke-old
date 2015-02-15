#ifndef BURKE_TYPES_H
#define BURKE_TYPES_H

#include <stddef.h>
#include <stdio.h> /* for ports */

/* LISP TYPES */

// incomplete type, to be completed by the allocation code.
union lispobj;
typedef union lispobj lispobj;

typedef int lisptag;
#define TAG_MAX INT_MAX
#define TAG_CONVERSION_SPEC "%d"

typedef long fixnum; // in case i want to change it later
#define FIXNUM_CONVERSION_SPEC "%ld" // for printf and scanf

/* fuck funptr syntax, holy shit */
typedef lispobj*(*fsubr_funptr)(lispobj*, lispobj*);

enum {
  // LT = "lisp tag"
  LT_FIXNUM = 0,
  LT_PAIR,
  LT_SYMBOL,
  LT_VECTOR,
  LT_CHARACTER,
  LT_FSUBR, // built in operator
  LT_FEXPR, // user defined operator
  LT_SMALLENV, // super simple environment just for fexpr use
  LT_NENV,
  LT_WRAPPED,
  LT_APPLICATIVE,
  LT_SINGLETON, // (), #t, #f, etc
  LT_PORT,
  LT_PACKAGE,
  LT_STRING,
  LT_MTAG,
}; // other LTs may be defined by the allocation code

extern lisptag next_tag; // defined in layout

inline lisptag tagof_lispobj(lispobj*);
inline int lispobj_tagp(lispobj*, lisptag);

#define DECREADER(OTYPE, VTYPE, REF) inline VTYPE OTYPE##_##REF(lispobj*);
#define DECWRITER(OTYPE, VTYPE, REF) inline void set_##OTYPE##_##REF(lispobj*,VTYPE);
#define DECACCESS(OTYPE, VTYPE, REF) DECREADER(OTYPE, VTYPE, REF); DECWRITER(OTYPE, VTYPE, REF);

DECREADER(fixnum, fixnum, num);
DECREADER(symbol, char*, name);
DECREADER(fsubr, fsubr_funptr, fun);
DECREADER(fexpr, lispobj*, arg);
DECREADER(fexpr, lispobj*, earg);
DECREADER(fexpr, lispobj*, env);
DECREADER(fexpr, lispobj*, body);
DECREADER(smallenv, lispobj*, parent);
DECACCESS(smallenv, lispobj*, bind1_name);
DECACCESS(smallenv, lispobj*, bind1_value);
DECACCESS(smallenv, lispobj*, bind2_name);
DECACCESS(smallenv, lispobj*, bind2_value);
DECREADER(nenv, lispobj*, parent);
DECREADER(nenv, size_t, length);
DECACCESS(nenv, size_t, fillptr);
DECREADER(nenv, lispobj**, names);
DECREADER(nenv, lispobj**, values);
DECREADER(wrapped, lispobj*, underlying);
DECACCESS(pair, lispobj*, car);
DECACCESS(pair, lispobj*, cdr);
DECREADER(port, FILE*, stream);
DECREADER(string, char*, string);
DECREADER(mtag, lisptag, mtag);

DECREADER(vector, fixnum, length);
lispobj* vref(lispobj*, fixnum);
void set_vref(lispobj*, fixnum, lispobj*);

int eqp(lispobj*, lispobj*);

#endif // guard
