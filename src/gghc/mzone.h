#ifndef MZONE_H
#define MZONE_H

#define malloc(X)    gghc_malloc(ctx, (X))
#define realloc(X,S) gghc_realloc(ctx, (X), (S))
#define free(X)      gghc_free(ctx, (X))
#define strdup(X) gghc_strdup(ctx, (X))
#define ssprintf(FMT,ARGS...) gghc_ssprintf(ctx, FMT, ## ARGS)

#endif
