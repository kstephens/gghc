/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#ifndef	_gghc_decl_h
#define	_gghc_decl_h

#include <stddef.h>	/* wchar_t */
#include "ggrt/mm_buf.h"

typedef struct gghc_declaration {
  ggrt_type_t *type; /* void, int, float, double, etc */
  int   storage;
  int   flags;
#define	GGHC_UNSIGNED	(1 << 0)
#define	GGHC_SIGNED	(1 << 1)
#define	GGHC_VOID	(1 << 2)
#define	GGHC_CHAR	(1 << 3)
#define	GGHC_SHORT	(1 << 4)
#define	GGHC_INT	(1 << 5)
#define	GGHC_LONG	(1 << 6) /* long long */
#define	GGHC_FLOAT	(1 << 8)
#define	GGHC_DOUBLE	(1 << 9)
#define GGHC_EXTERN     (1 << 10)
#define GGHC_STATIC     (1 << 11)
  char *expr; // emitted expr.
  struct gghc_declarator *declarators;
  struct gghc_declaration* prev; /* _begin(), _end(); */
} gghc_declaration;

typedef struct gghc_declarator {
  const char *syntax;
  char *identifier;
  ggrt_type_t *type; /* e.g.: void *, int [4], etc. */
  gghc_declaration *declaration;
  short	is_parenthised;
  char *bit_field_size;
  struct gghc_declarator* prev; /* declaration.declarations */
  struct gghc_declarator* prev_decl; /* _begin(), _end() */
} gghc_declarator;

typedef struct gghc_enum {
  int emitted;
  const char *name;
  ggrt_type_t *type;
  char *expr;
  short	named;
  struct gghc_enum *prev;
} gghc_enum;

typedef struct gghc_struct {
  int emitted;
  const char *name;
  ggrt_type_t *type;
  char *expr;
  int named;
  int nelem;
  char *slots;
  char *slots_text;
  struct gghc_struct *prev;
} gghc_struct;

typedef struct gghc_YYSTYPE {
  union {
    char		c;
    wchar_t		wc;
    unsigned char	uc;
    short		s;
    unsigned short	us;
    int			i;
    unsigned int	ui;
    long		l;
    unsigned long	ul;
    long long		ll;
    unsigned long long ull;
    float		f;
    double		d;
    long double		ld;	
    char*		cp;
    wchar_t*		wcp;
  } u;
  int token;
  int leng;
  mm_buf_region t;
  char *text; // text representation
  ggrt_type_t *type;
  ggrt_parameter_t *param;
  char *expr;
} gghc_YYSTYPE;

#define	YYSTYPE gghc_YYSTYPE

gghc_declaration *gghc_declaration_begin(gghc_ctx ctx);
gghc_declaration *gghc_declaration_end(gghc_ctx ctx);

gghc_declarator *gghc_declarator_begin(gghc_ctx ctx);
gghc_declarator *gghc_declarator_end(gghc_ctx ctx);

void gghc_struct_elem_decl(gghc_ctx ctx);

void gghc_array_decl(gghc_ctx ctx, const char *size);
void gghc_function_decl(gghc_ctx ctx, ggrt_parameter_t *_params);

/* extension: int (^) (int p1, int p2) */
void gghc_block_decl(gghc_ctx ctx, ggrt_parameter_t *_params);

#endif

