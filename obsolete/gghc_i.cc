/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#include "gghc_i.h"

/*
** Use C++ static constructors to keep a list of
** gghc output modules linked in.
**
** This was the only "portable" way I could think of
** to collect the gghc init functions at runtime.
*/
static _gghcInitializer* inits = 0;
_gghcInitializer::_gghcInitializer(gghc_funcp f)
{
  initfunc = f;
  next = inits;
  inits = this;
}

void	gghc_init(void)
{
  _gghcInitializer* l = inits;
  while ( l ) {
    (*l->initfunc)();
    l = l->next;
  }
}

