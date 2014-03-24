/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#ifndef _gghc_h_
#define	_gghc_h_

#include <stdio.h> /* FILE */
#include "mm_buf.h"

extern mm_buf *gghc_mb; /* preprocessed header file */
extern mm_buf *gghc_mb_token; /* token tracking stream. */
extern mm_buf_token *gghc_last_token;

extern
FILE
        *gghc_precomp0,  /* C constants */
	*gghc_precomp1, /* unnamed struct/union definitions */
	*gghc_precomp2,	/* struct/union slot accessors */
	*gghc_precomp30, /* initializer function declaration */
	*gghc_precomp3; /* initializers */

extern	int	gghc_parse_lineno;
extern	const char*	gghc_parse_filename;
extern  const char*   gghc_parse_top_level_filename;
extern	int	gghc_error_code;

typedef enum gghc_mode {
  gghc_mode_UNDEF = 0,
  gghc_mode_cxx = 1,
  gghc_mode_c = 2,
  gghc_mode_sexpr = 3
} gghc_mode;
#define mode_cxx   (gghc_output_mode == gghc_mode_cxx)
#define mode_c     (gghc_output_mode <= gghc_mode_c)
#define mode_sexpr (gghc_output_mode == gghc_mode_sexpr)

extern  gghc_mode gghc_output_mode;
extern  char	*gghc_debug;

int gghc_yyparse(mm_buf *mb);
int gghc_yyparse_y(mm_buf *mb);
extern	void	yywarning(const char* s);
extern	void	yyerror(const char* s);

#include "malloc_debug.h"
#endif
