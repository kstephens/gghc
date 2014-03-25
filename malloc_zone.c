#include "malloc_zone.h"
#include <string.h>
#include <assert.h>

#undef malloc
#undef realloc
#undef free

malloc_zone *malloc_zone_new()
{
    malloc_zone *zone = malloc(sizeof(*zone));
    memset(zone, 0, sizeof(*zone));
    zone->header.next = zone->header.prev = &zone->header;
    return zone;
}

void malloc_zone_clear(malloc_zone *zone)
{
    malloc_zone_object *obj, *next, *head = &zone->header;
    size_t n = 0;
    for ( obj = head->next; obj != head; obj = next ) {
        next = obj->next;
        free(obj);
        ++ n;
    }
    assert(zone->count == n);
    zone->count = 0;
    head->next = head->prev = head;
}

void malloc_zone_destroy(malloc_zone *zone)
{
    malloc_zone_clear(zone);
    free(zone);
}

static
void add_object(malloc_zone *zone, malloc_zone_object *obj)
{
    malloc_zone_object *x = &zone->header;

    zone->count ++;
    assert(zone->count);

    obj->prev = x;
    obj->next = x->next;

    x->next->prev = obj;
    x->next = obj;
}

void *malloc_zone_malloc(malloc_zone *zone, size_t size)
{
    malloc_zone_object *obj;
    obj = malloc(sizeof(*obj) - sizeof(double) + size);
    obj->size = size;
    add_object(zone, obj);
    return obj->data;
}

void _malloc_zone_free(malloc_zone *zone, void *ptr)
{
    malloc_zone_object *obj;
    if ( ! ptr ) return;
    assert(zone->count);
    obj = ptr - (sizeof(*obj) - sizeof(double));
    obj->prev->next = obj->next;
    obj->next->prev = obj->prev;
    free(obj);
    zone->count --;
}

void *malloc_zone_realloc(malloc_zone *zone, void *ptr, size_t size)
{
    malloc_zone_object *obj;
    void *new_ptr;
    size_t old_size;
    obj = ptr - (sizeof(*obj) - sizeof(double));
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

