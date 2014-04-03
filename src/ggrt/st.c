#include <stdlib.h> /* qsort() */
#include <string.h> /* strcmp(), memset() */
#include <assert.h>

#include "ggrt/st.h"

#include "mz.h"

ggrt_symbol_table* ggrt_m_symbol_table(ggrt_ctx ctx, const char *name)
{
  ggrt_symbol_table* st = ggrt_malloc(sizeof(*st));
  memset(st, 0, sizeof(*st));
  st->name = name ? ggrt_strdup(name) : name;
  return st;
}

ggrt_symbol *ggrt_m_symbol(ggrt_ctx ctx, const char *name, void *addr, ggrt_type_t *type)
{
  ggrt_symbol *sym = ggrt_malloc(sizeof(*sym));
  memset(sym, 0, sizeof(*sym));
  sym->name = name ? ggrt_strdup(name) : name;
  sym->addr = addr;
  sym->type = type;
  return sym;
}

static int by_name (const void *_a, const void *_b)
{
  ggrt_symbol *a = *(ggrt_symbol **)_a;
  ggrt_symbol *b = *(ggrt_symbol **)_b;
  return strcmp(a->name, b->name);
}

static int by_addr (const void *_a, const void *_b)
{
  ggrt_symbol *a = *(ggrt_symbol **)_a;
  ggrt_symbol *b = *(ggrt_symbol **)_b;
  return
    a->addr <  b->addr ? -1 :
    a->addr == b->addr ?  0 : 1;
}

ggrt_symbol *ggrt_symbol_table_get(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *sym)
{
  ggrt_symbol **base = sym->name ? st->by_name : st->by_addr;
  void *func         = sym->name ?     by_name :     by_addr;
  ggrt_symbol **symp = bsearch(&sym, base, st->nsymbs, sizeof(symp[0]), func);
  return symp ? *symp : 0;
}

void ggrt_symbol_table_add(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *sym)
{
  int i;
  assert(st);
  assert(! sym->st);

  sym->st = st;
  i = sym->st_i = st->nsymbs ++;

  st->by_name = ggrt_realloc(st->by_name, sizeof(st->by_name[0]) * st->nsymbs);
  st->by_addr = ggrt_realloc(st->by_addr, sizeof(st->by_addr[0]) * st->nsymbs);
  st->by_name[i] = st->by_addr[i] = sym;

  qsort(st->by_name, st->nsymbs, sizeof(st->by_name[0]), by_name);
  qsort(st->by_addr, st->nsymbs, sizeof(st->by_addr[0]), by_addr);

  sym->next = st->next;
  st->next = sym;
}

ggrt_symbol *ggrt_symbol_table_add_(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name, void *addr, ggrt_type_t *type)
{
  ggrt_symbol *sym = ggrt_m_symbol(ctx, name, addr, type);
  ggrt_symbol_table_add(ctx, st, sym);
  return sym;
}

ggrt_symbol *ggrt_symbol_table_by_name(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name)
{
  ggrt_symbol sym; sym.name = name;
  return ggrt_symbol_table_get(ctx, st, &sym);
}

ggrt_symbol *ggrt_symbol_table_by_add(ggrt_ctx ctx, ggrt_symbol_table *st, void *addr)
{
  ggrt_symbol sym; sym.addr = addr;
  return ggrt_symbol_table_get(ctx, st, &sym);
}

