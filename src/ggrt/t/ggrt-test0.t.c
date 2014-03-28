#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ggrt/ggrt.h"

struct align_struct_dummy {
  char c[23];
  void *p;
};

#define SELEMS(E)     \
  E(c,char,char)      \
  E(s,short,short)    \
  E(i,int,int)        \
  E(l,long,long)      \
  E(f,float,float)    \
  E(d,double,double)  \
  E(ld,long double, long_double) \
  E(p,void *, voidP)

typedef struct test_struct {
#define E(N,T,TN) T N;
  SELEMS(E)
#undef E
} test_struct;

static void fill_bytes(void *_p, size_t s)
{
  unsigned char *p = _p, *e = _p + s;
  int i = (int) s;
  while ( p < e ) {
    *(p ++) = i ++;
  }
}

#define E(N,T,TN)                                                       \
  static ggrt_type_t *st_##N;                                           \
  static struct test_align_##TN {                                       \
    char hdr[3]; T x; char ftr[5];                                      \
  } ta_##TN, tr_##TN;                                                   \
  static struct test_align_##TN tf_##TN(struct test_align_##TN a) {     \
    printf("%s()\n", "struct test_align_" #TN);                         \
    assert(memcmp(&a, &ta_##TN, sizeof(ta_##TN)) == 0);                 \
    return a;                                                           \
  }
  SELEMS(E)
#undef E

#define my_offsetof(T,E) ((size_t) &(((T*)0)->E))

static
void test_type_construction(ggrt_ctx ctx)
{
  ggrt_type_t *t = ctx->type_int;

  // Cached.
  assert(ggrt_pointer(ctx, t)  == ggrt_pointer(ctx, t));
  assert(ggrt_array(ctx, t, 0) == ggrt_array(ctx, t, 0));
  assert(ggrt_array(ctx, t, (size_t) -1) == ggrt_array(ctx, t, (size_t) -1));

  // Not cached.
  assert(ggrt_array(ctx, t, 1) != ggrt_array(ctx, t, 1));
}

static void test_struct_def(ggrt_ctx ctx)
{
  ggrt_type_t *st = ggrt_struct(ctx, "struct", "test_struct");
  test_struct v;
  struct align_struct_dummy v2;
#define E(N,C,T) \
  ggrt_struct_elem(ctx, st, #N, ctx->type_##T);
  SELEMS(E)
#undef E
    ggrt_struct_end(ctx, st);

  assert(ggrt_type_sizeof(ctx, st) == sizeof(v));
  assert(ggrt_type_alignof(ctx, st) == __alignof(v));

#define E(N,T,TN) \
  assert(ggrt_struct_get_elem(ctx, st, #N)->offset == my_offsetof(test_struct, N));
  SELEMS(E)
#undef E

  // Test alignment of prime size padding.
  {
    ggrt_type_t *st;

#define E(N,T,TN) \
    st = st_##N = ggrt_struct(ctx, "struct", 0); \
      ggrt_struct_elem(ctx, st, "hdr", ggrt_array(ctx, ctx->type_char, 3)); \
      ggrt_struct_elem(ctx, st, "x",   ctx->type_##TN); \
      ggrt_struct_elem(ctx, st, "ftr", ggrt_array(ctx, ctx->type_char, 5)); \
    ggrt_struct_end(ctx, st);
  SELEMS(E)
#undef E

#define E(N,T,TN)                                                     \
    assert(ggrt_type_sizeof(ctx,  st_##N) == sizeof(ta_##TN));        \
    assert(ggrt_type_alignof(ctx, st_##N) == __alignof(ta_##TN));
  SELEMS(E)
#undef E

#define E(N,T,TN) \
  assert(ggrt_struct_get_elem(ctx, st_##N, "x")->offset == my_offsetof(struct test_align_##TN, x));
  SELEMS(E)
#undef E

#define E(N,C,TN)                                               \
  fill_bytes(&ta_##TN, sizeof(ta_##TN));                        \
  tr_##TN = tf_##TN(ta_##TN);                                   \
  assert(memcmp(&tr_##TN, &ta_##TN, sizeof(tr_##TN)) == 0);  
  SELEMS(E)
#undef E

  }

}

typedef void *boxed;
static boxed identity(boxed x) { return x + 5; }

static void test_func_call(ggrt_ctx ctx)
{
  ggrt_type_t *boxed_type = ctx->type_voidP;
  ggrt_type_t *ct_rtn  = boxed_type;
  ggrt_type_t *ct_params[1] = { boxed_type };
  ggrt_type_t *ft = ggrt_func(ctx, ct_rtn, 1, ct_params);
  ggrt_symbol *sym = ggrt_global(ctx, "identity", &identity, ft);
  boxed rtn, args[10];

  args[0] = (boxed) 0x1234;
  ggrt_ffi_call(ctx, ft, &rtn, ggrt_global_get(ctx, "identity", 0)->addr, 1, args);
  printf("identity(%p) => %p\n", args[0], rtn);
  assert(rtn == args[0] + 5);
}

int main()
{
  ggrt_ctx ctx = ggrt_ctx_init_ffi(ggrt_ctx_init(ggrt_m_ctx()));

  test_type_construction(ctx);
  test_struct_def(ctx);
  test_func_call(ctx);

  return 0;
}

