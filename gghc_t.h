/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#ifndef	_kshc_t_h
#define	_kshc_t_h

#include <stddef.h>	/* wchar_t */

typedef
struct kshc_decl_spec {
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
} kshc_decl_spec;

typedef	kshc_decl_spec *kshc_decl_specp;

typedef
struct	kshc_decl {
	char*	identifier;
	char*	declarator;
	char*	declarator_text;
	char*	type;
	short	is_parenthised;
	short	is_bit_field;
        char* bit_field_size;
	struct kshc_decl* next;
} kshc_decl;

typedef	kshc_decl *kshc_declp;

typedef
struct	kshc_struct {
	char*	struct_or_union;
	char*	name;
	short	named;
	char*	slots;
	char*	slots_text;
	char*	type;
	struct	kshc_struct* prev;
} kshc_struct;

typedef struct kshc_YYSTYPE {
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
    kshc_decl_spec	decl_spec;
    kshc_declp		decl;
  } u;
  char*	text;
} kshc_YYSTYPE;

#define	YYSTYPE kshc_YYSTYPE


#endif

