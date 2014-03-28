#ifndef ggrt_malloc
#define ggrt_malloc(s)    malloc_zone_malloc(ctx->mz,(s))
#define ggrt_realloc(p,s) malloc_zone_realloc(ctx->mz,(p),(s))
#define ggrt_free(p)      malloc_zone_free(ctx-mz,(p))
#define ggrt_strdup(p)    malloc_zone_strdup(ctx->mz,(p))
#endif
