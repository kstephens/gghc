#include "gghc/gghc.h"
#include "ggrt/ggrt.h"
#include "gghc/output.h"
#include <assert.h>

gghc_declaration *gghc_declaration_begin(gghc_ctx ctx)
{
  gghc_declaration *obj = gghc_malloc(ctx, sizeof(*obj));
  gghc_obj_stack *obj_stack = gghc_malloc(ctx, sizeof(*obj_stack));

  obj_stack->obj = ctx->current_declaration;
  obj_stack->prev = ctx->declaration_stack;
  obj_stack->depth = obj_stack->prev ? obj_stack->prev->depth + 1 : 0;
  ctx->declaration_stack = obj_stack;

  ctx->current_declaration = obj;
  obj->type = ggrt_type(ctx->rt, "int");

  // fprintf(stderr, "   %3d gghc_declaration_begin(%p) => %p\n", obj_stack->depth, ctx, obj);

  return obj;
}

gghc_declaration *gghc_declaration_end(gghc_ctx ctx)
{
  gghc_declaration *obj = ctx->current_declaration;
  gghc_obj_stack *obj_stack = ctx->declaration_stack;

  // fprintf(stderr, "   %3d gghc_declaration_end(%p) => %p\n", obj_stack->depth, ctx, obj);

  assert(obj_stack);
  ctx->current_declaration = obj_stack->obj;
  ctx->declaration_stack = obj_stack->prev;

  assert(obj);
  return obj;
}

gghc_declarator *gghc_declarator_begin(gghc_ctx ctx)
{
  gghc_declarator *obj = gghc_malloc(ctx, sizeof(*obj));
  gghc_obj_stack *obj_stack = gghc_malloc(ctx, sizeof(*obj_stack));

  obj_stack->obj = ctx->current_declarator;
  obj_stack->prev = ctx->declarator_stack;
  obj_stack->depth = obj_stack->prev ? obj_stack->prev->depth + 1 : 0;
  ctx->declarator_stack = obj_stack;

  // fprintf(stderr, "  gghc_declarator_begin(): %p\n", obj);

  ctx->current_declarator = obj;

  assert(ctx->current_declaration);
  obj->declaration = ctx->current_declaration;
  obj->type = obj->declaration->type;

  // fprintf(stderr, "   %3d gghc_declarator_begin(%p) => %p\n", obj_stack->depth, ctx, obj);

  return obj;
}

gghc_declarator *gghc_declarator_end(gghc_ctx ctx)
{
  gghc_declarator *obj = ctx->current_declarator;
  gghc_obj_stack *obj_stack = ctx->declarator_stack;

  assert(obj_stack);
  ctx->current_declarator = obj_stack->obj;
  ctx->declarator_stack = obj_stack->prev;

  assert(obj);
  if ( ! obj->type )
    obj->type = obj->declaration->type;

  // fprintf(stderr, "   %3d gghc_declarator_end(%p) => %p\n", obj_stack->depth, ctx, obj);

  return obj;
}

gghc_declarator *gghc_add_declarator(gghc_ctx ctx, gghc_declarator *obj)
{
  gghc_declaration *cd = ctx->current_declaration;
  gghc_declarator *pd = cd->declarators;

  // fprintf(stderr, "   gghc_add_declarator(%p, %p)\n", ctx, obj);
  obj->prev_decl = pd;
  cd->declarators = obj;

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

ggrt_parameter_t *gghc_parameter_decl(gghc_ctx ctx, gghc_declarator *decl)
{
  gghc_declaration *declaration;
  ggrt_type_t *type;
  const char *name = 0;
  ggrt_parameter_t *param;

  assert(ctx);
  declaration = ctx->current_declaration;
  assert(declaration);
  assert(decl);
  assert(decl->declaration == declaration);

  if ( decl ) {
    // assert(decl == declaration->declarators);
    assert(decl->prev_decl == 0);
    name = decl->identifier;
    declaration = decl->declaration;
    assert(declaration);
    type = decl->type ? decl->type : declaration->type;
  } else {
    type = declaration->type;
  }
  if ( ! type ) type = ggrt_type(ctx->rt, "int");

  param = ggrt_parameter(ctx->rt, type, name);

  // fprintf(stderr, "  gghc_parameter_decl(%p, %p) => (%s, %s) %p\n", ctx, decl, type->name, name, param);

  return param;
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

