#ifndef _ggrt_DATA_H
#define _ggrt_DATA_H

struct ggrt_s_ctx;
typedef struct ggrt_s_ctx *ggrt_ctx;

struct ggrt_type_t;
typedef struct ggrt_type_t ggrt_type_t;

struct ggrt_elem_t;

struct ggrt_pragma_t;
struct ggrt_macro_t;
struct ggrt_constant_t;
struct ggrt_typedef_t;
struct ggrt_global_t;

typedef void *ggrt_user_data[4];

#define ggrt_HEADER(T) \
  ggrt_user_data user_data; \
  int _id; \
  struct ggrt_module_t *mod; \
  struct T *prev; \
  void *cb_val; /* callback value. */

#endif
