#ifndef MALLOC_DEBUG_H
#define MALLOC_DEBUG_H

extern int _malloc_debug;
extern void *_debug_malloc(unsigned int x, const char *file, unsigned int line);
extern void *_debug_realloc(void *x, unsigned int y, const char *file, unsigned int line);
extern void _debug_free(void *x, const char *file, unsigned int line);

#define malloc(x) _debug_malloc((x), __FILE__, __LINE__)
#define realloc(x,y) _debug_realloc((x), (y), __FILE__, __LINE__)
#define free(x) _debug_free((x), __FILE__, __LINE__)

#endif
