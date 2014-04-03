#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h> /* vasprintf() */
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

char *ggrt_ssprintf(ggrt_ctx ctx, const char* format, ...)
{
  char *buf = 0, *result = 0;
  va_list vap;
  va_start(vap, format);
  vasprintf(&buf, format, vap);
  va_end(vap);

  // Copy to zone.
  result = malloc_zone_strdup(ctx->mz, buf);
  free(buf);

  return result;
}

#include "mz.h"

ggrt_ctx ggrt_ctx_init(ggrt_ctx ctx)
{
  assert(ctx);

  ggrt_ctx_reset(ctx);

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

  ctx->type_varargs = ggrt_intrinsic(ctx, "...", 0);
  ctx->type_varargs->te = ggrt_te_varargs;

  return ctx;
}

void ggrt_emit_types_from_st(ggrt_ctx ctx, ggrt_symbol_table *st)
{
  int i;

  for ( i = 0; i < st->nsymbs; i ++ ) {
    ggrt_symbol *sym = st->by_name[i];
    ggrt_type_t *t = sym->addr;
    assert(t);
    if ( ctx->cb._intrinsic )
      ctx->cb._intrinsic(ctx, t);
  }
}

void ggrt_emit_types(ggrt_ctx ctx)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  
  ggrt_emit_types_from_st(ctx, mod->st._intrinsic);
  ggrt_emit_types_from_st(ctx, mod->st._type);
}

ggrt_ctx ggrt_ctx_reset(ggrt_ctx ctx)
{
  ctx->default_module = ggrt_m_module(ctx, "%default");
  ctx->current_module = 0;
  ctx->_next_id = 0;

  return ctx;
}

ggrt_module_t *ggrt_m_module(ggrt_ctx ctx, const char *name)
{
  ggrt_module_t *mod = ggrt_malloc(sizeof(*mod));
  mod->name = ggrt_strdup(name);
  mod->_id = ++ ctx->_next_id;

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

  mod->_next_te = (int) ggrt_te_GENERATED;

  return mod;
}

ggrt_module_t *ggrt_current_module(ggrt_ctx ctx)
{
  return ctx->current_module ? ctx->current_module : ctx->default_module;
}

ggrt_symbol *ggrt_global_get(ggrt_ctx ctx, const char *name, void *addr)
{
  ggrt_symbol proto;
  proto.name = name;
  proto.addr = addr;
  return ggrt_symbol_table_get(ctx, ggrt_current_module(ctx)->st._global, &proto);
}

char *ggrt_escape_string(ggrt_ctx ctx, const char *str)
{
    char *out = ggrt_malloc(strlen(str) * 2 + 1);
    char *t = out;
    const char *s = str;
    while ( *s ) {
        if ( *s == '\\' || *s == '"' )
            *(t ++) = '\\';
        *(t ++) = *(s ++);
    }
    *(t ++) = 0;

    return out;
}

