#include "nenv.h"
#include "amap.h"
#include "alloc.h"
#include "lisp.h"

lispobj* nenv_lookup(lisp_nenv* nenv, lispobj* name) {
  lispobj* value;
  if (amap_assoc(LO_GET(lisp_amap, *(nenv->map)), name, &value))
    return value;
  else if (nenv->parent)
    return lookup(name, nenv->parent);
  else {
    // dunno if i should force name to be a symbol
    return lerror("unbound symbol");
  }
}

void nenv_define(lisp_nenv* nenv, lispobj* name, lispobj* value) {
  // note this can add local bindings, which makes everything hard.
  lisp_amap* map = LO_GET(lisp_amap, *(nenv->map));
  if (!set_amap_assoc(map, name, value)) {
    nenv->map = amap_expand(map);
    if (!set_amap_assoc(LO_GET(lisp_amap, *(nenv->map)), name, value))
      lerror("Impossible error in nenv_define!");
  }
}

lispobj* make_nenv(lispobj* parent, size_t initial_size) {
  lispobj* ret;
  /* Allocate this beforehand to avoid more critical path whatevers.
   * Note that amap is a full lispobj because scan methods might have
   * to track it, even though we don't need the tag in nenv. */
  lispobj* map = make_amap(initial_size);
  size_t size = ALIGN_LO(lisp_nenv);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_NENV;
    LO_GET(lisp_nenv, *ret)->parent = parent;
    LO_GET(lisp_nenv, *ret)->map = map;
  } while (LALLOC_FAILED(addr, size));
  return ret;
}
