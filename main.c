#include <stdio.h>
#include "read.h"
#include "write.h"
#include "lisp.h"

int main(void) {
  initialize_globals();
  while(1) {
    write_lisp(eval(read_lisp(stdin), ground_environment), lstdout);
    putchar('\n');
    fflush(stdout);
  }
}
