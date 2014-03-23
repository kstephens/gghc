#include <stdlib.h>
#include <stdio.h>

/* malloc debugging */
int _malloc_debug = 0;
void *_debug_malloc(unsigned int x, const char *file, unsigned int line)
{
  void *p;
  if ( _malloc_debug )
  fprintf(stderr, "%16s:%6u: malloc(%u)", file, line, x); fflush(stderr);
  p = malloc(x);
  if ( _malloc_debug )
  fprintf(stderr, " => 0x%x\n", (unsigned int) p);
  return p;
}

void *_debug_realloc(void *x, unsigned int y, const char *file, unsigned int line)
{
  void *p = realloc(x, y);
  if ( _malloc_debug )
  fprintf(stderr, "%16s:%6u: realloc(0x%x, %u)", file, line, (unsigned int) x, y); fflush(stderr);
  p = realloc(x, y);
  if ( _malloc_debug )
  fprintf(stderr, " => 0x%x\n", (unsigned int) p);
  return p;
}
void _debug_free(void *x, const char *file, unsigned int line)
{
  if ( _malloc_debug )
  fprintf(stderr, "%16s:%6u: free(0x%x)\n", file, line, (unsigned int) x);
  free(x);
}
