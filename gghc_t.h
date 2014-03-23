/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#ifndef	_gghc_t_h
#define	_gghc_t_h

#include <stddef.h>	/* wchar_t */

typedef
struct gghc_decl_spec {
	int	storage;
	char*	type;
	char*	type_text;
	int	type_bits;
#define	ESIC_UNSIGNED	(1 << 0)
#define	ESIC_SIGNED	(1 << 1)
#define	ESIC_VOID	(1 << 2)
#define	ESIC_CHAR	(1 << 3)
#define	ESIC_SHORT	(1 << 4)
#define	ESIC_INT	(1 << 5)
#define	ESIC_LONG	(1 << 6) /* long long */
#define	ESIC_FLOAT	(1 << 8)
#define	ESIC_DOUBLE	(1 << 9)
} gghc_decl_spec;

typedef	gghc_decl_spec *gghc_decl_specp;

typedef
struct	gghc_decl {
	char*	identifier;
	char*	declarator;
	char*	declarator_text;
	char*	type;
	short	is_parenthised;
	short	is_bit_field;
        char* bit_field_size;
	struct gghc_decl* next;
} gghc_decl;

typedef	gghc_decl *gghc_declp;

typedef
struct	gghc_struct {
	char*	struct_or_union;
	char*	name;
	short	named;
	char*	slots;
	char*	slots_text;
	char*	type;
	struct	gghc_struct* prev;
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
    gghc_decl_spec	decl_spec;
    gghc_declp		decl;
  } u;
  char*	text;
} gghc_YYSTYPE;

#define	YYSTYPE gghc_YYSTYPE


#endif
