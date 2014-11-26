#ifndef BURKE_TYPES_H
#define BURKE_TYPES_H

#include <stddef.h>
#include <stdio.h> /* for ports */

/* LISP TYPES */

// incomplete type, to be completed by the allocation code.
union lispobj;
typedef union lispobj lispobj;

typedef int lisptag;

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
  LT_MTAG,
}; // other LTs may be defined by the allocation code

inline lisptag tagof_lispobj(lispobj*);
inline int lispobj_tagp(lispobj*, lisptag);

#define DECREADER(TYPENAME, REF) inline lispobj* TYPENAME##_##REF(lispobj*);
#define DECWRITER(TYPENAME, REF) inline void set_##TYPENAME##_##REF(lispobj*,lispobj*);
#define DECACCESS(TYPENAME, REF) DECREADER(TYPENAME, REF); DECWRITER(TYPENAME, REF);

inline fixnum fixnum_num(lispobj*);
//DECREADER(symbol, name);
inline char* symbol_name(lispobj*);
inline fsubr_funptr fsubr_fun(lispobj*);
DECREADER(fexpr, arg);
DECREADER(fexpr, earg);
DECREADER(fexpr, env);
DECREADER(fexpr, body);
DECREADER(smallenv, parent);
DECACCESS(smallenv, bind1_name);
DECACCESS(smallenv, bind1_value);
DECACCESS(smallenv, bind2_name);
DECACCESS(smallenv, bind2_value);
DECREADER(nenv, parent);
//DECREADER(nenv, length);
inline size_t nenv_length(lispobj*);
inline size_t nenv_fillptr(lispobj*);
inline void set_nenv_fillptr(lispobj*,size_t);
inline lispobj** nenv_names(lispobj*);
inline lispobj** nenv_values(lispobj*);
DECREADER(applicative, underlying);
DECACCESS(pair, car);
DECACCESS(pair, cdr);
inline FILE* port_stream(lispobj*);
inline lisptag mtag_mtag(lispobj*);

inline fixnum vlength(lispobj*);
lispobj* vref(lispobj*, fixnum);
void set_vref(lispobj*, fixnum, lispobj*);

int eqp(lispobj*, lispobj*);

#endif // guard
