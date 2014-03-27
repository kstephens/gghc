/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#ifndef _gghc_h_
#define	_gghc_h_

#include <stdio.h> /* FILE */
#include "ggrt/mm_buf.h"

typedef enum gghc_mode {
  gghc_mode_UNDEF = 0,
  gghc_mode_cxx = 1,
  gghc_mode_c = 2,
  gghc_mode_sexpr = 3
} gghc_mode;
#define mode_cxx   (ctx->output_mode == gghc_mode_cxx)
#define mode_c     (ctx->output_mode <= gghc_mode_c)
#define mode_sexpr (ctx->output_mode == gghc_mode_sexpr)

typedef struct gghc_ctx {
  mm_buf *mb, _mb; /* preprocessed header file */
  mm_buf *mb_token, _mb_token; /* token tracking stream. */
  mm_buf_region *last_token, _last_token;

  FILE *cpp_in;      /* preprocessed input */
  FILE *header_out;     /* header */
  FILE *constants_c; /* C constants */
  FILE *decl_out;
  FILE *body_out;
  FILE *footer_out;
  FILE *defines_in;
  FILE *defines_out;

  char *cpp_in_filename;
  char *cpp_out_filename;
  char *constants_c_filename;
  char *constants_x_filename;
  char *header_filename;
  char *constants_filename;
  char *decl_filename;
  char *body_filename;
  char *footer_filename;
  char *defines_in_filename;
  char *defines_out_filename;
  char *result_out_filename;

  int	parse_lineno;
  const char *parse_filename;
  const char * parse_top_level_filename;
  int	error_code;

  gghc_mode output_mode;
  char *verbose;
  char *debug;
  char *dump;

  char *cmd;
  char *cmd_cpp;
  char* cc_prog;
  char*	files;
  char*	filev[100];
  int	filen;
  char*	options;
  char*	output_pathname;

  char  *initfuncname;

  int yydebug;
} *gghc_ctx;

extern gghc_ctx _gghc_ctx;

gghc_ctx gghc_m_ctx();

int gghc_parse_argv(gghc_ctx ctx, int argc, char **argv);

void gghc_reset(gghc_ctx ctx, const char *filename);
void gghc_close_files(gghc_ctx ctx);
void gghc_cleanup(gghc_ctx ctx);

void gghc_system(gghc_ctx ctx, const char* cmd);

int gghc_yyparse(gghc_ctx ctx, mm_buf *mb);
int gghc_yyparse_y(gghc_ctx ctx, mm_buf *mb);
void	yywarning(const char* s);
void	yyerror(const char* s);

#include "ggrt/malloc_debug.h"
#endif
