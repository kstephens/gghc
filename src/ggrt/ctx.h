#ifndef __ggrt_CTX_H
#define __ggrt_CTX_H

#include <stdlib.h>

#ifndef GGRT_V
typedef void *GGRT_V;
#define GGRT_V GGRT_V
#endif

struct ggrt_s_ctx;
typedef struct ggrt_s_ctx *ggrt_ctx;

struct ggrt_type;
typedef struct ggrt_type ggrt_type;

typedef void *ggrt_user_data[4];

/* struct or enum element. */
typedef struct ggrt_symbol {
  const char *name;
  void *addr;
  ggrt_type *type;
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
  /* intrinsic types. */
#define TYPE(N,T,AN)  struct ggrt_type *type_##AN;
#define ATYPE(N,T,AN) struct ggrt_type *type_##N;
#include "ggrt/type.def"
  struct ggrt_type *type_pointer; /* void* */

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
ggrt_ctx ggrt_ctx_init(ggrt_ctx ctx);

ggrt_symbol_table* ggrt_m_symbol_table(ggrt_ctx ctx, const char *name);

void ggrt_symbol_table_add(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *sym);

ggrt_symbol *ggrt_symbol_table_get(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *proto);
ggrt_symbol *ggrt_symbol_table_by_name(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name);
ggrt_symbol *ggrt_symbol_table_by_addr(ggrt_ctx ctx, ggrt_symbol_table *st, void *addr);

ggrt_symbol *ggrt_m_symbol(ggrt_ctx ctx, const char *name, void *address, ggrt_type *type);

ggrt_symbol *ggrt_symbol_table_add_(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name, void *address, ggrt_type *type);

#endif
