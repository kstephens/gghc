/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#ifndef _gghc_h_
#define	_gghc_h_

#include <stdio.h> /* FILE */
#include "ggrt/malloc_zone.h"
#include "ggrt/mm_buf.h"
#include "gghc/decl.h"

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
  malloc_zone *mz;
  void *user_data[4];

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
  FILE *_stdin, *_stdout, *_stderr;

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

  int errors;
  int error_code;

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

  void *scanner; /* cl.l flex yyscan_t */
  int _yydebug;
  char *_yytext;
  int _yyleng;
  gghc_YYSTYPE *_yylvalp; /* &yylval */

  int _emit;
    int constant_id;
    gghc_enum *current_enum;
    int unnamed_enum_id;
    gghc_struct *current_struct;
    int unnamed_struct_id;

} *gghc_ctx;

gghc_ctx gghc_m_ctx();

int gghc_parse_argv(gghc_ctx ctx, int argc, char **argv);

void gghc_reset(gghc_ctx ctx, const char *filename);
void gghc_close_files(gghc_ctx ctx);
void gghc_cleanup(gghc_ctx ctx);

void gghc_process_files(gghc_ctx ctx);

int gghc_system(gghc_ctx ctx, const char* cmd);

int gghc_yyparse(gghc_ctx ctx, mm_buf *mb);
int gghc_yyparse_y(gghc_ctx ctx, mm_buf *mb);

void gghc_yywarning(gghc_ctx ctx, const char* s);
void gghc_yyerror(gghc_ctx ctx, const char* s);

int gghc_yylex (gghc_ctx ctx, YYSTYPE *lvalp); // , YYLTYPE *llocp);

#include "ggrt/malloc_debug.h"
#endif
