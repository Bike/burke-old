#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h> // strerror, and probably strings at some point

#include "alloc.h"
#include "error.h"
#include "read.h"
#include "lisp.h"
#include "package.h"
#include "types.h" // fixnum

lispobj* read_lisp(FILE *stream) {
  int c;
  while(1) { /* eat whitespace */
    c = getc(stream);
    switch (c) {
    case EOF: error("unexpected EOF\n");
    case '(': return read_delimited_list(stream, ')');
    case '#': return read_sharp(stream);
    default:
      if (isspace(c)) continue;
      else if (isdigit(c)) {
	ungetc(c, stream);
	return read_integer(stream);
      }
      else {
	ungetc(c, stream);
	return read_symbol(stream); }
    }
  }
}

lispobj* read_sharp(FILE* stream) {
#define VERCHAR(CHAR) if (getc(stream) != CHAR) error("bad sharp\n");
  int c = getc(stream);
  switch(c) {
  case EOF: error("unexpected EOF\n");
  case 't': return sharp_t;
  case 'f': return sharp_f;
  case 'i':
    c = getc(stream);
    switch(c) {
    case 'g':
      VERCHAR('n'); VERCHAR('o'); VERCHAR('r'); VERCHAR('e');
      return ignore;
    case 'n':
      VERCHAR('e'); VERCHAR('r'); VERCHAR('t');
      return inert;
    default: error("bad sharp\n");
    }
  default: error("bad sharp\n");
  }
}

lispobj* read_integer(FILE* stream) {
  fixnum in;
  int result;

  result = fscanf(stream, FIXNUM_CONVERSION_SPEC, &in);
  if (result == 1) return make_fixnum(in);
  error("problem parsing integer - %d: %s\n", errno, strerror(errno));
}

lispobj* read_delimited_list(FILE *stream, char stop) {
  lispobj *ret = make_pair(nil, nil), *head = ret;
  int c;
  while(1) {
    c = getc(stream);
    if (c == EOF) error("unexpected EOF\n");
    if (isspace(c)) continue;
    if (c == stop) return pair_cdr(ret);
    ungetc(c, stream);
    set_pair_cdr(head, make_pair(read_lisp(stream), nil));
    head = pair_cdr(head);
  }
}

inline int isterminating(char c) { return c == ')'; } /* wow */

#define BUFFER_MAX 256
/* originally i wrote something using heap allocation, but why bother
   (threading? argh i don't know) */
static char buf[BUFFER_MAX];

lispobj* read_symbol(FILE *stream) {
  int c;
  size_t so_far = 0;

  while((c = getc(stream)) != EOF) {
    if (isspace(c)) {
      buf[so_far] = '\0';
      return find_or_intern(buf); /* make_symbol has to copy! */
    }
    if (isterminating(c)) {
      ungetc(c, stream);
      buf[so_far] = '\0';
      return find_or_intern(buf);
    }
    buf[so_far] = c;
    ++so_far;
    if (so_far == BUFFER_MAX)
      error("symbol too long\n");
  }
  error("unexpected EOF\n");
}
