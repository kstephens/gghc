/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#ifndef _gghc_o_h
#define	_gghc_o_h

#include "gghc_t.h"

/*
** output functions for gghc
*/
char*	strdup(const char* s);
char*	ssprintf(const char* format, ...);

/* typedef */
void	gghc_typedef(const char *name, const char *type);
/* type reference */
char*	gghc_type(const char *typename);

/* enum */
char   *gghc_enum_type_forward(const char *name);
char*	gghc_enum_type(const char *name);
void	gghc_enum_type_element(const char *name);
char*	gghc_enum_type_end(void);

/* pointer */
char*	gghc_pointer_type(const char *type);

/* array */
char*	gghc_array_type(const char *type, const char *length);

/* struct/union */
char   *gghc_struct_type_forward(const char *s_or_u, const char *name);
char*	gghc_struct_type(const char *struct_or_union, const char *name);
void	gghc_struct_type_element(gghc_decl_spec *spec, gghc_decl *decl, const char *text);
char*	gghc_struct_type_end(void);

/* function */
char*	gghc_function_type(const char *type, const char *length);
char*	gghc_block_type(const char *type, const char *length);

/* top-level declaration */
void	gghc_declaration(gghc_decl_spec *spec, gghc_decl *decl);

/* global */
void	gghc_global(const char *name, const char *type);

void gghc_reset_state();

#endif
