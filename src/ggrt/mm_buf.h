#ifndef MM_BUF_H
#define MM_BUF_H

#include <sys/types.h>

typedef struct mm_buf_source {
  const char *filename;
  int lineno, column;
} mm_buf_source;

typedef struct mm_buf_state {
  unsigned char *beg, *end, *pos;
  size_t size, fpos;
  mm_buf_source src;
  int c; /* last char read. */
} mm_buf_state;

typedef struct mm_buf {
  mm_buf_state s, s_prev;
  int fd;
  const char *_errfunc;
  int _errno;
} mm_buf;

typedef struct mm_buf_region {
  mm_buf_state beg, end;
  mm_buf *mb;
  char *cstr;
} mm_buf_region;

int mm_buf_open(mm_buf *mb, const char *filename);
int mm_buf_close(mm_buf *mb);
int mm_buf_getc(mm_buf *mb);
int mm_buf_ungetc(mm_buf *mb, int c);
int mm_buf_read(mm_buf *mb, void *ptr, int size);

void mm_buf_region_init(mm_buf_region *mt);
int mm_buf_region_begin(mm_buf_region *mt, mm_buf *mb);
int mm_buf_region_end(mm_buf_region *mt, mm_buf *mb, size_t size);
mm_buf_region * mm_buf_region_union(mm_buf_region *mt, mm_buf_region *mt0, mm_buf_region *mt1);
char *mm_buf_region_cstr(mm_buf_region *mt);

#endif
