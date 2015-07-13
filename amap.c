#include "amap.h"
#include "alloc.h"
#include <string.h>

int amap_assoc(lisp_amap* map, lispobj* key, lispobj** value) {
  size_t i;
  for (i = 0; i < map->fillptr; ++i) {
    if (eqp(key, map->kv[2*i])) {
      *value = map->kv[2*i + 1];
      return 1;
    }
  }
  return 0; // failed
}

int set_amap_assoc(lisp_amap* map, lispobj* key, lispobj* value) {
  size_t i;
  for (i = 0; i < map->fillptr; ++i) {
    if (eqp(key, map->kv[2*i])) {
      map->kv[2*i+1] = value;
      return 1;
    }
  }
  if (map->fillptr == map->length) // outta space :(
    return 0;
  // add an entry
  map->kv[2*map->fillptr] = key;
  map->kv[2*map->fillptr+1] = value;
  map->fillptr++;
  return 2;
}

lispobj* make_amap(size_t length) {
  lispobj* ret;
  size_t bytes = 2*sizeof(lispobj*)*length;
  size_t size = ALIGN_LOD(offsetof(lisp_amap, kv) + bytes);
  void* addr;
  do {
    LALLOC_RESERVE(addr, size);
    ret = addr;
    LO_TAG(*ret) = LT_AMAP;
    LO_GET(lisp_amap, *ret)->length = length;
    LO_GET(lisp_amap, *ret)->fillptr = 0;
    memset(LO_GET(lisp_amap, *ret)->kv, 0, bytes);
  } while (LALLOC_FAILED(addr, size));
  return ret;
}

lispobj* amap_expand(lisp_amap* map) {
  lispobj* ret = make_amap(2*map->length); // try double the length
  lisp_amap* ret_map = LO_GET(lisp_amap, *ret);
  memcpy(ret_map->kv, map->kv, 2*map->length*sizeof(lispobj*));
  /* If a GC runs here things should still be consistent.
   * the new map's fillptr hasn't been updated
   * so the scan doesn't look through the array, and any objects in it
   * are still live due to being referenced by the old map. */
  ret_map->fillptr = map->length;
  return ret;
}
