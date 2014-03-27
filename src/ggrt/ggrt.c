#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <string.h> /* memset, memcpy */
#include <assert.h>

#include "ggrt.h"


size_t ggrt_ffi_unbox_default(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst)
{
  memset(dst, 0, ct->c_sizeof);
  memcpy(dst, valp, sizeof(*valp)); // dummy
  return ct->c_sizeof;
}

size_t ggrt_ffi_unbox_arg_default(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst)
{
  return ggrt_ffi_unbox(ctx, ct, valp, dst);
}

void ggrt_ffi_box_default(ggrt_ctx ctx, ggrt_type *ct, void *src, GGRT_V *dstp)
{
  memcpy(dstp, src, sizeof(*dstp)); // dummy
}

ggrt_ctx ggrt_m_ctx()
{
  ggrt_ctx ctx = malloc(sizeof(*ctx));
  memset(ctx, 0, sizeof(*ctx));

  ctx->_malloc  = malloc;
  ctx->_realloc = realloc;
  ctx->_free    = free;
  ctx->_strdup  = strdup;

  ctx->ffi_unbox     = ggrt_ffi_unbox_default;
  ctx->ffi_unbox_arg = ggrt_ffi_unbox_arg_default;
  ctx->ffi_box       = ggrt_ffi_box_default;

  return ctx;
}

#ifndef ggrt_malloc
#define ggrt_malloc(s)    ctx->_malloc(s)
#define ggrt_realloc(p,s) ctx->_realloc(p,s)
#define ggrt_free(p)      ctx->_free(p)
#define ggrt_strdup(p)    ctx->_strdup(p)
#endif


ggrt_type *ggrt_m_type(ggrt_ctx ctx, const char *name, size_t c_size, void *f_type)
{
  ggrt_type *ct = ggrt_malloc(sizeof(*ct));
  memset(ct, 0, sizeof(*ct));
  ct->name = name ? ggrt_strdup(name) : name;
  ct->type = "intrinsic";
  ct->c_sizeof = c_size;
  ct->c_alignof = c_size; /* guess. */
  ct->c_vararg_size = c_size; /* can be overridden. */
  ct->f_type = f_type;
  ct->param_type = ct;
  return ct;
}

/* intrinsic types. */
#define TYPE(N,T,AN)  ggrt_type *ggrt_type_##AN;
#define ATYPE(N,T,AN) ggrt_type *ggrt_type_##N;
#include "type.def"
ggrt_type *ggrt_type_pointer;

void ggrt_ctx_init(ggrt_ctx ctx)
{
  assert(ctx);

  ctx->st_type   = ggrt_m_symbol_table(ctx, "type");
  ctx->st_global = ggrt_m_symbol_table(ctx, "global");

  // Define basic types.
#define TYPE(N,T,AN) \
  ctx->type_##AN = ggrt_m_type(ctx, #T, sizeof(T), &ffi_type_##N);      \
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

}

ggrt_type *ggrt_m_pointer_type(ggrt_ctx ctx, ggrt_type *t)
{
  ggrt_type *pt;
  if ( t->pointer_to )
    return t->pointer_to;

  pt = ggrt_m_type(ctx, 0, sizeof(void*), &ffi_type_pointer);
  pt->type = "pointer";
  pt->rtn_type = t;
  pt->c_sizeof = sizeof(void*);
  pt->c_alignof = ctx->type_pointer->c_alignof;
  pt->c_vararg_size = sizeof(void*);

  t->pointer_to = pt;
  return pt;
}

ggrt_type *ggrt_m_array_type(ggrt_ctx ctx, ggrt_type *t, size_t len)
{
  ggrt_type *pt;
  pt = ggrt_m_type(ctx, 0, sizeof(void*), &ffi_type_pointer);
  pt->type = "array";
  pt->rtn_type = t;
  pt->nelems = len;
  // int a[10];
  // typeof(&a) == typeof(&a[0]);
  pt->pointer_to = ggrt_m_pointer_type(ctx, t);
  return pt;
}

ggrt_elem *ggrt_m_elem(ggrt_ctx ctx, const char *name, ggrt_type *t)
{
  ggrt_elem *e = ggrt_malloc(sizeof(*e));
  memset(e, 0, sizeof(*e));
  e->name = name ? ggrt_strdup(name) : name;
  e->type = t;
  return e;
}

enum ggrt_enum {
  x, y, z
};

ggrt_type *ggrt_m_enum_type(ggrt_ctx ctx, const char *name, int nelems, const char **names, long *values)
{
  ggrt_type *ct = ggrt_m_type(ctx, name, sizeof(enum ggrt_enum), &ffi_type_sint);
  assert(sizeof(enum ggrt_enum) == sizeof(int));
  ct->type = "enum";
  if ( nelems && names ) {
    ggrt_m_enum_type_define(ctx, ct, nelems, names, values);
  }
  return ct;
}

ggrt_type *ggrt_m_enum_type_define(ggrt_ctx ctx, ggrt_type *ct, int nelems, const char **names, long *values)
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
      ggrt_elem *e = ct->elems[i] = ggrt_m_elem(ctx, names[i], ct);
      e->parent = ct;
      e->parent_i = i;
      e->enum_val = values ? values[i] : default_value ++;
    }
  }
  return ct;
}

ggrt_elem *ggrt_enum_elem(ggrt_ctx ctx, ggrt_type *st, const char *name)
{
  return ggrt_struct_elem(ctx, st, name);
}

static ggrt_type *current_st; /* NOT THREAD SAFE! */
ggrt_type *ggrt_m_struct_type(ggrt_ctx ctx, const char *s_or_u, const char *name)
{
  ggrt_type *st = ggrt_m_type(ctx, name, 0, 0);
  st->type = s_or_u;
  st->struct_scope = current_st;

  st->elems = ggrt_malloc(sizeof(st->elems[0]) * st->nelems);

  current_st = st;
  return st;
}

ggrt_elem *ggrt_m_struct_elem(ggrt_ctx ctx, ggrt_type *st, const char *name, ggrt_type *t)
{
  int i;
  ggrt_elem *e;
  if ( ! st )
    st = current_st;

  i = st->nelems ++;
  st->elems = ggrt_realloc(st->elems, sizeof(st->elems[0]) * st->nelems);

  e = st->elems[i] = ggrt_m_elem(ctx, name, t);
  e->parent = st;
  e->parent_i = i;

  return e;
}

ggrt_elem *ggrt_struct_elem(ggrt_ctx ctx, ggrt_type *st, const char *name)
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

ggrt_type *ggrt_m_struct_type_end(ggrt_ctx ctx, ggrt_type *st)
{
  if ( ! st )
    st = current_st;

  current_st = st->struct_scope;
  return st;
}

size_t ggrt_type_sizeof(ggrt_ctx ctx, ggrt_type *st)
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
    size_t e_alignof, e_sizeof, adjust_alignof;
    ggrt_elem *e;

    for ( i = 0; i < st->nelems; ++ i ) {
      e = st->elems[i];
      e_sizeof  = ggrt_type_sizeof(ctx, e->type);
      e_alignof = ggrt_type_alignof(ctx, e->type);
      switch ( *st->type ) {
      case 's':
        if ( (adjust_alignof = offset % e_alignof) )
          offset += e_alignof - adjust_alignof;
        e->offset = offset;
        offset += ggrt_type_sizeof(ctx, e->type);
        break;
      case 'u':
        // align and size of a union is the max of all elements.
        // Actually: it should be the LCD of all elements
        if ( size <= e_sizeof )
          size = e_sizeof;
        if ( offset <= e_alignof )
          offset = e_alignof;
        e->offset = 0;
        break;
      default:
        abort();
      }
    }

    switch ( *st->type ) {
    case 's':
      // Align to first elem so array of this struct will align.
      e = st->elems[0];

      e_alignof = ggrt_type_alignof(ctx, e->type);
      if ( (adjust_alignof = offset % e_alignof) )
        offset += e_alignof - adjust_alignof;

      // FIXME: Align to 16-byte word?
      e_alignof = 16;
      if ( (adjust_alignof = offset % e_alignof) )
        offset += e_alignof - adjust_alignof;

      st->c_sizeof = offset;
      st->c_alignof = ggrt_type_alignof(ctx, e->type);
      break;
    case 'u':
      st->c_sizeof = size;
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

size_t ggrt_type_alignof(ggrt_ctx ctx, ggrt_type *st)
{
  if ( st->c_alignof )
    return st->c_alignof;
  ggrt_type_sizeof(ctx, st);
  return st->c_alignof;
}

ggrt_type *ggrt_m_func_type(ggrt_ctx ctx, void *rtn_type, int nelems, ggrt_type **param_types)
{
  ggrt_type *ct = ggrt_m_type(ctx, 0, 0, 0);
  ct->type = "function";
  ct->param_type = ctx->type_pointer;
  ct->rtn_type = rtn_type;
  ct->nelems = nelems;
  ct->elems = ggrt_malloc(sizeof(ct->elems[0]) * ct->nelems);
  {
    int i;
    for ( i = 0; i < nelems; ++ i ) {
      ggrt_type *pt = param_types[i];
      ggrt_elem *e = ct->elems[i] = ggrt_m_elem(ctx, 0, pt);
      e->parent = ct;
      e->parent_i = i;
    }
  }
  return ct;
}

ggrt_type *ggrt_ffi_prepare(ggrt_ctx ctx, ggrt_type *ft)
{
  if ( ! ft->f_cif_inited ) {
    ft->f_rtn_type = ft->rtn_type->f_type;
    if ( ! ft->f_elem_types ) {
      int i;
      size_t offset = 0;
      ft->f_elem_types = ggrt_malloc(sizeof(ft->f_elem_types) * ft->nelems);
      for ( i = 0; i < ft->nelems; ++ i ) {
        ggrt_elem *e = ft->elems[i];
        ft->f_elem_types[i] = e->type->f_type;
        e->offset = offset;
        offset += ggrt_type_sizeof(ctx, e->type);
        ft->c_args_size = offset;
      }
      ft->c_args_size = offset;
    }
    if ( ffi_prep_cif(&ft->f_cif, FFI_DEFAULT_ABI, ft->nelems, ft->f_rtn_type, ft->f_elem_types) != FFI_OK )
      abort();
    ft->f_cif_inited = 1;
  }
  return ft;
}

size_t ggrt_ffi_unbox(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst)
{
  return ctx->ffi_unbox(ctx, ct, valp, dst);
}

size_t ggrt_ffi_unbox_arg(ggrt_ctx ctx, ggrt_type *ct, GGRT_V *valp, void *dst)
{
  return ctx->ffi_unbox(ctx, ct, valp, dst);
}

void ggrt_ffi_box(ggrt_ctx ctx, ggrt_type *ct, void *src, GGRT_V *dstp)
{
  ctx->ffi_box(ctx, ct, src, dstp);
}

void ggrt_ffi_call(ggrt_ctx ctx, ggrt_type *ft, GGRT_V *rtn_valp, void *cfunc, int argc, GGRT_V *argv)
{
  void **f_args   = alloca(sizeof(*f_args) * ggrt_ffi_prepare(ctx, ft)->nelems);
  void *arg_space = alloca(ft->c_args_size);
  void *rtn_space = alloca(ggrt_type_sizeof(ctx, ft->rtn_type));
  GGRT_V rtn_val;

  memset(arg_space, 0, ft->c_args_size);
  {
    void *arg_p = arg_space;
    int i;
    for ( i = 0; i < argc; ++ i ) {
      f_args[i] = arg_p;
      arg_p += ggrt_ffi_unbox_arg(ctx, ft->elems[i]->type, &argv[i], arg_p);
    }
  }

  ffi_call(&ft->f_cif, cfunc, rtn_space, f_args);
   
  ggrt_ffi_box(ctx, ft->rtn_type, rtn_space, rtn_valp);
}

/* Create a symbol definition. */
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

ggrt_symbol *ggrt_symbol_table_add_(ggrt_ctx ctx, ggrt_symbol_table *st, const char *name, void *addr, ggrt_type *type)
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

ggrt_symbol *ggrt_m_symbol(ggrt_ctx ctx, const char *name, void *addr, ggrt_type *type)
{
  ggrt_symbol *sym = ggrt_malloc(sizeof(*sym));
  memset(sym, 0, sizeof(*sym));
  sym->name = name ? ggrt_strdup(name) : name;
  sym->type = type;
  sym->addr = addr;
  return sym;
}

ggrt_symbol *ggrt_global(ggrt_ctx ctx, const char *name, void *addr, ggrt_type *type)
{
  ggrt_symbol *sym = ggrt_m_symbol(ctx, name, addr, type);
  ggrt_symbol_table_add(ctx, ctx->st_global, sym);
  return sym;
}

