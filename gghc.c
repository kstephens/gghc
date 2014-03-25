/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>       /* time() */
#include <unistd.h>     /* getpid() */
#include <sys/types.h>  /* pid_t */
#include <sys/param.h>  /* MAXPATHLEN */
#include <ctype.h>
#include "gghc.h"
#include "gghc_o.h"
#include "gghc_sym.h"

/*
** MAIN
*/
FILE *gghc_cpp_in;      /* preprocessed input */
FILE *gghc_header_out;     /* header */
FILE *gghc_constants_c; /* C constants */
FILE *gghc_decl_out;
FILE *gghc_body_out;
FILE *gghc_footer_out;
FILE *gghc_defines_in;
FILE *gghc_defines_out;

mm_buf *gghc_mb; /* preprocessed header file */
mm_buf *gghc_mb_token; /* token tracking stream. */
mm_buf_region *gghc_last_token; 

const char *gghc_parse_filename;
const char *gghc_parse_top_level_filename;
int   gghc_parse_lineno;

static	char*	dump = 0;

char*	gghc_cpp_in_filename;
char*	gghc_cpp_out_filename;
char*	gghc_constants_c_filename;
char*	gghc_constants_x_filename;
char*   gghc_header_filename;
char*   gghc_constants_filename;
char*	gghc_decl_filename;
char*	gghc_body_filename;
char*	gghc_footer_filename;
char*	gghc_defines_in_filename;
char*	gghc_defines_out_filename;
char*   gghc_result_out_filename;

gghc_mode gghc_output_mode;

int	gghc_error_code = 0;

char    *gghc_verbose = 0;
char 	*gghc_debug = 0;

extern int yydebug;
extern	int	yyparse(void);


void gghc_reset(const char *filename)
{
  filename = strdup(filename);
  gghc_parse_filename = filename;
  gghc_parse_top_level_filename = filename;
  gghc_parse_lineno = 1;
  gghc_reset_state();
}

static
void	gghc_system(const char* cmd)
{
  if ( gghc_debug || gghc_verbose )
    fprintf(stderr, "gghc: %s\n", cmd);
  
  gghc_error_code = system(cmd);
  if ( gghc_error_code ) {
    fprintf(stderr, "gghc: error executing '%s'\n", cmd);
    exit(gghc_error_code);
  }
}

static
void close_files(void)
{
#define C(X) if ( X ) fclose(X); X = 0;
    C(gghc_cpp_in);
    C(gghc_header_out);
    C(gghc_constants_c);
    C(gghc_decl_out);
    C(gghc_body_out);
    C(gghc_footer_out);
    C(gghc_defines_in);
    C(gghc_defines_out);
#undef C
}

static
void	gghc_cleanup(void)
{
    close_files();

  if ( ! gghc_debug ) {
#define ul(X) if ( X ) unlink(X)
    ul(gghc_cpp_in_filename);
    ul(gghc_cpp_out_filename);
    ul(gghc_constants_c_filename);
    ul(gghc_constants_x_filename);

    ul(gghc_header_filename);
    ul(gghc_constants_filename);
    ul(gghc_decl_filename);
    ul(gghc_body_filename);
    ul(gghc_defines_in_filename);
    ul(gghc_defines_out_filename);
    ul(gghc_footer_filename);
#undef ul
  }
}

static int parse_C_defines(FILE *fp)
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

int	main(int argc, char** argv)
{
  char	cmd[2048];
  char* cc_prog = 0;
  char*	files = "";
  char*	filev[100] = { 0 };
  int	filen = 0;
  char*	options = "-I.";
  char*	output_pathname = 0;
/*	char*	dump = 0; */
  int	i;
  int pid = (int) getpid();
  mm_buf mb;
  char  *initfuncname = 0;

  gghc_cpp_in_filename       = ssprintf("/tmp/gghc-%d-01-cpp_in.c", pid);
  gghc_cpp_out_filename      = ssprintf("/tmp/gghc-%d-02-cpp_out.i", pid);
  gghc_constants_c_filename  = ssprintf("/tmp/gghc-%d-03-constants.c", pid);
  gghc_constants_x_filename  = ssprintf("/tmp/gghc-%d-04-constants.exe", pid);
  gghc_defines_in_filename   = ssprintf("/tmp/gghc-%d-05-defines.in", pid);
  gghc_header_filename       = ssprintf("/tmp/gghc-%d-10-header.out", pid);
  gghc_constants_filename    = ssprintf("/tmp/gghc-%d-11-constants.out", pid);
  gghc_decl_filename         = ssprintf("/tmp/gghc-%d-12-decl.out", pid);
  gghc_body_filename         = ssprintf("/tmp/gghc-%d-13-body.out", pid);
  gghc_defines_out_filename  = ssprintf("/tmp/gghc-%d-14-defines.out", pid);
  gghc_footer_filename       = ssprintf("/tmp/gghc-%d-15-footer.out", pid);
  gghc_result_out_filename   = ssprintf("/tmp/gghc-%d-99-result.out", pid);
  gghc_output_mode = gghc_mode_sexpr;

  atexit(gghc_cleanup);

  for ( i = 1; i < argc; i ++ ) {
    if ( strcmp(argv[i], "-v") == 0 ) {
      gghc_verbose = argv[i];
    } else
    if ( strcmp(argv[i], "-o") == 0 ) {
      output_pathname = argv[++ i];
    } else
    if ( strcmp(argv[i], "-typedef") == 0 ) {
      gghc_symbol* s;

      s = gghc_symbol_set(argv[++ i]);
      s->value = "typedef";
    } else
    if ( strcmp(argv[i], "-dump") == 0 ) {
      dump = argv[i];
    } else
    if ( strcmp(argv[i], "-debug") == 0 ) {
      gghc_debug = argv[i];
    } else
    if ( strcmp(argv[i], "-yydebug") == 0 ) {
      yydebug = 1;
    } else
    if ( strcmp(argv[i], "-mallocdebug") == 0 ) {
      _malloc_debug = 1;
    } else
    if ( strcmp(argv[i], "-sexpr") == 0 ) {
      gghc_output_mode = gghc_mode_sexpr;
    } else
    if ( strcmp(argv[i], "-C") == 0 ) {
      gghc_output_mode = gghc_mode_c;
    } else
    if ( strcmp(argv[i], "-C++") == 0 ) {
      gghc_output_mode = gghc_mode_cxx;
    } else
    if ( argv[i][0] == '-' ) {
      options = ssprintf("%s %s", options, argv[i]);
    } else {
      if ( cc_prog ) {
        files = ssprintf((files[0] ? "%s %s" : "%s%s"), files, argv[i]);
        filev[filen ++] = argv[i];
        if ( filen > 100 ) {
          fprintf(stderr, "gghc: too many files\n");
          exit(1);
        }
      } else {
        cc_prog = argv[i];
      }
    }
  }
  if ( ! (cc_prog && cc_prog[0]) ) cc_prog = "cc";

  if ( filen == 0 ) {
      fprintf(stderr, "gghc: no files given.\n");
      exit(0);
  }

  /* Initialize function name */
  if ( mode_c ) {
    char *o, *t;
  
    o = (output_pathname == 0 || strcmp(output_pathname, "-") == 0) ?
      "" :
      output_pathname;
  
    if ( (t = strrchr(o, '/')) )
      t ++;
    else
      t = o;
    initfuncname = ssprintf("_gghc_initfunc_%s", t);
    if ( (t = strrchr(initfuncname, '.')) )
      *t = '\0';
  }
 
  /* Reset state. */
  gghc_reset(filev[0]);

  /**************************************/
  /* Run the specified header files through CPP */

  gghc_cpp_in = fopen(gghc_cpp_in_filename, "w+");
  for ( i = 0; i < filen; i ++ ) {
    fprintf(gghc_cpp_in, "#include \"%s\"\n", filev[i]);
  }
  fclose(gghc_cpp_in); gghc_cpp_in = 0;
  
  /**************************************/
  /* Collect defines. */

  sprintf(cmd, "%s -E -dM %s '-D__gghc__' '%s' > '%s'", cc_prog, options, gghc_cpp_in_filename, gghc_defines_in_filename);
  gghc_system(cmd);

  sprintf(cmd, "/usr/bin/sort %s > %s.tmp && /bin/mv %s.tmp %s", gghc_defines_in_filename, gghc_defines_in_filename, gghc_defines_in_filename, gghc_defines_in_filename);
  gghc_system(cmd);

  sprintf(cmd, "%s -E %s '-D__gghc__' '%s' > '%s'", cc_prog, options, gghc_cpp_in_filename, gghc_cpp_out_filename);
  gghc_system(cmd);

  /* Create output files */
  gghc_header_out  = fopen(gghc_header_filename, "w+");
  gghc_decl_out    = fopen(gghc_decl_filename, "w+");
  gghc_body_out    = fopen(gghc_body_filename, "w+");
  gghc_footer_out  = fopen(gghc_footer_filename, "w+");
  gghc_defines_in  = fopen(gghc_defines_in_filename, "r");
  gghc_defines_out = fopen(gghc_defines_out_filename, "w+");

  /**************************************/
  /* Create constant.c file. */

  gghc_constants_c = fopen(gghc_constants_c_filename, "w+");
  fprintf(gghc_constants_c, "/* Created by gghc 0.1, built %s %s */\n\n", __DATE__, __TIME__);
  fprintf(gghc_constants_c, "#define __gghc_cc__ 1\n\n");
  fprintf(gghc_constants_c, "#define main __gghc_main\n\n");

  /* Include the specified header files */
  if ( mode_cxx ) {
    fprintf(gghc_constants_c, "#define cplusplus__ 1\n");
    fprintf(gghc_constants_c, "extern \"C\" {\n\n");
  }

  fprintf(gghc_constants_c, "#include <stdio.h>\n\n");
  fprintf(gghc_constants_c, "/* input files: */\n");
  for ( i = 0; i < filen; i ++ ) {
    fprintf(gghc_constants_c, "#include \"%s\"\n", filev[i]);
  }
  fprintf(gghc_constants_c, "\n\n");

  if ( mode_cxx )
    fprintf(gghc_constants_c, "\n}\n\n");

  /* _gghc_constant constant definition. */
  {
    char *fmt;
    if ( mode_sexpr) {
      fmt = "  (gghc:constant gghc:constant_%d %ld \\\"%s\\\")";
    }
    if ( mode_c ) {
      fmt = "static long _gghc_constant_%d = (long) (%ld); /* %%s */";
    }
    fprintf(gghc_constants_c, "#define _gghc_constant(ID,EXPR) printf(\"%s\\n\", ID, (long) (EXPR), #EXPR)\n", fmt);
    fprintf(gghc_constants_c, "#define _gghc_offsetof(T,N) ((size_t) &((T*) 0)->N)\n");
  }

  /* Begin constant generation prog. */
  fprintf(gghc_constants_c, "\n\n#undef main\n");
  fprintf(gghc_constants_c, "\n\nint main(int argc, char **argv)\n{\n");

  /**************************************/
  /* output header */

  if ( mode_sexpr ) {
    fprintf(gghc_header_out, ";; Created by gghc 0.1, built %s %s */\n", __DATE__, __TIME__);

    fprintf(gghc_header_out,    "\n(gghc:module \"%s\"\n\n", files);
    fprintf(gghc_header_out,    "  (gghc:info 'command \"%s\")\n\n", cmd);
    for ( i = 0; i < filen; i ++ ) {
      fprintf(gghc_header_out,  "  (gghc:info 'input   %2d \"%s\")\n", i, filev[i]);
    }
  }

  if ( mode_c ) {
    fprintf(gghc_header_out, "#include \"gghc_i.h\"\n\n");
  }

  if ( mode_c ) {
    fprintf(gghc_header_out, "\n\n%svoid %s(void) {\n", (mode_cxx ? "static " : ""), initfuncname);
    fprintf(gghc_header_out, "\n  gghc_begin_module(\"%s\");\n\n", files);
  }

  /**************************************/
  /* Parse the preprocessed input file */

  mm_buf_open(&mb, gghc_cpp_out_filename);
  gghc_yyparse(&mb);
  mm_buf_close(&mb);
  // gghc_cpp_out_filename = 0;

  /* Parse C defines. */
  parse_C_defines(gghc_defines_in);

  /**************************************/
  /* Terminate C constant input */

  fprintf(gghc_constants_c, "  printf(\"\\n\");\n\n");
  fprintf(gghc_constants_c, "\n  return 0;\n}\n");

  /* End module. */
  if ( mode_sexpr ) {
    fprintf(gghc_footer_out, "\n) ;; module %s\n\n", files);
  }
  if ( mode_c ) {
    /* Terminate the initializer function */
    fprintf(gghc_footer_out, "  gghc_end_module(\"%s\");\n", files);
    fprintf(gghc_footer_out, "\n}\n\n");
  }
  /* Create an initializer object */
  if ( mode_cxx ) {
    fprintf(gghc_footer_out, "static _gghcInitializer _gghc_initializer(%s);\n\n", initfuncname);
  }

  /* Close the temp files */
  close_files();

  /**************************************/
  /* Compile constants. */
  sprintf(cmd, "%s %s '%s' -o '%s'",
          cc_prog, options, gghc_constants_c_filename, gghc_constants_x_filename);
  // gghc_constants_c_filename = 0;
  gghc_system(cmd);

  /* Generate constants. */
  sprintf(cmd, "%s > %s",
          gghc_constants_x_filename, gghc_constants_filename);
  gghc_system(cmd);
  
  /**************************************/
  /* Concat output files. */

  sprintf(cmd, "/bin/cat %s %s %s %s %s %s > %s",
          gghc_header_filename,
          gghc_constants_filename,
          gghc_decl_filename,
          gghc_body_filename,
          gghc_defines_out_filename,
          gghc_footer_filename,
          gghc_result_out_filename);
  gghc_system(cmd);

  /**************************************/
  /* Generate output. */

  sprintf(cmd, "/bin/cat %s", gghc_result_out_filename);
  if ( ! (output_pathname == 0 || strcmp(output_pathname, "-") == 0) ) {
    strcat(cmd, " >'");
    strcat(cmd, output_pathname);
    strcat(cmd, "'");
  }
  gghc_system(cmd);

  if ( dump ) {
    sprintf(cmd, "/bin/cat %s %s %s %s %s %s %s %s %s 1>&2",
            gghc_cpp_in_filename,
            gghc_cpp_out_filename,
            gghc_constants_c_filename,
            gghc_header_filename,
            gghc_constants_filename,
            gghc_decl_filename,
            gghc_body_filename,
            gghc_defines_out_filename,
            gghc_footer_filename
            );
    gghc_system(cmd);
  }

  if ( gghc_error_code ) exit(gghc_error_code);
 
  return gghc_error_code;
}

