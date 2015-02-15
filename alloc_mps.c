/* Objects containing other objects */

inline lispobj* make_pair(lispobj *car, lispobj *cdr) {
  lispobj* obj;
  mps_addr_t addr;

  size_t size = ALIGN_OBJ(sizeof(lisp_pair));

  do {
    mps_res_t res = mps_reserve(&addr, obj_ap, size);
    if (res != MPS_RES_OK) return lerror("OOM\n");
    obj = addr;
    obj->pair.tag = LT_PAIR;
    obj->pair.car = car;
    obj->pair.cdr = cdr;
  } while(!mps_commit(obj_ap, addr, size));

  return obj;
}

inline lispobj* make_applicative(lispobj *underlying) {
  lispobj *obj;
  mps_addr_t addr;

  size_t size = ALIGN_OBJ(sizeof(lisp_applicative));

  do {
    mps_res_t res = mps_reserve(&addr, obj_ap, size);
    if (res != MPS_RES_OK) return lerror("OOM\n");
    obj = addr;
    obj->applicative.tag = LT_APPLICATIVE;
    obj->applicative.underlying = underlying;
  } while(!mps_commit(obj_ap, addr, size));

  return obj;
}

inline lispobj* make_fexpr(lispobj *arg, lispobj *earg, lispobj *env, lispobj *body) {
  lispobj *ret;
  mps_addr_t addr;

  size_t size = ALIGN_OBJ(sizeof(lisp_fexpr));

  do {
    mps_res_t res = mps_reserve(&addr, obj_ap, size);
    if (res != MPS_RES_OK) return lerror("OOM\n");
    obj = addr;
    obj->fexpr.arg = arg;
    obj->fexpr.earg = earg;
    obj->fexpr.env = env;
    obj->fexpr.body = body;
  } while(!mps_commit(obj_ap, addr, size));

  return obj;
}

inline lispobj* make_vector(size_t length, lispobj *fill) {
  lispobj *obj;
  mps_addr_t addr;

  size_t size = ALIGN_OBJ(offsetof(lisp_vector, data) + length * sizeof(lispobj*));

  do {
    size_t i;
    mps_res_t res = mps_reserve(&addr, obj_ap, size);
    if (res != MPS_RES_OK) return lerror("OOM\n");
    obj = addr;
    obj->vector.tag = LT_VECTOR;
    obj->vector.length = length;
    for (i = 0; i < length; ++i)
      obj->vector.data[i] = fill;
  } while(!mps_commit(obj_ap, addr, size));

  return obj;
}
