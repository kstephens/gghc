#ifndef _gghc_gghc_sym_h
#define _gghc_gghc_sym_h

typedef struct gghc_symbol
{
	char *identifier;
	char *value;
	struct gghc_symbol *next;
} gghc_symbol;

gghc_symbol*	gghc_symbol_set(const char* identifier);
gghc_symbol*	gghc_symbol_get(const char* identifier);

#endif
