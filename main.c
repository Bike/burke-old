#include <stdio.h>
#include <setjmp.h>
#include "read.h"
#include "write.h"
#include "lisp.h"
#include "error.h"

jmp_buf err_jmp;

lispobj* jump(const char* format, ...) {
  va_list vargs;
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  va_end(vargs);
  longjmp(err_jmp, 0);
}

int main(void) {
  initialize_globals();
  error = jump; // set up error handler
  setjmp(err_jmp); // don't care about return
  while(1) {
    write_lisp(eval(read_lisp(stdin), ground_environment), lstdout);
    putchar('\n');
    fflush(stdout);
  }
}
