#include <stdlib.h>
#include <string.h> /* memset, memcpy */
#include <alloca.h>
#include <assert.h>

#include "ggrt.h"
#include <ffi.h>

size_t ggrt_ffi_unbox_default(ggrt_ctx ctx, ggrt_type_t *ct, GGRT_V *valp, void *dst)
{
  memset(dst, 0, ct->c_sizeof);
  memcpy(dst, valp, sizeof(*valp)); // dummy
  return ct->c_sizeof;
}

size_t ggrt_ffi_unbox_arg_default(ggrt_ctx ctx, ggrt_type_t *ct, GGRT_V *valp, void *dst)
{
  return ggrt_ffi_unbox(ctx, ct, valp, dst);
}

void ggrt_ffi_box_default(ggrt_ctx ctx, ggrt_type_t *ct, void *src, GGRT_V *dstp)
{
  memcpy(dstp, src, sizeof(*dstp)); // dummy
}

ggrt_ctx ggrt_ctx_init_ffi(ggrt_ctx ctx)
{
  assert(ctx);

  ggrt_ctx_init(ctx);

  // Patch in libffi types.
#define BOTH_TYPE(FFI,T) ctx->_ffi_type_##T   = &ffi_type_##FFI;
#define FFI_TYPE(FFI,T)  ctx->_ffi_type_##FFI = &ffi_type_##FFI;
#include "type.def"

  // Patch into ggrt types.
#define GG_TYPE(FFI,T,N) ctx->type_##N->_ffi_type = ctx->_ffi_type_##FFI;
#define BOTH_TYPE(FFI,T) ctx->type_##T->_ffi_type = ctx->_ffi_type_##FFI;
#include "type.def"

  ctx->_ffi_unbox     = ggrt_ffi_unbox_default;
  ctx->_ffi_unbox_arg = ggrt_ffi_unbox_arg_default;
  ctx->_ffi_box       = ggrt_ffi_box_default;

  return ctx;
}

size_t ggrt_ffi_unbox(ggrt_ctx ctx, ggrt_type_t *ct, GGRT_V *valp, void *dst)
{
  return ctx->_ffi_unbox(ctx, ct, valp, dst);
}

size_t ggrt_ffi_unbox_arg(ggrt_ctx ctx, ggrt_type_t *ct, GGRT_V *valp, void *dst)
{
  return ctx->_ffi_unbox(ctx, ct, valp, dst);
}

void ggrt_ffi_box(ggrt_ctx ctx, ggrt_type_t *ct, void *src, GGRT_V *dstp)
{
  ctx->_ffi_box(ctx, ct, src, dstp);
}

void ggrt_ffi_call(ggrt_ctx ctx, ggrt_type_t *ft, GGRT_V *rtn_valp, void *cfunc, int argc, GGRT_V *argv)
{
  void **f_args   = alloca(sizeof(*f_args) * ggrt_ffi_prepare(ctx, ft)->nelems);
  void *arg_space = alloca(ft->c_args_size);
  void *rtn_space = alloca(ggrt_type_sizeof(ctx, ft->rtn_type));
  GGRT_V rtn_val;

  memset(arg_space, 0, ft->c_args_size);
  {
    void *arg_p = arg_space;
    int i;
    for ( i = 0; i < argc; ++ i ) {
      f_args[i] = arg_p;
      arg_p += ggrt_ffi_unbox_arg(ctx, ft->elems[i]->type, &argv[i], arg_p);
    }
  }

  ffi_call(ft->_ffi_cif, cfunc, rtn_space, f_args);
   
  ggrt_ffi_box(ctx, ft->rtn_type, rtn_space, rtn_valp);
}

#ifndef ggrt_malloc
#define ggrt_malloc(s)    ctx->_malloc(s)
#define ggrt_realloc(p,s) ctx->_realloc(p,s)
#define ggrt_free(p)      ctx->_free(p)
#define ggrt_strdup(p)    ctx->_strdup(p)
#endif

ffi_type *ggrt_ffi_type(ggrt_ctx ctx, ggrt_type_t *t)
{
  if ( t->_ffi_type )
    return t->_ffi_type;
  // TODO BUILD ffi_type for struct/union.
  assert(t->_ffi_type);
  return 0;
}

ggrt_type_t *ggrt_ffi_prepare(ggrt_ctx ctx, ggrt_type_t *ft)
{
  if ( ! ft->_ffi_cif_inited ) {
    if ( ! ft->_ffi_cif )
      ft->_ffi_cif = ggrt_malloc(sizeof(ffi_cif));
    ft->_ffi_rtn_type = ggrt_ffi_type(ctx, ft->rtn_type);
    if ( ! ft->_ffi_elem_types ) {
      int i;
      size_t offset = 0;
      ft->_ffi_elem_types = ggrt_malloc(sizeof(ft->_ffi_elem_types[0]) * ft->nelems);
      for ( i = 0; i < ft->nelems; ++ i ) {
        ggrt_elem_t *e = ft->elems[i];
        assert(e->type->_ffi_type);
        ft->_ffi_elem_types[i] = ggrt_ffi_type(ctx, e->type);
        e->offset = offset;
        offset += ggrt_type_sizeof(ctx, e->type);
        ft->c_args_size = offset;
      }
      ft->c_args_size = offset;
    }
    if ( ffi_prep_cif(ft->_ffi_cif, FFI_DEFAULT_ABI, ft->nelems, ft->_ffi_rtn_type, (ffi_type **) ft->_ffi_elem_types) != FFI_OK )
      abort();
    ft->_ffi_cif_inited = 1;
  }
  return ft;
}

