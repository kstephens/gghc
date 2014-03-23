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
#include "kshc_o.h"
#include "kshc_sym.h"

/* malloc debugging */
static int _malloc_debug = 0;
void *_debug_malloc(unsigned int x, const char *file, unsigned int line)
{
  void *p;
  if ( _malloc_debug )
  fprintf(stderr, "%16s:%6u: malloc(%u)", file, line, x); fflush(stderr);
  p = malloc(x);
  if ( _malloc_debug )
  fprintf(stderr, " => 0x%x\n", (unsigned int) p);
  return p;
}

void *_debug_realloc(void *x, unsigned int y, const char *file, unsigned int line)
{
  void *p = realloc(x, y);
  if ( _malloc_debug )
  fprintf(stderr, "%16s:%6u: realloc(0x%x, %u)", file, line, (unsigned int) x, y); fflush(stderr);
  p = realloc(x, y);
  if ( _malloc_debug )
  fprintf(stderr, " => 0x%x\n", (unsigned int) p);
  return p;
}
void _debug_free(void *x, const char *file, unsigned int line)
{
  if ( _malloc_debug )
  fprintf(stderr, "%16s:%6u: free(0x%x)\n", file, line, (unsigned int) x);
  free(x);
}


/*
** MAIN
*/
FILE	*kshc_input,	/* preprocessed header file */
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
char*	kshc_precomp1_filename = "/tmp/kshc_precomp1.c";
char*	kshc_precomp2_filename = "/tmp/kshc_precomp2.c";
char*	kshc_precomp30_filename = "/tmp/kshc_precomp30.c";
char*	kshc_precomp3_filename = "/tmp/kshc_precomp3.c";

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
  if ( kshc_precomp1 ) fclose(kshc_precomp1); kshc_precomp1 = 0;
  if ( kshc_precomp2 ) fclose(kshc_precomp2); kshc_precomp2 = 0;
  if ( kshc_precomp30 ) fclose(kshc_precomp30); kshc_precomp30 = 0;
  if ( kshc_precomp3 ) fclose(kshc_precomp3); kshc_precomp3 = 0;

	/* Delete temp files */
  if ( ! kshc_debug ) {
    if ( ! dump ) 
      unlink(kshc_input_filename);
    unlink(kshc_precomp1_filename);
    unlink(kshc_precomp2_filename);
    unlink(kshc_precomp30_filename);
    unlink(kshc_precomp3_filename);
  }
}

int	main(int argc, char** argv)
{
  char	cmd[2048];
  char*	files = "";
  char*	filev[15];
  int	filen = 0;
  char*	options = "-I.";
  char*	output_pathname = 0;
  char*	cplusplus = 0;
/*	char*	dump = 0; */
  int	i;

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
    if ( strcmp(argv[i], "-C++") == 0 ) {
      cplusplus = argv[i];
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
  kshc_precomp1 = fopen(kshc_precomp1_filename, "w+");
  kshc_precomp2 = fopen(kshc_precomp2_filename, "w+");
  kshc_precomp30 = fopen(kshc_precomp30_filename, "w+");
  kshc_precomp3 = fopen(kshc_precomp3_filename, "w+");

  /* Put a header at the top */
  fprintf(kshc_precomp1, "/* Created by kshc 0.1, built %s %s */\n\n", __DATE__, __TIME__);

  fprintf(kshc_precomp1, "#define __kshc_cc_ 1\n\n");


  /* Include the specified header files */
  if ( cplusplus )
    fprintf(kshc_precomp1, "extern \"C\" {\n\n");

  fprintf(kshc_precomp1, "#include <string.h> /* memcpy() */\n\n");

  for ( i = 0; i < filen; i ++ ) {
    fprintf(kshc_precomp1, "#include \"%s\"\n", filev[i]);
  }

  if ( cplusplus )
    fprintf(kshc_precomp1, "\n}\n\n");

  /* Include the kshc->ES interface declarations */
  if ( cplusplus ) 
    fprintf(kshc_precomp1, "#define cplusplus__ 1\n");

  fprintf(kshc_precomp1, "#include \"%skshc_i.h\"\n\n", KSHC_LIB_DIR);

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
  
  fprintf(kshc_precomp30, "\n\n%svoid %s(void) {\n", (cplusplus ? "static " : ""), initfuncname);
  fprintf(kshc_precomp3, "\n  kshc_begin_module(\"%s\");\n\n", files);

  /* Parse the input file */

  yyparse();
  if ( kshc_error_code ) exit(kshc_error_code);

  /* Terminate the initializer function */
  fprintf(kshc_precomp3, "  kshc_end_module(\"%s\");\n", files);
  fprintf(kshc_precomp3, "\n}\n\n");
  
  /* Create an initializer object */
  if ( cplusplus ) {
    fprintf(kshc_precomp3, "static _kshcInitializer _kshc_initializer(%s);\n\n", initfuncname);
  }
  
  }
  /* Close the temp files */
  fclose(kshc_input); kshc_input = 0;
  fclose(kshc_precomp1); kshc_precomp1 = 0;
  fclose(kshc_precomp2); kshc_precomp2 = 0;
  fclose(kshc_precomp30); kshc_precomp30 = 0;
  fclose(kshc_precomp3); kshc_precomp3 = 0;

  if ( dump ) {
    sprintf(cmd, "cat %s %s %s %s", kshc_precomp1_filename, kshc_precomp2_filename, kshc_precomp30_filename, kshc_precomp3_filename);
    kshc_system(cmd);
  }

  /* Concat output files and compile */
  if ( output_pathname == 0 || strcmp(output_pathname, "-") == 0 ) {
    sprintf(cmd, "cat %s %s %s %s", kshc_precomp1_filename, kshc_precomp2_filename, kshc_precomp30_filename, kshc_precomp3_filename);
    kshc_system(cmd);
  } else {
    char cfile[MAXPATHLEN+1];
    char *suffix;
    
    if ( cplusplus )
      options = ssprintf("-ObjC++ %s", options);
      
    /* Output to a .c file of the same name as output_pathname */
    strcpy(cfile, output_pathname);
    suffix = strrchr(cfile, '/');
    if ( ! suffix ) suffix = cfile;
    suffix = strrchr(suffix, '.');
    if ( suffix ) {
      *(suffix ++) = '\0';
      suffix = output_pathname + (suffix - cfile);
    } else {
      suffix = "";
    }
    strcat(cfile, ".c");
    
    /* Concat the precomp files */
    sprintf(cmd, "cat %s %s %s %s > %s",
      kshc_precomp1_filename, kshc_precomp2_filename, kshc_precomp30_filename, kshc_precomp3_filename, cfile);
    kshc_system(cmd);

    /* Run the cfile through CC if ".o" was needed */
    if ( strcmp(suffix, "o") == 0 ) {
      sprintf(cmd, "cc %s -c '%s' -o '%s'",
	    options, cfile, output_pathname);
      kshc_system(cmd);
    }
  }

  return kshc_error_code;
}

