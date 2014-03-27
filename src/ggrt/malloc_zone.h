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
  void *(*_malloc)(size_t size);
  void *(*_realloc)(void *ptr, size_t size);
  void  (*_free)(void *ptr);
} malloc_zone;

malloc_zone *malloc_zone_new();
void malloc_zone_destroy(malloc_zone *zone);
void malloc_zone_clear(malloc_zone *zone);

void *malloc_zone_malloc(malloc_zone *zone, size_t size);
void *malloc_zone_realloc(malloc_zone *zone, void *ptr, size_t size);
void  malloc_zone_free(malloc_zone *zone, void *ptr);

#endif
