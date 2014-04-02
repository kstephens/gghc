#ifndef __ggrt_CTX_H
#define __ggrt_CTX_H

#include <stdlib.h>
#include "ggrt/data.h"
#include "ggrt/st.h"
#include "ggrt/malloc_zone.h"

struct ggrt_sts {
  ggrt_symbol_table *_type, *_intrinsic, *_struct, *_union, *_enum, *_global, *_macro;
} st;

struct ggrt_s_ctx {
  ggrt_HEADER(ggrt_s_ctx);

  malloc_zone *mz;
  int _next_id;

  /* intrinsic types. */
#define GG_TYPE(FFI,T,N)  struct ggrt_type_t *type_##N;
#define BOTH_TYPE(FFI,T,N)  struct ggrt_type_t *type_##N;
#include "ggrt/type.def"
  struct ggrt_type_t *type_varargs;

  /* Collected C runtime data. */
  struct ggrt_module_t *current_module, *default_module;

  /* Callbacks */
  struct ggrt_cb {
    ggrt_user_data user_data; /* opaque callback data hook. */
    void (*_module_begin)(ggrt_ctx ctx, struct ggrt_module_t *mod);
    void (*_module_end)(ggrt_ctx ctx, struct ggrt_module_t *mod);
    void (*_pragma)(ggrt_ctx ctx, struct ggrt_pragma_t *obj);
    void (*_macro)(ggrt_ctx ctx, struct ggrt_macro_t *obj);
    void (*_constant)(ggrt_ctx ctx, struct ggrt_constant_t *obj);
    void (*_intrinsic)(ggrt_ctx ctx, struct ggrt_type_t *t);
    void (*_typedef)(ggrt_ctx ctx, struct ggrt_typedef_t *obj);
    void (*_pointer)(ggrt_ctx ctx, struct ggrt_type_t *t);
    void (*_array)(ggrt_ctx ctx, struct ggrt_type_t *at);
    void (*_enum_forward)(ggrt_ctx ctx, struct ggrt_type_t *et);
    void (*_enum)(ggrt_ctx ctx, struct ggrt_type_t *et);
    void (*_enum_elem)(ggrt_ctx ctx, struct ggrt_type_t *et, struct ggrt_elem_t *elem);
    void (*_enum_end)(ggrt_ctx ctx, struct ggrt_type_t *et);
    void (*_struct_forward)(ggrt_ctx ctx, struct ggrt_type_t *st);
    void (*_struct)(ggrt_ctx ctx, struct ggrt_type_t *st);
    void (*_struct_elem)(ggrt_ctx ctx, struct ggrt_type_t *st, struct ggrt_elem_t *elem);
    void (*_struct_end)(ggrt_ctx ctx, struct ggrt_type_t *st);
    void (*_func)(ggrt_ctx ctx, struct ggrt_type_t *ft);
    void (*_global)(ggrt_ctx ctx, struct ggrt_global_t *g);
  } cb;

  /* ffi support. */
  // libffi type names.
#define FFI_TYPE(FFI,T)    void *_ffi_type_##FFI;
#define BOTH_TYPE(FFI,T,N) void *_ffi_type_##N;
#include "ggrt/type.def"

  /* Users must define these functions. */
  size_t (*_ffi_unbox)(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst);
  size_t (*_ffi_unbox_arg)(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst);
  void   (*_ffi_box)(ggrt_ctx ctx, ggrt_type_t *ct, const void *src, void *boxp);
};

typedef struct ggrt_module_t {
  ggrt_HEADER(ggrt_module_t);
  const char *name;

  struct ggrt_sts st;

  struct ggrt_pragma_t   *pragmas;
  struct ggrt_macro_t    *macros;
  struct ggrt_constant_t *constants;
  struct ggrt_typedef_t  *typedefs;
  struct ggrt_global_t   *globals;

  struct ggrt_type_t *current_enum;
  struct ggrt_type_t *current_struct;

  int _next_te;
  int _next_id;
} ggrt_module_t;

typedef struct ggrt_pragma_t {
  ggrt_HEADER(ggrt_pragma_t);
  const char *text;
} ggrt_pragma_t;

typedef struct ggrt_macro_t {
  ggrt_HEADER(ggrt_macro_t);
  const char *name;
  const char *text;
} ggrt_macro_t;

typedef struct ggrt_constant_t {
  ggrt_HEADER(ggrt_constant_t);
  char *name;
  char *c_expr;
} ggrt_constant_t;

typedef struct ggrt_typedef_t {
  ggrt_HEADER(ggrt_typedef_t);
  const char *name;
  struct ggrt_type_t *type;
} ggrt_typedef_t;

typedef struct ggrt_global_t {
  ggrt_HEADER(ggrt_global_t);
  const char *name;
  void *addr;
  struct ggrt_type_t *type;
  ggrt_symbol *sym;
} ggrt_global_t;

typedef struct ggrt_parameter_t {
  ggrt_HEADER(ggrt_parameter_t);
  ggrt_type_t *type;
  const char *name;
  int varargs;
} ggrt_parameter_t;

ggrt_ctx ggrt_m_ctx();

/* Must call before use. */
ggrt_ctx ggrt_ctx_init(ggrt_ctx ctx);
ggrt_ctx ggrt_ctx_reset(ggrt_ctx ctx);

/* Module. */
ggrt_module_t *ggrt_m_module(ggrt_ctx ctx, const char *name);
ggrt_module_t *ggrt_module_begin(ggrt_ctx ctx, const char *name);
ggrt_module_t *ggrt_module_end(ggrt_ctx ctx, ggrt_module_t *m);
ggrt_module_t *ggrt_current_module(ggrt_ctx ctx);

/* Pragma */
ggrt_pragma_t *ggrt_m_pragma(ggrt_ctx ctx, const char *text);
ggrt_pragma_t *ggrt_pragma(ggrt_ctx ctx, const char *text);

/* Macro */
ggrt_macro_t *ggrt_m_macro(ggrt_ctx ctx, const char *name, const char *text);
ggrt_macro_t *ggrt_macro(ggrt_ctx ctx, const char *name, const char *text);

/* Global */
ggrt_global_t *ggrt_m_global(ggrt_ctx ctx, const char *name, ggrt_type_t *t, void *addr);
ggrt_global_t *ggrt_global(ggrt_ctx ctx, const char *name, ggrt_type_t *t, void *addr);

/* Memory/String. */
char *ggrt_escape_string(ggrt_ctx ctx, const char *str);

#endif
