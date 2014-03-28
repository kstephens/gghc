#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ggrt/ggrt.h"
#define GGRT_V_type ctx->type_voidP

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

#define my_offsetof(T,E) ((size_t) &(((T*)0)->E))

static void test_struct_def(ggrt_ctx ctx)
{
  ggrt_type_t *st = ggrt_m_struct_type(ctx, "struct", "test_struct");
  test_struct v;
  struct align_struct_dummy v2;
#define E(N,C,T) ggrt_m_struct_elem(ctx, st, #N, ctx->type_##T);
  SELEMS(E)
#undef E
    ggrt_m_struct_type_end(ctx, st);

  assert(ggrt_type_sizeof(ctx, st) == sizeof(v));
  assert(ggrt_type_alignof(ctx, st) == __alignof(v));

#define E(N,T,TN) \
  assert(ggrt_struct_elem(ctx, st, #N)->offset == my_offsetof(test_struct, N));
  SELEMS(E)
#undef E

  // Test alignment of prime size padding.
  {
    ggrt_type_t *st;
#define E(N,T,TN)                                                       \
    ggrt_type_t *st_##N;                                                \
    struct test_align_##TN { char hdr[3]; T x; char ftr[5]; } N;
  SELEMS(E)
#undef E

#define E(N,T,TN) \
    st = st_##N = ggrt_m_struct_type(ctx, "struct", 0); \
      ggrt_m_struct_elem(ctx, st, "hdr", ggrt_m_array_type(ctx, ctx->type_char, 3)); \
      ggrt_m_struct_elem(ctx, st, "x",   ctx->type_##TN); \
      ggrt_m_struct_elem(ctx, st, "ftr", ggrt_m_array_type(ctx, ctx->type_char, 5)); \
    ggrt_m_struct_type_end(ctx, st);
  SELEMS(E)
#undef E

#define E(N,T,TN)                                               \
    assert(ggrt_type_sizeof(ctx,  st_##N) == sizeof(N));        \
    assert(ggrt_type_alignof(ctx, st_##N) == __alignof(N));
  SELEMS(E)
#undef E

#define E(N,T,TN) \
  assert(ggrt_struct_elem(ctx, st_##N, "x")->offset == my_offsetof(struct test_align_##TN, x));
  SELEMS(E)
#undef E
  }

}

static GGRT_V identity(GGRT_V x) { return x + 5; }

static void test_func_call(ggrt_ctx ctx)
{
  ggrt_type_t *ct_rtn  = GGRT_V_type;
  ggrt_type_t *ct_params[1] = { GGRT_V_type };
  ggrt_type_t *ft = ggrt_m_func_type(ctx, ct_rtn, 1, ct_params);

  GGRT_V rtn, args[10];

  ggrt_symbol *sym = ggrt_global(ctx, "identity", &identity, ft);

  args[0] = (GGRT_V) 0x1234;
  ggrt_ffi_call(ctx, ft, &rtn, ggrt_global_get(ctx, "identity", 0)->addr, 1, args);
  printf("identity(%p) => %p\n", args[0], rtn);
  assert(rtn == args[0] + 5);
}

int main()
{
  ggrt_ctx ctx = ggrt_ctx_init_ffi(ggrt_m_ctx());

  test_struct_def(ctx);
  test_func_call(ctx);

  return 0;
}

