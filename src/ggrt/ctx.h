#ifndef __ggrt_CTX_H
#define __ggrt_CTX_H

#include <stdlib.h>

#ifndef GGRT_V
typedef void *GGRT_V;
#define GGRT_V GGRT_V
#endif

struct ggrt_s_ctx;
typedef struct ggrt_s_ctx *ggrt_ctx;

struct ggrt_type_t;
typedef struct ggrt_type_t ggrt_type_t;

typedef void *ggrt_user_data[4];

/* struct or enum element. */
typedef struct ggrt_symbol {
  const char *name;
  void *addr;
  ggrt_type_t *type;
  void *value;

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
  ggrt_user_data user_data;

  /* intrinsic types. */
#define GG_TYPE(FFI,T,N)  struct ggrt_type_t *type_##N;
#define BOTH_TYPE(FFI,T)  struct ggrt_type_t *type_##T;
#include "ggrt/type.def"

  ggrt_symbol_table *st_type, *st_struct, *st_union, *st_enum, *st_global, *st_macro;

  void *(*_malloc)(size_t size);
  void *(*_realloc)(void *ptr, size_t size);
  void  (*_free)(void *ptr);
  char *(*_strdup)(const char *);

  struct ggrt_pragma_t *pragmas;
  struct ggrt_macro_t *macros;
  struct ggrt_module_t *current_module;
  struct ggrt_type_t *current_enum;
  struct ggrt_type_t *current_struct;

  /* ffi support. */
  // libffi type names.
#define FFI_TYPE(FFI,T)  void *_ffi_type_##FFI;
#define BOTH_TYPE(FFI,T) void *_ffi_type_##T;
#include "ggrt/type.def"

  /* Users must define these functions. */
  size_t (*_ffi_unbox)(ggrt_ctx ctx, ggrt_type_t *ct, GGRT_V *valp, void *dst);
  size_t (*_ffi_unbox_arg)(ggrt_ctx ctx, ggrt_type_t *ct, GGRT_V *valp, void *dst);
  void   (*_ffi_box)(ggrt_ctx ctx, ggrt_type_t *ct, void *src, GGRT_V *dstp);
};

typedef struct ggrt_module_t {
  const char *name;

  ggrt_symbol_table *st_type, *st_struct, *st_union, *st_enum;

  struct ggrt_module_t *prev;
} ggrt_module_t;

typedef struct ggrt_pragma_t {
  const char *text;
  struct ggrt_module_t *mod;
  struct ggrt_pragma_t *prev;
} ggrt_pragma_t;

typedef struct ggrt_macro_t {
  const char *name;
  const char *text;
  struct ggrt_module_t *mod;
  struct ggrt_macro_t *prev;
} ggrt_macro_t;

ggrt_ctx ggrt_m_ctx();

/* Must call before use. */
ggrt_ctx ggrt_ctx_init(ggrt_ctx ctx);

/* Module. */
ggrt_module_t *ggrt_m_module(ggrt_ctx ctx, const char *name);
ggrt_module_t *ggrt_module_begin(ggrt_ctx ctx, const char *name);
ggrt_module_t *ggrt_module_end(ggrt_ctx ctx, ggrt_module_t *m);

/* Pragma */
ggrt_pragma_t *ggrt_m_pragma(ggrt_ctx ctx, const char *text);
ggrt_pragma_t *ggrt_pragma(ggrt_ctx ctx, const char *text);

/* Macro */
ggrt_macro_t *ggrt_m_macro(ggrt_ctx ctx, const char *name, const char *text);
ggrt_macro_t *ggrt_macro(ggrt_ctx ctx, const char *name, const char *text);

/* Symbol tables. */
ggrt_symbol_table* ggrt_m_symbol_table(ggrt_ctx ctx, const char *name);

void ggrt_symbol_table_add(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *sym);

ggrt_symbol *ggrt_symbol_table_get(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *proto);
ggrt_symbol *ggrt_symbol_table_by_name(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name);
ggrt_symbol *ggrt_symbol_table_by_addr(ggrt_ctx ctx, ggrt_symbol_table *st, void *addr);

ggrt_symbol *ggrt_m_symbol(ggrt_ctx ctx, const char *name, void *address, ggrt_type_t *type);

ggrt_symbol *ggrt_symbol_table_add_(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name, void *address, ggrt_type_t *type);

#endif
