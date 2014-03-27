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

/*
** MAIN
*/
static void gghc_at_exit()
{
  gghc_cleanup(_gghc_ctx);
}

int	main(int argc, char** argv)
{
  gghc_ctx ctx;
/*	char*	dump = 0; */
  int	i;

  ctx = gghc_m_ctx();
  gghc_parse_argv(ctx, argc, argv);

  _gghc_ctx = ctx;
  atexit(gghc_at_exit);

  if ( ctx->filen == 0 ) {
    fprintf(stderr, "gghc: no files given.\n");
    exit(1);
  }

  gghc_process_files(ctx);

  return ctx->error_code;
}

