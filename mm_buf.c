#include "mm_buf.h"
#include <stdlib.h> /* malloc(), free() */
#include <stdio.h> /* EOF */
#include <fcntl.h> /* open() */
#include <string.h> /* memset() */
#include <unistd.h> 
#include <sys/mman.h> /* mmap(), munmap() */
#include <sys/stat.h> /* stat() */
#include <sys/errno.h> /* errno */
#include <assert.h>

int mm_buf_open(mm_buf *mb, const char *filename)
{
  int result = 0;
  struct stat sb;

  memset(mb, 0, sizeof(*mb));
  mb->fd = -1;
  mb->s.filename = strdup(filename);
  mb->_errfunc = "stat";
  if ( (result = stat(filename, &sb)) != 0 )
    goto rtn;
  mb->s.size = sb.st_size;
  mb->_errfunc = "open";
  if ( (result = mb->fd = open(filename, O_RDONLY)) < 0 )
    goto rtn;
  mb->_errfunc = "mmap";
  if ( (mb->s.beg = mmap(0, mb->s.size, PROT_READ, MAP_PRIVATE, mb->fd, 0)) == 0 ) {
    result = -1;
    goto rtn;
  }
  mb->s.end = mb->s.beg + mb->s.size;
  mb->s.pos = mb->s.beg;
  mb->s.lineno = 1;
  mb->s.column = 0;
  mb->s.fpos = 0;
  mb->_errfunc = 0;

 rtn:
  if ( result ) {
    mb->_errno = errno;
  }
  return result;
}

int mm_buf_close(mm_buf *mb)
{
  munmap(mb->s.beg, mb->s.size);
  if ( mb->fd >= 0 ) {
    close(mb->fd);
  }
  // free((void*) mb->s.filename); // shared
  memset(mb, 0, sizeof(*mb));
  mb->fd = -1;
  return 0;
}

int mm_buf_getc(mm_buf *mb)
{
  int c;

  mb->s_prev = mb->s;
  if ( mb->s.pos >= mb->s.end ) {
    c = EOF;
    goto rtn;
  }
  mb->s.fpos ++;
  c = *(mb->s.pos ++);
  if ( c == '\n' ) {
    mb->s.lineno ++;
    mb->s.column = 0;
  } else if ( c == '\t' ) {
    mb->s.column += 8 - (mb->s.column % 8);
  } else {
    mb->s.column ++;
  }

 rtn:
  mb->s.c = c;
  return c;
}

int mm_buf_ungetc(mm_buf *mb, int c)
{
  assert(c == mb->s.c);
  mb->s = mb->s_prev;
  return 0;
}

int mm_buf_read(mm_buf *mb, void *ptr, int size)
{
  unsigned char *cp = ptr;
  int c, l = 0;
  while ( l < size && (c = mm_buf_getc(mb)) != EOF ) {
    if ( l >= size ) break;
    if ( cp ) *(cp ++) = c;
    l ++;
  }
  return l;
}

int mm_buf_token_begin(mm_buf_token *t, mm_buf *mb)
{
  t->mb = mb;
  t->text = 0;
  t->beg = mb->s;
  t->beg.beg  = t->beg.end  = mb->s.pos;
  t->beg.size = 0;
  t->beg.fpos = mb->s.fpos;
  t->end = t->beg;
  return 0;
}

int mm_buf_token_end(mm_buf_token *t, mm_buf *mb, size_t size)
{
  assert(t->mb == mb);
  if ( 0 ) {
    t->text = malloc(size + 1);
    memcpy(t->text, t->beg.pos, size);
    t->text[size] = 0;
  }

  t->beg.end  = mb->s.pos;
  t->beg.size = size;

  t->end.fpos = mb->s.fpos;
  t->end.beg  = mb->s.pos;
  t->end.end  = mb->s.pos;
  t->end.size = 0;

  t->end.filename = mb->s.filename;
  t->end.lineno   = mb->s.lineno;
  t->end.column   = mb->s.column;

  return 0;
}

