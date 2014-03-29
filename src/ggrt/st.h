#ifndef _ggrt_ST_H
#define _ggrt_ST_H

#include "ggrt/data.h"

/* struct or enum element. */
typedef struct ggrt_symbol {
  ggrt_HEADER(ggrt_symbol);

  const char *name;
  void *addr;
  ggrt_type_t *type;
  void *value;

  struct ggrt_symbol *next;
  struct ggrt_symbol_table *st;
  int st_i;
} ggrt_symbol;

typedef struct ggrt_symbol_table {
  ggrt_HEADER(ggrt_symbol_table);

  const char *name;
  int nsymbs;
  ggrt_symbol **by_name;
  ggrt_symbol **by_addr;

  ggrt_symbol *next; // FIXME already in HEADER.
} ggrt_symbol_table;

/* Symbol tables. */
ggrt_symbol_table* ggrt_m_symbol_table(ggrt_ctx ctx, const char *name);

void ggrt_symbol_table_add(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *sym);

ggrt_symbol *ggrt_symbol_table_get(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *proto);
ggrt_symbol *ggrt_symbol_table_by_name(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name);
ggrt_symbol *ggrt_symbol_table_by_addr(ggrt_ctx ctx, ggrt_symbol_table *st, void *addr);

ggrt_symbol *ggrt_m_symbol(ggrt_ctx ctx, const char *name, void *address, ggrt_type_t *type);

ggrt_symbol *ggrt_symbol_table_add_(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name, void *address, ggrt_type_t *type);

#endif
