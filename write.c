#include <stdio.h>
#include "write.h"
#include "types.h"
#include "error.h"
#include "lisp.h"
#include "port.h"
#include "vector.h"
#include "singleton.h"
#include "pair.h"
#include "fixnum.h"
#include "symbol.h"
#include "string.h"
#include "fsubr.h"
#include "mtag.h"

/* i've been vacillating about what should have ports instead of
 * streams, and such. conventions are hard to decide on! */

void write_lisp(lispobj *obj, lispobj *port) {
  if (undefinedp(obj)) { // KLUDGE
    fputs("#<null>", port_stream(LO_GET(lisp_port, *port)));
    return;
  }
  lisptag objtag = LO_TAG(*obj);
  lispobj *user_write = vref(LO_GET(lisp_vector, *user_writes), objtag);
  
  if (undefinedp(user_write)) {
    fputs("#<unknown (tag ", port_stream(LO_GET(lisp_port, *port)));
    fprintf(port_stream(LO_GET(lisp_port, *port)),
	    TAG_CONVERSION_SPEC, objtag);
    fputs(")>", port_stream(LO_GET(lisp_port, *port)));
  }
  else
    combine(user_write, list(2, obj, port), empty_environment);
}

// FIXME (along with the cases in the general function)
void write_singleton(lispobj *singleton, lispobj *port) {
  FILE *stream;

  check_tag(port, LT_PORT);

  stream = port_stream(LO_GET(lisp_port, *port));

  /* This could be yet another layer of dispatch.
   * Maybe to just a vector of strings to print, rather than whole functions.
   * I wonder about users though. Gotta null terminate that shit?
   */

  // FIXME: eqp for ==
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

  stream = port_stream(LO_GET(lisp_port, *port));

  putc('(', stream);
  while (1) {
    write_lisp(pair_car(LO_GET(lisp_pair, *pair)), port);
    cdr = pair_cdr(LO_GET(lisp_pair, *pair));
    switch(LO_TAG(*cdr)) {
    case LT_PAIR:
      putc(' ', stream);
      pair = cdr;
      continue; /* while (1) */
    case LT_NIL:
    case LT_IGNORE:
    case LT_INERT:
    case LT_TRUE:
    case LT_FALSE:
      if (nullp(cdr)) {
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

void write_fixnum(lispobj *fixnum_o, lispobj *port) {
  // weird name to avoid conflict with the type.
  check_tag(fixnum_o, LT_FIXNUM);
  check_tag(port, LT_PORT);
  fprintf(port_stream(LO_GET(lisp_port, *port)),
	  FIXNUM_CONVERSION_SPEC, *LO_GET(fixnum, *fixnum_o));
}

void write_symbol(lispobj *symbol, lispobj *port) {
  check_tag(symbol, LT_SYMBOL);
  check_tag(port, LT_PORT);
  fputs(symbol_name(LO_GET(lisp_symbol, *symbol)),
	port_stream(LO_GET(lisp_port, *port)));
}

void write_string(lispobj *string, lispobj *port) {
  FILE *stream;
  check_tag(string, LT_STRING);
  check_tag(port, LT_PORT);

  stream = port_stream(LO_GET(lisp_port, *port));
  putc('"', stream);
  fputs(string_string(LO_GET(lisp_string, *string)), stream);
  putc('"', stream);
}

void write_vector(lispobj *v, lispobj *port) {
  fixnum i, len;
  FILE *stream;
  lisp_vector* vector;
  check_tag(v, LT_VECTOR);
  check_tag(port, LT_PORT);

  vector = LO_GET(lisp_vector, *v);
  stream = port_stream(LO_GET(lisp_port, *port));

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

  stream = port_stream(LO_GET(lisp_port, *port));

  fputs("#<fsubr {", stream);
  fprintf(stream, "%p", (void*)LO_GET(lisp_fsubr, *fsubr));
  fputs("}>", stream);
}

void write_mtag(lispobj *mtag, lispobj *port) {
  FILE *stream;

  check_tag(mtag, LT_MTAG);
  check_tag(port, LT_PORT);

  stream = port_stream(LO_GET(lisp_port, *port));

  fputs("#<mtag (", stream);
  fprintf(stream,
	  TAG_CONVERSION_SPEC, mtag_mtag(LO_GET(lisptag, *mtag)));
  fputs(")>", stream);
}
