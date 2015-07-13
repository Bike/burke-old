#ifndef BURKE_PORT_H
#define BURKE_PORT_H

#include <stdio.h>
#include "types.h"

/* An I/O port (stream). */

#define LT_PORT 12

typedef FILE* lisp_port;

//TODO inline
FILE* port_stream(lisp_port*);

lispobj* make_port(FILE*);

#endif /* guard */
