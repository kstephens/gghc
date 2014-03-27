#ifndef __gg_ggrt_h
#define __gg_ggrt_h

#include <ffi.h>

#ifndef GGRT_V
typedef void *GGRT_V;
#define GGRT_V GGRT_V
#endif

struct ggrt_s_ctx;
typedef struct ggrt_s_ctx *ggrt_ctx;

typedef void *ggrt_user_data[4];

typedef struct ggrt_type {
  const char *name;
  const char *type; /* "pointer", "array", "struct", "union", "function". */
  size_t c_sizeof;  /* C sizeof() */
  size_t c_alignof; /* C __alignof__() */
  size_t c_vararg_size;
  ffi_type *f_type;
  struct ggrt_type *param_type; /* as an function parameter. */
  struct ggrt_type *alias_of;
  struct ggrt_type *pointer_to; /* this type. */

  /* struct, union, enum, func type */
  struct ggrt_type *rtn_type; /* AND array, pointer element type. */
  size_t nelems;
  struct ggrt_elem **elems;
  struct ggrt_type *struct_scope;

  /* func type: generated */
  ffi_cif f_cif;
  short f_cif_inited;
  ffi_type *f_rtn_type;
  ffi_type **f_elem_types;
  size_t c_args_size;

  /* C declarator. */
  const char *c_declarator; /* printf format with %s for the name. */

  /* user data */
  ggrt_user_data user_data;
} ggrt_type;

/* struct or enum element. */
typedef struct ggrt_elem {
  const char *name;
  struct ggrt_type *type;
  long enum_val;
  size_t offset;
  ggrt_type *parent;
  int parent_i;

  /* user data */
  ggrt_user_data user_data;
} ggrt_elem;

/* struct or enum element. */
typedef struct ggrt_symbol {
  const char *name;
  void *addr;
  ggrt_type *type;
  struct ggrt_symbol *next;
  struct ggrt_symbol_table *st;
  int st_i;

  ggrt_user_data user_data;
} ggrt_symbol;

typedef struct ggrt_symbol_table {
  const char *name;
  int nsymbs;
  ggrt_symbol **by_name;
  ggrt_symbol **by_addr;
  ggrt_symbol *next;

  ggrt_user_data user_data;
} ggrt_symbol_table;


struct ggrt_s_ctx {
  /* intrinsic types. */
#define TYPE(N,T,AN)  ggrt_type *type_##AN;
#define ATYPE(N,T,AN) ggrt_type *type_##N;
#include "ggrt/type.def"
  ggrt_type *type_pointer; /* void* */

  ggrt_symbol_table *st_global, *st_type;

  void *(*_malloc)(size_t size);
  void *(*_realloc)(void *ptr, size_t size);
  void  (*_free)(void *ptr);
  char *(*_strdup)(const char *);

  /* Users must define these functions. */
  size_t (*ffi_unbox)(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst);
  size_t (*ffi_unbox_arg)(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst);
  void   (*ffi_box)(ggrt_ctx ctx, ggrt_type *ct, void *src, GGRT_V *dstp);

  ggrt_user_data user_data;
};

ggrt_ctx ggrt_m_ctx();

/* Must call before use. */
void ggrt_ctx_init(ggrt_ctx ctx);

/* Queries */
size_t ggrt_type_sizeof(ggrt_ctx ctx, ggrt_type *st);
size_t ggrt_type_alignof(ggrt_ctx ctx, ggrt_type *st);

/* Make intrinsic type. */
ggrt_type *ggrt_m_type(ggrt_ctx ctx, const char *name, size_t c_size, void *f_type);

ggrt_type *ggrt_m_pointer_type(ggrt_ctx ctx, ggrt_type *t);
ggrt_type *ggrt_m_array_type(ggrt_ctx ctx, ggrt_type *t, size_t len);

/* Make enum type. */
ggrt_type *ggrt_m_enum_type(ggrt_ctx ctx, const char *name, int nelem, const char **names, long *elem_values);
ggrt_elem *ggrt_enum_elem(ggrt_ctx ctx, ggrt_type *st, const char *name);
ggrt_type *ggrt_m_enum_type_define(ggrt_ctx ctx, ggrt_type *ct, int nelems, const char **names, long *values);

/* Make struct type. */
ggrt_type *ggrt_m_struct_type(ggrt_ctx ctx, const char *s_or_u, const char *name);
ggrt_elem *ggrt_m_struct_elem(ggrt_ctx ctx, ggrt_type *st, const char *name, ggrt_type *t);
ggrt_type *ggrt_m_struct_type_end(ggrt_ctx ctx, ggrt_type *st);
ggrt_elem *ggrt_struct_elem(ggrt_ctx ctx, ggrt_type *st, const char *name);

/* Make function type. */
ggrt_type *ggrt_m_func_type(ggrt_ctx ctx, void *rtn_type, int nelem, ggrt_type **elem_types);

/* Typedefs */
ggrt_type *ggrt_type_by_name(ggrt_ctx ctx, const char *name);
ggrt_type *ggrt_typedef(ggrt_ctx ctx, const char *name, ggrt_type *type);

/* Globals */
ggrt_symbol *ggrt_global(ggrt_ctx ctx, const char *name, void *address, ggrt_type *type);
ggrt_symbol *ggrt_global_get(ggrt_ctx ctx, const char *name, void *addr);

/* Create a symbol definition. */
extern ggrt_symbol_table *ggrt_st_type, *ggrt_st_global;;
ggrt_symbol_table* ggrt_m_symbol_table(ggrt_ctx ctx, const char *name);
void ggrt_symbol_table_add(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *sym);
ggrt_symbol *ggrt_symbol_table_get(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *proto);
ggrt_symbol *ggrt_m_symbol(ggrt_ctx ctx, const char *name, void *address, ggrt_type *type);

ggrt_symbol *ggrt_symbol_table_add_(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name, void *address, ggrt_type *type);

/* Func call. */
void ggrt_ffi_call(ggrt_ctx ctx, ggrt_type *ft, GGRT_V *rtn_valp, void *cfunc, int argc, GGRT_V *argv);

size_t ggrt_ffi_unbox(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst);
size_t ggrt_ffi_unbox_arg(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst);
void   ggrt_ffi_box(ggrt_ctx ctx, ggrt_type *ct, void *src, GGRT_V *dstp);

/* Internal */
ggrt_type *ggrt_ffi_prepare(ggrt_ctx ctx, ggrt_type *ft);

#endif
