typedef struct kshc_symbol
{
	char *identifier;
	char *value;
	struct kshc_symbol *next;
} kshc_symbol;

kshc_symbol*	kshc_symbol_set(const char* identifier);
kshc_symbol*	kshc_symbol_get(const char* identifier);


