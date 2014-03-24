#include "mm_buf.h"
#include <stdlib.h> /* malloc(), free() */
#include <fcntl.h> /* open() */
#include <string.h> /* memset() */
#include <unistd.h> 
#include <sys/mman.h> /* mmap(), munmap() */
#include <sys/stat.h> /* stat() */

int mm_buf_open(mm_buf *mb, const char *filename)
{
  int result = 0;
  struct stat sb;

  memset(mb, 0, sizeof(*mb));
  mb->fd = -1;
  mb->s.filename = strdup(filename);
  if ( (result = stat(filename, &sb)) )
    return result;
  mb->s.size = sb.st_size;
  if ( (result = mb->fd = open(filename, O_RDONLY)) < 0 )
    return result;
  mb->s.beg = mmap(0, mb->s.size, PROT_READ, MAP_PRIVATE, mb->fd, 0);
  mb->s.end = mb->s.beg + mb->s.size;
  mb->s.pos = mb->s.beg;
  mb->s.column = 0;
  mb->s.lineno = 0;
  mb->s.fpos = 0;
  return 0;
}

int mm_buf_close(mm_buf *mb)
{
  munmap(mb->s.beg, mb->s.size);
  if ( mb->fd >= 0 ) {
    close(mb->fd);
    mb->fd = -1;
  }
  free((void*) mb->s.filename);
  memset(mb, 0, sizeof(*mb));
  return 0;
}

int mm_buf_getc(mm_buf *mb)
{
  int c;
  if ( mb->s.pos >= mb->s.end )
    return -1;
  mb->s.fpos ++;
  c = *(mb->s.pos ++);
  if ( c == '\n' ) {
    mb->s.lineno ++;
    mb->s.column = 1;
  }
  return c;
}

int mm_buf_token_begin(mm_buf_token *t, mm_buf *mb)
{
  t->mb = mb;
  t->text = 0;
  t->beg.size = t->end.size = 0;
  t->beg.fpos = t->end.fpos = mb->s.fpos;
  t->beg.beg = t->end.beg = mb->s.pos;
  t->beg.end = t->end.end = mb->s.pos;
  t->beg.filename = t->end.filename = mb->s.filename;
  t->beg.lineno = t->end.lineno = mb->s.lineno;
  t->beg.column = t->end.column = mb->s.column;
  return 0;
}

int mm_buf_token_end(mm_buf_token *t, mm_buf *mb, size_t size)
{
  t->text = malloc(size + 1);
  memcpy(t->text, mb->s.pos, size);
  t->text[size] = 0;

  t->beg.size = t->end.size = size;
  t->end.fpos = mb->s.fpos;
  t->end.beg = mb->s.pos;
  t->end.end = t->beg.end = mb->s.pos;
  t->end.filename = mb->s.filename;
  t->end.lineno = mb->s.lineno;
  t->end.column = mb->s.column;
  return 0;
}

