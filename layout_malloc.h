#ifndef BURKE_LAYOUT_MALLOC_H
#define BURKE_LAYOUT_MALLOC_H

#include <stdio.h> // FILE
#include "types.h" // for lispobj, lisptype

typedef struct lisp_pair {
  lisptype type;
  lispobj *car, *cdr;
} lisp_pair;

typedef struct lisp_fixnum {
  lisptype type;
  fixnum num;
} lisp_fixnum;

typedef struct lisp_symbol {
  lisptype type;
  /*lisp_vector *name;*/
  char name[1]; // not lisp accessible, alas
} lisp_symbol;

typedef struct lisp_vector {
  // specialized vectors gone have to be elsewhere
  lisptype type;
  fixnum length;
  lispobj *data[1];
} lisp_vector;

typedef struct lisp_character {
  lisptype type;
  char id; // TODO: unicode
} lisp_character;

typedef struct lisp_singleton {
  lisptype type;
  unsigned short id; // unique ID for each singleton
} lisp_singleton;

typedef struct lisp_fsubr {
  lisptype type;
  fsubr_funptr fun;
} lisp_fsubr;

/* fexprs are very primitive because i want to bother with that in lisp, not C */
typedef struct lisp_fexpr {
  lisptype type;
  lispobj* arg; // name of sole argument (i dun wanna destructure in C) or #ignore
  lispobj* earg; // name of environment argument or #ignore
  lispobj* env; // closure environment
  lispobj* body;
} lisp_fexpr;

typedef struct lisp_smallenv {
  lisptype type;
  lispobj* parent;
  lispobj* bind1_name;
  lispobj* bind1_value;
  lispobj* bind2_name;
  lispobj* bind2_value;
} lisp_smallenv;

typedef struct lisp_nenv {
  /* a dirt simple associative map.
     TODO: at least make it binary search, symbols have order
     maybe separate out the map part into an independent thing? */
  lisptype type;
  lispobj* parent;
  size_t length;
  size_t fillptr;
  lispobj** names;
  lispobj** values;
} lisp_nenv;

typedef struct lisp_applicative {
  lisptype type;
  lispobj* underlying;
} lisp_applicative;

typedef struct lisp_port {
  lisptype type;
  FILE *stream;
} lisp_port;

// everything without one of the above IDs: defined by the user.
typedef struct lisp_custom {
  // essentially a short name->value map.
  // a packed struct could be made by passing a vector around,
  // which would essentially be this but without the names.
  lisptype type;
  long num_of_fields;
  lispobj* names;
  lispobj* values;
} lisp_custom;

typedef struct type_hack {
  lisptype type;
} type_hack;

union lispobj {
  type_hack type;
  lisp_pair pair;
  lisp_fixnum fixnum;
  lisp_symbol symbol;
  lisp_vector vector;
  lisp_character character;
  lisp_fsubr fsubr;
  lisp_fexpr fexpr;
  lisp_smallenv smallenv;
  lisp_nenv nenv;
  lisp_applicative applicative;
  lisp_custom custom;
  lisp_singleton singleton;
  lisp_port port;
};

#endif /* guard */
