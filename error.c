#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "types.h" // lispobj

_Thread_local error_handler error;

// if C didn't suck this is where
//  I'd have functions to make common handlers.

/* old
void lispobj* error(const char* format, ...) {
  va_list vargs;
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  va_end(vargs);
  exit(1);
}
*/
