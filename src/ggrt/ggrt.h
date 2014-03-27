#ifndef __gg_ggrt_h
#define __gg_ggrt_h

#include <ffi.h>

#ifndef GGRT_V
typedef void *GGRT_V;
#define GGRT_V GGRT_V
#endif

#ifndef ggrt_malloc
#define ggrt_malloc(s)    malloc(s)
#define ggrt_realloc(p,s) realloc(p,s)
#define ggrt_free(p)      free(p)
#define ggrt_strdup(p)    strdup(p)
#endif

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
  void *user_data[4];
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
  void *user_data[4];
} ggrt_elem;

/* struct or enum element. */
typedef struct ggrt_symbol {
  const char *name;
  void *addr;
  ggrt_type *type;
  struct ggrt_symbol *next;
  struct ggrt_symbol_table *st;
  int st_i;
} ggrt_symbol;

typedef struct ggrt_symbol_table {
  const char *name;
  int nsymbs;
  ggrt_symbol **by_name;
  ggrt_symbol **by_addr;
  ggrt_symbol *next;
} ggrt_symbol_table;


/* intrinsic types. */
#define TYPE(N,T,AN)  extern ggrt_type *ggrt_type_##AN;
#define ATYPE(N,T,AN) extern ggrt_type *ggrt_type_##N;
#include "ggrt/type.def"
extern ggrt_type *ggrt_type_pointer; /* void* */

/* Must call before use. */
void ggrt_init();

/* Queries */
size_t ggrt_type_sizeof(ggrt_type *st);
size_t ggrt_type_alignof(ggrt_type *st);

/* Make intrinsic type. */
ggrt_type *ggrt_m_type(const char *name, size_t c_size, void *f_type);

ggrt_type *ggrt_m_pointer_type(ggrt_type *t);
ggrt_type *ggrt_m_array_type(ggrt_type *t, size_t len);

/* Make enum type. */
ggrt_type *ggrt_m_enum_type(const char *name, int nelem, const char **names, long *elem_values);
ggrt_elem *ggrt_enum_elem(ggrt_type *st, const char *name);
ggrt_type *ggrt_m_enum_type_define(ggrt_type *ct, int nelems, const char **names, long *values);

/* Make struct type. */
ggrt_type *ggrt_m_struct_type(const char *s_or_u, const char *name);
ggrt_elem *ggrt_m_struct_elem(ggrt_type *st, const char *name, ggrt_type *t);
ggrt_type *ggrt_m_struct_type_end(ggrt_type *st);
ggrt_elem *ggrt_struct_elem(ggrt_type *st, const char *name);

/* Make function type. */
ggrt_type *ggrt_m_func_type(void *rtn_type, int nelem, ggrt_type **elem_types);

/* Typedefs */
ggrt_type *ggrt_type_by_name(const char *name);
ggrt_type *ggrt_typedef(const char *name, ggrt_type *type);

/* Globals */
ggrt_symbol *ggrt_global(const char *name, void *address, ggrt_type *type);
ggrt_symbol *ggrt_global_get(const char *name, void *addr);

/* Create a symbol definition. */
extern ggrt_symbol_table *ggrt_st_type, *ggrt_st_global;;
ggrt_symbol_table* ggrt_m_symbol_table(const char *name);
void ggrt_symbol_table_add(ggrt_symbol_table *st, ggrt_symbol *sym);
ggrt_symbol *ggrt_symbol_table_get(ggrt_symbol_table *st, ggrt_symbol *proto);
ggrt_symbol *ggrt_m_symbol(const char *name, void *address, ggrt_type *type);

ggrt_symbol *ggrt_symbol_table_add_(ggrt_symbol_table *st, const char *name, void *address, ggrt_type *type);

/* Func call. */
void ggrt_ffi_call(ggrt_type *ft, GGRT_V *rtn_valp, void *cfunc, int argc, GGRT_V *argv);

/* Users must define these functions. */
size_t ggrt_ffi_unbox(ggrt_type *ct, GGRT_V *valp, void *dst);
size_t ggrt_ffi_unbox_arg(ggrt_type *ct, GGRT_V *valp, void *dst);
void   ggrt_ffi_box(ggrt_type *ct, void *src, GGRT_V *dstp);

/* Internal */
ggrt_type *ggrt_ffi_prepare(ggrt_type *ft);

#endif
