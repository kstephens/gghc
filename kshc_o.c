/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "kshc_o.h"
#include "kshc_sym.h"
#include "kshc.h"	/* kshc_debug */
#include "cy.h"

extern	FILE*	kshc_precomp3;
extern	void	yyerror(const char*);

/*
** Input/output functions for kshc
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
  char	buf[8192]; /* ARRGGH! */
  va_list vap;
  va_start(vap, format);

  vsprintf(buf, format, vap);

  va_end(vap);
  return strdup(buf);
}

/***************************************************************************************************/

void	kshc_typedef(const char *name, const char *type)
{
  fprintf(kshc_precomp3,"\
  /* typedef %s */\n\
  kshc_typedef(\"%s\", %s);\n", name, name, type);
  if ( kshc_debug )
    printf("/* typedef %s */\n", name);
}

	/* integer constant */
char*	kshc_type(const char *name)
{
  return ssprintf("kshc_type(\"%s\")", name);
}

/* enum */
static struct kshc_enum {
  char *name;
  int named;
  char *type;
  struct kshc_enum *prev;
} *current_enum = 0;

char*	kshc_enum_type(const char *name)
{
  static int unnamed_enum_id = 1;
  struct kshc_enum *en = malloc(sizeof(struct kshc_enum));
  
  en->named = en->name != 0;
  if ( ! en->named ) {
    name = ssprintf("_kshc_unamed_%d", unnamed_enum_id ++);
  }
  en->name = (char*) name;
  en->prev = current_enum;
  current_enum = en;
  
  /* Create a enum type variable declaration */
  current_enum->type = ssprintf("_kshc_enum_%s_type", current_enum->name);
  fprintf(kshc_precomp30, "  KSHCT %s;\n", current_enum->type);
  
  /* Create the enum declaration initializer */
  fprintf(kshc_precomp3, "\
  /* enum %s */\n\
  %s = kshc_enum_type(\"%s\");\n", name, current_enum->type, (current_enum->named ? name : ""));
  if ( kshc_debug )
    printf("/* enum %s */\n", name);
  
  return current_enum->type;
}

void	kshc_enum_type_element(const char *name)
{
  fprintf(kshc_precomp3, "    kshc_enum_type_element(%s, \"%s\", (int) /* enum %s */ %s);\n", current_enum->type, name, current_enum->name, name);
  if ( kshc_debug )
    printf("  /* enum %s:%s */\n", current_enum->name, name);
}

void	kshc_enum_type_end(void)
{
  struct kshc_enum *en = current_enum->prev;

  fprintf(kshc_precomp3, "  kshc_enum_type_end(%s);\n\n", current_enum->type);
  
  free(current_enum->name);
  free(current_enum->type);
  free(current_enum);
  current_enum = en;
  
}

/*
** pointer
*/
char*	kshc_pointer_type(const char *type)
{
  return ssprintf("kshc_pointer_type(%s)", type);
}


/*
** array
*/
char*	kshc_array_type(const char *type, const char *length)
{
  return ssprintf("kshc_array_type(%s, %s)", type, (length && length[0] ? length : "-1"));
}


/*
** struct/union context
*/
static kshc_struct *current_struct = 0;
char *kshc_struct_type(const char *s_or_u, const char *name)
{
  static int unnamed_struct_id = 1;
  kshc_struct* s = malloc(sizeof(kshc_struct));
  s->prev = current_struct;
  current_struct = s;

  s->struct_or_union = (char*) s_or_u;
  s->named = name ? 1 : 0;
  s->name = (char*) (name ? name : ssprintf("_kshc_unamed_%s_%d", s_or_u, unnamed_struct_id ++));
  s->slots = strdup("");
  s->slots_text = strdup("");
  fprintf(kshc_precomp3, "/* %s %s */\n", current_struct->struct_or_union, current_struct->name);
  
  /* Create a struct type variable declaration */
  current_struct->type = ssprintf("_kshc_%s_type", current_struct->name);
  fprintf(kshc_precomp30, "  KSHCT %s;\n", current_struct->type);

  fprintf(kshc_precomp2, "/* %s %s */\n", current_struct->struct_or_union, current_struct->name);
  
  /* Create a struct type */
  fprintf(kshc_precomp3, "  %s = kshc_struct_type(\"%s\", \"%s\", sizeof(%s %s));\n", current_struct->type, current_struct->struct_or_union, (current_struct->named ? current_struct->name : ""), current_struct->struct_or_union, current_struct->name);
  
  return current_struct->type;
}

/* struct/union element */
void	kshc_struct_type_element(kshc_decl_spec *spec, kshc_decl *decl, const char *text)
{
  if ( decl->next ) {
    kshc_struct_type_element(spec, decl->next, text);
  } else {
    /* Add a Text slot */
    if ( ! current_struct->named ) {
      char *prev = current_struct->slots_text;
      current_struct->slots_text = ssprintf("%s%s\n", current_struct->slots_text, text);
      free(prev);
    }
  }
  {
    char*	stype; 
    char*	sprefix;
    char*	ctype;
    char*	estype;
  
    /* Create a C pointer type for the getter/setter functions */
    stype = ssprintf("((%s %s*)_s_)->%s", current_struct->struct_or_union, current_struct->name, decl->identifier);
  
    /* Create a common function prefix for the struct's slot */
    sprefix = ssprintf("_%s_%s_%s", current_struct->struct_or_union, current_struct->name, decl->identifier);
  
    /* Create getter/setter functions */
    if ( decl->is_bit_field ) {
      /* Create a C pointer type for the slot. */
      ctype = ssprintf(decl->declarator_text, "*");
      ctype = ssprintf("*((%s %s)_v_)", spec->type_text, ctype);
  
      fprintf(kshc_precomp2, "\
static void _kshc%s_get (void *_s_, void *_v_){%s = %s;}\n\
static void _kshc%s_set (void *_s_, void *_v_){%s = %s;}\n",
	      sprefix, ctype, stype,
	      sprefix, stype, ctype
      );
    } else {
      ctype = ssprintf("_v_");
      fprintf(kshc_precomp2,"\
static void _kshc%s_get (void *_s_, void *_v_){memcpy(%s, &(%s), sizeof(%s));}\n\
static void _kshc%s_set (void *_s_, void *_v_){memcpy(&(%s), %s, sizeof(%s));}\n",
	      sprefix, ctype, stype, stype,
	      sprefix, stype, ctype, stype
      );
    }
  
  
    /* Gencode for the element. */
    estype = ssprintf(decl->declarator, spec->type);
    fprintf(kshc_precomp3, "    kshc_struct_type_element(%s, %s, \"%s\", (kshc_funcp) &_kshc%s_get, (kshc_funcp) &_kshc%s_set);\n", current_struct->type, estype, decl->identifier, sprefix, sprefix);
      
    free(stype);
    free(sprefix);
    free(ctype);
    free(estype); 
  }
}

char *kshc_struct_type_end(void)
{
  char *type = current_struct->type;
  
  fprintf(kshc_precomp2, "\n\n");

  fprintf(kshc_precomp3, "  kshc_struct_type_end(%s);\n", current_struct->type);
  fprintf(kshc_precomp3, "  /* %s %s */\n\n", current_struct->struct_or_union, current_struct->name);

  if ( ! current_struct->named ) {
    fprintf(kshc_precomp1, "%s %s {\n%s\n};\n\n", 
	    current_struct->struct_or_union, current_struct->name,
	    current_struct->slots_text);

  }

  {
  kshc_struct* s = current_struct->prev;
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
char *kshc_function_type(const char *rtntype, const char *argtypes)
{
  if ( strcmp(argtypes, "kshc_type(\"void\")") == 0 ) {
    argtypes = "KSHCT_NULL";
  } else {
    argtypes = ssprintf("%s, KSHCT_NULL", argtypes);
  }
  return ssprintf("kshc_function_type(%s, %s)", rtntype, argtypes);
}


/*
** Top-level declaration;
*/
void kshc_declaration(kshc_decl_spec *spec, kshc_decl *decl)
{
  while ( decl ) {
    char *type = ssprintf(decl->declarator, spec->type);
    if ( ! (decl->identifier && decl->identifier[0]) ) {
      yyerror(ssprintf("no identifier for type %s", type));
    } else {
      /* The declaration is a typedef */
      if ( spec->storage == TYPEDEF ) {
	kshc_symbol* s;

	if ( kshc_symbol_get(decl->identifier) ) {
	  yyerror(ssprintf("%s already typedef", decl->identifier));
	}
	s = kshc_symbol_set(decl->identifier);
	s->value = "typedef";
	kshc_typedef(decl->identifier, type);
      } else
      /* The declaration is a global. */
      if ( 1 ) {
        kshc_global(decl->identifier, type);      
      } else {
      /* The declaration is something else */
	yywarning(ssprintf("ignoring declaration of '%s'", decl->identifier));
      }
    }

    free(type);
    {
    kshc_decl *decl__next = decl->next;
    free(decl);
    decl = decl__next;
    }
  }
  fprintf(kshc_precomp3, "\n");
}


/* global */
void	kshc_global(const char *name, const char *type)
{
  fprintf(kshc_precomp3, "  kshc_global(\"%s\", %s, (void*) &%s);\n", name, type, name);
}



