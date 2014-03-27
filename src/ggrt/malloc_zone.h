#ifndef MALLOC_ZONE_H
#define MALLOC_ZONE_H

#include <stdlib.h>

typedef struct malloc_zone_object {
  struct malloc_zone_object *prev, *next;
  size_t size;
  long double data[1]; /* probably the largest alignment of all intrinsic types. */
} malloc_zone_object;

typedef struct malloc_zone {
  size_t count;
  struct malloc_zone_object header;
} malloc_zone;

malloc_zone *malloc_zone_new();
void malloc_zone_destroy(malloc_zone *zone);

void *malloc_zone_malloc(malloc_zone *zone, size_t size);
void *malloc_zone_realloc(malloc_zone *zone, void *ptr, size_t size);
void  malloc_zone_free(malloc_zone *zone, void *ptr);

#define malloc(X)    malloc_zone_malloc(MZONE, (X))
#define realloc(X,S) malloc_zone_realloc(MZONE, (X), (S))
#define free(X)      malloc_zone_free(MZONE, X)

#endif