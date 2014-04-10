#include "gghc/gghc.h"
#include "ggrt/ggrt.h"
#include "gghc/output.h"
#include <assert.h>

gghc_declaration *gghc_declaration_begin(gghc_ctx ctx)
{
  gghc_declaration *obj = gghc_malloc(ctx, sizeof(*obj));
  gghc_obj_stack *obj_stack = gghc_malloc(ctx, sizeof(*ctx->declaration_stack));

  obj_stack->obj = ctx->current_declaration;
  obj_stack->prev = ctx->declaration_stack;
  ctx->declaration_stack = obj_stack;

  ctx->current_declaration = obj;
  obj->type = ggrt_type(ctx->rt, "int");
  return obj;
}

gghc_declaration *gghc_declaration_end(gghc_ctx ctx)
{
  gghc_declaration *obj = ctx->current_declaration;
  gghc_obj_stack *obj_stack = ctx->declaration_stack;

  assert(obj_stack);
  ctx->current_declaration = obj_stack->obj;
  ctx->declaration_stack = obj_stack->prev;

  assert(obj);
  return obj;
}

gghc_declarator *gghc_declarator_begin(gghc_ctx ctx)
{
  gghc_declarator *obj = gghc_malloc(ctx, sizeof(*obj));
  gghc_obj_stack *obj_stack = gghc_malloc(ctx, sizeof(*ctx->declaration_stack));

  obj_stack->obj = ctx->current_declarator;
  obj_stack->prev = ctx->declarator_stack;
  ctx->declarator_stack = obj_stack;

  // fprintf(stderr, "  gghc_declarator_begin(): %p\n", obj);

  ctx->current_declarator = obj;

  assert(ctx->current_declaration);
  obj->declaration = ctx->current_declaration;
  obj->type = obj->declaration->type;

  return obj;
}

gghc_declarator *gghc_declarator_end(gghc_ctx ctx)
{
  gghc_declarator *obj = ctx->current_declarator;
  gghc_obj_stack *obj_stack = ctx->declaration_stack;

  assert(obj_stack);
  ctx->current_declarator = obj_stack->obj;
  ctx->declarator_stack = obj_stack->prev;

  assert(obj);
  return obj;
}

gghc_declarator *gghc_add_declarator(gghc_ctx ctx, gghc_declarator *obj)
{
  obj->prev_decl = ctx->current_declaration->declarators;
  ctx->current_declaration->declarators = obj;

  return obj;
}


void gghc_struct_elem_decl(gghc_ctx ctx, gghc_declarator *decl)
{
  if ( ! decl->type )
    decl->type = ggrt_type(ctx->rt, "int");
  if ( decl->bit_field_size ) {
    decl->type = gghc_bitfield(ctx, decl->type, decl->bit_field_size);
  }
  gghc_struct_elem(ctx, decl);
}

gghc_declarator *gghc_m_array_decl(gghc_ctx ctx, const char *size)
{
  gghc_declarator *decl = gghc_malloc(ctx, sizeof(*decl));

  decl->syntax = "array";
  decl->array_size = size;

  return decl;
}

void gghc_array_decl(gghc_ctx ctx, gghc_declarator *array_decl)
{
  gghc_declarator *decl = ctx->current_declarator;
  while ( array_decl ) {
    decl->type = gghc_array(ctx, decl->type, array_decl->array_size);
    array_decl = array_decl->prev;
  }
}

void gghc_function_decl(gghc_ctx ctx, ggrt_parameter_t *params)
{
  gghc_declarator *decl = ctx->current_declarator;
  ggrt_type_t *rtn_type = decl->type ? decl->type : ctx->current_declaration->type;

  decl->type = ggrt_func_params(ctx->rt, rtn_type, params);

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

