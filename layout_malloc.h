#ifndef BURKE_LAYOUT_MALLOC_H
#define BURKE_LAYOUT_MALLOC_H

#include <stdio.h> // FILE
#include "types.h" // for lispobj, lisptype

typedef struct lisp_pair {
  lisptag tag;
  lispobj *car, *cdr;
} lisp_pair;

typedef struct lisp_fixnum {
  lisptag tag;
  fixnum num;
} lisp_fixnum;

typedef struct lisp_symbol {
  lisptag tag;
  /*lisp_vector *name;*/
  char name[1]; // not lisp accessible, alas
} lisp_symbol;

typedef struct lisp_vector {
  // specialized vectors gone have to be elsewhere
  lisptag tag;
  fixnum length;
  lispobj *data[1];
} lisp_vector;

typedef struct lisp_character {
  lisptag tag;
  char id; // TODO: unicode
} lisp_character;

typedef struct lisp_singleton {
  lisptag tag;
  unsigned short id; // unique ID for each singleton
} lisp_singleton;

typedef struct lisp_fsubr {
  lisptag tag;
  fsubr_funptr fun;
} lisp_fsubr;

/* fexprs are very primitive because i want to bother with that in lisp, not C */
typedef struct lisp_fexpr {
  lisptag tag;
  lispobj* arg; // name of sole argument (i dun wanna destructure in C) or #ignore
  lispobj* earg; // name of environment argument or #ignore
  lispobj* env; // closure environment
  lispobj* body;
} lisp_fexpr;

typedef struct lisp_smallenv {
  lisptag tag;
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
  lisptag tag;
  lispobj* parent;
  size_t length;
  size_t fillptr;
  lispobj** names;
  lispobj** values;
} lisp_nenv;

typedef struct lisp_wrapped {
  lisptag tag;
  lispobj* underlying;
} lisp_wrapped;

typedef struct lisp_port {
  lisptag tag;
  FILE *stream;
} lisp_port;

// for dealing with tags within lisp
typedef struct lisp_mtag {
  lisptag tag;
  lisptag mtag;
} lisp_mtag;

// everything without one of the above IDs: defined by the user.
typedef struct lisp_custom {
  // essentially a short name->value map.
  // a packed struct could be made by passing a vector around,
  // which would essentially be this but without the names.
  lisptag tag;
  long num_of_fields;
  lispobj* names;
  lispobj* values;
} lisp_custom;

typedef struct lisp_package {
  lisptag tag;
  size_t size;
  size_t fill;
  lispobj** symbols; // can't resize with a flexible w/o more indirect
} lisp_package;

typedef struct tag_hack {
  lisptag tag;
} tag_hack;

union lispobj {
  tag_hack tag;
  lisp_pair pair;
  lisp_fixnum fixnum;
  lisp_symbol symbol;
  lisp_vector vector;
  lisp_character character;
  lisp_fsubr fsubr;
  lisp_fexpr fexpr;
  lisp_smallenv smallenv;
  lisp_nenv nenv;
  lisp_wrapped wrapped;
  lisp_custom custom;
  lisp_singleton singleton;
  lisp_package package;
  lisp_port port;
  lisp_mtag mtag;
};

#endif /* guard */
