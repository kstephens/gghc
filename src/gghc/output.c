/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#define _GNU_SOURCE /* vasprintf() */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "gghc/output.h"
#include "gghc.h"
#include "cy.h"
#include "mzone.h"

/*
** callbacks for gghc
*/

void gghc_debug_stop()
{
}

void gghc_emit_control(gghc_ctx ctx, int i)
{
  ctx->_emit += i;
}

#define eprintf(FH,FMT,ARGS...) \
    do { if ( ctx->_emit ) fprintf(FH,FMT, ## ARGS); } while(0)

char *gghc_constant(gghc_ctx ctx, const char *c_expr)
{
  const char *prefix = "";
  char *expr = 0;
  int id = ++ ctx->constant_id;
  char *name;

  if ( mode_sexpr(ctx) ) {
    prefix = "gghc:";
  }

  name = ssprintf("%sconstant_%d", prefix, id);

  if ( mode_sexpr(ctx) ) {
    expr = ssprintf("%s #| %s |# ", name, c_expr);
  }
  if ( mode_c(ctx) ) {
    expr = ssprintf("%s /* %s */ ", name, id, c_expr);
  }
  eprintf(ctx->constants_c, "  _gghc_constant(%d,%s);\n", id, c_expr);

  ggrt_constant(ctx->rt, name, c_expr);

  return expr;
}

#define ctx ((gghc_ctx) rtctx->cb.user_data[0])

void _gghc_module_begin(ggrt_ctx rtctx, struct ggrt_module_t *mod)
{
    /* NOTHING */
}

void _gghc_module_end(ggrt_ctx rtctx, struct ggrt_module_t *mod)
{
    /* NOTHING */
}

void _gghc_pragma(ggrt_ctx rtctx, struct ggrt_pragma_t *obj)
{
  char *out = ggrt_escape_string(ctx->rt, obj->text);
  char *rep;

  if ( strncmp(obj->text, "gghc:yydebug", strlen("gghc:yydebug")) == 0 ) {
    int v = atoi(obj->text + strlen("gghc:yydebug") + 1);
    ctx->_yydebug = v;
  } else if ( strncmp(obj->text, "gghc:lexdebug", strlen("gghc:lexdebug")) == 0 ) {
    int v = atoi(obj->text + strlen("gghc:lexdebug") + 1);
    ctx->_lexdebug = v;
  }

  if ( mode_sexpr(ctx) ) {
    rep = ssprintf("(gghc:pragma \"%s\")\n", out);
    eprintf(ctx->body_out, "  %s", rep);
  }
  if ( mode_c(ctx) ) {
    abort();
  }
}

void _gghc_macro(ggrt_ctx rtctx, ggrt_macro_t *m)
{
  char *out = ggrt_escape_string(ctx->rt, m->text);
  char *rep;
  if ( mode_sexpr(ctx) ) {
    rep = ssprintf("(gghc:macro \"%s\" \"%s\")\n", m->name, out);
    if ( ctx->_emit_macros ) eprintf(ctx->defines_out, "  %s", rep);
  }
  if ( mode_c(ctx) ) {
    abort();
  }
  malloc_zone_free(ctx->rt->mz, out);
  m->cb_data[2] = rep;
}

void _gghc_typedef(ggrt_ctx rtctx, ggrt_typedef_t *td)
{
  const char *name = td->name;
  const char *type = td->type->cb_data[0];

  if ( mode_sexpr(ctx) ) {
    eprintf(ctx->body_out,"  (gghc:typedef \"%s\" %s)\n", name, type);
  }
  if ( mode_c(ctx) ) {
    eprintf(ctx->body_out,"  gghc_typedef(\"%s\", %s);\n", name, type);
  }
}

void _gghc_intrinsic(ggrt_ctx rtctx, struct ggrt_type_t *t)
{
  char *result;

  if ( t->cb_data[0] )
    return;

  if ( mode_sexpr(ctx) ) {
    result = ssprintf("(gghc:type \"%s\")", t->name);
  }
  if ( mode_c(ctx) ) {
    result = ssprintf("gghc_type(\"%s\")", t->name);
  }

  t->cb_data[0] = result;
}

/* enum */
gghc_enum *__gghc_enum(ggrt_ctx rtctx, struct ggrt_type_t *et)
{
  gghc_enum *s;
  char *name = 0;

  if ( et->cb_data[1] )
    return et->cb_data[1];

  s = malloc(sizeof(*s));
  memset(s, 0, sizeof(*s));
  s->type = et;

  if ( (et->name && *et->name) ) {
    name = (char*) et->name;
  } else {
    name = ssprintf("_gghc_unamed_%d", ++ ctx->unnamed_enum_id);
  }
  s->name = (char*) name;
  
  /* Create a enum type variable declaration */
  s->expr = ssprintf("_gghc_type_enum_%s", s->name);

  et->cb_data[0] = s->expr;
  et->cb_data[1] = s;

  return s;
}

void _gghc_enum(ggrt_ctx rtctx, struct ggrt_type_t *et)
{
  gghc_enum *s = __gghc_enum(rtctx, et);
  
  if ( ! s->emitted ) {
    s->emitted = 1;
    if ( mode_sexpr(ctx) ) {
      eprintf(ctx->body_out,
              "  (gghc:enum %s \"%s\") ;; forward\n", s->expr, (s->name ? s->name : ""));
    }
    if ( mode_c(ctx) ) {
      abort();
    }
  }
}

char *__gghc_enum_begin(ggrt_ctx rtctx, struct ggrt_type_t *et)
{
  gghc_enum *s = __gghc_enum(rtctx, et);
  const char * name = et->name && *et->name ? et->name : "";
  if ( mode_sexpr(ctx) ) {
    eprintf(ctx->body_out, "  (gghc:enum %s \"%s\"\n", s->expr, name);
  }
  if ( mode_c(ctx) ) {
  eprintf(ctx->decl_out, "  GGHCT %s;\n", s->expr);
  
  /* Create the enum declaration initializer */
  eprintf(ctx->body_out, "  %s = gghc_enum_type(\"%s\");\n", s->expr, name);
  }

  return s->expr;
}

void _gghc_enum_elem(ggrt_ctx rtctx, struct ggrt_type_t *et, struct ggrt_elem_t *elem)
{
  gghc_enum *s = __gghc_enum(rtctx, et);

  if ( s->emitted < 2 ) {
    s->emitted = 2;
    __gghc_enum_begin(rtctx, et);
  }

  if ( mode_sexpr(ctx) ) {
    eprintf(ctx->body_out, "    (gghc:enum-element \"%s\" %s)\n",
            elem->name,
            gghc_constant(ctx, elem->name));
  }
  if ( mode_c(ctx) ) {
  eprintf(ctx->body_out, "    gghc_enum_type_element(%s, \"%s\", (int) /* enum %s */ %s);\n",
          s->expr,
          elem->name,
          s->name,
          gghc_constant(ctx, elem->name));
  }
}

void _gghc_enum_end(ggrt_ctx rtctx, struct ggrt_type_t *et)
{
  gghc_enum *s = __gghc_enum(rtctx, et);

  if ( mode_sexpr(ctx) ) {
    eprintf(ctx->body_out, "  ) ;; enum %s\n", s->expr);
  }
  if ( mode_c(ctx) ) {
    eprintf(ctx->body_out, "  gghc_enum_type_end(%s);\n", s->expr);
  }
}

void _gghc_pointer(ggrt_ctx rtctx, struct ggrt_type_t *t)
{
  char *expr;

  if ( mode_sexpr(ctx) ) {
    expr = ssprintf("(gghc:pointer %s)", t->rtn_type->cb_data[0]);
  }
  if ( mode_c(ctx) ) {
    expr = ssprintf("gghc_pointer_type(%s)", t->rtn_type->cb_data[0]);
  }
  t->cb_data[0] = expr;
}

void _gghc_array(ggrt_ctx rtctx, struct ggrt_type_t *at)
{
}

static
int str_to_size_t(const char *str, size_t *sizep)
{
  char *str_end = 0;
  long long x = strtoll(str, &str_end, 0);
  if ( *str && str_end == strchr(str, 0) ) {
    *sizep = (size_t) x;
    return 1;
  }
  return 0;
}

static
const char *str_to_size_t_or_constant(gghc_ctx hcctx, const char *str, size_t *sizep)
{
  ggrt_ctx rtctx = hcctx->rt;
  *sizep = (size_t) -1L;
  if ( str_to_size_t(str, sizep) ) {
    if ( sizeof(size_t) == sizeof(int) ) {
      str = ssprintf("%d", (int) *sizep);
    } else if ( sizeof(size_t) == sizeof(long) ) {
      str = ssprintf("%ld", (long) *sizep);
    } else if ( sizeof(size_t) == sizeof(long long) ) {
      str = ssprintf("%lld", (long long) *sizep);
    } else {
      abort();
    }
  } else {
    str = gghc_constant(hcctx, str);
  }

  return str;
}

ggrt_type_t *gghc_array(gghc_ctx hcctx, struct ggrt_type_t *t, const char *length)
{
  ggrt_ctx rtctx = hcctx->rt;
  ggrt_type_t *at;
  char *expr;
  const char *length_expr = length;
  size_t len_val = (size_t) -1;

  length_expr = str_to_size_t_or_constant(hcctx, length, &len_val);
  at = ggrt_array(rtctx, t, len_val);

  if ( mode_sexpr(ctx) ) {
    expr = ssprintf("(gghc:array %s %s)", t->cb_data[0], length_expr);
  }
  if ( mode_c(ctx) ) {
    expr = ssprintf("gghc_array_type(%s, (size_t) (%s))", t->cb_data[0], length_expr);
  }

  at->cb_data[0] = expr;
  at->cb_data[5] = (void*) length;

  // fprintf(stderr, "    gghc_array(%p, \"%s\") => %p %s\n", t, length, at, expr);

  return at;
}

ggrt_type_t *gghc_bitfield(gghc_ctx hcctx, struct ggrt_type_t *t, const char *length)
{
  ggrt_ctx rtctx = hcctx->rt;
  ggrt_type_t *bt;
  char *expr;
  const char *length_expr = length;
  size_t len_val = (size_t) -1;

  length_expr = str_to_size_t_or_constant(hcctx, length, &len_val);
  bt = ggrt_t_bitfield(rtctx, t, len_val);

  if ( mode_sexpr(ctx) ) {
    expr = ssprintf("(gghc:bitfield %s %s)", t->cb_data[0], length_expr);
  }
  if ( mode_c(ctx) ) {
    expr = ssprintf("gghc_bitfield_type(%s, %s)", t->cb_data[0], length_expr);
  }

  bt->cb_data[0] = expr;
  bt->cb_data[5] = (void*) length;

  return bt;
}


gghc_struct *__gghc_struct(ggrt_ctx rtctx, struct ggrt_type_t *st)
{
  gghc_struct *s;

  if ( st->cb_data[1] )
    return st->cb_data[1];

  s = malloc(sizeof(*s));
  memset(s, 0, sizeof(*s));
  s->named = st->name && *st->name;
  s->name = (char*) (s->named ? st->name : ssprintf("_gghc_unamed_%s_%d", st->type, ++ ctx->unnamed_struct_id));
  s->slots = strdup("");
  s->slots_text = strdup("");
  s->expr = ssprintf("_gghc_type_%s_%s", st->type, s->name);

  s->type = st;
  st->cb_data[0] = s->expr;
  st->cb_data[1] = s;

  return s;
}

ggrt_type_t *gghc_struct_forward(gghc_ctx hcctx, const char *s_or_u, const char *name)
{
  ggrt_ctx rtctx = hcctx->rt;
  ggrt_type_t *st = ggrt_struct(rtctx, s_or_u, name);
  gghc_struct *s = __gghc_struct(rtctx, st);

  if ( ! s->emitted ) {
    s->emitted = 1;
    if ( mode_sexpr(ctx) ) {
        eprintf(ctx->body_out, "  (gghc:struct %s \"%s\" \"%s\") ;; forward\n",
              s->expr,
              st->type,
              st->name && *st->name ? s->name : "");
    }
    if ( mode_c(ctx) ) {
      abort();
    }
  }

  return st;
}

void _gghc_struct(ggrt_ctx rtctx, ggrt_type_t *st)
{
  gghc_struct *s = __gghc_struct(rtctx, st);

  s->prev = ctx->current_struct;
  ctx->current_struct = s;

  s->emitted = 1;
  if ( mode_sexpr(ctx) ) {
    eprintf(ctx->body_out, "  (gghc:struct %s \"%s\" \"%s\"\n",
            s->expr,
            st->type,
            s->named ? s->name : "");
  }

  if ( mode_c(ctx) ) {
    eprintf(ctx->body_out, "/* %s %s */\n", st->type, s->name);
  
    /* Create a struct type variable declaration */
    eprintf(ctx->decl_out, "  GGHCT %s;\n", s->expr);
  
    /* Create a struct type */
    eprintf(ctx->body_out, "  %s = gghc_struct_type(\"%s\", \"%s\", sizeof(%s %s));\n", s->expr, st->type, (s->named ? s->name : ""), st->type, s->name);
  }
}

void _gghc_struct_elem(ggrt_ctx rtctx, struct ggrt_type_t *st, ggrt_elem_t *elem)
{
}

/* struct/union element */
void gghc_struct_elem(gghc_ctx hcctx, gghc_declarator *decl)
{
  ggrt_ctx rtctx = hcctx->rt;
  gghc_struct *s = ctx->current_struct;
  ggrt_type_t *st = s->type;
  char *name = decl->identifier;
  char *text = ""; // ???
  char *stype = 0; 
  char *sprefix = 0;
  char *ctype = 0;
  char *estype = decl->type->cb_data[0];
  char *offset_expr = 0;
  char *sizeof_expr = 0;
  ggrt_elem_t *elem = 0; // FIXME

  elem = ggrt_struct_elem(rtctx, st, name, decl->type);

  /* Add a Text slot */
  if ( ! s->named ) {
    char *prev = s->slots_text;
    s->slots_text = ssprintf("%s%s\n", s->slots_text, text);
    free(prev);
  }
  
  if ( s->nelem == 0 ) {
    if ( s->named ) {
      sizeof_expr = gghc_constant(ctx,
                                  ssprintf("sizeof(%s %s)",
                                           st->type, st->name));
    }
    eprintf(ctx->body_out,
            "      (gghc:sizeof %s)\n",
            sizeof_expr ? sizeof_expr : "-1");
  }

  s->nelem ++;
  if ( s->named && ! decl->bit_field_size ) {
    offset_expr = gghc_constant(ctx,
                                ssprintf("_gghc_offsetof(%s %s, %s)",
                                         st->type, s->name,
                                         decl->identifier));
  }

  if ( mode_sexpr(ctx) ) {
    eprintf(ctx->body_out,
            "    (gghc:element \"%s\" %s\n      (ggch:offsetof %s))\n",
            decl->identifier,
            estype,
            offset_expr ? offset_expr : "-1"
            );
  }

  if ( mode_c(ctx) ) {
    /* Create a C pointer type for the getter/setter functions */
    stype = ssprintf("((%s %s*)_s_)->%s", st->type, s->name, decl->identifier);
  
    /* Create a common function prefix for the struct's slot */
    sprefix = ssprintf("_%s_%s_%s", st->type, s->name, decl->identifier);
  
    /* Create getter/setter functions */
    if ( decl->bit_field_size ) {
      /* Create a C pointer type for the slot. */
      ctype = ssprintf(ggrt_c_declarator(rtctx, ggrt_pointer(rtctx, decl->type)), "*");
      ctype = ssprintf("*((%s %s)_v_)", "~~FIXME~~", ctype);
  
      eprintf(ctx->decl_out, "\
static void _gghc%s_get (void *_s_, void *_v_){%s = %s;}\n\
static void _gghc%s_set (void *_s_, void *_v_){%s = %s;}\n",
	      sprefix, ctype, stype,
	      sprefix, stype, ctype
              );
    } else {
      ctype = ssprintf("_v_");
      eprintf(ctx->decl_out,"\
static void _gghc%s_get (void *_s_, void *_v_){memcpy(%s, &(%s), sizeof(%s));}\n\
static void _gghc%s_set (void *_s_, void *_v_){memcpy(&(%s), %s, sizeof(%s));}\n",
	      sprefix, ctype, stype, stype,
	      sprefix, stype, ctype, stype
              );
    }
  
    /* Gencode for the element. */
    estype = "~~FIXME~~";
    eprintf(ctx->body_out, "    gghc_struct_elem(%s, %s, \"%s\", (gghc_funcp) &_gghc%s_get, (gghc_funcp) &_gghc%s_set);\n", s->expr, estype, decl->identifier, sprefix, sprefix);
  }

  free(offset_expr);
  free(stype);
  free(sprefix);
  free(ctype);
  free(sizeof_expr);
}

void _gghc_struct_end(ggrt_ctx rtctx, ggrt_type_t *st)
{
  gghc_struct *s = st->cb_data[1];

  assert(s == ctx->current_struct);

  if ( mode_sexpr(ctx) ) {
    eprintf(ctx->body_out, "  )         ;; %s \"%s\" \"%s\"\n", s->expr, st->type, (s->named ? s->name : ""));
  }
  if ( mode_c(ctx) ) {
    eprintf(ctx->body_out, "  gghc_struct_type_end(%s);\n", s->expr);
    eprintf(ctx->body_out, "  /* %s %s */\n\n", st->type, s->name);

    if ( ! s->named ) {
      eprintf(ctx->decl_out, "%s %s {\n%s\n};\n\n", 
              st->type, s->name,
              s->slots_text);

    }
  }

  if ( s->nelem ) {
    free(s->slots);
    free(s->slots_text);
    // free(s->type);
    // free(s);
  }
  ctx->current_struct = s->prev;
}

void _gghc_func(ggrt_ctx rtctx, ggrt_type_t *ft)
{
  char *result;
  char *rtntype = ft->rtn_type->cb_data[0];
  char *argtypes = "";
  int argi;

  if ( mode_sexpr(ctx) ) {
    for ( argi = 0; argi < ft->nelems; argi ++ ) {
      ggrt_type_t *arg_type = ft->elems[argi]->type;
      argtypes = ssprintf("%s %s", argtypes, arg_type->cb_data[0]);
    }
    if ( strcmp(argtypes, "(gghc:type \"void\")") == 0 ) {
      argtypes = "";
    }
    result = ssprintf("(gghc:function %s%s)", rtntype, argtypes);
  }
  if ( mode_c(ctx) ) {
    if ( strcmp(argtypes, "gghc_type(\"void\")") == 0 ) {
      argtypes = "GGHCT_NULL";
    } else {
      argtypes = ssprintf("%s, GGHCT_NULL", argtypes);
    }
    result = ssprintf("gghc_function_type(%s, %s)", rtntype, argtypes);
  }
  ft->cb_data[0] = result;
}

void _gghc_global(ggrt_ctx rtctx, ggrt_global_t *g)
{
  const char *name = g->name;
  const char *type = g->type->cb_data[0];
  char *expr;
  if ( mode_sexpr(ctx) ) {
    expr = ssprintf("(gghc:global \"%s\" %s)", name, type);
  }
  if ( mode_c(ctx) ) {
    expr = ssprintf("  gghc_global(\"%s\", %s, (void*) &%s);", name, type, name);
  }
  eprintf(ctx->body_out, "  %s\n", expr);
  g->cb_data[0] = expr;
}

#undef ctx

void gghc_init_callbacks(gghc_ctx ctx)
{
  ctx->rt->cb.user_data[0] = ctx;

#define CB(N) ctx->rt->cb.N = _gghc##N
  CB(_module_begin);
  CB(_module_end);
  CB(_pragma);
  CB(_macro);
  // CB(_constant);
  CB(_intrinsic);
  CB(_typedef);
  CB(_pointer);
  CB(_array);
  // CB(_enum_forward);
  CB(_enum);
  CB(_enum_elem);
  CB(_enum_end);
  // CB(_struct_forward);
  CB(_struct);
  CB(_struct_elem);
  CB(_struct_end);
  CB(_func);
  CB(_global);
#undef CB
}

void gghc_emit_declarator(gghc_ctx ctx, gghc_declarator *decl)
{
  ggrt_type_t *t = decl->type;
  if ( ! t ) {
    gghc_yyerror(ctx, ssprintf("no type for identifier '%s'", decl->identifier));
  } else if ( ! (decl->identifier && decl->identifier[0]) ) {
    gghc_yyerror(ctx, ssprintf("no identifier for type '%s'", t->name));
  } else {
    if ( decl->declaration->storage == TYPEDEF ) {
      /* The declaration is a typedef */
      ggrt_typedef(ctx->rt, decl->identifier, t);
    } else if ( 1 ) {
      /* The declaration is a global. */
      ggrt_global(ctx->rt, decl->identifier, t, 0);
    } else {
      /* The declaration is something else */
      gghc_yywarning(ctx, ssprintf("ignoring declaration of '%s'", decl->identifier));
    }
  }
}

void gghc_emit_declaration(gghc_ctx ctx, gghc_declaration *decl)
{
  gghc_declarator *d = decl->declarators;
  while ( d ) {
    gghc_emit_declarator(ctx, d);
    d = d->prev_decl;
  }
}

