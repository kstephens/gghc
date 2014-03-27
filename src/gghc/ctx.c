#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>       /* time() */
#include <unistd.h>     /* getpid() */
#include <sys/types.h>  /* pid_t */
#include <assert.h>
#include "gghc.h"
#include "gghc/output.h"
#include "gghc_sym.h"

gghc_ctx gghc_m_ctx()
{
  int pid = (int) getpid();
  gghc_ctx ctx = malloc(sizeof(*ctx));
  memset(ctx, 0, sizeof(*ctx));

  ctx->mz = malloc_zone_new();

  ctx->mb = &ctx->_mb;
  ctx->mb_token = &ctx->_mb_token;
  ctx->last_token = &ctx->_last_token;

  ctx->_stdin  = stdin;
  ctx->_stdout = stdout;
  ctx->_stderr = stderr;

  ctx->cpp_in_filename       = ssprintf("/tmp/gghc-%d-01-cpp_in.c", pid);
  ctx->cpp_out_filename      = ssprintf("/tmp/gghc-%d-02-cpp_out.i", pid);
  ctx->constants_c_filename  = ssprintf("/tmp/gghc-%d-03-constants.c", pid);
  ctx->constants_x_filename  = ssprintf("/tmp/gghc-%d-04-constants.exe", pid);
  ctx->defines_in_filename   = ssprintf("/tmp/gghc-%d-05-defines.in", pid);
  ctx->header_filename       = ssprintf("/tmp/gghc-%d-10-header.out", pid);
  ctx->constants_filename    = ssprintf("/tmp/gghc-%d-11-constants.out", pid);
  ctx->decl_filename         = ssprintf("/tmp/gghc-%d-12-decl.out", pid);
  ctx->body_filename         = ssprintf("/tmp/gghc-%d-13-body.out", pid);
  ctx->defines_out_filename  = ssprintf("/tmp/gghc-%d-14-defines.out", pid);
  ctx->footer_filename       = ssprintf("/tmp/gghc-%d-15-footer.out", pid);
  ctx->result_out_filename   = ssprintf("/tmp/gghc-%d-99-result.out", pid);
  ctx->output_mode = gghc_mode_sexpr;

  return ctx;
}

void gghc_ctx_destroy(gghc_ctx ctx)
{
  assert(ctx);
}

void *gghc_malloc(gghc_ctx ctx, size_t s)
{
  return malloc_zone_malloc(ctx->mz, s);
}
void *gghc_realloc(gghc_ctx ctx, void *p, size_t s)
{
  return malloc_zone_realloc(ctx->mz, p, s);
}
void  gghc_free(gghc_ctx ctx, void *p)
{
  return malloc_zone_free(ctx->mz, p);
}
char *gghc_strdup(gghc_ctx ctx, const char* s)
{
  return malloc_zone_strdup(ctx->mz, s);
}

char *ssprintf(const char* format, ...)
{
  char *buf = 0;
  va_list vap;
  va_start(vap, format);
  vasprintf(&buf, format, vap);
  va_end(vap);
  return buf;
}

char *gghc_ssprintf(gghc_ctx ctx, const char* format, ...)
{
  char *buf = 0, *result = 0;
  va_list vap;
  va_start(vap, format);
  vasprintf(&buf, format, vap);
  va_end(vap);

  // Copy to zone.
  result = malloc_zone_strdup(ctx->mz, buf);
  free(buf);

  return result;
}

#include "mzone.h"

int gghc_parse_argv(gghc_ctx ctx, int argc, char **argv)
{
  int	i;

  ctx->filen = 0;
  ctx->files = "";
  ctx->options = "-I.";
  for ( i = 1; i < argc; i ++ ) {
    if ( strcmp(argv[i], "-v") == 0 ) {
      ctx->verbose = argv[i];
    } else
    if ( strcmp(argv[i], "-o") == 0 ) {
      ctx->output_pathname = argv[++ i];
    } else
    if ( strcmp(argv[i], "-typedef") == 0 ) {
      gghc_symbol* s;

      s = gghc_symbol_set(argv[++ i]);
      s->value = "typedef";
    } else
    if ( strcmp(argv[i], "-dump") == 0 ) {
      ctx->dump = argv[i];
    } else
    if ( strcmp(argv[i], "-debug") == 0 ) {
      ctx->debug = argv[i];
    } else
    if ( strcmp(argv[i], "-yydebug") == 0 ) {
      ctx->_yydebug ++;
    } else
    if ( strcmp(argv[i], "-mallocdebug") == 0 ) {
      ctx->_malloc_debug = 1;
    } else
    if ( strcmp(argv[i], "-sexpr") == 0 ) {
      ctx->output_mode = gghc_mode_sexpr;
    } else
    if ( strcmp(argv[i], "-C") == 0 ) {
      ctx->output_mode = gghc_mode_c;
    } else
    if ( strcmp(argv[i], "-C++") == 0 ) {
      ctx->output_mode = gghc_mode_cxx;
    } else
    if ( argv[i][0] == '-' ) {
      ctx->options = ssprintf("%s %s", ctx->options, argv[i]);
    } else {
      if ( ctx->cc_prog ) {
        ctx->files = ssprintf((ctx->files[0] ? "%s %s" : "%s%s"), ctx->files, argv[i]);
        ctx->filev[ctx->filen ++] = argv[i];
        if ( ctx->filen > 100 ) {
          fprintf(stderr, "gghc: too many files\n");
          exit(1);
        }
      } else {
        ctx->cc_prog = argv[i];
      }
    }
  }
  if ( ! (ctx->cc_prog && ctx->cc_prog[0]) ) ctx->cc_prog = "cc";

  return 0;
}

void gghc_reset_state(gghc_ctx ctx)
{
  ctx->_emit = 1;
  ctx->constant_id = 0;
  ctx->current_enum = 0;
  ctx->unnamed_enum_id = 0;
  ctx->current_struct = 0;
  ctx->unnamed_struct_id = 0;
}

void gghc_reset(gghc_ctx ctx, const char *filename)
{
  filename = strdup(filename);
  ctx->parse_filename = filename;
  ctx->parse_top_level_filename = filename;
  ctx->parse_lineno = 1;
  gghc_reset_state(ctx);
}


int gghc_system(gghc_ctx ctx, const char* cmd)
{
  int code;
  if ( ctx->debug || ctx->verbose )
    fprintf(stderr, "gghc: %s\n", cmd);
  
  code = system(cmd);

  if ( code ) {
    fprintf(stderr, "gghc: error executing '%s'\n", cmd);
    ctx->error_code = code;
    ctx->errors ++;
  }

  return code;
}

void gghc_close_files(gghc_ctx ctx)
{
#define C(X) if ( X ) fclose(X); X = 0;
    C(ctx->cpp_in);
    C(ctx->header_out);
    C(ctx->constants_c);
    C(ctx->decl_out);
    C(ctx->body_out);
    C(ctx->footer_out);
    C(ctx->defines_in);
    C(ctx->defines_out);
#undef C
}

void gghc_cleanup(gghc_ctx ctx)
{
  gghc_close_files(ctx);

  if ( ! ctx->debug ) {
#define ul(X) if ( X ) unlink(X)
    ul(ctx->cpp_in_filename);
    ul(ctx->cpp_out_filename);
    ul(ctx->constants_c_filename);
    ul(ctx->constants_x_filename);

    ul(ctx->header_filename);
    ul(ctx->constants_filename);
    ul(ctx->decl_filename);
    ul(ctx->body_filename);
    ul(ctx->defines_in_filename);
    ul(ctx->defines_out_filename);
    ul(ctx->footer_filename);
#undef ul
  }
}

