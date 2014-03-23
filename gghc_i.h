/*
** Copyright 1993, 1994 Kurt A. Stephens.
*/
#ifndef _kshc_i_h
#define	_kshc_i_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>	/* size_t */

#ifndef KSHCT
typedef void *KSHCT;
#endif

typedef	void	(*kshc_funcp)(void);

/* Begin/end module */
void	kshc_begin_module(const char *module);
void	kshc_end_module(const char *module);

/* Accept a #pragma kshc <str> */
void	kshc_pragma(const char *file, unsigned int line, const char *str);

/* Create a typedef */
void kshc_typedef(const char *symbol, KSHCT ctype);

/* Get a type by name */
KSHCT kshc_type(const char *type_name);

/* Create a enum type */
KSHCT kshc_enum_type(const char *enumname);
void kshc_enum_type_element(KSHCT enumtype, const char *name, int enumvalue);
void kshc_enum_type_end(KSHCT enumtype);

/* Create a pointer type */
KSHCT kshc_pointer_type(KSHCT elementtype);

/* Create an array type */
KSHCT kshc_array_type(KSHCT elementtype, int length);

/* Create a struct or union type */
KSHCT kshc_struct_type(const char *struct_or_union, const char *structname, size_t _sizeof);
void kshc_struct_type_element(KSHCT structtype, KSHCT elementtype, const char *elementname, kshc_funcp getfunc, kshc_funcp setfunc);
void kshc_struct_type_end(KSHCT structtype);

/* Create a function type */
#ifndef KSHCT_NULL
#define KSHCT_NULL (KSHCT)0
#endif
#ifndef KSHCT_VARARGS
#define KSHCT_VARARGS (KSHCT)-1
#endif
KSHCT kshc_function_type(KSHCT rtntype, ...);

/* Define a global */
void kshc_global(const char *symbol, KSHCT ctype, void *address);

/* Execute the initializers */
void	kshc_init(void); 

#ifdef __cplusplus
}

class _kshcInitializer {
public:
  kshc_funcp initfunc;
  _kshcInitializer* next;

  _kshcInitializer(kshc_funcp f);
};
#endif

#endif
