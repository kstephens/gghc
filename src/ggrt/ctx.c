#include <stdlib.h>
#include <string.h> /* memset, memcpy */
#include <assert.h>

#include "ggrt/ctx.h"
#include "ggrt/ggrt.h"

ggrt_ctx ggrt_m_ctx()
{
  ggrt_ctx ctx = malloc(sizeof(*ctx));
  memset(ctx, 0, sizeof(*ctx));

  ctx->_malloc  = malloc;
  ctx->_realloc = realloc;
  ctx->_free    = free;
  ctx->_strdup  = strdup;

  return ctx;
}

#ifndef ggrt_malloc
#define ggrt_malloc(s)    ctx->_malloc(s)
#define ggrt_realloc(p,s) ctx->_realloc(p,s)
#define ggrt_free(p)      ctx->_free(p)
#define ggrt_strdup(p)    ctx->_strdup(p)
#endif

ggrt_ctx ggrt_ctx_init(ggrt_ctx ctx)
{
  assert(ctx);

  ctx->st_type   = ggrt_m_symbol_table(ctx, "type");
  ctx->st_struct = ggrt_m_symbol_table(ctx, "struct");
  ctx->st_union  = ggrt_m_symbol_table(ctx, "union");
  ctx->st_enum   = ggrt_m_symbol_table(ctx, "enum");
  ctx->st_global = ggrt_m_symbol_table(ctx, "global");
  ctx->st_macro  = ggrt_m_symbol_table(ctx, "macro");

  // Define basic types.
#define TYPE(N,T,AN) \
  ctx->type_##AN = ggrt_m_type(ctx, #T, sizeof(T));      \
  ctx->type_##AN->c_alignof = __alignof__(T);
#include "type.def"

  /* Aliased types */
#define TYPE(N,T,AN)
#define ATYPE(N,T,AN) ctx->type_##N = ctx->type_##AN;
#include "type.def"
  ctx->type_pointer = ctx->type_voidP;

  /* Coerce args to int */
#define TYPE(N,T,AN) if ( sizeof(T) < sizeof(int) ) ctx->type_##AN->param_type = ctx->type_int;
#include "type.def"

  /* Declarators */
#define TYPE(N,T,AN) ctx->type_##AN->c_declarator = #T " %s";
#include "type.def"

  /* In symbol table */
#define TYPE(N,T,AN) ggrt_symbol_table_add_(ctx, ctx->st_type, #T, ctx->type_##AN, 0);
#include "type.def"

  return ctx;
}

/* Symbol table */
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

ggrt_symbol_table* ggrt_m_symbol_table(ggrt_ctx ctx, const char *name)
{
  ggrt_symbol_table* st = ggrt_malloc(sizeof(*st));
  memset(st, 0, sizeof(*st));
  st->name = name ? ggrt_strdup(name) : name;
  return st;
}

ggrt_symbol *ggrt_symbol_table_add_(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name, void *addr, ggrt_type_t *type)
{
  ggrt_symbol *sym = ggrt_m_symbol(ctx, name, addr, type);
  ggrt_symbol_table_add(ctx, st, sym);
  return sym;
}

ggrt_symbol *ggrt_symbol_table_get(ggrt_ctx ctx, ggrt_symbol_table *st, ggrt_symbol *sym)
{
  ggrt_symbol **base = sym->name ? st->by_name : st->by_addr;
  void *func         = sym->name ?     by_name :     by_addr;
  ggrt_symbol **symp = bsearch(&sym, base, st->nsymbs, sizeof(symp[0]), func);
  return symp ? *symp : 0;
}

ggrt_symbol *ggrt_symbol_table_by_name(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name)
{
  ggrt_symbol sym; sym.name = name;
  return ggrt_symbol_table_get(ctx, st, &sym);
}

ggrt_symbol *ggrt_global_get(ggrt_ctx ctx, const char *name, void *addr)
{
  ggrt_symbol proto = { name, addr };
  return ggrt_symbol_table_get(ctx, ctx->st_global, &proto);
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

ggrt_symbol *ggrt_m_symbol(ggrt_ctx ctx, const char *name, void *addr, ggrt_type_t *type)
{
  ggrt_symbol *sym = ggrt_malloc(sizeof(*sym));
  memset(sym, 0, sizeof(*sym));
  sym->name = name ? ggrt_strdup(name) : name;
  sym->type = type;
  sym->addr = addr;
  return sym;
}

ggrt_symbol *ggrt_global(ggrt_ctx ctx, const char *name, void *addr, ggrt_type_t *type)
{
  ggrt_symbol *sym = ggrt_m_symbol(ctx, name, addr, type);
  ggrt_symbol_table_add(ctx, ctx->st_global, sym);
  return sym;
}
