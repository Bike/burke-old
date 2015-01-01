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

lispobj* read_lisp(FILE *stream, lisp_package *p) {
  int c;
  while(1) { /* eat whitespace */
    c = getc(stream);
    switch (c) {
    case EOF: return lerror("unexpected EOF\n");
    case '(': return read_delimited_list(stream, p, ')');
    case ')': return lerror("unexpected )\n");
    case '#': return read_sharp(stream);
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

lispobj* read_sharp(FILE* stream) {
#define VERCHAR(CHAR) if (getc(stream) != CHAR) return lerror("bad sharp\n");
  int c = getc(stream);
  switch(c) {
  case EOF: return lerror("unexpected EOF\n");
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
    default: return lerror("bad sharp\n");
    }
  default: return lerror("bad sharp\n");
  }
}

lispobj* read_integer(FILE* stream) {
  fixnum in;
  int result;

  result = fscanf(stream, FIXNUM_CONVERSION_SPEC, &in);
  if (result == 1) return make_fixnum(in);
  return lerror("problem parsing integer - %d: %s\n", errno, strerror(errno));
}

lispobj* read_delimited_list(FILE *stream, lisp_package *p, char stop)
{
  lispobj *ret = make_pair(nil, nil), *head = ret;
  int c;
  while(1) {
    c = getc(stream);
    if (c == EOF) return lerror("unexpected EOF\n");
    if (isspace(c)) continue;
    if (c == stop) return pair_cdr(ret);
    if (c == '.') {
      // this may seem weird but I think it's a reasonable way
      //  to catch the error while reading the list fully
      lispobj *rest = read_delimited_list(stream, p, stop);
      if (nullp(rest)) // catch lists with nothing after .
	return lerror("Nothing follows . in list\n");
      if (head == ret) // catch (. whatever)
	return lerror("Nothing appears before . in list\n");
      set_pair_cdr(head, pair_car(rest));
      if (!nullp(pair_cdr(rest)))
	return lerror("More than one object follows . in list\n");
      else
	return pair_cdr(ret);
    }
    ungetc(c, stream);
    set_pair_cdr(head, make_pair(read_lisp(stream, p), nil));
    head = pair_cdr(head);
  }
}

inline int isterminating(char c) { return c == ')'; } /* wow */

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
      return lerror("symbol too long\n");
  }
  return lerror("unexpected EOF\n");
}
