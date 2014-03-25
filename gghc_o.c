/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "gghc_o.h"
#include "gghc_sym.h"
#include "gghc.h"	/* gghc_debug */
#include "cy.h"

extern	FILE*	gghc_precomp3;
extern	void	yyerror(const char*);

/*
** Input/output functions for gghc
*/

char*	strdup(const char* s)
{
  size_t size = strlen(s) + 1;
  char*	p = malloc(size);
  memcpy(p, s, size);
  return p;
}

char*	ssprintf(const char* format, ...)
{
  char *buf = 0;
  va_list vap;
  va_start(vap, format);
  vasprintf(&buf, format, vap);
  va_end(vap);
  return buf;
}

void gghc_debug_stop()
{
}

/***************************************************************************************************/

static int constant_id = 0;
char *  gghc_constant(const char *c_expr)
{
  const char *prefix = "";
  char *expr = 0;
  int id = ++ constant_id;

  if ( mode_sexpr ) {
    prefix = "gghc:";
    expr = ssprintf("%sconstant_%d #| %s |#", prefix, id, c_expr);
  }
  if ( mode_c ) {
    expr = ssprintf("%sconstant_%d /* %s */", prefix, id, c_expr);
  }
  fprintf(gghc_precomp0, "  _gghc_constant(%d,%s);\n", id, c_expr);
  return expr;
}

void	gghc_typedef(const char *name, const char *type)
{
  gghc_symbol *sym;

  if ( (sym = gghc_symbol_get(name)) ) {
    yyerror(ssprintf("'%s' is already a typedef", name));
    return;
  }

  fprintf(stderr, "  @@@ gghc_typedef %s ==> %s\n", name, type);

  sym = gghc_symbol_set(name);
  sym->value = strdup(type);
  sym->type  = strdup("typedef");

  if ( mode_sexpr ) {
  fprintf(gghc_precomp3,"\
  ;; typedef %s \n\
  (gghc:typedef \"%s\" %s)\n", name, name, type);
  }
  if ( mode_c ) {
  fprintf(gghc_precomp3,"\
  /* typedef %s */\n\
  gghc_typedef(\"%s\", %s);\n", name, name, type);
  if ( gghc_debug )
    printf("/* typedef %s */\n", name);
  }
}

char*	gghc_type(const char *name)
{
  gghc_symbol *sym;

  if ( (sym = gghc_symbol_get(name)) ) {
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
static struct gghc_enum {
  char *name;
  int named;
  char *type;
  struct gghc_enum *prev;
} *current_enum = 0;

static int unnamed_enum_id = 1;
char*	gghc_enum_type(const char *name)
{
  struct gghc_enum *en;
  int named = name && *name;
  char *typename = 0;

  if ( named ) {
    gghc_symbol *sym;
    typename = ssprintf("enum %s", name);
    if ( (sym = gghc_symbol_get(typename)) ) {
      fprintf(stderr, "  @@@ gghc_enum_type enum %s ==> %s\n", typename, sym->value);
      return sym->value;
    }
  }

  en = malloc(sizeof(struct gghc_enum));
  memset(en, 0, sizeof(*en));

  en->named = named;
  if ( ! en->named ) {
    name = ssprintf("_gghc_unamed_%d", unnamed_enum_id ++);
  }
  en->name = (char*) name;
  en->prev = current_enum;
  current_enum = en;
  
  /* Create a enum type variable declaration */
  current_enum->type = ssprintf("_gghc_enum_%s_type", current_enum->name);

  if ( mode_sexpr ) {
  fprintf(gghc_precomp3, "\
  ;; enum %s\n\
  (gghc:enum %s \"%s\"\n", name, current_enum->type, (current_enum->named ? name : ""));
  }
  if ( mode_c ) {
  fprintf(gghc_precomp30, "  GGHCT %s;\n", current_enum->type);
  
  /* Create the enum declaration initializer */
  fprintf(gghc_precomp3, "\
  /* enum %s */\n\
  %s = gghc_enum_type(\"%s\");\n", name, current_enum->type, (current_enum->named ? name : ""));
  }
  if ( gghc_debug )
    fprintf(stderr, "/* enum %s */\n", name);

  if ( typename ) {
    gghc_symbol *sym = gghc_symbol_set(typename);
    sym->value = strdup(en->type);
    sym->type  = strdup("enum");
  }

  return en->type;
}

void	gghc_enum_type_element(const char *name)
{
  if ( mode_sexpr ) {
    fprintf(gghc_precomp3, "    (gghc:enum-element \"%s\" %s)\n",
            name,
            gghc_constant(name));
  }
  if ( mode_c ) {
  fprintf(gghc_precomp3, "    gghc_enum_type_element(%s, \"%s\", (int) /* enum %s */ %s);\n",
          current_enum->type,
          name,
          current_enum->name,
          gghc_constant(name));
  }
  if ( gghc_debug )
    printf("  /* enum %s:%s */\n", current_enum->name, name);
}

char *gghc_enum_type_end(void)
{
  struct gghc_enum *s = current_enum, *prev = s->prev;
  char *result = s->type;

  if ( mode_sexpr ) {
    fprintf(gghc_precomp3, "  ) ;; enum %s)\n\n", s->type);
  }
  if ( mode_c ) {
    fprintf(gghc_precomp3, "  gghc_enum_type_end(%s);\n\n", s->type);
  }

  free(s->name);
  // free(s->type);
  free(s);
  current_enum = prev;
  return result;
}

/*
** pointer
*/
char*	gghc_pointer_type(const char *type)
{
    assert(type && type[0]);
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
char*	gghc_array_type(const char *type, const char *length)
{
  if ( mode_sexpr ) {
    return ssprintf("(gghc:array %s %s)", type, (length && length[0] ? gghc_constant(length) : "-1"));
  }
  if ( mode_c ) {
  return ssprintf("gghc_array_type(%s, %s)", type, (length && length[0] ? length : "-1"));
  }
  return 0;
}


/*
** struct/union context
*/
static gghc_struct *current_struct = 0;
static int unnamed_struct_id = 1;
char *gghc_struct_type(const char *s_or_u, const char *name)
{
  char *sizeof_expr = 0;
  gghc_struct* s;
  char *typename = 0;
  int named = name && *name;

  if ( named ) {
    gghc_symbol *sym;
    typename = ssprintf("%s %s", s_or_u, name);
    if ( (sym = gghc_symbol_get(typename)) ) {
      fprintf(stderr, "  @@@ gghc_struct_type %s ==> %s\n", typename, sym->value);
      return sym->value;
    }
  }

  s = malloc(sizeof(*s));
  memset(s, 0, sizeof(*s));

  s->prev = current_struct;
  current_struct = s;

  s->struct_or_union = (char*) s_or_u;
  s->named = named;
  s->name = (char*) (s->named ? name : ssprintf("_gghc_unamed_%s_%d", s_or_u, unnamed_struct_id ++));
  s->slots = strdup("");
  s->slots_text = strdup("");
  current_struct->type = ssprintf("_gghc_%s_type", current_struct->name);

  if ( current_struct->named ) {
    sizeof_expr = gghc_constant(ssprintf("sizeof(%s %s)", current_struct->struct_or_union, current_struct->name));
  }

  if ( mode_sexpr ) {
  fprintf(gghc_precomp3, "  ;; %s %s {\n", current_struct->struct_or_union, current_struct->name);
  fprintf(gghc_precomp3, "  (gghc:struct %s \"%s\" \"%s\" (gghc:sizeof %s))\n",
          current_struct->type,
          current_struct->struct_or_union,
          current_struct->named ? current_struct->name : "",
          sizeof_expr ? sizeof_expr : "-1");
  }

  if ( mode_c ) {
  fprintf(gghc_precomp3, "/* %s %s */\n", current_struct->struct_or_union, current_struct->name);
  
  /* Create a struct type variable declaration */
  fprintf(gghc_precomp30, "  GGHCT %s;\n", current_struct->type);

  fprintf(gghc_precomp2, "/* %s %s */\n", current_struct->struct_or_union, current_struct->name);
  
  /* Create a struct type */
  fprintf(gghc_precomp3, "  %s = gghc_struct_type(\"%s\", \"%s\", sizeof(%s %s));\n", current_struct->type, current_struct->struct_or_union, (current_struct->named ? current_struct->name : ""), current_struct->struct_or_union, current_struct->name);
  }

  if ( typename ) {
    gghc_symbol *sym = gghc_symbol_set(typename);
    sym->value = strdup(s->type);
    sym->type  = strdup(s->struct_or_union);
  }

  return current_struct->type;
}

/* struct/union element */
void	gghc_struct_type_element(gghc_decl_spec *spec, gghc_decl *decl, const char *text)
{
    char*	stype = 0; 
    char*	sprefix = 0;
    char*	ctype = 0;
    char*	estype = 0;
      char *offset_expr = 0;

  if ( decl->next ) {
    gghc_struct_type_element(spec, decl->next, text);
  } else {
    /* Add a Text slot */
    if ( ! current_struct->named ) {
      char *prev = current_struct->slots_text;
      current_struct->slots_text = ssprintf("%s%s\n", current_struct->slots_text, text);
      free(prev);
    }
  }

  if ( current_struct->named && ! decl->is_bit_field ) {
    offset_expr = gghc_constant(ssprintf("_gghc_offsetof(%s %s, %s)",
                                    current_struct->struct_or_union, current_struct->name,
                                    decl->identifier));
  }

  if ( mode_sexpr ) {
    assert(decl->declarator[0]);
    assert(spec->type[0]);
    estype = ssprintf(decl->declarator, spec->type);
    if ( decl->is_bit_field ) {
      estype = ssprintf("(gghc:bitfield %s %s)", estype, gghc_constant(decl->bit_field_size));
    }
    fprintf(gghc_precomp3, "    (gghc:element %s \"%s\" %s (ggch:offsetof %s))\n",
            current_struct->type,
            decl->identifier,
            estype,
            offset_expr ? offset_expr : "-1"
            );
  }
  if ( mode_c ) {
  {
    /* Create a C pointer type for the getter/setter functions */
    stype = ssprintf("((%s %s*)_s_)->%s", current_struct->struct_or_union, current_struct->name, decl->identifier);
  
    /* Create a common function prefix for the struct's slot */
    sprefix = ssprintf("_%s_%s_%s", current_struct->struct_or_union, current_struct->name, decl->identifier);
  
    /* Create getter/setter functions */
    if ( decl->is_bit_field ) {
      /* Create a C pointer type for the slot. */
      ctype = ssprintf(decl->declarator_text, "*");
      ctype = ssprintf("*((%s %s)_v_)", spec->type_text, ctype);
  
      fprintf(gghc_precomp2, "\
static void _gghc%s_get (void *_s_, void *_v_){%s = %s;}\n\
static void _gghc%s_set (void *_s_, void *_v_){%s = %s;}\n",
	      sprefix, ctype, stype,
	      sprefix, stype, ctype
      );
    } else {
      ctype = ssprintf("_v_");
      fprintf(gghc_precomp2,"\
static void _gghc%s_get (void *_s_, void *_v_){memcpy(%s, &(%s), sizeof(%s));}\n\
static void _gghc%s_set (void *_s_, void *_v_){memcpy(&(%s), %s, sizeof(%s));}\n",
	      sprefix, ctype, stype, stype,
	      sprefix, stype, ctype, stype
      );
    }
  
    /* Gencode for the element. */
    estype = ssprintf(decl->declarator, spec->type);
    fprintf(gghc_precomp3, "    gghc_struct_type_element(%s, %s, \"%s\", (gghc_funcp) &_gghc%s_get, (gghc_funcp) &_gghc%s_set);\n", current_struct->type, estype, decl->identifier, sprefix, sprefix);
      
  }
  }
  free(offset_expr);
    free(stype);
    free(sprefix);
    free(ctype);
    free(estype); 
}

char *gghc_struct_type_end(void)
{
  char *type = current_struct->type;
  
  if ( mode_sexpr ) {
  fprintf(gghc_precomp3, "  (gghc:struct-end %s)\n", current_struct->type);
  fprintf(gghc_precomp3, "  ;; %s %s }\n\n", current_struct->struct_or_union, current_struct->name);
  }
  if ( mode_c ) {
  fprintf(gghc_precomp2, "\n\n");

  fprintf(gghc_precomp3, "  gghc_struct_type_end(%s);\n", current_struct->type);
  fprintf(gghc_precomp3, "  /* %s %s */\n\n", current_struct->struct_or_union, current_struct->name);

  if ( ! current_struct->named ) {
    fprintf(gghc_precomp1, "%s %s {\n%s\n};\n\n", 
	    current_struct->struct_or_union, current_struct->name,
	    current_struct->slots_text);

  }
  }

  {
  gghc_struct* s = current_struct->prev;
  free(current_struct->slots);
  free(current_struct->slots_text);
  free(current_struct);
  current_struct = s;
  }
  
  return type;
}


/*
** function;
*/
char *gghc_function_type(const char *rtntype, const char *argtypes)
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


/*
** Top-level declaration;
*/
void gghc_declaration(gghc_decl_spec *spec, gghc_decl *decl)
{
  while ( decl ) {
    char *type = ssprintf(decl->declarator, spec->type);
    fprintf(stderr, "  gghc_declaration: type=%s ident=%s\n", type, decl->identifier);
    if ( ! (spec->type && spec->type[0]) ) {
      yyerror(ssprintf("no type for identifier '%s'", decl->identifier));
    } else
    if ( ! (decl->identifier && decl->identifier[0]) ) {
      yyerror(ssprintf("no identifier for type '%s'", type));
    } else {
      /* The declaration is a typedef */
      if ( spec->storage == TYPEDEF ) {
	gghc_typedef(decl->identifier, type);
      } else
      /* The declaration is a global. */
      if ( 1 ) {
        gghc_global(decl->identifier, type);      
      } else {
      /* The declaration is something else */
	yywarning(ssprintf("ignoring declaration of '%s'", decl->identifier));
      }
    }

    free(type);
    {
    gghc_decl *decl__next = decl->next;
    free(decl);
    decl = decl__next;
    }
  }
  fprintf(gghc_precomp3, "\n");
}


/* global */
void	gghc_global(const char *name, const char *type)
{
  if ( mode_sexpr ) {
    fprintf(gghc_precomp3, "  (gghc:global \"%s\" %s)\n", name, type);
  }
  if ( mode_c ) {
  fprintf(gghc_precomp3, "  gghc_global(\"%s\", %s, (void*) &%s);\n", name, type, name);
  }
}

void gghc_reset_state()
{
  constant_id = 0;
  current_enum = 0;
  unnamed_enum_id = 0;
  current_struct = 0;
  unnamed_struct_id = 0;
}



