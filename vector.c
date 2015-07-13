#include "vector.h"
#include "fixnum.h"
#include "error.h"
#include "alloc.h"
#include <string.h> // memset

fixnum vector_length(lisp_vector* vector) {
  return vector->length;
}

lispobj* vref(lisp_vector* vector, fixnum index) {
  if ((index >= vector->length) || (index < 0)) {
    return lerror("out of bounds: index " FIXNUM_CONVERSION_SPEC "\n",
		  index);
  } else return (vector->data)[index];
}

void set_vref(lisp_vector* vector, fixnum index, lispobj* value) {
  if ((index >= vector->length) || (index < 0)) {
    lerror("out of bounds: index " FIXNUM_CONVERSION_SPEC "\n",
	   index);
  } else (vector->data)[index] = value;
}

lispobj* make_vector(fixnum length) {
  lispobj* ret;
  size_t bytes = length * sizeof(lispobj*);
  size_t size = ALIGN_LOD(offsetof(lisp_vector, data) + bytes);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_VECTOR;
    LO_GET(lisp_vector, *ret)->length = length;
    memset(LO_GET(lisp_vector, *ret)->data, 0, bytes);
  } while (LALLOC_FAILED(addr, size));

  return ret;
}
