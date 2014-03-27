/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#include <stdlib.h>
#include "gghc/gghc.h"

static gghc_ctx _gghc_ctx;
static void gghc_at_exit()
{
  gghc_cleanup(_gghc_ctx);
}

int main(int argc, char** argv)
{
  gghc_ctx ctx;
/*	char*	dump = 0; */
  int	i, result;

  ctx = gghc_m_ctx();
  gghc_parse_argv(ctx, argc, argv);
  if ( ctx->fatal )
    exit(9);

  _gghc_ctx = ctx;
  atexit(gghc_at_exit);

  if ( ctx->filen == 0 ) {
    fprintf(stderr, "gghc: no files given.\n");
    exit(1);
  }

  gghc_process_files(ctx);

  result = ctx->error_code;

  gghc_ctx_destroy(ctx);

  return result;
}

