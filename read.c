#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h> // strerror
#include <stddef.h> // size_t
#include <stdint.h> // SIZE_MAX
#include <stdlib.h> // *alloc

#include "types.h"
#include "error.h"
#include "read.h"
#include "package.h"
#include "fixnum.h"
#include "singleton.h"
#include "string.h"
#include "pair.h"
#include "symbol.h"
#include "lisp.h" // nullp

lispobj* read_lisp(FILE *stream, lisp_package *p) {
  int c;
  while(1) { /* eat whitespace */
    c = getc(stream);
    switch (c) {
    case EOF: return lerror("unexpected EOF");
    case '(': return read_delimited_list(stream, p, ')');
    case ')': return lerror("unexpected )");
    case '#': return read_sharp(stream);
    case '"': return read_string(stream);
    default:
      if (isspace(c)) continue;
      else if (isdigit(c)) {
	ungetc(c, stream);
	return read_integer(stream);
      }
      else {
	ungetc(c, stream);
	return read_symbol(stream, p); }
    }
  }
}

#define VERCHAR(CHAR)					\
  if (getc(stream) != CHAR) return lerror("bad sharp");

lispobj* read_sharp(FILE* stream) {
  int c = getc(stream);
  switch(c) {
  case EOF: return lerror("unexpected EOF");
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
    default: return lerror("bad sharp");
    }
  default: return lerror("bad sharp");
  }
}

lispobj* read_integer(FILE* stream) {
  fixnum in;
  int result;

  errno = 0; // fscanf keeps chugging along but sets ERANGE, if that comes up.
  result = fscanf(stream, FIXNUM_CONVERSION_SPEC, &in);
  if (!errno && (result == 1)) return make_fixnum(in);
  return lerror("problem parsing integer - %d: %s", errno, strerror(errno));
}

lispobj* read_delimited_list(FILE *stream, lisp_package *p, char stop)
{
  lispobj *ret = make_pair(nil, nil), *head = ret;
  int c;
  while(1) {
    c = getc(stream);
    if (c == EOF) return lerror("unexpected EOF");
    if (isspace(c)) continue;
    if (c == stop) return pair_cdr(LO_GET(lisp_pair, *ret));
    if (c == '.') {
      // this may seem weird but I think it's a reasonable way
      //  to catch the error while reading the list fully
      lispobj *rest = read_delimited_list(stream, p, stop);
      if (nullp(rest)) // catch lists with nothing after .
	return lerror("Nothing follows . in list");
      if (head == ret) // catch (. whatever)
	return lerror("Nothing appears before . in list");
      set_pair_cdr(LO_GET(lisp_pair, *head),
		   pair_car(LO_GET(lisp_pair, *rest)));
      if (!nullp(pair_cdr(LO_GET(lisp_pair, *rest))))
	return lerror("More than one object follows . in list");
      else
	return pair_cdr(LO_GET(lisp_pair, *ret));
    }
    ungetc(c, stream);
    set_pair_cdr(LO_GET(lisp_pair, *head),
		 make_pair(read_lisp(stream, p), nil));
    head = pair_cdr(LO_GET(lisp_pair, *head));
  }
}

static inline int isterminating(char c) { return c == ')'; } /* wow */

#define BUFFER_MAX 256
/* originally i wrote something using heap allocation, but why bother
   (threading? argh i don't know) */
static char buf[BUFFER_MAX];

lispobj* read_symbol(FILE *stream, lisp_package *p) {
  int c;
  size_t so_far = 0;

  while((c = getc(stream)) != EOF) {
    if (isspace(c)) {
      buf[so_far] = '\0';
      return find_or_intern(buf, p); /* make_symbol has to copy! */
    }
    if (isterminating(c)) {
      ungetc(c, stream);
      buf[so_far] = '\0';
      return find_or_intern(buf, p);
    }
    buf[so_far] = c;
    ++so_far;
    if (so_far == BUFFER_MAX)
      return lerror("symbol too long");
  }
  return lerror("unexpected EOF");
}

lispobj* read_string(FILE *stream) {
  int c;
  size_t so_far = 0, len = 10;
  char *buf = malloc(len);
  while((c = getc(stream)) != '"') {
    if (c == EOF) lerror("unexpected EOF");
    if (c == '\\') {
      c = getc(stream);
      if (c == EOF) lerror("unexpected EOF");
    }
    buf[so_far++] = c;
    if (so_far == len) {
      if (len > SIZE_MAX - len)
	// maginot line programming
	return lerror("string too long");
      len *= 2;
      buf = realloc(buf, len);
    }
  }
  buf[so_far] = '\0';
  return make_string(buf);
}
