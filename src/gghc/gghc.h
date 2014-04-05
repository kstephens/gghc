/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#ifndef _gghc_h_
#define	_gghc_h_

#include <stdio.h> /* FILE */
#include "ggrt/ggrt.h"
#include "ggrt/malloc_zone.h"
#include "ggrt/mm_buf.h"

struct gghc_ctx;
typedef struct gghc_ctx *gghc_ctx;

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

gghc_declaration *gghc_declaration_begin(gghc_ctx ctx);
gghc_declaration *gghc_declaration_end(gghc_ctx ctx);

gghc_declarator *gghc_declarator_begin(gghc_ctx ctx);
gghc_declarator *gghc_declarator_end(gghc_ctx ctx);

void gghc_struct_elem_decl(gghc_ctx ctx);

void gghc_array_decl(gghc_ctx ctx, const char *size);
void gghc_function_decl(gghc_ctx ctx, ggrt_parameter_t *_params);

/* extension: int (^) (int p1, int p2) */
void gghc_block_decl(gghc_ctx ctx, ggrt_parameter_t *_params);

typedef enum gghc_mode {
  gghc_mode_UNDEF = 0,
  gghc_mode_cxx = 1,
  gghc_mode_c = 2,
  gghc_mode_sexpr = 3
} gghc_mode;
#define mode_cxx(ctx)   ((ctx)->output_mode == gghc_mode_cxx)
#define mode_c(ctx)     ((ctx)->output_mode <= gghc_mode_c)
#define mode_sexpr(ctx) ((ctx)->output_mode == gghc_mode_sexpr)

struct gghc_ctx {
  malloc_zone *mz;
  void *user_data[4];

  ggrt_ctx rt;

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

  int errors, fatals;
  int error_code;

  /* Options */
  char *args_str;
  gghc_mode output_mode;
  int verbose;
  int debug;
  int dump;
  int _malloc_debug;
  char *output_pathname;

  /* Processing */
  char *cmd;
  char *cmd_cpp;
  char *cc_prog;
  char *cc_options;
  char *files;
  char *filev[100];
  int   filen;
  char *initfuncname;

  // Parser state.
  struct gghc_declaration *current_declaration;
  struct gghc_declarator  *current_declarator;

  // Parser/Lexer control.
  void *scanner; /* cl.l flex yyscan_t */
  int _yydebug;
  int _lexdebug;
  char *_yytext;
  int _yyleng;
  gghc_YYSTYPE *_yylvalp; /* &yylval */

  /* Output control. */
  int _emit;
  int constant_id;
  int unnamed_enum_id, unnamed_struct_id;
  gghc_struct *current_struct;
  gghc_enum *current_enum;
};

gghc_ctx gghc_m_ctx();
void gghc_ctx_destroy(gghc_ctx ctx);

void *gghc_malloc(gghc_ctx ctx, size_t s);
void *gghc_realloc(gghc_ctx ctx, void *p, size_t s);
void  gghc_free(gghc_ctx ctx, void *p);
char *gghc_strdup(gghc_ctx ctx, const char* s);
char *gghc_ssprintf(gghc_ctx ctx, const char* format, ...); // uses gghc_malloc().
char *ssprintf(const char* format, ...); // uses malloc() */

int gghc_parse_argv(gghc_ctx ctx, int argc, char **argv);

void gghc_module_begin(gghc_ctx ctx, const char *modname);
void gghc_module_end(gghc_ctx ctx, const char *modname);

void gghc_reset(gghc_ctx ctx, const char *filename);
void gghc_close_files(gghc_ctx ctx);
void gghc_cleanup(gghc_ctx ctx);

void gghc_process_files(gghc_ctx ctx);

int gghc_system(gghc_ctx ctx, const char* cmd);

int gghc_yyparse(gghc_ctx ctx, mm_buf *mb);
int gghc_yyparse_y(gghc_ctx ctx, mm_buf *mb);

void gghc_yywarning(gghc_ctx ctx, const char* s);
void gghc_yyerror(gghc_ctx ctx, const char* s);

int gghc_yylex (gghc_ctx ctx, gghc_YYSTYPE *lvalp); // , YYLTYPE *llocp);

#endif
