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
** Input/output functions for gghc
*/

void gghc_debug_stop()
{
}

/***************************************************************************************************/

void gghc_emit_control(gghc_ctx ctx, int i)
{
  ctx->_emit += i;
}

#define eprintf(FH,FMT,ARGS...) \
    do { if ( ctx->_emit ) fprintf(FH,FMT, ## ARGS); } while(0)

void gghc_pragma(gghc_ctx ctx, const char *c_expr)
{
    /* NOTHING */
}

char *  gghc_constant(gghc_ctx ctx, const char *c_expr)
{
  const char *prefix = "";
  char *expr = 0;
  int id = ++ ctx->constant_id;

  if ( mode_sexpr ) {
    prefix = "gghc:";
    expr = ssprintf("%sconstant_%d #| %s |# ", prefix, id, c_expr);
  }
  if ( mode_c ) {
    expr = ssprintf("%sconstant_%d /* %s */ ", prefix, id, c_expr);
  }
  eprintf(ctx->constants_c, "  _gghc_constant(%d,%s);\n", id, c_expr);
  return expr;
}

void gghc_typedef(gghc_ctx ctx, const char *name, const char *type)
{
  ggrt_symbol *sym;

  if ( (sym = ggrt_symbol_table_by_name(ctx->rt, ctx->st_type, name)) ) {
    gghc_yyerror(ctx, ssprintf("'%s' is already a typedef", name));
    return;
  }

  // fprintf(stderr, "  @@@ gghc_typedef %s ==> %s\n", name, type);

  sym = ggrt_symbol_table_add_(ctx->rt, ctx->st_type, name, 0, 0);
  sym->value = strdup(type);

  if ( mode_sexpr ) {
  eprintf(ctx->body_out,"\
  ;; typedef %s \n\
  (gghc:typedef \"%s\" %s)\n", name, name, type);
  }
  if ( mode_c ) {
  eprintf(ctx->body_out,"\
  /* typedef %s */\n\
  gghc_typedef(\"%s\", %s);\n", name, name, type);
  if ( ctx->debug )
    printf("/* typedef %s */\n", name);
  }
}

char*	gghc_type(gghc_ctx ctx, const char *name)
{
  ggrt_symbol *sym;

  if ( (sym = ggrt_symbol_table_by_name(ctx->rt, ctx->st_type, name)) ) {
    return sym->value;
  }

  if ( mode_sexpr ) {
  return ssprintf("(gghc:type \"%s\")", name);
  }
  if ( mode_c ) {
  return ssprintf("gghc_type(\"%s\")", name);
  }
  return 0;
}

/* enum */
gghc_enum *_gghc_enum_type(gghc_ctx ctx, const char *name)
{
  gghc_enum *s;
  int named = name && *name;

  if ( named ) {
    ggrt_symbol *sym;
    if ( (sym = ggrt_symbol_table_by_name(ctx->rt, ctx->st_enum, name)) ) {
      // fprintf(stderr, "  @@@ gghc_enum_type enum %s ==> %s\n", name, sym->value);
      s = sym->addr;
      return s;
    }
  }

  s = malloc(sizeof(*s));
  memset(s, 0, sizeof(*s));

  s->named = named;
  if ( ! s->named ) {
    name = ssprintf("_gghc_unamed_%d", ++ ctx->unnamed_enum_id);
  }
  s->name = (char*) name;
  
  /* Create a enum type variable declaration */
  s->type = ssprintf("_gghc_type_enum_%s", s->name);

  if ( s->named ) {
    ggrt_symbol *sym = ggrt_symbol_table_add_(ctx->rt, ctx->st_enum, name, 0, 0);
    sym->value = strdup(s->type);
    sym->addr  = s;
  }

  return s;
}

char   *gghc_enum_type_forward(gghc_ctx ctx, const char *name)
{
  gghc_enum *s = _gghc_enum_type(ctx, name);

  if ( ! s->emitted ) {
    s->emitted = 1;
    if ( mode_sexpr ) {
      eprintf(ctx->body_out,
              "  (gghc:enum %s \"%s\") ;; forward\n", s->type, (s->named ? name : ""));
    }
    if ( mode_c ) {
      abort();
    }
  }

  return s->type;
}

char*	gghc_enum_type(gghc_ctx ctx, const char *name)
{
    gghc_enum *s = _gghc_enum_type(ctx, name);

  if ( mode_sexpr ) {
    eprintf(ctx->body_out, "  (gghc:enum %s \"%s\"\n",
            name, s->type);
  }
  if ( mode_c ) {
  eprintf(ctx->decl_out, "  GGHCT %s;\n", s->type);
  
  /* Create the enum declaration initializer */
  eprintf(ctx->body_out, "  %s = gghc_enum_type(\"%s\"); /* %s */\n", name, s->type, (s->named ? name : ""));
  }

  s->prev = ctx->current_enum;
  ctx->current_enum = s;

  return s->type;
}

void	gghc_enum_type_element(gghc_ctx ctx, const char *name)
{
  gghc_enum *s = ctx->current_enum;
  assert(s);

  s->nelem ++;
  if ( mode_sexpr ) {
    eprintf(ctx->body_out, "    (gghc:enum-element \"%s\" %s)\n",
            name,
            gghc_constant(ctx, name));
  }
  if ( mode_c ) {
  eprintf(ctx->body_out, "    gghc_enum_type_element(%s, \"%s\", (int) /* enum %s */ %s);\n",
          s->type,
          name,
          s->name,
          gghc_constant(ctx, name));
  }
  // if ( gghc_debug ) fprintf(stderr, "  /* enum %s:%s */\n", current_enum->name, name);
}

char *gghc_enum_type_end(gghc_ctx ctx) // FIXME
{
  gghc_enum *s = ctx->current_enum, *prev = s->prev;
  char *result = s->type;

  if ( mode_sexpr ) {
    eprintf(ctx->body_out, "  ) ;; enum %s\n\n", s->type);
  }
  if ( mode_c ) {
    eprintf(ctx->body_out, "  gghc_enum_type_end(%s);\n\n", s->type);
  }

  if ( s->nelem ) {
    // free(s->name);
    // free(s->type);
    // free(s);
  }

  ctx->current_enum = prev;
  return result;
}


/*
** pointer
*/
char*	gghc_pointer_type(gghc_ctx ctx, const char *type)
{
  assert(type && type[0]);
  assert(! (type[0] == 'c' && type[1] == 'o' && type[2] == 'n' && type[3] == 's' && type[4] == 't' && type[5] == ' '));

  if ( mode_sexpr ) {
  return ssprintf("(gghc:pointer %s)", type);
  }
  if ( mode_c ) {
  return ssprintf("gghc_pointer_type(%s)", type);
  }
  return 0;
}

/*
** array
*/
char*	gghc_array_type(gghc_ctx ctx, const char *type, const char *length)
{
  if ( mode_sexpr ) {
      return ssprintf("(gghc:array %s %s)", type, (length && length[0] ? gghc_constant(ctx, length) : "-1"));
  }
  if ( mode_c ) {
  return ssprintf("gghc_array_type(%s, %s)", type, (length && length[0] ? length : "-1"));
  }
  return 0;
}


/*
** struct/union context
*/
gghc_struct *_gghc_struct_type(gghc_ctx ctx, const char *s_or_u, const char *name)
{
  gghc_struct* s;
  int named = name && *name;
  ggrt_symbol_table *st = s_or_u[0] == 's' ? ctx->st_struct : ctx->st_union;

  if ( named ) {
    ggrt_symbol *sym;
    if ( (sym = ggrt_symbol_table_by_name(ctx->rt, st, name))  ) {
      // fprintf(stderr, "  @@@ gghc_struct_type %s ==> %s\n", typename, sym->value);
      return sym->addr;
    }
  }

  s = malloc(sizeof(*s));
  memset(s, 0, sizeof(*s));

  s->struct_or_union = (char*) s_or_u;
  s->named = named;
  s->name = (char*) (s->named ? name : ssprintf("_gghc_unamed_%s_%d", s_or_u, ++ ctx->unnamed_struct_id));
  s->slots = strdup("");
  s->slots_text = strdup("");
  s->type = ssprintf("_gghc_type_%s_%s", s->struct_or_union, s->name);

  if ( s->named ) {
    ggrt_symbol *sym = sym = ggrt_symbol_table_add_(ctx->rt, st, name, 0, 0);
    sym->value = strdup(s->type);
    sym->addr  = s;
  }
  return s;
}

char *gghc_struct_type_forward(gghc_ctx ctx, const char *s_or_u, const char *name)
{
  gghc_struct *s = _gghc_struct_type(ctx, s_or_u, name);

  if ( ! s->emitted ) {
    s->emitted = 1;
    if ( mode_sexpr ) {
        eprintf(ctx->body_out, "  (gghc:struct %s \"%s\" \"%s\") ;; forward\n",
              s->type,
              s->struct_or_union,
              s->named ? s->name : "");
    }
    if ( mode_c ) {
      abort();
    }
  }

  return s->type;
}

char *gghc_struct_type(gghc_ctx ctx, const char *s_or_u, const char *name)
{
  gghc_struct *s = _gghc_struct_type(ctx, s_or_u, name);

  s->emitted = 1;
  if ( mode_sexpr ) {
    eprintf(ctx->body_out, "  (gghc:struct %s \"%s\" \"%s\"\n",
            s->type,
            s->struct_or_union,
            s->named ? s->name : "");
  }

  if ( mode_c ) {
  eprintf(ctx->body_out, "/* %s %s */\n", s->struct_or_union, s->name);
  
  /* Create a struct type variable declaration */
  eprintf(ctx->decl_out, "  GGHCT %s;\n", s->type);
  
  /* Create a struct type */
  eprintf(ctx->body_out, "  %s = gghc_struct_type(\"%s\", \"%s\", sizeof(%s %s));\n", s->type, s->struct_or_union, (s->named ? s->name : ""), s->struct_or_union, s->name);
  }

  s->prev = ctx->current_struct;
  ctx->current_struct = s;

  return s->type;
}

/* struct/union element */
void	gghc_struct_type_element(gghc_ctx ctx, gghc_decl_spec *spec, gghc_decl *decl, const char *text)
{
  gghc_struct *s = ctx->current_struct;
    char*	stype = 0; 
    char*	sprefix = 0;
    char*	ctype = 0;
    char*	estype = 0;
      char *offset_expr = 0;
  char *sizeof_expr = 0;

  if ( decl->next ) {
      gghc_struct_type_element(ctx, spec, decl->next, text);
  } else {
    /* Add a Text slot */
    if ( ! ctx->current_struct->named ) {
      char *prev = s->slots_text;
      s->slots_text = ssprintf("%s%s\n", s->slots_text, text);
      free(prev);
    }
  }
  
  if ( s->nelem == 0 ) {
    if ( s->named ) {
        sizeof_expr = gghc_constant(ctx, ssprintf("sizeof(%s %s)", s->struct_or_union, s->name));
    }
    eprintf(ctx->body_out, "      (gghc:sizeof %s)\n",
          sizeof_expr ? sizeof_expr : "-1");
  }

  s->nelem ++;
  if ( s->named && ! decl->is_bit_field ) {
      offset_expr = gghc_constant(ctx, ssprintf("_gghc_offsetof(%s %s, %s)",
                                    s->struct_or_union, s->name,
                                    decl->identifier));
  }

  if ( mode_sexpr ) {
    assert(decl->declarator[0]);
    assert(spec->type[0]);
    estype = ssprintf(decl->declarator, spec->type);
    if ( decl->is_bit_field ) {
        estype = ssprintf("(gghc:bitfield %s %s)", estype, gghc_constant(ctx, decl->bit_field_size));
    }
    eprintf(ctx->body_out, "    (gghc:element \"%s\" %s\n      (ggch:offsetof %s))\n",
            decl->identifier,
            estype,
            offset_expr ? offset_expr : "-1"
            );
  }

  if ( mode_c ) {
    /* Create a C pointer type for the getter/setter functions */
    stype = ssprintf("((%s %s*)_s_)->%s", s->struct_or_union, s->name, decl->identifier);
  
    /* Create a common function prefix for the struct's slot */
    sprefix = ssprintf("_%s_%s_%s", s->struct_or_union, s->name, decl->identifier);
  
    /* Create getter/setter functions */
    if ( decl->is_bit_field ) {
      /* Create a C pointer type for the slot. */
      ctype = ssprintf(decl->declarator_text, "*");
      ctype = ssprintf("*((%s %s)_v_)", spec->type_text, ctype);
  
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
    estype = ssprintf(decl->declarator, spec->type);
    eprintf(ctx->body_out, "    gghc_struct_type_element(%s, %s, \"%s\", (gghc_funcp) &_gghc%s_get, (gghc_funcp) &_gghc%s_set);\n", s->type, estype, decl->identifier, sprefix, sprefix);
  }

  free(offset_expr);
    free(stype);
    free(sprefix);
    free(ctype);
    free(estype);
  free(sizeof_expr); 
}

char *gghc_struct_type_end(gghc_ctx ctx)
{
  gghc_struct *s = ctx->current_struct, *prev = s->prev;
  char *type = s->type;
  
  if ( mode_sexpr ) {
      eprintf(ctx->body_out, "  )         ;; %s \"%s\" \"%s\"\n", s->type, s->struct_or_union, (s->named ? s->name : ""));
  }
  if ( mode_c ) {
  eprintf(ctx->body_out, "  gghc_struct_type_end(%s);\n", s->type);
  eprintf(ctx->body_out, "  /* %s %s */\n\n", s->struct_or_union, s->name);

  if ( ! s->named ) {
    eprintf(ctx->decl_out, "%s %s {\n%s\n};\n\n", 
	    s->struct_or_union, s->name,
	    s->slots_text);

  }
  }

  if ( s->nelem ) {
    free(s->slots);
    free(s->slots_text);
    // free(s->type);
    // free(s);
  }
  ctx->current_struct = prev;
  
  return type;
}


/*
** function;
*/
char *gghc_function_type(gghc_ctx ctx, const char *rtntype, const char *argtypes)
{
  if ( mode_sexpr ) {
    if ( strcmp(argtypes, "(gghc:type \"void\")") == 0 ) {
      argtypes = "";
    }
    return ssprintf("(gghc:function %s %s)", rtntype, argtypes);
  }
  if ( mode_c ) {
  if ( strcmp(argtypes, "gghc_type(\"void\")") == 0 ) {
    argtypes = "GGHCT_NULL";
  } else {
    argtypes = ssprintf("%s, GGHCT_NULL", argtypes);
  }
  return ssprintf("gghc_function_type(%s, %s)", rtntype, argtypes);
  }
  return 0;
}

char *gghc_block_type(gghc_ctx ctx, const char *rtntype, const char *argtypes)
{
  if ( mode_sexpr ) {
    if ( strcmp(argtypes, "(gghc:type \"void\")") == 0 ) {
      argtypes = "";
    }
    return ssprintf("(gghc:block %s %s)", rtntype, argtypes);
  }
  if ( mode_c ) {
  if ( strcmp(argtypes, "gghc_type(\"void\")") == 0 ) {
    argtypes = "GGHCT_NULL";
  } else {
    argtypes = ssprintf("%s, GGHCT_NULL", argtypes);
  }
  return ssprintf("gghc_block_type(%s, %s)", rtntype, argtypes);
  }
  return 0;
}

/*
** Top-level declaration;
*/
void gghc_declaration(gghc_ctx ctx, gghc_decl_spec *spec, gghc_decl *decl)
{
  while ( decl ) {
    gghc_decl *next = decl->next;
    char *type = ssprintf(decl->declarator, spec->type);
    // fprintf(stderr, "  gghc_declaration: type=%s ident=%s\n", type, decl->identifier);
    if ( ! (spec->type && spec->type[0]) ) {
        gghc_yyerror(ctx, ssprintf("no type for identifier '%s'", decl->identifier));
    } else
    if ( ! (decl->identifier && decl->identifier[0]) ) {
        gghc_yyerror(ctx, ssprintf("no identifier for type '%s'", type));
    } else {
      /* The declaration is a typedef */
      if ( spec->storage == TYPEDEF ) {
          gghc_typedef(ctx, decl->identifier, type);
      } else
      /* The declaration is a global. */
      if ( 1 ) {
          gghc_global(ctx, decl->identifier, type);
      } else {
      /* The declaration is something else */
        gghc_yywarning(ctx, ssprintf("ignoring declaration of '%s'", decl->identifier));
      }
    }

    free(type);
    free(decl);
    decl = next;
  }
  eprintf(ctx->body_out, "\n");
}


/* global */
void	gghc_global(gghc_ctx ctx, const char *name, const char *type)
{
  if ( mode_sexpr ) {
    eprintf(ctx->body_out, "  (gghc:global \"%s\" %s)\n", name, type);
  }
  if ( mode_c ) {
  eprintf(ctx->body_out, "  gghc_global(\"%s\", %s, (void*) &%s);\n", name, type, name);
  }
}

char *gghc_escape_string(gghc_ctx ctx, const char *str)
{
    char *out = malloc(strlen(str) * 2 + 1);
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

void    gghc_define(gghc_ctx ctx, const char *name, const char *str)
{
  char *out = gghc_escape_string(ctx, str);
  if ( mode_sexpr ) {
    eprintf(ctx->defines_out, "  (gghc:define \"%s\" \"%s\")\n", name, out);
  }
  if ( mode_c ) {
    abort();
  }
  free(out);
}




