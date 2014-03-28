#include <stdlib.h>
#include <string.h> /* memset, memcpy */
#include <alloca.h>
#include <assert.h>

#include "ggrt.h"

#include "mz.h"

ggrt_module_t *ggrt_module_begin(ggrt_ctx ctx, const char *name)
{
  ggrt_module_t *mod = ggrt_m_module(ctx, name);
  mod->prev = ctx->current_module;
  ctx->current_module = mod;
  return mod;
}

ggrt_module_t *ggrt_module_end(ggrt_ctx ctx, ggrt_module_t *mod)
{
  if ( ! mod ) {
    mod = ctx->current_module;
  }
  ctx->current_module = mod->prev;
  return mod;
}

ggrt_pragma_t *ggrt_m_pragma(ggrt_ctx ctx, const char *text)
{
  ggrt_pragma_t *p = ggrt_malloc(sizeof(*p));
  p->text = ggrt_strdup(text);
  p->prev = ctx->pragmas;
  ctx->pragmas = p;
  return p;
}

ggrt_pragma_t *ggrt_pragma(ggrt_ctx ctx, const char *text)
{
  ggrt_pragma_t *p = ggrt_m_pragma(ctx, text);
  p->prev = ctx->pragmas;
  ctx->pragmas = p;
  return p;
}

ggrt_macro_t *ggrt_m_macro(ggrt_ctx ctx, const char *name, const char *text)
{
  ggrt_macro_t *p = ggrt_malloc(sizeof(*p));
  p->name = ggrt_strdup(name);
  p->text = ggrt_strdup(text);
  return p;
}

ggrt_macro_t *ggrt_macro(ggrt_ctx ctx, const char *name, const char *text)
{
  ggrt_macro_t *p = ggrt_m_macro(ctx, name, text);
  p->prev = ctx->macros;
  ctx->macros = p;
  return p;
}

static
ggrt_type_t *ggrt_m_type(ggrt_ctx ctx, const char *name, size_t c_size)
{
  ggrt_type_t *ct = ggrt_malloc(sizeof(*ct));
  memset(ct, 0, sizeof(*ct));
  ct->name = name ? ggrt_strdup(name) : name;
  ct->type = "intrinsic";
  ct->c_sizeof = c_size;
  ct->c_alignof = c_size; /* guess. */
  ct->c_vararg_size = c_size; /* can be overridden. */
  ct->param_type = ct;
  return ct;
}

ggrt_type_t *ggrt_intrinsic(ggrt_ctx ctx, const char *name, size_t c_size)
{
  ggrt_type_t *t = ggrt_m_type(ctx, name, c_size);
  ggrt_symbol *sym;
  assert(name && *name);
  if ( (sym = ggrt_symbol_table_by_name(ctx, ggrt_current_module(ctx)->st._intrinsic, name)) ) {
    if ( sym->addr != t )
      // FIXME: Check for existing typedef.
      abort();
    return sym->addr;
  }

  ggrt_symbol_table_add_(ctx, ggrt_current_module(ctx)->st._intrinsic, name, 0, t);

  if ( ctx->cb._intrinsic )
    t->cb_val = ctx->cb._intrinsic(ctx, t);

  ggrt_typedef(ctx, name, t);

  return t;
}

ggrt_type_t *ggrt_typedef(ggrt_ctx ctx, const char *name, ggrt_type_t *t)
{
  ggrt_symbol *sym;
  assert(name && *name);
  if ( (sym = ggrt_symbol_table_by_name(ctx, ggrt_current_module(ctx)->st._type, name)) ) {
    if ( sym->addr != t )
      // FIXME: Check for existing typedef.
      abort();
    return sym->addr;
  }

  ggrt_symbol_table_add_(ctx, ggrt_current_module(ctx)->st._type, name, 0, t);

  if ( ctx->cb._typedef )
    t->cb_val = ctx->cb._typedef(ctx, name, t);
  return t;
}

/* intrinsic types. */
ggrt_type_t *ggrt_pointer(ggrt_ctx ctx, ggrt_type_t *t)
{
  ggrt_type_t *pt;
  if ( t->pointer_to )
    return t->pointer_to;

  pt = ggrt_m_type(ctx, 0, sizeof(void*));
  pt->_ffi_type = pt->_ffi_arg_type = ctx->_ffi_type_pointer;
  pt->type = "pointer";
  pt->rtn_type = t;
  pt->c_sizeof = sizeof(void*);
  pt->c_alignof = ctx->type_voidP->c_alignof;
  pt->c_vararg_size = sizeof(void*);

  t->pointer_to = pt;

  if ( ctx->cb._pointer )
    t->cb_val = ctx->cb._pointer(ctx, t);

  return pt;
}

ggrt_type_t *ggrt_array(ggrt_ctx ctx, ggrt_type_t *t, size_t len)
{
  ggrt_type_t *pt;

  if ( (len == 0 || len == (size_t) -1) && t->array0_of )
    return t->array0_of;

  pt = ggrt_m_type(ctx, 0, 0);
  pt->_ffi_type = pt->_ffi_arg_type = ctx->_ffi_type_pointer;
  pt->type = "array";
  pt->rtn_type = t;
  pt->nelems = len;
  if ( (len == 0 || len == (size_t) -1) )
    t->array0_of = pt;
  // int a[10];
  // typeof(&a) == typeof(&a[0]);
  pt->pointer_to = ggrt_pointer(ctx, t);

  if ( ctx->cb._array )
    t->cb_val = ctx->cb._array(ctx, t);

  return pt;
}

ggrt_elem_t *ggrt_m_elem(ggrt_ctx ctx, const char *name, ggrt_type_t *t)
{
  ggrt_elem_t *e = ggrt_malloc(sizeof(*e));
  memset(e, 0, sizeof(*e));
  e->name = name ? ggrt_strdup(name) : name;
  e->type = t;
  return e;
}

enum ggrt_enum {
  x, y, z
};

ggrt_type_t *ggrt_enum(ggrt_ctx ctx, const char *name, int nelems, const char **names, long *values)
{
  ggrt_type_t *ct;
  ggrt_symbol *sym;
  if ( name && *name && (sym = ggrt_symbol_table_by_name(ctx, ggrt_current_module(ctx)->st._enum, name)) ) {
    return sym->addr;
  }

  ct = ggrt_m_type(ctx, name, sizeof(enum ggrt_enum));
  ct->_ffi_type = ctx->_ffi_type_sint;
  assert(sizeof(enum ggrt_enum) == sizeof(int));
  ct->type = "enum";

  if ( name && *name )
    ggrt_symbol_table_add_(ctx, ggrt_current_module(ctx)->st._enum, name, 0, ct);

  if ( nelems && names )
    ggrt_enum_define(ctx, ct, nelems, names, values);

  if ( ctx->cb._enum )
    ct->cb_val = ctx->cb._enum(ctx, ct);

  return ct;
}

ggrt_type_t *ggrt_enum_define(ggrt_ctx ctx, ggrt_type_t *ct, int nelems, const char **names, long *values)
{
  assert(ct);
  assert(nelems);
  assert(names);
  ct->nelems = nelems;
  ct->elems = ggrt_malloc(sizeof(ct->elems[0]) * ct->nelems);
  {
    int i;
    int default_value = 0;
    for ( i = 0; i < ct->nelems; ++ i ) {
      ggrt_elem_t *e = ct->elems[i] = ggrt_m_elem(ctx, names[i], ct);
      e->parent = ct;
      e->parent_i = i;
      e->enum_val = values ? values[i] : default_value ++;
    }
  }

  if ( ctx->cb._enum_define )
    ct->cb_val = ctx->cb._enum_define(ctx, ct);

  return ct;
}

ggrt_elem_t *ggrt_enum_get_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name)
{
  return ggrt_struct_get_elem(ctx, st, name);
}

ggrt_type_t *ggrt_struct(ggrt_ctx ctx, const char *s_or_u, const char *name)
{
  ggrt_type_t *st;
  ggrt_symbol *sym;

  if ( name && *name && (sym = ggrt_symbol_table_by_name(ctx, s_or_u[0] == 's' ? ggrt_current_module(ctx)->st._struct : ggrt_current_module(ctx)->st._union, name)) ) {
    return sym->addr;
  }

  st = ggrt_m_type(ctx, name, 0);
  st->type = s_or_u;
  st->struct_scope = ctx->current_struct;

  st->elems = ggrt_malloc(sizeof(st->elems[0]) * st->nelems);

  ctx->current_struct = st;

  if ( name && *name )
    ggrt_symbol_table_add_(ctx, ggrt_current_module(ctx)->st._enum, name, 0, st);

  if ( ctx->cb._struct )
    st->cb_val = ctx->cb._struct(ctx, st);

  return st;
}

ggrt_elem_t *ggrt_struct_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name, ggrt_type_t *t)
{
  int i;
  ggrt_elem_t *e;

  if ( ! st )
    st = ctx->current_struct;

  i = st->nelems ++;
  st->elems = ggrt_realloc(st->elems, sizeof(st->elems[0]) * st->nelems);

  e = st->elems[i] = ggrt_m_elem(ctx, name, t);
  e->parent = st;
  e->parent_i = i;

  if ( ctx->cb._struct_elem )
    e->cb_val = ctx->cb._struct_elem(ctx, st, e);

  return e;
}

ggrt_elem_t *ggrt_struct_get_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name)
{
  int i;

  assert(st);
  assert(name && *name);

  for ( i = 0; i < st->nelems; ++ i ) {
    if ( strcmp(st->elems[i]->name, name) == 0 )
      return st->elems[i];
  }
  return 0;
}

ggrt_type_t *ggrt_struct_end(ggrt_ctx ctx, ggrt_type_t *st)
{
  if ( ! st )
    st = ctx->current_struct;

  if ( ctx->cb._struct_end )
    st->cb_val = ctx->cb._struct_end(ctx, st);

  ctx->current_struct = st->struct_scope;

  return st;
}

size_t ggrt_type_sizeof(ggrt_ctx ctx, ggrt_type_t *st)
{
  if ( st->c_sizeof )
    return st->c_sizeof;
  switch ( st->type[0] ) {
  case 'a': // array
    if ( st->nelems != (size_t) -1 ) {
      st->c_sizeof = ggrt_type_sizeof(ctx, st->rtn_type) * st->nelems;
    }
    st->c_alignof = ggrt_type_alignof(ctx, st->rtn_type);
    break;
  case 's': case 'u':
  if ( st->nelems ) {
    size_t offset = 0, size = 0;
    int i;
    size_t e_alignof, e_sizeof, adjust_alignof, x;
    ggrt_elem_t *e;

    for ( i = 0; i < st->nelems; ++ i ) {
      e = st->elems[i];
      e_sizeof  = ggrt_type_sizeof(ctx, e->type);
      e_alignof = ggrt_type_alignof(ctx, e->type);
      switch ( *st->type ) {
      case 's':
        if ( (adjust_alignof = offset % e_alignof) )
          offset += e_alignof - adjust_alignof;
        e->offset = offset;
        offset += e_sizeof;
        break;
      case 'u':
        // align and size of a union is the max of all elements.
        // Actually: it should be the LCD of all elements
        if ( size < e_sizeof )
          size = e_sizeof;
        if ( offset < e_alignof )
          offset = e_alignof;
        e->offset = 0;
        break;
      default:
        abort();
      }
    }

    switch ( *st->type ) {
    case 's':
      // Align all elems so array of this struct will align its members.
      x = 0;
      for ( i = 0; i < st->nelems; ++ i ) {
        e = st->elems[i];
        e_alignof = ggrt_type_alignof(ctx, e->type);
        if ( (adjust_alignof = offset % e_alignof) )
          offset += e_alignof - adjust_alignof;
        if ( x < e_alignof ) // LCD == max alignment of all members?
          x = e_alignof;
      }
      st->c_sizeof  = offset;
      st->c_alignof = x;
      break;
    case 'u':
      st->c_sizeof  = size;
      st->c_alignof = offset;
      break;
    default:
      abort();
    }
    st->c_vararg_size = st->c_sizeof; // ???
  }
  break;
  default: abort();
  }
  return st->c_sizeof;
}

size_t ggrt_type_alignof(ggrt_ctx ctx, ggrt_type_t *st)
{
  if ( st->c_alignof )
    return st->c_alignof;
  ggrt_type_sizeof(ctx, st);
  return st->c_alignof;
}

ggrt_type_t *ggrt_func(ggrt_ctx ctx, void *rtn_type, int nelems, ggrt_type_t **param_types)
{
  ggrt_type_t *ct = ggrt_m_type(ctx, 0, 0);
  ct->type = "function";
  ct->param_type = ctx->type_voidP;
  ct->rtn_type = rtn_type;
  ct->nelems = nelems;
  ct->elems = ggrt_malloc(sizeof(ct->elems[0]) * ct->nelems);
  {
    int i;
    for ( i = 0; i < nelems; ++ i ) {
      ggrt_type_t *pt = param_types[i];
      ggrt_elem_t *e = ct->elems[i] = ggrt_m_elem(ctx, 0, pt);
      e->parent = ct;
      e->parent_i = i;
    }
  }

  if ( ctx->cb._func )
    ct->cb_val = ctx->cb._func(ctx, ct);

  return ct;
}


