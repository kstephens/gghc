/*
** Copyright 1993, 1994 Kurt A. Stephens.
*/
#ifndef _gghc_i_h
#define	_gghc_i_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>	/* size_t */

#ifndef KSHCT
typedef void *KSHCT;
#endif

typedef	void	(*gghc_funcp)(void);

/* Begin/end module */
void	gghc_begin_module(const char *module);
void	gghc_end_module(const char *module);

/* Accept a #pragma gghc <str> */
void	gghc_pragma(const char *file, unsigned int line, const char *str);

/* Create a typedef */
void gghc_typedef(const char *symbol, KSHCT ctype);

/* Get a type by name */
KSHCT gghc_type(const char *type_name);

/* Create a enum type */
KSHCT gghc_enum_type(const char *enumname);
void gghc_enum_type_element(KSHCT enumtype, const char *name, int enumvalue);
void gghc_enum_type_end(KSHCT enumtype);

/* Create a pointer type */
KSHCT gghc_pointer_type(KSHCT elementtype);

/* Create an array type */
KSHCT gghc_array_type(KSHCT elementtype, int length);

/* Create a struct or union type */
KSHCT gghc_struct_type(const char *struct_or_union, const char *structname, size_t _sizeof);
void gghc_struct_type_element(KSHCT structtype, KSHCT elementtype, const char *elementname, gghc_funcp getfunc, gghc_funcp setfunc);
void gghc_struct_type_end(KSHCT structtype);

/* Create a function type */
#ifndef KSHCT_NULL
#define KSHCT_NULL (KSHCT)0
#endif
#ifndef KSHCT_VARARGS
#define KSHCT_VARARGS (KSHCT)-1
#endif
KSHCT gghc_function_type(KSHCT rtntype, ...);

/* Define a global */
void gghc_global(const char *symbol, KSHCT ctype, void *address);

/* Execute the initializers */
void	gghc_init(void); 

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
