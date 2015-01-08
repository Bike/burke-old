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

  if (undefinedp(user_write)) {
    fputs("#<unknown (tag ", port_stream(port));
    fprintf(port_stream(port), TAG_CONVERSION_SPEC, objtag);
    fputs(")>", port_stream(port));
  }
  else
    combine(user_write, list(2, obj, port), empty_environment);
}

void write_singleton(lispobj *singleton, lispobj *port) {
  FILE *stream;

  check_tag(singleton, LT_SINGLETON);
  check_tag(port, LT_PORT);

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
  check_tag(pair, LT_PAIR);
  check_tag(port, LT_PORT);

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
  check_tag(fixnum, LT_FIXNUM);
  check_tag(port, LT_PORT);
  fprintf(port_stream(port), FIXNUM_CONVERSION_SPEC, fixnum_num(fixnum));
}

void write_symbol(lispobj *symbol, lispobj *port) {
  check_tag(symbol, LT_SYMBOL);
  check_tag(port, LT_PORT);
  fputs(symbol_name(symbol), port_stream(port));
}

void write_string(lispobj *string, lispobj *port) {
  FILE *stream;
  check_tag(string, LT_STRING);
  check_tag(port, LT_PORT);

  stream = port_stream(port);
  putc('"', stream);
  fputs(string_string(string), stream);
  putc('"', stream);
}

void write_vector(lispobj *vector, lispobj *port) {
  fixnum i, len;
  FILE *stream;
  check_tag(vector, LT_VECTOR);
  check_tag(port, LT_PORT);

  stream = port_stream(port);

  len = vector_length(vector);

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

  check_tag(fsubr, LT_FSUBR);
  check_tag(port, LT_PORT);

  stream = port_stream(port);

  fputs("#<fsubr {", stream);
  fprintf(stream, "%p", fsubr_fun(fsubr));
  fputs("}>", stream);
}

void write_mtag(lispobj *mtag, lispobj *port) {
  FILE *stream;

  check_tag(mtag, LT_MTAG);
  check_tag(port, LT_PORT);

  stream = port_stream(port);

  fputs("#<mtag (", stream);
  fprintf(stream, TAG_CONVERSION_SPEC, mtag_mtag(mtag));
  fputs(")>", stream);
}
