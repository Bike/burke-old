#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

void main(void) {
  unsigned long foo;
  printf("%d\n", sscanf("2222222222222222222222222222222222", "%ld", &foo));
  printf("%d: %s\n", errno, strerror(errno));
  printf("%ld\n", foo);
  printf("%ld\n", LONG_MAX);
}
