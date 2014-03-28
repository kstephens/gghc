#include <stdlib.h>
#include <string.h> /* memset, memcpy */
#include <alloca.h>
#include <assert.h>

#include "ggrt.h"
#include <ffi.h>

size_t ggrt_ffi_unbox_dummy(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst)
{
  memset(dst, 0, ct->c_sizeof);
  memcpy(dst, boxp, sizeof(void*)); // dummy
  return ct->c_sizeof;
}

size_t ggrt_ffi_unbox_arg_dummy(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst)
{
  return ggrt_ffi_unbox(ctx, ct, boxp, dst);
}

void ggrt_ffi_box_dummy(ggrt_ctx ctx, ggrt_type_t *ct, const void *src, void *boxp)
{
  *(void**) boxp = *(void**) src;
}

ggrt_ctx ggrt_ctx_init_ffi(ggrt_ctx ctx)
{
  assert(ctx);

  // ggrt_ctx_init() should have been called.
  assert(ctx->st_type);

  // Patch in libffi types.
#define BOTH_TYPE(FFI,T) \
  ctx->_ffi_type_##T = &ffi_type_##FFI;
#define FFI_TYPE(FFI,T) \
  ctx->_ffi_type_##FFI = &ffi_type_##FFI;
#include "type.def"

  // Patch into ggrt types.
#define GG_TYPE(FFI,T,N) ctx->type_##N->_ffi_type = ctx->_ffi_type_##FFI;
#define BOTH_TYPE(FFI,T) ctx->type_##T->_ffi_type = ctx->_ffi_type_##FFI;
#include "type.def"

  ctx->_ffi_unbox     = ggrt_ffi_unbox_dummy;
  ctx->_ffi_unbox_arg = ggrt_ffi_unbox_arg_dummy;
  ctx->_ffi_box       = ggrt_ffi_box_dummy;

  return ctx;
}

size_t ggrt_ffi_unbox(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst)
{
  return ctx->_ffi_unbox(ctx, ct, boxp, dst);
}

size_t ggrt_ffi_unbox_arg(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst)
{
  return ctx->_ffi_unbox(ctx, ct, boxp, dst);
}

void ggrt_ffi_box(ggrt_ctx ctx, ggrt_type_t *ct, const void *src, void *boxp)
{
  ctx->_ffi_box(ctx, ct, src, boxp);
}

void ggrt_ffi_call(ggrt_ctx ctx, ggrt_type_t *ft, void *rtn_valp, void *cfunc, int argc, const void *argv)
{
  // Array of pointers to individual arguments.
  void **f_args     = alloca(sizeof(*f_args) * ggrt_ffi_prepare(ctx, ft)->nelems);
  // Space for unboxed arguments.
  void *f_arg_space = alloca(ft->c_args_size);
  // Space for return value.
  void *rtn_space   = alloca(ggrt_type_sizeof(ctx, ft->rtn_type));

  memset(f_arg_space, 0, ft->c_args_size);
  {
    int i;
    for ( i = 0; i < argc; ++ i ) {
      ggrt_type_t *param_t = ft->elems[i]->type->param_type;
      f_args[i] = f_arg_space;
      argv += ggrt_ffi_unbox_arg(ctx, param_t, argv, f_arg_space);
      f_arg_space += ggrt_type_sizeof(ctx, param_t);
    }
  }

  ffi_call(ft->_ffi_cif, cfunc, rtn_space, f_args);
   
  ggrt_ffi_box(ctx, ft->rtn_type, rtn_space, rtn_valp);
}

#include "mz.h"

ffi_type *ggrt_ffi_type(ggrt_ctx ctx, ggrt_type_t *t)
{
  ffi_type *ft;
  if ( t->_ffi_type )
    return t->_ffi_type;

  switch ( t->type[0] ) {
  case 'a': // array
    assert(t->nelems);
    ft = ggrt_malloc(sizeof(*ft));
    ft->size      = ggrt_type_sizeof(ctx, t) * t->nelems;
    ft->alignment = ggrt_type_alignof(ctx, t);
    t->_ffi_type = ft;
    break;
  case 's': // struct
    assert(t->nelems);
    ft = ggrt_malloc(sizeof(*ft));
    ft->size = ft->alignment = 0;
    ft->type = FFI_TYPE_STRUCT;
    ft->elements = ggrt_malloc(sizeof(ft->elements[0]) * (t->nelems + 1));
    {
      int i;
      for ( i = 0; i < t->nelems; ++ i )
        ft->elements[i] = ggrt_ffi_type(ctx, t->elems[i]->type);
      ft->elements[i] = 0;
    }
    t->_ffi_type = ft;
    break;
    // case 'u': //??
  default:
    abort();
    break;
  }

  assert(t->_ffi_type);
  return 0;
}

ffi_type *ggrt_ffi_arg_type(ggrt_ctx ctx, ggrt_type_t *t)
{
  if ( ! t->_ffi_arg_type )
    t->_ffi_arg_type = ggrt_ffi_type(ctx, t);
  assert(t->_ffi_arg_type);
  return t->_ffi_arg_type;
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
        ft->_ffi_elem_types[i] = ggrt_ffi_arg_type(ctx, e->type);
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

