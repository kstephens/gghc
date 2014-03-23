/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#include "kshc_i.h"

/*
** Use C++ static constructors to keep a list of
** kshc output modules linked in.
**
** This was the only "portable" way I could think of
** to collect the kshc init functions at runtime.
*/
static _kshcInitializer* inits = 0;
_kshcInitializer::_kshcInitializer(kshc_funcp f)
{
  initfunc = f;
  next = inits;
  inits = this;
}

void	kshc_init(void)
{
  _kshcInitializer* l = inits;
  while ( l ) {
    (*l->initfunc)();
    l = l->next;
  }
}

