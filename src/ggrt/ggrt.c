#include <stdlib.h>
#include <stdio.h> /* asprintf() */
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

  if ( ctx->cb._module_begin )
    ctx->cb._module_begin(ctx, mod);

  return mod;
}

ggrt_module_t *ggrt_module_end(ggrt_ctx ctx, ggrt_module_t *mod)
{
  if ( ! mod ) {
    mod = ctx->current_module;
  }

  if ( ctx->cb._module_end )
    ctx->cb._module_end(ctx, mod);

  ctx->current_module = mod->prev;
  return mod;
}

ggrt_pragma_t *ggrt_m_pragma(ggrt_ctx ctx, const char *text)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_pragma_t *p = ggrt_malloc(sizeof(*p));
  p->text = ggrt_strdup(text);
  return p;
}

ggrt_pragma_t *ggrt_pragma(ggrt_ctx ctx, const char *text)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_pragma_t *obj = ggrt_m_pragma(ctx, text);
  obj->_id = ++ mod->_next_id;
  if ( ctx->cb._pragma )
    ctx->cb._pragma(ctx, obj);
  obj->prev = mod->pragmas;
  mod->pragmas = obj;
  return obj;
}

ggrt_macro_t *ggrt_m_macro(ggrt_ctx ctx, const char *name, const char *text)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_macro_t *p = ggrt_malloc(sizeof(*p));
  p->name = ggrt_strdup(name);
  p->text = ggrt_strdup(text);
  return p;
}

ggrt_macro_t *ggrt_macro(ggrt_ctx ctx, const char *name, const char *text)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_macro_t *obj = ggrt_m_macro(ctx, name, text);
  obj->_id = ++ mod->_next_id;
  if ( ctx->cb._macro )
    ctx->cb._macro(ctx, obj);
  obj->prev = mod->macros;
  mod->macros = obj;
  return obj;
}

ggrt_constant_t *ggrt_m_constant(ggrt_ctx ctx, const char *name, const char *text)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_constant_t *p = ggrt_malloc(sizeof(*p));
  p->name = ggrt_strdup(name);
  p->c_expr = ggrt_strdup(text);
  return p;
}

ggrt_constant_t *ggrt_constant(ggrt_ctx ctx, const char *name, const char *text)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_constant_t *obj = ggrt_m_constant(ctx, name, text);
  obj->_id = ++ mod->_next_id;
  if ( ctx->cb._constant )
    ctx->cb._constant(ctx, obj);
  obj->prev = mod->constants;
  mod->constants = obj;
  return obj;
}

ggrt_global_t *ggrt_m_global(ggrt_ctx ctx, const char *name, ggrt_type_t *type, void *addr)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_global_t *p = ggrt_malloc(sizeof(*p));
  p->name = ggrt_strdup(name);
  p->addr = addr;
  p->type = type;
  return p;
}

ggrt_global_t *ggrt_global(ggrt_ctx ctx, const char *name, ggrt_type_t *type, void *addr)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_global_t *obj = ggrt_m_global(ctx, name, type, addr);
  ggrt_symbol *sym;
  obj->_id = ++ mod->_next_id;
  if ( ctx->cb._global )
    ctx->cb._global(ctx, obj);
  sym = ggrt_symbol_table_add_(ctx, mod->st._global, name, addr, type);
  sym->value = obj;
  obj->prev = mod->globals;
  mod->globals = obj;
  return obj;
}

static
ggrt_type_t *ggrt_m_type(ggrt_ctx ctx, const char *name, size_t c_size)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *ct = ggrt_malloc(sizeof(*ct));
  memset(ct, 0, sizeof(*ct));
  ct->name = name ? ggrt_strdup(name) : name;
  ct->te = ++ mod->_next_te;
  ct->c_sizeof = c_size;
  ct->c_alignof = c_size; /* guess. */
  ct->c_vararg_size = c_size; /* can be overridden. */
  ct->param_type = ct;
  ct->last_value = -1;
  return ct;
}

ggrt_type_t *ggrt_intrinsic(ggrt_ctx ctx, const char *name, size_t c_size)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *t;
  ggrt_symbol *sym;

  assert(name && *name);
  if ( (sym = ggrt_symbol_table_by_name(ctx, mod->st._intrinsic, name)) )
    return sym->addr;

  t = ggrt_m_type(ctx, name, c_size);
  if ( strcmp(name, "...") == 0 ) {
    t->type = "varargs";
  } else {
    t->type = "intrinsic";
  }

  ggrt_symbol_table_add_(ctx, mod->st._intrinsic, name, 0, t);

  if ( ctx->cb._intrinsic )
    ctx->cb._intrinsic(ctx, t);

  ggrt_symbol_table_add_(ctx, mod->st._type, name, 0, t);

  return t;
}

ggrt_type_t *ggrt_type(ggrt_ctx ctx, const char *name)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_symbol *sym;

  if ( (sym = ggrt_symbol_table_by_name(ctx, mod->st._intrinsic, name)) )
  assert(name && *name);
    return sym->addr;

  return 0;
}

ggrt_typedef_t *ggrt_m_typedef(ggrt_ctx ctx, const char *name, ggrt_type_t *t)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_typedef_t *obj = ggrt_malloc(sizeof(*obj));
  obj->name = ggrt_strdup(name);
  obj->type = t;
  return obj;
}

ggrt_type_t *ggrt_typedef(ggrt_ctx ctx, const char *name, ggrt_type_t *t)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_typedef_t *obj;
  ggrt_symbol *sym;
  assert(name && *name);
  if ( (sym = ggrt_symbol_table_by_name(ctx, mod->st._type, name)) ) {
    if ( sym->addr != t )
      // FIXME: Check for existing typedef.
      abort();
    return sym->addr;
  }

  obj = ggrt_m_typedef(ctx, name, t);
  obj->_id = ++ mod->_next_id;
  obj->prev = mod->typedefs;
  mod->typedefs = obj;

  ggrt_symbol_table_add_(ctx, mod->st._type, name, obj, t);

  if ( ctx->cb._typedef )
    ctx->cb._typedef(ctx, obj);
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
  pt->te += ggrt_te_pointer;
  pt->rtn_type = t;
  pt->c_sizeof = sizeof(void*);
  pt->c_alignof = ctx->type_voidP->c_alignof;
  pt->c_vararg_size = sizeof(void*);

  if ( t->type[1] != 'l' ) // placeholder
    t->pointer_to = pt;

  if ( ctx->cb._pointer )
    ctx->cb._pointer(ctx, t);

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
  pt->te += ggrt_te_array;
  pt->rtn_type = t;
  pt->nelems = len;
  if ( t->type[1] != 'l' ) // placeholder
    if ( (len == 0 || len == (size_t) -1) )
      t->array0_of = pt;
  // int a[10];
  // typeof(&a) == typeof(&a[0]);
  pt->pointer_to = ggrt_pointer(ctx, t);

  if ( ctx->cb._array )
    ctx->cb._array(ctx, t);

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

ggrt_type_t *_ggrt_enum(ggrt_ctx ctx, const char *name)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *ct;
  ggrt_symbol *sym;

  if ( name && *name && (sym = ggrt_symbol_table_by_name(ctx, mod->st._enum, name)) ) {
    return sym->addr;
  }

  ct = ggrt_m_type(ctx, name, sizeof(enum ggrt_enum));
  ct->_ffi_type = ctx->_ffi_type_sint;
  assert(sizeof(enum ggrt_enum) == sizeof(int));
  ct->type = "enum";
  ct->te += ggrt_te_enum;

  if ( name && *name )
    ggrt_symbol_table_add_(ctx, mod->st._enum, name, 0, ct);

  return ct;
}

ggrt_type_t *ggrt_enum_forward(ggrt_ctx ctx, const char *name)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *et;

  assert(name && *name);
  return et = _ggrt_enum(ctx, name);
}

ggrt_type_t *ggrt_enum(ggrt_ctx ctx, const char *name, int nelems, const char **names, const long *values)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *ct = _ggrt_enum(ctx, name);

  ct->prev = mod->current_enum;
  ct->struct_scope = mod->current_struct;
  mod->current_enum = ct;

  if ( nelems && names ) {
    int i;
    for ( i = 0; i < nelems; ++ i ) {
      const long *valuep = values ? &values[i] : 0;
      ggrt_enum_elem(ctx, ct, names[i], valuep);
    }
  }

  if ( ctx->cb._enum )
    ctx->cb._enum(ctx, ct);

  return ct;
}

ggrt_elem_t *ggrt_enum_elem(ggrt_ctx ctx, ggrt_type_t *ct, const char *name, const long *valuep)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_elem_t *e;
  int i;
  assert(ct);
  assert(name && *name);

  i = ct->nelems ++;
  ct->elems = ggrt_realloc(ct->elems, sizeof(ct->elems[0]) * ct->nelems);
  ct->elems[i] = e = ggrt_m_elem(ctx, name, ct);
  e->parent_i = i;
  e->enum_val = valuep ? *valuep : ++ ct->last_value;
  ct->last_value = e->enum_val;

  if ( ctx->cb._enum_elem )
    ctx->cb._enum_elem(ctx, ct, e);

  return e;
}

ggrt_type_t *ggrt_enum_end(ggrt_ctx ctx, ggrt_type_t *ct, const char *name)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);

  if ( ! ct ) {
    assert(name && *name);
    ct = ggrt_enum(ctx, name, 0, 0, 0);
  }

  if ( ctx->cb._enum_end )
    ctx->cb._enum_end(ctx, ct);

  mod->current_enum = ct->prev;

  return ct;
}

ggrt_elem_t *ggrt_enum_get_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name)
{
  return ggrt_struct_get_elem(ctx, st, name);
}

ggrt_type_t *_ggrt_struct(ggrt_ctx ctx, const char *s_or_u, const char *name)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *st;
  ggrt_symbol *sym;

  if ( name && *name && (sym = ggrt_symbol_table_by_name(ctx, s_or_u[0] == 's' ? mod->st._struct : mod->st._union, name)) ) {
    return sym->addr;
  }

  st = ggrt_m_type(ctx, name, 0);
  st->type = s_or_u;
  st->te += s_or_u[0] == 's' ? ggrt_te_struct : ggrt_te_union;

  st->elems = ggrt_malloc(sizeof(st->elems[0]) * st->nelems);

  if ( name && *name )
    ggrt_symbol_table_add_(ctx, mod->st._enum, name, 0, st);

  return st;
}

ggrt_type_t *ggrt_struct(ggrt_ctx ctx, const char *s_or_u, const char *name)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *st = _ggrt_struct(ctx, s_or_u, name);

  st->struct_scope = mod->current_struct;
  st->prev = mod->current_struct;
  mod->current_struct = st;

  if ( ctx->cb._struct )
    ctx->cb._struct(ctx, st);

  return st;
}

ggrt_type_t *ggrt_struct_forward(ggrt_ctx ctx, const char *s_or_u, const char *name)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  ggrt_type_t *st;

  assert(name && *name);
  st = _ggrt_struct(ctx, s_or_u, name);
  
  if ( ctx->cb._struct_forward )
    ctx->cb._struct_forward(ctx, st);

  return st;
}


ggrt_elem_t *ggrt_struct_elem(ggrt_ctx ctx, ggrt_type_t *st, const char *name, ggrt_type_t *t)
{
  ggrt_module_t *mod = ggrt_current_module(ctx);
  int i;
  ggrt_elem_t *e;

  if ( ! st )
    st = mod->current_struct;

  i = st->nelems ++;
  st->elems = ggrt_realloc(st->elems, sizeof(st->elems[0]) * st->nelems);

  e = st->elems[i] = ggrt_m_elem(ctx, name, t);
  e->parent = st;
  e->parent_i = i;

  if ( ctx->cb._struct_elem )
    ctx->cb._struct_elem(ctx, st, e);

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
  ggrt_module_t *mod = ggrt_current_module(ctx);
  if ( ! st )
    st = mod->current_struct;

  if ( ctx->cb._struct_end )
    ctx->cb._struct_end(ctx, st);

  mod->current_struct = st->prev;

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
  ct->te += ggrt_te_func;
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
    ctx->cb._func(ctx, ct);

  return ct;
}

ggrt_type_t *ggrt_varargs(ggrt_ctx ctx)
{
  return ctx->type_varargs;
}

ggrt_parameter_t *ggrt_parameter(ggrt_ctx ctx, ggrt_type_t *type, const char *name)
{
  ggrt_parameter_t *p = ggrt_malloc(sizeof(*p));
  p->type = type;
  p->name = ggrt_strdup(name);
  return p;
}

void ggrt_parameter_list(ggrt_ctx ctx, ggrt_parameter_t *params, int *nparamp, ggrt_type_t ***param_typesp)
{
  int nparam = 0, i;
  ggrt_type_t **param_types = 0;
  ggrt_parameter_t *head = 0, *next;

  // Reverse and conunt linked list.
  while ( params ) {
    next = params->prev;
    nparam ++;
    params->prev = head;
    head = params;
    params = next;
  }
  params = head;

  param_types = ggrt_malloc(sizeof(param_types[0]) * nparam);
  for ( i = 0; i < nparam; ++ i ) {
    param_types[i] = params->type;
    params = params->prev;
  }
  assert(params == 0);
}

ggrt_type_t *ggrt_func_params(ggrt_ctx ctx, void *rtn_type, ggrt_parameter_t *params)
{
  int nparam = 0;
  ggrt_type_t **param_types = 0;

  ggrt_parameter_list(ctx, params, &nparam, &param_types);
  return ggrt_func(ctx, rtn_type, nparam, param_types);
}


const char *ggrt_c_declarator(ggrt_ctx ctx, ggrt_type_t *t)
{
  char *d = 0;
  int i;

  if ( t->c_declarator )
    return t->c_declarator;

  switch ( t->type[0] ) {
  case 'p':
    asprintf(&d, ggrt_c_declarator(ctx, t->rtn_type), "*%s");
    break;
  case 'a':
    if ( t->nelems != (size_t) -1 ) {
      asprintf(&d, ggrt_c_declarator(ctx, t->rtn_type), "%s[%llu]", (unsigned long long) t->nelems);
    } else {
      asprintf(&d, ggrt_c_declarator(ctx, t->rtn_type), "%s[]");
    }
    break;
  case 'f':
    asprintf(&d, ggrt_c_declarator(ctx, t->rtn_type), "%s(");
    for ( i = 0; i < t->nelems;  ) {
      asprintf(&d, ggrt_c_declarator(ctx, t->elems[i]->type), "");
      if ( ++ i < t->nelems )
        asprintf(&d, ", ");
    }
    asprintf(&d, ")");
    break;
  case 's': case 'u':
    if ( t->name && *t->name ) {
      asprintf(&d, "%s %s %s", t->type, t->name, "%s");
    } else {
      abort();
    }
    break;
  case 'e': // enum
    if ( t->name && *t->name ) {
      asprintf(&d, "enum %s %s", t->name, "%s");
    } else {
      abort();
    }
    break;

  default:
    abort();
  }
  t->c_declarator = ggrt_strdup(d);
  free(d);
  return t->c_declarator;
}

ggrt_type_t *ggrt_placeholder(ggrt_ctx ctx, const char *name)
{
  ggrt_type_t *t = ggrt_m_type(ctx, name, 0);
  t->type = "placeholder";
  t->c_declarator = "%s";
  return t;
}

ggrt_type_t *ggrt_replace(ggrt_ctx ctx, ggrt_type_t *t, ggrt_type_t *ph, ggrt_type_t *new_val)
{
  int i;

  if ( t == ph )
    return new_val;

  switch ( t->type[0] ) {
  case 'p': case 'a':
    t->rtn_type = ggrt_replace(ctx, t->rtn_type, ph, new_val);
    break;
  case 'f':
    t->rtn_type = ggrt_replace(ctx, t->rtn_type, ph, new_val);
    /* FALL THROUGH */
  case 's': case 'u':
    for ( i = 0; i < t->nelems; ++ i ) {
      t->elems[i]->type = ggrt_replace(ctx, t->elems[i]->type, ph, new_val);
    }
    break;
  case 'e': // enum
    break;

  default:
    abort();
  }
  return t;
}

