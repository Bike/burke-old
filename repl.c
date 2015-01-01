#include <stdio.h>
#include <setjmp.h>
#include <string.h> // strcmp
#include <stdlib.h> // exit
#include "read.h"
#include "write.h"
#include "lisp.h"
#include "error.h"
#include "package.h"
#include "alloc.h"

jmp_buf err_jmp;

lispobj* jump_with_eof(const char* format, ...) {
  va_list vargs;
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  va_end(vargs);
  if (!strcmp(format, "unexpected EOF\n")) // ugh
    exit(1);
  longjmp(err_jmp, 0);
}

int main(void) {
  initialize_globals(); // nil etc., same across threads
  // burke_state = initialize_state(); // "dynamic binding" through TLS
  lisp_package* package = make_package(100);
  lispobj* ground = make_ground(package);
  lispobj* lstdout = make_port(stdout);
  lerror = jump_with_eof; // set up error handler
  setjmp(err_jmp); // don't care about return
  while(1) {
    write_lisp(eval(read_lisp(stdin, package), ground), lstdout);
    putchar('\n');
    fflush(stdout);
  }
}
