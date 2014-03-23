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
#include "kshc.h"
#include "kshc_o.h"
#include "kshc_sym.h"

/*
** MAIN
*/
FILE	*kshc_input,	/* preprocessed header file */
	*kshc_precomp0, /* C constants */
	*kshc_precomp1, /* unnamed struct/union definitions */
	*kshc_precomp2,	/* struct/union slot accessors */
	*kshc_precomp30, /* initializer declaration */
	*kshc_precomp3; /* ES initializers */

static	char*	dump = 0;

/*
** output file (kshc_input_filename) must have a different name each time
** because C++ (on NEXTSTEP) generates the static initializers for
** each module as a function based on the name of the source file.
** i.e. "__GLOBAL_$I$_tmp_kshc_input_c"
** So if we want to link in multiple kshc generated object files using
** the -C++ option the C++ output files must have different names.
*/
char	kshc_input_filename_template[MAXPATHLEN+1] = "/tmp/kshc_input%x.c";
char	kshc_input_filename[MAXPATHLEN+1];
char*	kshc_precomp0_filename;
char*	kshc_precomp0x_filename;
char*   kshc_constants_filename;
char*	kshc_precomp1_filename;
char*	kshc_precomp2_filename;
char*	kshc_precomp30_filename;
char*	kshc_precomp3_filename;
kshc_mode kshc_output_mode;

int	kshc_error_code = 0;

char    *kshc_verbose = 0;
char 	*kshc_debug = 0;

extern int yydebug;
extern	int	yyparse(void);


static
void	kshc_system(const char* cmd)
{
  if ( kshc_debug || kshc_verbose ) 
    fprintf(stderr, "kshc: %s\n", cmd);
  
  kshc_error_code = system(cmd);
  if ( kshc_error_code ) {
    fprintf(stderr, "kshc: error executing '%s'\n", cmd);
    exit(kshc_error_code);
  }
}

static
void	kshc_cleanup(void)
{
  /* Close temp files */
  if ( kshc_input ) fclose(kshc_input); kshc_input = 0;
  if ( kshc_precomp0 ) fclose(kshc_precomp0); kshc_precomp1 = 0;
  if ( kshc_precomp1 ) fclose(kshc_precomp1); kshc_precomp1 = 0;
  if ( kshc_precomp2 ) fclose(kshc_precomp2); kshc_precomp2 = 0;
  if ( kshc_precomp30 ) fclose(kshc_precomp30); kshc_precomp30 = 0;
  if ( kshc_precomp3 ) fclose(kshc_precomp3); kshc_precomp3 = 0;

	/* Delete temp files */
  if ( ! kshc_debug ) {
    if ( ! dump ) 
      unlink(kshc_input_filename);
    unlink(kshc_precomp0_filename);
    unlink(kshc_precomp0x_filename);
    unlink(kshc_constants_filename);
    unlink(kshc_precomp1_filename);
    unlink(kshc_precomp2_filename);
    unlink(kshc_precomp30_filename);
    unlink(kshc_precomp3_filename);
  }

  free(kshc_precomp0_filename);
  free(kshc_precomp0x_filename);
  free(kshc_constants_filename);
  free(kshc_precomp1_filename);
  free(kshc_precomp2_filename);
  free(kshc_precomp30_filename);
  free(kshc_precomp3_filename);
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

  kshc_precomp0_filename  = ssprintf("/tmp/kshc_%d_constants.c", pid);
  kshc_precomp0x_filename = ssprintf("/tmp/kshc_%d_constants.exe", pid);
  kshc_constants_filename = ssprintf("/tmp/kshc_%d_constants.out", pid);
  kshc_precomp1_filename  = ssprintf("/tmp/kshc_%d_precomp1.c", pid);
  kshc_precomp2_filename  = ssprintf("/tmp/kshc_%d_precomp2.c", pid);
  kshc_precomp30_filename = ssprintf("/tmp/kshc_%d_precomp30.c", pid);
  kshc_precomp3_filename  = ssprintf("/tmp/kshc_%d_precomp3.c", pid);

  kshc_output_mode = kshc_mode_sexpr;

  atexit(kshc_cleanup);

  for ( i = 1; i < argc; i ++ ) {
    if ( strcmp(argv[i], "-v") == 0 ) {
      kshc_verbose = argv[i];
    } else
    if ( strcmp(argv[i], "-o") == 0 ) {
      output_pathname = argv[++ i];
    } else
    if ( strcmp(argv[i], "-typedef") == 0 ) {
      kshc_symbol* s;

      s = kshc_symbol_set(argv[++ i]);
      s->value = "typedef";
    } else
    if ( strcmp(argv[i], "-dump") == 0 ) {
      dump = argv[i];
    } else
    if ( strcmp(argv[i], "-debug") == 0 ) {
      kshc_debug = argv[i];
    } else
    if ( strcmp(argv[i], "-yydebug") == 0 ) {
      yydebug = 1;
    } else
    if ( strcmp(argv[i], "-mallocdebug") == 0 ) {
      _malloc_debug = 1;
    } else
    if ( strcmp(argv[i], "-sexpr") == 0 ) {
      kshc_output_mode = kshc_mode_sexpr;
    } else
    if ( strcmp(argv[i], "-C") == 0 ) {
      kshc_output_mode = kshc_mode_c;
    } else
    if ( strcmp(argv[i], "-C++") == 0 ) {
      kshc_output_mode = kshc_mode_cxx;
    } else
    if ( argv[i][0] == '-' ) {
      options = ssprintf("%s %s", options, argv[i]);
    } else {
      files = ssprintf((files[0] ? "%s %s" : "%s%s"), files, argv[i]);
      filev[filen ++] = argv[i];
    }
  }

  /* Generate unique file output file name */
  sprintf(kshc_input_filename, kshc_input_filename_template, getpid() ^ time(0));
  
  /* Run the specified header files through CPP */
  kshc_precomp1 = fopen(kshc_precomp1_filename, "w+");
  for ( i = 0; i < filen; i ++ ) {
    fprintf(kshc_precomp1, "#include \"%s\"\n", filev[i]);
  }
  fclose(kshc_precomp1); kshc_precomp1 = 0;
  
  sprintf(cmd, "cc -E %s '-D__kshc__' '%s' >> '%s'", options, kshc_precomp1_filename, kshc_input_filename);
  kshc_system(cmd);
  kshc_input = fopen(kshc_input_filename, "r");

  /* Create output files */
  kshc_precomp0 = fopen(kshc_precomp0_filename, "w+");
  kshc_precomp1 = fopen(kshc_precomp1_filename, "w+");
  kshc_precomp2 = fopen(kshc_precomp2_filename, "w+");
  kshc_precomp30 = fopen(kshc_precomp30_filename, "w+");
  kshc_precomp3 = fopen(kshc_precomp3_filename, "w+");

  /* Put a header at the top */
  fprintf(kshc_precomp0, "/* Created by kshc 0.1, built %s %s */\n\n", __DATE__, __TIME__);
  fprintf(kshc_precomp0, "#define __kshc_cc_ 1\n\n");

  /* Include the specified header files */
  if ( mode_cxx )
    fprintf(kshc_precomp0, "extern \"C\" {\n\n");

  fprintf(kshc_precomp0, "#include <stdio.h>\n\n");
  fprintf(kshc_precomp0, "/* input files: */");
  for ( i = 0; i < filen; i ++ ) {
    fprintf(kshc_precomp0, "#include \"%s\"\n", filev[i]);
  }
  fprintf(kshc_precomp0, "\n\n");

  if ( mode_cxx )
    fprintf(kshc_precomp0, "\n}\n\n");

  /* Include the kshc->ES interface declarations */
  if ( mode_cxx ) 
    fprintf(kshc_precomp0, "#define cplusplus__ 1\n");

  /* constant support. */
  {
    char *fmt;
    if ( mode_sexpr) {
      fmt = "  (constant constant_%d %ld \\\"%s\\\")";
    }
    if ( mode_c ) {
      fmt = "static long _kshc_constant_%d = (long) (%ld); /* %%s */";
    }
    fprintf(kshc_precomp0, "#define _kshc_constant(ID,EXPR) printf(\"%s\\n\", ID, (long) (EXPR), #EXPR)\n", fmt);
    fprintf(kshc_precomp0, "#define _kshc_offsetof(T,N) ((size_t) &((T*) 0)->N)\n");
  }

  /* Begin constant generation prog. */
  fprintf(kshc_precomp0, "\n\nint main(int argc, char **argv)\n{\n");

  if ( mode_sexpr ) {
  fprintf(kshc_precomp1, ";; Created by kshc 0.1, built %s %s */\n\n", __DATE__, __TIME__);
  }
  if ( mode_c ) {
  fprintf(kshc_precomp1, "#include \"%skshc_i.h\"\n\n", KSHC_LIB_DIR);
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
  strcpy(initfuncname, "_kshc_initfunc_");
  strcat(initfuncname, t);
  if ( (t = strrchr(initfuncname, '.')) )
    *t = '\0';
  
  if ( mode_c ) {
  fprintf(kshc_precomp30, "\n\n%svoid %s(void) {\n", (mode_cxx ? "static " : ""), initfuncname);
  fprintf(kshc_precomp3, "\n  kshc_begin_module(\"%s\");\n\n", files);
  }
  if ( mode_sexpr ) {
  fprintf(kshc_precomp1, "\n(module \"%s\"\n\n", files);
  }

  /* Parse the input file */

  yyparse();
  if ( kshc_error_code ) exit(kshc_error_code);
 
  fprintf(kshc_precomp0, "  printf(\"\\n\");\n\n");
  fprintf(kshc_precomp0, "\n  return 0;\n}\n");
  fclose(kshc_precomp0); kshc_precomp0 = 0;

  if ( mode_sexpr ) {
  fprintf(kshc_precomp3, "\n) ;; module %s\n\n", files);
  }
  if ( mode_c ) {
  /* Terminate the initializer function */
  fprintf(kshc_precomp3, "  kshc_end_module(\"%s\");\n", files);
  fprintf(kshc_precomp3, "\n}\n\n");
  
  /* Create an initializer object */
  if ( mode_cxx ) {
    fprintf(kshc_precomp3, "static _kshcInitializer _kshc_initializer(%s);\n\n", initfuncname);
  }
  
  }
  }

  /* Close the temp files */
  fclose(kshc_input); kshc_input = 0;
  if ( kshc_precomp0 ) fclose(kshc_precomp0); kshc_precomp0 = 0;
  fclose(kshc_precomp1); kshc_precomp1 = 0;
  fclose(kshc_precomp2); kshc_precomp2 = 0;
  fclose(kshc_precomp30); kshc_precomp30 = 0;
  fclose(kshc_precomp3); kshc_precomp3 = 0;

  /* Compile constants. */
  sprintf(cmd, "cc %s '%s' -o '%s'",
          options, kshc_precomp0_filename, kshc_precomp0x_filename);
  kshc_system(cmd);

  /* Generate constants. */
  sprintf(cmd, "%s > %s",
          kshc_precomp0x_filename, kshc_constants_filename);
  kshc_system(cmd);
  
  if ( dump ) {
    sprintf(cmd, "cat %s %s %s %s %s 1>&2", kshc_precomp1_filename, kshc_constants_filename, kshc_precomp2_filename, kshc_precomp30_filename, kshc_precomp3_filename);
    kshc_system(cmd);
  }

  /* Concat output files. */
  sprintf(cmd, "cat %s %s %s %s %s", kshc_precomp1_filename, kshc_constants_filename, kshc_precomp2_filename, kshc_precomp30_filename, kshc_precomp3_filename);
  if ( ! (output_pathname == 0 || strcmp(output_pathname, "-") == 0) ) {
    strcat(cmd, " >'");
    strcat(cmd, output_pathname);
    strcat(cmd, "'");
  }
  kshc_system(cmd);

  return kshc_error_code;
}

