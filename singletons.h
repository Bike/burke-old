#include "types.h"

extern lispobj *nil, *inert, *ignore, *sharp_t, *sharp_f;

int truth(lispobj*);
inline int nullp(lispobj*);
inline int ignorep(lispobj*);
