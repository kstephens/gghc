# GGHC

GGHC - GlueGun C header compiler

## Overview

GGHC extracts C type information and other data from C header files, suitable for FFI generation.

## S-EXPR Output

GGHC will generate S-EXPR output suitable for parsing with a LISP reader.

## Examples:

    $ cd gghc
    $ make clean all
    $ bin/gghc gcc t/test.c > test.gghc.sexpr
    $ bin/gghc gcc stdlib.h | less

## Supported

* Operating Systems: OS X, Linux.
* C Compliers: GCC, CLANG.
* Headers: Parses stdlib.h, stdio.h.

## UNSUPPORTED

* typedef __typeof__ clang stdlib.h on Linux:

Earlier versions of gghc could not parse __typeof__(expression):

    $ bin/gghc clang stdlib.h
    gghc: ERROR: /usr/include/clang/3.2/include/stddef.h:31:19 syntax error
    gghc: token:                    `('
    gghc: line:  `typedef __typeof__(((int*)0)-((int*)0)) ptrdiff_t;'

Newer versions will grok it as "void".

* GCC inline union:

    struct test__with_inlne_union
    {
        int a;
        __extension__ union
        {
          int b;
          int c;
        };
        int d;
    };

gghc will parse it but ignores b and c.

## Dependencies

* make
* bison
* flex
* perl
* libffi

## C runtime

GGHC mode (unsupported) to generate C code that will call into a C type library.

1. GGHC runtime interface functions:

These functions define the interface between the output of GGHC and the runtime system using the definitions.

1.1. Module definition blocks

  void gghc_begin_module(const char *modulename);
  void gghc_end_module(const char *modulename);

  are called at the beginning and end of a module (or modules) ran through gghc.  The implementation can use this to notify the user of the modules being loaded or to place different modules into different namespaces.

1.2. Intrinsic types, typedefs and type references

1.2.1. Intrinsic types

  The runtime system must predefine the following following intrinsic types:
  
  "char"
  "unsigned char"
  "short"
  "unsigned short"
  "int"
  "unsigned int"
  "long"
  "unsigned long"
  "long long"			(if supported by the C compiler)
  "unsigned long long"	(if supported by the C compiler)
  "float"
  "double"
  "long double"		(if supported by the C compiler)

1.2.2. Typedefs

  void gghc_typedef(const char *symbol, gghcT ctype);

 binds a symbol to a C type object.  Generally the output of a typedef operation.

1.2.3. Type references.

  gghcT gghc_type(const char *typename);

is called when a type is required by name. Should raise an error if the type is not yet defined.

1.3. Composite types

 An implementation is free to cache composite types to reduce memory and preserve pointer equality.

1.3.1. Enumerations

  gghcT gghc_enum_type(const char *enumname);
  void gghc_enum_type_element(gghcT enumtype, const char *name, int enumvalue);
  void gghc_enum_type_end(gghcT enumtype);

 creates a new C enum type and binds enumname to the type.  If the enumname is "", the enum type is anonymous.

1.3.2. Pointers

  gghcT gghc_pointer_type(gghcT elementtype);

 creates a C pointer type.

1.3.2. Arrays

  gghcT gghc_array_type(gghcT elementtype, unsigned int length);

 creates a C array type of a given length. if the array is of an undetermined length, length is -1.

1.3.2. Structures and Unions

  gghcT gghc_struct_type(const char *struct_or_union, const char *structname, size_t _sizeof);
  void gghc_struct_type_element(gghcT structtype, gghcT elementtype, const char *elementname, gghc_funcp getfunc, gghc_funcp setfunc);
  void gghc_struct_type_end(gghcT structtype);

  gghc_struct_type() returns a uninitialized C struct type and binds the supplied name to the type.  If the name is "", the struct type is anonymous.
  
  The return value will be used in any self-referencing C struct or union types declarations such as:

  struct somestruct {
    int foo;
    struct somestruct *next;
  };
  
  For a structure of this type, gghc will generate:
  
  var = gghc_struct_type("somestruct", sizeof(struct somestruct));
  gghc_struct_type_element(var, "foo", gghc_type("int"), ...);
  gghc_struct_type_element(var, "next", gghc_pointer(var), ...);
  gghc_struct_type_end(var);

  The getfunc and setfunc for each element are declared as:
  
  void (*getfunc)(<struct> *structp, <elementtype> *dst);
  void (*setfunc)(<struct> *structp, <elementtype> *src);
  
  Where <struct> and <elementtype> are the C types of the struct and element respectively.
  
1.3.2. Functions
 
  gghcT gghc_function_type(gghcT rtntype, gghcT argtype, ...);

  creates a C function type object with a return and a list of required argument types.  The argument type list is terminated with a gghcT_NULL object.  If the function takes a variable number of arguments the argument type list is terminated with a gghcT_VARARGS type object.
 
1.4. Globals

  void gghc_global(const char *symbol, gghcT ctype, void *address);

 binds symbol to a new C object at an address.   This is called for each variable, array and function declaration.

1.5. Defines

  void gghc_define(const char *name, const char *tokens);

 captures #defines.  Client must decide if name has arguments or tokens are parsible as simple values.

## Copyright

Copyright 1993, 1994, 2014 Kurt A. Stephens

