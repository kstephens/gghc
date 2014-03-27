/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#ifndef _gghc_output_h
#define	_gghc_output_h

#include "gghc.h"
#include "gghc/decl.h"

/*
** output functions for gghc
*/
char*	strdup(const char* s);
char*	ssprintf(const char* format, ...);

void gghc_emit_control(gghc_ctx ctx, int i); /* -1 to pause, 1 to resume */

/* typedef */
void	gghc_typedef(gghc_ctx ctx, const char *name, const char *type);
/* type reference */
char*	gghc_type(gghc_ctx ctx, const char *typename);

/* enum */
char   *gghc_enum_type_forward(gghc_ctx ctx, const char *name);
char*	gghc_enum_type(gghc_ctx ctx, const char *name);
void	gghc_enum_type_element(gghc_ctx ctx, const char *name);
char*	gghc_enum_type_end(gghc_ctx ctx);

/* pointer */
char*	gghc_pointer_type(gghc_ctx ctx, const char *type);

/* array */
char*	gghc_array_type(gghc_ctx ctx, const char *type, const char *length);

/* struct/union */
char   *gghc_struct_type_forward(gghc_ctx ctx, const char *s_or_u, const char *name);
char*	gghc_struct_type(gghc_ctx ctx, const char *struct_or_union, const char *name);
void	gghc_struct_type_element(gghc_ctx ctx, gghc_decl_spec *spec, gghc_decl *decl, const char *text);
char*	gghc_struct_type_end(gghc_ctx ctx);

/* function */
char*	gghc_function_type(gghc_ctx ctx, const char *type, const char *length);
char*	gghc_block_type(gghc_ctx ctx, const char *type, const char *length);

/* top-level declaration */
void	gghc_declaration(gghc_ctx ctx, gghc_decl_spec *spec, gghc_decl *decl);

/* global */
void	gghc_global(gghc_ctx ctx, const char *name, const char *type);

/* CCP defines */
void    gghc_define(gghc_ctx ctx, const char *name, const char *str);

void gghc_reset_state(gghc_ctx ctx);

#endif
