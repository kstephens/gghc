#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "malloc_zone.h"

malloc_zone *malloc_zone_new()
{
  malloc_zone *zone = malloc(sizeof(*zone));
  memset(zone, 0, sizeof(*zone));
  zone->header.next = zone->header.prev = &zone->header;
  zone->_malloc  = malloc;
  zone->_realloc = realloc;
  zone->_free    = free;
  return zone;
}

void malloc_zone_destroy(malloc_zone *zone)
{
  malloc_zone_clear(zone);
  free(zone);
}

void malloc_zone_clear(malloc_zone *zone)
{
  malloc_zone_object *obj, *next, *head = &zone->header;
  size_t n = 0;
  if ( ! zone->passthru ) {
  for ( obj = head->next; obj != head; obj = next ) {
    next = obj->next;
    zone->_free(obj);
    ++ n;
  }
  assert(zone->nobjects == n);
  }
  zone->nobjects = zone->nbytes = zone->nmallocs = zone->nfrees = 0;
  head->next = head->prev = head;
}

static
void add_object(malloc_zone *zone, malloc_zone_object *obj)
{
  malloc_zone_object *x = &zone->header;

  zone->nobjects ++;
  assert(zone->nobjects);
  zone->nbytes += obj->size;
  zone->nmallocs ++;

  obj->prev = x;
  obj->next = x->next;

  x->next->prev = obj;
  x->next = obj;
}

static malloc_zone_object* _obj;
static inline size_t mzo_size(size_t size)
{
  return sizeof(*_obj) - sizeof(_obj->data) + size;
}

static inline void *mzo_to_ptr(malloc_zone_object *obj)
{
  return obj->data;
}

static inline malloc_zone_object *ptr_to_mzo(void *ptr)
{
  return ptr - (sizeof(*_obj) - sizeof(_obj->data));
}

void *malloc_zone_malloc(malloc_zone *zone, size_t size)
{
  malloc_zone_object *obj;
  if ( zone->passthru )
    return zone->_malloc(size);
  obj = zone->_malloc(mzo_size(size));
  obj->size = size;
  memset(obj->data, 0, obj->size);
  add_object(zone, obj);
  return obj->data;
}

void _malloc_zone_free(malloc_zone *zone, void *ptr)
{
  malloc_zone_object *obj;
  if ( zone->passthru )
    return zone->_free(ptr);
  if ( ! ptr ) return;
  assert(zone->nobjects);
  assert(zone->nbytes);
  obj = ptr_to_mzo(ptr);
  obj->prev->next = obj->next;
  obj->next->prev = obj->prev;
  memset(obj->data, 0, obj->size);
  zone->nobjects --;
  zone->nbytes -= obj->size;
  zone->nfrees ++;
  zone->_free(obj);
}

void *malloc_zone_realloc(malloc_zone *zone, void *ptr, size_t size)
{
  malloc_zone_object *obj;
  void *new_ptr;
  size_t old_size;
  if ( zone->passthru )
    return zone->_realloc(ptr, size);
  obj = ptr_to_mzo(ptr);
  old_size = ptr ? obj->size : 0;
  new_ptr = malloc_zone_malloc(zone, size);
  memcpy(new_ptr, ptr, size < old_size ? size : old_size);
  _malloc_zone_free(zone, ptr);
  return new_ptr;
}

void  malloc_zone_free(malloc_zone *zone, void *ptr)
{
  _malloc_zone_free(zone, ptr);
}

char *malloc_zone_strdup(malloc_zone *zone, const char *ptr)
{
  if ( ! ptr ) return 0;
  return strcpy(malloc_zone_malloc(zone, strlen(ptr) + 1), ptr);
}

