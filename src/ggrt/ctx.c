#include <stdlib.h>
#include <string.h> /* memset, memcpy */
#include <assert.h>

#include "ggrt/ctx.h"
#include "ggrt/ggrt.h"

ggrt_ctx ggrt_m_ctx()
{
  ggrt_ctx ctx = malloc(sizeof(*ctx));
  memset(ctx, 0, sizeof(*ctx));

  ctx->mz = malloc_zone_new();

  return ctx;
}

#include "mz.h"

ggrt_ctx ggrt_ctx_init(ggrt_ctx ctx)
{
  assert(ctx);

  ctx->default_module = ggrt_m_module(ctx, "%default");

  // Define basic types.
#define GG_TYPE(FFI,T,N)                                \
  ctx->type_##N = ggrt_intrinsic(ctx, #T, sizeof(T));      \
  ctx->type_##N->c_alignof = __alignof__(T); \
  ctx->type_##N->te = ggrt_te_##N;
#define BOTH_TYPE(FFI,T,N) GG_TYPE(FFI,T,N)
#include "type.def"

  /* Coerce args to int */
#define GG_TYPE(FFI,T,N) if ( sizeof(T) < sizeof(int) ) ctx->type_##N->param_type = ctx->type_int;
#include "type.def"

  /* Declarators */
#define GG_TYPE(FFI,T,N) ctx->type_##N->c_declarator = #T " %s";
#include "type.def"

  return ctx;
}

ggrt_module_t *ggrt_m_module(ggrt_ctx ctx, const char *name)
{
  ggrt_module_t *mod = ggrt_malloc(sizeof(*mod));
  mod->name = ggrt_strdup(name);

  mod->st._type   = ggrt_m_symbol_table(ctx, "type");
  mod->st._struct = ggrt_m_symbol_table(ctx, "struct");
  mod->st._union  = ggrt_m_symbol_table(ctx, "union");
  mod->st._enum   = ggrt_m_symbol_table(ctx, "enum");
  mod->st._global = ggrt_m_symbol_table(ctx, "global");
  mod->st._macro  = ggrt_m_symbol_table(ctx, "macro");

  /* intrinsics are shared. */
  if ( ctx->default_module ) {
    mod->st._intrinsic = ctx->default_module->st._intrinsic;
  } else {
    mod->st._intrinsic = ggrt_m_symbol_table(ctx, "intrinsic");
  }

  return mod;
}

ggrt_module_t *ggrt_current_module(ggrt_ctx ctx)
{
  return ctx->current_module ? ctx->current_module : ctx->default_module;
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
  ggrt_symbol proto;
  proto.name = name;
  proto.addr = addr;
  return ggrt_symbol_table_get(ctx, ggrt_current_module(ctx)->st._global, &proto);
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
  ggrt_symbol_table_add(ctx, ggrt_current_module(ctx)->st._global, sym);
  return sym;
}
