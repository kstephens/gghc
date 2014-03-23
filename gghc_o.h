/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#ifndef _gghc_o_h
#define	_gghc_o_h

#include "gghc_t.h"

/*
** output functions for kshc
*/
char*	strdup(const char* s);
char*	ssprintf(const char* format, ...);

/* typedef */
void	kshc_typedef(const char *name, const char *type);
/* type reference */
char*	kshc_type(const char *typename);

/* enum */
char*	kshc_enum_type(const char *name);
void	kshc_enum_type_element(const char *name);
void	kshc_enum_type_end(void);

/* pointer */
char*	kshc_pointer_type(const char *type);

/* array */
char*	kshc_array_type(const char *type, const char *length);

/* struct/union */
char*	kshc_struct_type(const char *struct_or_union, const char *name);
void	kshc_struct_type_element(kshc_decl_spec *spec, kshc_decl *decl, const char *text);
char*	kshc_struct_type_end(void);

/* function */
char*	kshc_function_type(const char *type, const char *length);

/* top-level declaration */
void	kshc_declaration(kshc_decl_spec *spec, kshc_decl *decl);

/* global */
void	kshc_global(const char *name, const char *type);

#endif
