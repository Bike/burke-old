#include <stdio.h>
#include "write.h"
#include "types.h"
#include "error.h"
#include "lisp.h"

/* i've been vacillating about what should have ports instead of streams,
   and such. conventions are hard to decide on! */

void write_lisp(lispobj *obj, lispobj *port) {
  if (undefinedp(obj)) { // KLUDGE
    fputs("#<null>", port_stream(port));
    return;
  }
  lisptag objtag = tagof_lispobj(obj);
  lispobj *user_write = vref(user_writes, objtag);

  if (undefinedp(user_write))
    fputs("#<unknown>", port_stream(port));
  else
    combine(user_write, list(2, obj, port), empty_environment);
}

void write_singleton(lispobj *singleton, lispobj *port) {
  FILE *stream;

  assert_tag(singleton, LT_SINGLETON);
  assert_tag(port, LT_PORT);

  stream = port_stream(port);

  /* This could be yet another layer of dispatch.
   * Maybe to just a vector of strings to print, rather than whole functions.
   * I wonder about users though. Gotta null terminate that shit?
   */

  if (singleton == nil)
    fputs("()", stream);
  else if (singleton == inert)
    fputs("#inert", stream);
  else if (singleton == ignore)
    fputs("#ignore", stream);
  else if (singleton == sharp_t)
    fputs("#t", stream);
  else if (singleton == sharp_f)
    fputs("#f", stream);
  else
    fputs("#<unknown singleton>", stream);
}

void write_pair(lispobj *pair, lispobj *port) {
  lispobj *cdr;
  FILE *stream;
  assert_tag(pair, LT_PAIR);
  assert_tag(port, LT_PORT);

  stream = port_stream(port);

  putc('(', stream);
  while (1) {
    write_lisp(pair_car(pair), port);
    cdr = pair_cdr(pair);
    switch(tagof_lispobj(cdr)) {
    case LT_PAIR:
      putc(' ', stream);
      pair = cdr;
      continue; /* while (1) */
    case LT_SINGLETON:
      if (cdr == nil) {
	putc(')', stream);
	return;
      }
      /* fall through for other singletons */
    default: /* dotted list */
      fputs(" . ", stream);
      write_lisp(cdr, port);
      putc(')', stream);
      return;
    }
  }
}

void write_fixnum(lispobj *fixnum, lispobj *port) {
  // UNUSED(stream);
  assert_tag(fixnum, LT_FIXNUM);
  assert_tag(port, LT_PORT);
  fprintf(port_stream(port), FIXNUM_CONVERSION_SPEC, fixnum_num(fixnum));
}

void write_symbol(lispobj *symbol, lispobj *port) {
  assert_tag(symbol, LT_SYMBOL);
  assert_tag(port, LT_PORT);
  fputs(symbol_name(symbol), port_stream(port));
}

void write_vector(lispobj *vector, lispobj *port) {
  fixnum i, len;
  FILE *stream;
  assert_tag(vector, LT_VECTOR);
  assert_tag(port, LT_PORT);

  stream = port_stream(port);

  len = vlength(vector);

  fputs("#(", stream);
  for(i = 0; i < len - 1; ++i) {
    write_lisp(vref(vector, i), port);
    putc(' ', stream);
  }
  write_lisp(vref(vector, len - 1), port);
  putc(')', stream);
}

void write_fsubr(lispobj *fsubr, lispobj *port) {
  FILE *stream;

  assert_tag(fsubr, LT_FSUBR);
  assert_tag(port, LT_PORT);

  stream = port_stream(port);

  fputs("#<FSUBR {", stream);
  fprintf(stream, "%p", fsubr_fun(fsubr));
  fputs("}>", stream);
}
