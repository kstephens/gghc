#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>       /* time() */
#include <unistd.h>     /* getpid() */
#include <sys/types.h>  /* pid_t */
#include <assert.h>
#include "gghc.h"
#include "gghc/output.h"

gghc_ctx gghc_m_ctx()
{
  int pid = (int) getpid();
  gghc_ctx ctx = malloc(sizeof(*ctx));
  memset(ctx, 0, sizeof(*ctx));

  ctx->mz = malloc_zone_new();
  ctx->rt = ggrt_m_ctx();
  ggrt_ctx_init(ctx->rt);

  gghc_init_callbacks(ctx);

  ctx->mb = &ctx->_mb;
  ctx->mb_token = &ctx->_mb_token;
  ctx->last_token = &ctx->_last_token;

  ctx->_stdin  = stdin;
  ctx->_stdout = stdout;
  ctx->_stderr = stderr;

#define ssprintf(FMT,ARGS...) ggrt_ssprintf(ctx->rt,FMT,##ARGS)

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

#undef ssprintf

  ctx->_emit_macros = 0;
  ctx->_emit_globals = 1;

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

#include "mzone.h"

static void usage(gghc_ctx ctx, FILE *out)
{
  if ( ! out ) out = ctx->_stderr;
  fprintf(out,
"gghc: USAGE: \n"
"  gghc OPTIONS -- CC CC-OPTIONS ... -- INPUT-FILES ... \n"
"OPTIONS: \n"
"  -h, --help \n"
"  -v, --verbose \n"
"  -dM, --macros \n"
"  --dump \n"
"  --debug \n"
"  --yydebug \n"
"  --lexdebug \n"
"  --mallocdebug \n"
"  --sexpr \n"
"  --C \n"
"  --C++ \n"
          );
}

static void add_input_file(gghc_ctx ctx, char *arg)
{
  // fprintf(stderr, "  add_input_file `%s'\n", arg);
  ctx->files = ssprintf((ctx->files[0] ? "%s %s" : "%s%s"), ctx->files, arg);
  ctx->filev[ctx->filen ++] = arg;
  if ( ctx->filen > 100 ) {
    fprintf(ctx->_stderr, "gghc: too many files: '%s'\n", arg);
    ctx->fatals ++;
  }
}

int gghc_parse_argv(gghc_ctx ctx, int argc, char **argv)
{
  int argi;
  int dash_dash_count = 0;
  char *arg = 0;

  ctx->filen = 0;
  ctx->files = "";
  ctx->cc_options = "-I.";
  ctx->args_str = strdup(argv[0]);

  for ( argi = 1; argi < argc && (arg = strdup(argv[argi])); argi ++ ) {
    int incr =
        arg[0] == '-' && arg[1] == '-' && arg[2] ? 1 :
        arg[0] == '+' && arg[1] == '+' && arg[2] ? -1 : 0;
    char *opt = incr == 1 ? arg : arg[0] == '-' && arg[1] && arg[1] != '-' ? arg : "";
    char *val = 0;
    int val_argd = 0;
    char *t = 0;

    ctx->args_str = ssprintf("%s %s", ctx->args_str, arg);

#if 0
#define PS(x) fprintf(stderr, "    arg `%s': %s=`%s'\n", arg, #x, x)
#else
#define PS(x) (void) x;
#endif

    PS(arg);

    if ( strcmp(arg, "--") == 0 ) {
      ++ dash_dash_count;
      continue;
    }

    if ( incr ) {
      if ( (t = strchr(arg, '=')) ) {
        opt = strdup(opt);
        *(strchr(opt, '=') + 1) = 0;
        val = strdup(t + 1);
      } else {
        val = strdup(argv[argi + 1]);
        val_argd = 1;
      }
    } else if ( opt[0] ) {
      val = strdup(argv[argi + 1]);
      val_argd = 1;
    }

    PS(opt);
    PS(val);

#define OPT(X) strcmp(opt, X) == 0

    if ( dash_dash_count == 0 ) {
      if ( OPT("-h") || OPT("--help") ) {
        usage(ctx, ctx->_stdout);
      } else if ( OPT("-v") || OPT("--verbose") ) {
        ctx->verbose ++;
      } else if ( OPT("-o") || OPT("--output") ) {
        ctx->output_pathname = val;
        argi += val_argd;
      } else if ( OPT("-dM") || OPT("--macros") ) {
        ctx->_emit_macros += 1;
      } else if ( OPT("--typedef") ) {
        const char *name = val;
        argi += val_argd;
        assert(! "--typedef supported");
        // s = ggrt_symbol_table_add_(ctx->rt, ctx->st_type, name, 0, 0);
        // s->value = strdup(name);
      } else if ( OPT("--dump") ) {
        ctx->dump ++;
      } else if ( OPT("--debug") ) {
        ctx->debug += incr;
      } else if ( OPT("--yydebug") ) {
        ctx->_yydebug += incr;
      } else if ( OPT("--mallocdebug") ) {
        ctx->_malloc_debug += incr;
      } else if ( OPT("--sexpr") ) {
        ctx->output_mode = gghc_mode_sexpr;
      } else if ( OPT("--C") ) {
        ctx->output_mode = gghc_mode_c;
      } else if ( OPT("--C++") ) {
        ctx->output_mode = gghc_mode_cxx;
      } else if ( opt[0] ) {
        fprintf(ctx->_stderr, "gghc: unknown option '%s'\n", arg);
        ctx->fatals ++;
      } else {
        add_input_file(ctx, arg);
      }
    } else if ( dash_dash_count == 1 ) {
      if ( ! ctx->cc_prog ) {
        ctx->cc_prog = arg;
      } else {
        ctx->cc_options = ssprintf("%s %s", ctx->cc_options, arg);
      }
    } else if ( dash_dash_count == 2 ) {
      add_input_file(ctx, arg);
    } else {
      fprintf(ctx->_stderr, "gghc: unknown option '%s'\n", arg);
      ctx->fatals ++;
    }
  }
  if ( ! (ctx->cc_prog && ctx->cc_prog[0]) ) ctx->cc_prog = "cc";

  PS(ctx->cc_prog);
  PS(ctx->cc_options);
  PS(ctx->files);
#undef PS

  if ( ctx->fatals )
    exit(1);

#undef OPT

  return 0;
}

void gghc_reset_state(gghc_ctx ctx)
{
  ctx->_emit = 1;
  ctx->constant_id = 0;
  ggrt_ctx_reset(ctx->rt);
}

void gghc_reset(gghc_ctx ctx, const char *filename)
{
  filename = strdup(filename);
  ctx->parse_filename = filename;
  ctx->parse_top_level_filename = filename;
  ctx->parse_lineno = 1;
  gghc_reset_state(ctx);
}

void gghc_module_begin(gghc_ctx ctx, const char *modname)
{
  ctx->declaration_stack = 0;
  ctx->current_declaration = 0;

  ctx->declarator_stack = 0;
  ctx->current_declarator = 0;

  ggrt_module_begin(ctx->rt, modname);

  ggrt_emit_types(ctx->rt);

  /* EXT: NATIVE TYPES */
  // gghc_typedef("__uint16_t", gghc_type("unsigned short"));
  ggrt_typedef(ctx->rt, "_Bool", ggrt_type(ctx->rt, "int"));
}

void gghc_module_end(gghc_ctx ctx, const char *modname)
{
  ggrt_module_end(ctx->rt, 0); // modname);
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

