/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>       /* time() */
#include <unistd.h>     /* getpid() */
#include <sys/types.h>  /* pid_t */
#include <sys/param.h>  /* MAXPATHLEN */
#include "gghc.h"
#include "gghc_o.h"
#include "gghc_sym.h"

/*
** MAIN
*/
FILE
	*gghc_precomp0, /* C constants */
	*gghc_precomp1, /* unnamed struct/union definitions */
	*gghc_precomp2,	/* struct/union slot accessors */
	*gghc_precomp30, /* initializer declaration */
	*gghc_precomp3; /* ES initializers */

mm_buf *gghc_mb; /* preprocessed header file */
mm_buf *gghc_mb_token; /* token tracking stream. */
mm_buf_token *gghc_last_token; 

const char *gghc_parse_filename;
const char *gghc_parse_top_level_filename;
int   gghc_parse_lineno;

static	char*	dump = 0;

char*	gghc_input_filename;
char*	gghc_precomp0_filename;
char*	gghc_precomp0x_filename;
char*   gghc_constants_filename;
char*	gghc_precomp1_filename;
char*	gghc_precomp2_filename;
char*	gghc_precomp30_filename;
char*	gghc_precomp3_filename;
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
void	gghc_cleanup(void)
{
  /* Close temp files */
  if ( gghc_precomp0 ) fclose(gghc_precomp0); gghc_precomp1 = 0;
  if ( gghc_precomp1 ) fclose(gghc_precomp1); gghc_precomp1 = 0;
  if ( gghc_precomp2 ) fclose(gghc_precomp2); gghc_precomp2 = 0;
  if ( gghc_precomp30 ) fclose(gghc_precomp30); gghc_precomp30 = 0;
  if ( gghc_precomp3 ) fclose(gghc_precomp3); gghc_precomp3 = 0;

	/* Delete temp files */
  if ( ! gghc_debug ) {
    if ( ! dump ) 
      unlink(gghc_input_filename);
    unlink(gghc_precomp0_filename);
    unlink(gghc_precomp0x_filename);
    unlink(gghc_constants_filename);
    unlink(gghc_precomp1_filename);
    unlink(gghc_precomp2_filename);
    unlink(gghc_precomp30_filename);
    unlink(gghc_precomp3_filename);
  }

  free(gghc_input_filename);
  free(gghc_precomp0_filename);
  free(gghc_precomp0x_filename);
  free(gghc_constants_filename);
  free(gghc_precomp1_filename);
  free(gghc_precomp2_filename);
  free(gghc_precomp30_filename);
  free(gghc_precomp3_filename);
}

int	main(int argc, char** argv)
{
  char	cmd[2048];
  char*	files = "";
  char*	filev[15];
  int	filen = 0;
  char*	options = "-I.";
  char*	output_pathname = 0;
/*	char*	dump = 0; */
  int	i;
  int pid = (int) getpid();
  mm_buf mb;

  gghc_input_filename     = ssprintf("/tmp/gghc_%d_input.c", pid);
  gghc_precomp0_filename  = ssprintf("/tmp/gghc_%d_constants.c", pid);
  gghc_precomp0x_filename = ssprintf("/tmp/gghc_%d_constants.exe", pid);
  gghc_constants_filename = ssprintf("/tmp/gghc_%d_constants.out", pid);
  gghc_precomp1_filename  = ssprintf("/tmp/gghc_%d_precomp1.c", pid);
  gghc_precomp2_filename  = ssprintf("/tmp/gghc_%d_precomp2.c", pid);
  gghc_precomp30_filename = ssprintf("/tmp/gghc_%d_precomp30.c", pid);
  gghc_precomp3_filename  = ssprintf("/tmp/gghc_%d_precomp3.c", pid);

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
      files = ssprintf((files[0] ? "%s %s" : "%s%s"), files, argv[i]);
      filev[filen ++] = argv[i];
    }
  }

  /* Reset state. */
  gghc_reset(filev[0]);

  /* Run the specified header files through CPP */
  gghc_precomp1 = fopen(gghc_precomp1_filename, "w+");
  for ( i = 0; i < filen; i ++ ) {
    fprintf(gghc_precomp1, "#include \"%s\"\n", filev[i]);
  }
  fclose(gghc_precomp1); gghc_precomp1 = 0;
  
  sprintf(cmd, "cc -E %s '-D__gghc__' '%s' > '%s'", options, gghc_precomp1_filename, gghc_input_filename);
  gghc_system(cmd);

  /* Create output files */
  gghc_precomp0 = fopen(gghc_precomp0_filename, "w+");
  gghc_precomp1 = fopen(gghc_precomp1_filename, "w+");
  gghc_precomp2 = fopen(gghc_precomp2_filename, "w+");
  gghc_precomp30 = fopen(gghc_precomp30_filename, "w+");
  gghc_precomp3 = fopen(gghc_precomp3_filename, "w+");

  /* Put a header at the top */
  fprintf(gghc_precomp0, "/* Created by gghc 0.1, built %s %s */\n\n", __DATE__, __TIME__);
  fprintf(gghc_precomp0, "#define __gghc_cc_ 1\n\n");

  /* Include the specified header files */
  if ( mode_cxx )
    fprintf(gghc_precomp0, "extern \"C\" {\n\n");

  fprintf(gghc_precomp0, "#include <stdio.h>\n\n");
  fprintf(gghc_precomp0, "/* input files: */");
  for ( i = 0; i < filen; i ++ ) {
    fprintf(gghc_precomp0, "#include \"%s\"\n", filev[i]);
  }
  fprintf(gghc_precomp0, "\n\n");

  if ( mode_cxx )
    fprintf(gghc_precomp0, "\n}\n\n");

  /* Include the gghc->ES interface declarations */
  if ( mode_cxx ) 
    fprintf(gghc_precomp0, "#define cplusplus__ 1\n");

  /* constant support. */
  {
    char *fmt;
    if ( mode_sexpr) {
      fmt = "  (gghc:constant gghc:constant_%d %ld \\\"%s\\\")";
    }
    if ( mode_c ) {
      fmt = "static long _gghc_constant_%d = (long) (%ld); /* %%s */";
    }
    fprintf(gghc_precomp0, "#define _gghc_constant(ID,EXPR) printf(\"%s\\n\", ID, (long) (EXPR), #EXPR)\n", fmt);
    fprintf(gghc_precomp0, "#define _gghc_offsetof(T,N) ((size_t) &((T*) 0)->N)\n");
  }

  /* Begin constant generation prog. */
  fprintf(gghc_precomp0, "\n\nint main(int argc, char **argv)\n{\n");

  if ( mode_sexpr ) {
  fprintf(gghc_precomp1, ";; Created by gghc 0.1, built %s %s */\n\n", __DATE__, __TIME__);
  }
  if ( mode_c ) {
  fprintf(gghc_precomp1, "#include \"%sgghc_i.h\"\n\n", GGHC_LIB_DIR);
  }

  /* Create an initializer function */
  {
  char  initfuncname[256];
  char *o, *t;
  
  o = (output_pathname == 0 || strcmp(output_pathname, "-") == 0) ?
    "" :
    output_pathname;
  
  if ( (t = strrchr(o, '/')) )
    t ++;
  else
    t = o;
  strcpy(initfuncname, "_gghc_initfunc_");
  strcat(initfuncname, t);
  if ( (t = strrchr(initfuncname, '.')) )
    *t = '\0';
  
  if ( mode_c ) {
  fprintf(gghc_precomp30, "\n\n%svoid %s(void) {\n", (mode_cxx ? "static " : ""), initfuncname);
  fprintf(gghc_precomp3, "\n  gghc_begin_module(\"%s\");\n\n", files);
  }
  if ( mode_sexpr ) {
  fprintf(gghc_precomp1, "\n(gghc:module \"%s\"\n\n", files);
  }

  /* Parse the input file */
  mm_buf_open(&mb, gghc_input_filename);
  gghc_yyparse(&mb);
  mm_buf_close(&mb);

  if ( gghc_error_code ) exit(gghc_error_code);
 
  fprintf(gghc_precomp0, "  printf(\"\\n\");\n\n");
  fprintf(gghc_precomp0, "\n  return 0;\n}\n");
  fclose(gghc_precomp0); gghc_precomp0 = 0;

  if ( mode_sexpr ) {
  fprintf(gghc_precomp3, "\n) ;; module %s\n\n", files);
  }
  if ( mode_c ) {
  /* Terminate the initializer function */
  fprintf(gghc_precomp3, "  gghc_end_module(\"%s\");\n", files);
  fprintf(gghc_precomp3, "\n}\n\n");
  
  /* Create an initializer object */
  if ( mode_cxx ) {
    fprintf(gghc_precomp3, "static _gghcInitializer _gghc_initializer(%s);\n\n", initfuncname);
  }
  
  }
  }

  /* Close the temp files */
  if ( gghc_precomp0 ) fclose(gghc_precomp0); gghc_precomp0 = 0;
  fclose(gghc_precomp1); gghc_precomp1 = 0;
  fclose(gghc_precomp2); gghc_precomp2 = 0;
  fclose(gghc_precomp30); gghc_precomp30 = 0;
  fclose(gghc_precomp3); gghc_precomp3 = 0;

  /* Compile constants. */
  sprintf(cmd, "cc %s '%s' -o '%s'",
          options, gghc_precomp0_filename, gghc_precomp0x_filename);
  gghc_system(cmd);

  /* Generate constants. */
  sprintf(cmd, "%s > %s",
          gghc_precomp0x_filename, gghc_constants_filename);
  gghc_system(cmd);
  
  if ( dump ) {
    sprintf(cmd, "cat %s %s %s %s %s 1>&2", gghc_precomp1_filename, gghc_constants_filename, gghc_precomp2_filename, gghc_precomp30_filename, gghc_precomp3_filename);
    gghc_system(cmd);
  }

  /* Concat output files. */
  sprintf(cmd, "cat %s %s %s %s %s", gghc_precomp1_filename, gghc_constants_filename, gghc_precomp2_filename, gghc_precomp30_filename, gghc_precomp3_filename);
  if ( ! (output_pathname == 0 || strcmp(output_pathname, "-") == 0) ) {
    strcat(cmd, " >'");
    strcat(cmd, output_pathname);
    strcat(cmd, "'");
  }
  gghc_system(cmd);

  return gghc_error_code;
}

