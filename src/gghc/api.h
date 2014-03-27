/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens.
*/
#ifndef _gghc_i_h
#define	_gghc_i_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>	/* size_t */

#ifndef gghc_ctx
typedef void *gghc_ctx;
#define gghc_ctx gghc_ctx
#endif

#ifndef GGHCT
typedef void *GGHCT;
#endif

typedef	void (*gghc_funcp)(void);

/* Begin/end module */
void	gghc_begin_module(gghc_ctx ctx, const char *module);
void	gghc_end_module(gghc_ctx ctx, const char *module);

/* Accept a #pragma gghc <str> */
void	gghc_pragma(gghc_ctx ctx, const char *file, unsigned int line, const char *str);

/* Create a typedef */
void gghc_typedef(gghc_ctx ctx, const char *symbol, GGHCT ctype);

/* Get a type by name */
GGHCT gghc_type(gghc_ctx ctx, const char *type_name);

/* Create a enum type */
GGHCT gghc_enum_type(gghc_ctx ctx, const char *enumname);
void gghc_enum_type_element(gghc_ctx ctx, GGHCT enumtype, const char *name, int enumvalue);
void gghc_enum_type_end(gghc_ctx ctx, GGHCT enumtype);

/* Create a pointer type */
GGHCT gghc_pointer_type(gghc_ctx ctx, GGHCT elementtype);

/* Create an array type */
GGHCT gghc_array_type(gghc_ctx ctx, GGHCT elementtype, int length);

/* Create a struct or union type */
GGHCT gghc_struct_type(gghc_ctx ctx, const char *struct_or_union, const char *structname, size_t _sizeof);
void gghc_struct_type_element(gghc_ctx ctx, GGHCT structtype, GGHCT elementtype, const char *elementname, gghc_funcp getfunc, gghc_funcp setfunc);
void gghc_struct_type_end(gghc_ctx ctx, GGHCT structtype);

/* Create a function type */
#ifndef GGHCT_NULL
#define GGHCT_NULL (GGHCT)0
#endif
#ifndef GGHCT_VARARGS
#define GGHCT_VARARGS (GGHCT)-1
#endif
GGHCT gghc_function_type(gghc_ctx ctx, GGHCT rtntype, ...);

/* Define a global */
void gghc_global(gghc_ctx ctx, const char *symbol, GGHCT ctype, void *address);

/* Execute the initializers */
void	gghc_init(gghc_ctx ctx);

#ifdef __cplusplus
}

class _gghcInitializer {
public:
  gghc_funcp initfunc;
  _gghcInitializer* next;

  _gghcInitializer(gghc_funcp f);
};
#endif

#endif
