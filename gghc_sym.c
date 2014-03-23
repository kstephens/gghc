/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#include "gghc_sym.h"
#include <stdlib.h>
#include <string.h>

static kshc_symbol* symbols = 0;

kshc_symbol*	kshc_symbol_set(const char* identifier)
{
	kshc_symbol* s = kshc_symbol_get(identifier);
	if ( s == 0 ) {
		s = malloc(sizeof(*s));
		s->identifier = (char*) identifier;
		s->next = symbols;
		symbols = s;
	}
	return s;
}

kshc_symbol*	kshc_symbol_get(const char* identifier)
{
	kshc_symbol* s = symbols;
	while ( s ) {
		if ( strcmp(identifier, s->identifier) == 0 ) break;
		s = s->next;
	}
	return s;
}


