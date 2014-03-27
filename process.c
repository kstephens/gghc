/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>  /* MAXPATHLEN */
#include <ctype.h>
#include "gghc.h"
#include "gghc_o.h"
#include "gghc_sym.h"

static int parse_C_defines(gghc_ctx ctx, FILE *fp)
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    int count = 0;
    while ((linelen = getline(&line, &linecap, fp)) > 0) {
        char *s = line;
        char *name = 0;
        char *value = 0;
        s[linelen - 1] = 0;
        while ( *s && isspace(*s) ) s ++;
        if ( strncmp(s, "#define ", 8) == 0 ) {
            s += 8;
            while ( *s && isspace(*s) ) s ++;
            name = s;
            while ( *s && ! isspace(*s) ) s ++;
            *(s ++) = 0;
            while ( *s && isspace(*s) ) s ++;
            value = s;
            gghc_define(name, value);
            ++ count;
        }
    }
    free(line);
    return count;
}

void gghc_process_files(gghc_ctx ctx)
{
  int i;

  /* Initialize function name */
  if ( mode_c ) {
    char *o, *t;
  
    o = (ctx->output_pathname == 0 || strcmp(ctx->output_pathname, "-") == 0) ?
      "" :
      ctx->output_pathname;
  
    if ( (t = strrchr(o, '/')) )
      t ++;
    else
      t = o;
    ctx->initfuncname = ssprintf("_gghc_initfunc_%s", t);
    if ( (t = strrchr(ctx->initfuncname, '.')) )
      *t = '\0';
  }
 
  /* Reset state. */
  gghc_reset(ctx, ctx->filev[0]);

  /* Create output files */
  ctx->header_out  = fopen(ctx->header_filename, "w+");
  ctx->decl_out    = fopen(ctx->decl_filename, "w+");
  ctx->body_out    = fopen(ctx->body_filename, "w+");
  ctx->footer_out  = fopen(ctx->footer_filename, "w+");
  ctx->defines_out = fopen(ctx->defines_out_filename, "w+");

  /**************************************/
  /* Run the specified header files through CPP */

  ctx->cpp_in = fopen(ctx->cpp_in_filename, "w+");
  for ( i = 0; i < ctx->filen; i ++ ) {
    fprintf(ctx->cpp_in, "#include \"%s\"\n", ctx->filev[i]);
  }
  fclose(ctx->cpp_in); ctx->cpp_in = 0;

  ctx->cmd = ssprintf("%s -E %s '-D__gghc__' '%s' > '%s'", ctx->cc_prog, ctx->options, ctx->cpp_in_filename, ctx->cpp_out_filename);
  gghc_system(ctx, ctx->cmd);
  ctx->cmd_cpp = ssprintf("%s", ctx->cmd);

  /**************************************/
  /* Collect defines. */

  ctx->cmd = ssprintf("%s -E -dM %s '-D__gghc__' '%s' > '%s'", ctx->cc_prog, ctx->options, ctx->cpp_in_filename, ctx->defines_in_filename);
  gghc_system(ctx, ctx->cmd);

  ctx->cmd = ssprintf("/usr/bin/sort %s > %s.tmp && /bin/mv %s.tmp %s", ctx->defines_in_filename, ctx->defines_in_filename, ctx->defines_in_filename, ctx->defines_in_filename);
  gghc_system(ctx, ctx->cmd);


  /**************************************/
  /* Create constant.c file. */

  ctx->constants_c = fopen(ctx->constants_c_filename, "w+");
  fprintf(ctx->constants_c, "/* Created by gghc 0.1, built %s %s */\n\n", __DATE__, __TIME__);
  fprintf(ctx->constants_c, "#define __gghc_cc__ 1\n\n");
  fprintf(ctx->constants_c, "#define main __gghc_main\n\n");

  /* Include the specified header files */
  if ( mode_cxx ) {
    fprintf(ctx->constants_c, "#define cplusplus__ 1\n");
    fprintf(ctx->constants_c, "extern \"C\" {\n\n");
  }

  fprintf(ctx->constants_c, "#include <stdio.h>\n\n");
  fprintf(ctx->constants_c, "/* input files: */\n");
  for ( i = 0; i < ctx->filen; i ++ ) {
    fprintf(ctx->constants_c, "#include \"%s\"\n", ctx->filev[i]);
  }
  fprintf(ctx->constants_c, "\n\n");

  if ( mode_cxx )
    fprintf(ctx->constants_c, "\n}\n\n");

  /* _gghc_constant constant definition. */
  {
    char *fmt;
    if ( mode_sexpr) {
      fmt = "  (gghc:constant gghc:constant_%d %ld \\\"%s\\\")";
    }
    if ( mode_c ) {
      fmt = "static long _gghc_constant_%d = (long) (%ld); /* %%s */";
    }
    fprintf(ctx->constants_c, "#define _gghc_constant(ID,EXPR) printf(\"%s\\n\", ID, (long) (EXPR), #EXPR)\n", fmt);
    fprintf(ctx->constants_c, "#define _gghc_offsetof(T,N) ((size_t) &((T*) 0)->N)\n");
  }

  /* Begin constant generation prog. */
  fprintf(ctx->constants_c, "\n\n#undef main\n");
  fprintf(ctx->constants_c, "\n\nint main(int argc, char **argv)\n{\n");

  /**************************************/
  /* output header */

  if ( mode_sexpr ) {
    fprintf(ctx->header_out, ";; Created by gghc 0.1, built %s %s */\n", __DATE__, __TIME__);

    fprintf(ctx->header_out,    "\n(gghc:module \"%s\"\n\n", ctx->files);
    for ( i = 0; i < ctx->filen; i ++ ) {
      fprintf(ctx->header_out,  "  (gghc:info 'input  %2d \"%s\")\n", i, ctx->filev[i]);
    }
    fprintf(ctx->header_out,    "  (gghc:info 'cpp   \"%s\")\n\n", ctx->cmd_cpp);
  }

  if ( mode_c ) {
    fprintf(ctx->header_out, "/* Created by gghc 0.1, built %s %s */\n", __DATE__, __TIME__);
    fprintf(ctx->header_out, "/* input: %s\n */\n", ctx->files);
    fprintf(ctx->header_out, "/* cpp:   %s\n */\n\n", ctx->cmd_cpp);
    fprintf(ctx->header_out, "#include \"gghc_i.h\"\n\n");
  }

  if ( mode_c ) {
    fprintf(ctx->header_out, "\n\n%svoid %s(void) {\n", (mode_cxx ? "static " : ""), ctx->initfuncname);
    fprintf(ctx->header_out, "\n  gghc_begin_module(\"%s\");\n\n", ctx->files);
  }

  /**************************************/
  /* Parse the preprocessed input file */

  mm_buf_open(ctx->mb, ctx->cpp_out_filename);
  gghc_yyparse(ctx, ctx->mb);
  mm_buf_close(ctx->mb);
  // ctx->cpp_out_filename = 0;

  /* Parse C defines. */
  ctx->defines_in = fopen(ctx->defines_in_filename, "r");
  parse_C_defines(ctx, ctx->defines_in);
  fclose(ctx->defines_in); ctx->defines_in = 0;

  /**************************************/
  /* Terminate C constant input */

  fprintf(ctx->constants_c, "  printf(\"\\n\");\n\n");
  fprintf(ctx->constants_c, "\n  return 0;\n}\n");

  /* End module. */
  if ( mode_sexpr ) {
    fprintf(ctx->footer_out, "\n) ;; module %s\n\n", ctx->files);
  }
  if ( mode_c ) {
    /* Terminate the initializer function */
    fprintf(ctx->footer_out, "  gghc_end_module(\"%s\");\n", ctx->files);
    fprintf(ctx->footer_out, "\n}\n\n");
  }
  /* Create an initializer object */
  if ( mode_cxx ) {
    fprintf(ctx->footer_out, "static _gghcInitializer _gghc_initializer(%s);\n\n", ctx->initfuncname);
  }

  /* Close files */
  gghc_close_files(ctx);

  /**************************************/
  /* Compile constants. */

  ctx->cmd = ssprintf("%s %s '%s' -o '%s'",
          ctx->cc_prog, ctx->options, ctx->constants_c_filename, ctx->constants_x_filename);
  // ctx->constants_c_filename = 0;
  gghc_system(ctx, ctx->cmd);

  /* Generate constants. */
  ctx->cmd = ssprintf("%s > %s",
          ctx->constants_x_filename, ctx->constants_filename);
  gghc_system(ctx, ctx->cmd);
  
  /**************************************/
  /* Concat output files. */

  ctx->cmd = ssprintf("/bin/cat %s %s %s %s %s %s > %s",
          ctx->header_filename,
          ctx->constants_filename,
          ctx->decl_filename,
          ctx->body_filename,
          ctx->defines_out_filename,
          ctx->footer_filename,
          ctx->result_out_filename);
  gghc_system(ctx, ctx->cmd);

  /**************************************/
  /* Generate output. */

  ctx->cmd = ssprintf("/bin/cat %s", ctx->result_out_filename);
  if ( ! (ctx->output_pathname == 0 || strcmp(ctx->output_pathname, "-") == 0) ) {
    ctx->cmd = ssprintf("%s >'%s'", ctx->cmd, ctx->output_pathname);
  }
  gghc_system(ctx, ctx->cmd);

  if ( ctx->dump ) {
    ctx->cmd = ssprintf("/bin/cat %s %s %s %s %s %s %s %s %s 1>&2",
            ctx->cpp_in_filename,
            ctx->cpp_out_filename,
            ctx->constants_c_filename,
            ctx->header_filename,
            ctx->constants_filename,
            ctx->decl_filename,
            ctx->body_filename,
            ctx->defines_out_filename,
            ctx->footer_filename
            );
    gghc_system(ctx, ctx->cmd);
  }
}

