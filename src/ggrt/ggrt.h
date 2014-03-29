#ifndef __gg_ggrt_h
#define __gg_ggrt_h

#include "ggrt/ctx.h"

enum ggrt_type_enum_t {
  ggrt_te_UNDEF = 0,
  ggrt_te_INTRINSICS_BEGIN,
#define GG_TYPE(FFI,T,N)   ggrt_te_##N,
#define BOTH_TYPE(FFI,T,N) ggrt_te_##N,
#include "ggrt/type.def"
  ggrt_te_INTRINSICS_END,
  ggrt_te_GENERATED,
  ggrt_te_enum        = 0x010000,
  ggrt_te_enum_END    = 0x020000,
  ggrt_te_union       = 0x020000,
  ggrt_te_union_END   = 0x040000,
  ggrt_te_struct      = 0x040000,
  ggrt_te_struct_END  = 0x080000,
  ggrt_te_pointer     = 0x080000,
  ggrt_te_pointer_END = 0x100000,
  ggrt_te_array       = 0x100000,
  ggrt_te_array_END   = 0x200000,
  ggrt_te_func        = 0x200000,
  ggrt_te_func_END    = 0x400000,
  ggrt_te_END
};

struct ggrt_type_t {
  /* user data */
  ggrt_user_data user_data;

  const char *name;
  const char *type; /* "intrinsic", "pointer", "array", "struct", "union", "function". */
  enum ggrt_type_enum_t te;
  size_t c_sizeof;  /* C sizeof() */
  size_t c_alignof; /* C __alignof__() */
  size_t c_vararg_size;
  struct ggrt_type_t *param_type; /* as an function parameter. */
  struct ggrt_type_t *alias_of;
  struct ggrt_type_t *pointer_to; /* this type. */
  struct ggrt_type_t *array0_of;  /* this type. */

  /* struct, union, enum, func type */
  struct ggrt_type_t *rtn_type; /* AND array, pointer element type. */
  size_t nelems;
  struct ggrt_elem_t **elems;
  struct ggrt_type_t *struct_scope;

  /* C declarator. */
  const char *c_declarator; /* printf format with %s for the name. */

  void *cb_val; /* callback value. */

  /* func type: generated */
  void *_ffi_type, *_ffi_arg_type;
  void *_ffi_cif;
  short _ffi_cif_inited;
  void *_ffi_rtn_type;
  void **_ffi_elem_types;
  size_t c_args_size;
};

/* struct or enum element. */
typedef struct ggrt_elem_t {
  /* user data */
  ggrt_user_data user_data;

  const char *name;
  struct ggrt_type_t *type;
  long enum_val;
  size_t offset;
  ggrt_type_t *parent;
  int parent_i;
  void *cb_val; /* callback value. */
} ggrt_elem_t;

/* Must call before using libffi interfaces. */
ggrt_ctx ggrt_ctx_init_ffi(ggrt_ctx ctx);

/* Queries */
size_t ggrt_type_sizeof(ggrt_ctx ctx, ggrt_type_t *st);
size_t ggrt_type_alignof(ggrt_ctx ctx, ggrt_type_t *st);

/* Make intrinsic type. */
ggrt_type_t *ggrt_intrinsic(ggrt_ctx ctx, const char *name, size_t c_size);

ggrt_type_t *ggrt_pointer(ggrt_ctx ctx, ggrt_type_t *t);
/* len = (size_t) -1 if unspecified. */
ggrt_type_t *ggrt_array(ggrt_ctx ctx, ggrt_type_t *t, size_t len);

/* Make enum type. */
ggrt_type_t *ggrt_enum(ggrt_ctx ctx, const char *name, int nelem, const char **names, long *elem_values);
ggrt_type_t *ggrt_enum_define(ggrt_ctx ctx, ggrt_type_t *ct, int nelems, const char **names, long *values);
ggrt_elem_t *ggrt_enum_get_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name);

/* Make struct type. */
ggrt_type_t *ggrt_struct(ggrt_ctx ctx, const char *s_or_u, const char *name);
ggrt_elem_t *ggrt_struct_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name, ggrt_type_t *t);
ggrt_type_t *ggrt_struct_end(ggrt_ctx ctx, ggrt_type_t *st);
ggrt_elem_t *ggrt_struct_get_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name);

/* Make bitfield type. */
ggrt_type_t *ggrt_bitfield(ggrt_ctx ctx, ggrt_type_t *t, int bits);

/* Make function type. */
ggrt_type_t *ggrt_func(ggrt_ctx ctx, void *rtn_type, int nelem, ggrt_type_t **elem_types);

/* Typedefs */
ggrt_type_t *ggrt_type_by_name(ggrt_ctx ctx, const char *name);
ggrt_type_t *ggrt_typedef(ggrt_ctx ctx, const char *name, ggrt_type_t *type);

/* Globals */
ggrt_symbol *ggrt_global_get(ggrt_ctx ctx, const char *name, void *addr);

/* Func call. */
void ggrt_ffi_call(ggrt_ctx ctx, ggrt_type_t *ft, void *rtn_boxp, void *cfunc, int argc, const void *argv);

/* Unboxes a boxed value into a destination of type t.
 * Must return the number of bytes to increment ggrt_ffi_call's argv to next
 * boxed argument.
 * Often a boxed value is a tagged pointer, therefore the return
 * value is sizeof(void*).
 */
size_t ggrt_ffi_unbox(ggrt_ctx ctx, ggrt_type_t *t, const void *boxp, void *dst);

/* Unboxes a boxed value into an argument of type t. */
size_t ggrt_ffi_unbox_arg(ggrt_ctx ctx, ggrt_type_t *t, const void *boxp, void *dst);
/* Boxes a C value of type t. */
void   ggrt_ffi_box(ggrt_ctx ctx, ggrt_type_t *t, const void *src, void *boxp);

/* Internal */
ggrt_type_t *ggrt_ffi_prepare(ggrt_ctx ctx, ggrt_type_t *ft);

#endif
