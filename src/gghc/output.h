/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#ifndef _gghc_output_h
#define	_gghc_output_h

#include "gghc.h"
#include "gghc/decl.h"
#include "ggrt/ggrt.h"

void ghhc_init_callbacks(gghc_ctx ctx);

void gghc_emit_control(gghc_ctx ctx, int i); /* -1 to pause, 1 to resume */

void gghc_reset_state(gghc_ctx ctx);

char *gghc_constant(gghc_ctx ctx, const char *c_expr);

ggrt_type_t *gghc_bitfield(gghc_ctx hcctx, struct ggrt_type_t *t, const char *length);
ggrt_type_t *gghc_array(gghc_ctx hcctx, struct ggrt_type_t *t, const char *length);


void gghc_emit_declarator(gghc_ctx ctx, struct gghc_declarator *decl);
void gghc_emit_declaration(gghc_ctx ctx, struct gghc_declaration *decl);

#endif
