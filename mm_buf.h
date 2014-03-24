#ifndef MM_BUF_H
#define MM_BUF_H

#include <sys/types.h>

typedef struct mm_buf_state {
  unsigned char *beg, *end, *pos;
  size_t size, fpos;
  const char *filename;
  int lineno, column;
  int c; /* last char read. */
} mm_buf_state;

typedef struct mm_buf {
  mm_buf_state s, s_prev;
  int fd;
  const char *_errfunc;
  int _errno;
} mm_buf;

typedef struct mm_buf_token {
  mm_buf_state beg, end;
  mm_buf *mb;
  char *text;
} mm_buf_token;

int mm_buf_open(mm_buf *mb, const char *filename);
int mm_buf_close(mm_buf *mb);
int mm_buf_getc(mm_buf *mb);
int mm_buf_ungetc(mm_buf *mb, int c);
int mm_buf_read(mm_buf *mb, void *ptr, int size);
int mm_buf_token_begin(mm_buf_token *mt, mm_buf *mb);
int mm_buf_token_end(mm_buf_token *mt, mm_buf *mb, size_t size);

#endif
