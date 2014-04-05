#include "gghc/gghc.h"
#include "ggrt/ggrt.h"
#include <assert.h>

gghc_declaration *gghc_declaration_begin(gghc_ctx ctx)
{
  gghc_declaration *obj = gghc_malloc(ctx, sizeof(*obj));
  obj->prev = ctx->current_declaration;
  ctx->current_declaration = obj;
  obj->type = ggrt_type(ctx->rt, "int");
  return obj;
}

gghc_declaration *gghc_declaration_end(gghc_ctx ctx)
{
  gghc_declaration *obj = ctx->current_declaration;
  assert(ctx->current_declaration);
  ctx->current_declaration = obj->prev;
  return obj;
}

gghc_declarator *gghc_declarator_begin(gghc_ctx ctx)
{
  gghc_declarator *obj = gghc_malloc(ctx, sizeof(*obj));

  obj->prev_decl = ctx->current_declarator;
  ctx->current_declarator = obj;

  assert(ctx->current_declaration);
  obj->declaration = ctx->current_declaration;
  obj->type = obj->declaration->type;

  // List of declarations.
  obj->prev = ctx->current_declaration->declarators;
  ctx->current_declaration->declarators = obj;

  return obj;
}

gghc_declarator *gghc_declarator_end(gghc_ctx ctx)
{
  gghc_declarator *obj;

  assert(ctx->current_declarator);
  obj = ctx->current_declarator;

  // assert(ctx->current_declaration->declarators == obj);
  ctx->current_declarator = obj->prev_decl;

  return obj;
}

void gghc_struct_elem_decl(gghc_ctx ctx)
{
  gghc_declarator *decl = ctx->current_declarator;
  ggrt_type_t *type = decl->type;
  char *name = decl->identifier;
  if ( decl->bit_field_size ) {
    type = gghc_bitfield(ctx, type, decl->bit_field_size);
  }
  ggrt_struct_elem(ctx->rt, 0, name, type);
}

void gghc_array_decl(gghc_ctx ctx, const char *size)
{
  gghc_declarator *decl = ctx->current_declarator;
  ggrt_type_t *type = gghc_array(ctx, decl->type, size);
  if ( decl->is_parenthised ) {
    decl->type = type;
  } else {
    decl->type = type;
  }
  decl->syntax = "array";
}

void gghc_function_decl(gghc_ctx ctx, ggrt_parameter_t *_params)
{
  gghc_declarator *decl = ctx->current_declarator;
  int nparams = 0;
  ggrt_parameter_t *params = 0;
  ggrt_type_t **parm_types = 0;
  ggrt_type_t *type;
  
  decl->type = ggrt_func_params(ctx->rt, decl->type, params);

  if ( decl->is_parenthised ) {
    // TYPE (*foo)(...)
    decl->type = ggrt_pointer(ctx->rt, decl->type);
  } else {
    // TYPE foo(...)
    decl->syntax = "function";
  }
}

void gghc_block_decl(gghc_ctx ctx, ggrt_parameter_t *_params)
{
  // FIXME
}

