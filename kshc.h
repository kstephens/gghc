/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#ifndef _kshc_h_
#define	_kshc_h_

#include <stdio.h> /* FILE */

extern
FILE	*kshc_input,	/* preprocessed header file */
	*kshc_precomp1, /* unnamed struct/union definitions */
	*kshc_precomp2,	/* struct/union slot accessors */
	*kshc_precomp30, /* initializer function declaration */
	*kshc_precomp3; /* initializers */

extern	int	kshc_parse_lineno;
extern	char*	kshc_parse_filename;
extern  char*   kshc_parse_top_level_filename;
extern	int	kshc_error_code;

extern	char*	kshc_parse_last_text;
extern  char	*kshc_debug;

extern	void	yywarning(const char* s);
extern	void	yyerror(const char* s);

#include "malloc_debug.h"
#endif
