%{ /* -*- bison-mode -*- */
/* From http://www.lysator.liu.se/c/ANSI-C-grammar-y.html. */

/*
** Changes: Copyright 1993, 1994, 2014 Kurt A. Stephens
*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>	/* strstr() */
#include "gghc.h"
#include "gghc_sym.h"
#include "gghc_o.h"

#define YYDEBUG 1
#define YYPRINT(output, toknum, value) ((void) toknum)

extern int yylex();

 void debug_stop_here()
 {
 }

void *gghc_malloc0(size_t size)
{
   void *ptr = malloc(size);
   memset(ptr, 0, size);
   return ptr;
}

gghc_decl *make_decl()
{
  gghc_decl *x = gghc_malloc0(sizeof(gghc_decl));
  x->identifier = "";
  x->declarator = "%s";
  x->declarator_text = "%s";
  x->type = "variable";
  return x;
}

void	yywarning(const char* s)
{
  mm_buf_region *t = gghc_last_token;
  if ( t ) {
    fprintf(stderr, "gghc: %s:%d:%d %s\n",
          t->beg.src.filename ? t->beg.src.filename : "UNKNOWN",
          t->beg.src.lineno,
          t->beg.src.column,
          s
          );
    fprintf(stderr, "gghc: near '%*s'\n", (int) t->beg.size, t->beg.pos);
  } else {
    fprintf(stderr, "gghc: 0:0:0 %s\n", s);
  }
}

void	yyerror(const char* s)
{
  yywarning(s);
  gghc_error_code ++;
}

/****************************************************************************************/

int	yydebug = 0;

static char *_to_expr(YYSTYPE *yyvsp)
{
  if ( yyvsp->expr ) return yyvsp->expr;
  if ( yyvsp->type ) return yyvsp->type;
  return yyvsp->expr = mm_buf_region_str(&(yyvsp->t));
}
#define EXPR(YYV) _to_expr((void*)&(YYV))

static char *_to_type(YYSTYPE *yyvsp)
{
  if ( yyvsp->type ) return yyvsp->type;
  if ( yyvsp->expr ) return yyvsp->expr;
  return yyvsp->type = mm_buf_region_str(&(yyvsp->t));
}
#define TYPE(YYV) _to_type((void*)&(YYV))

#define YY_USER_ACTION(yyn) token_merge(yyn, yylen, &yyval, yyvsp);
static void token_merge(int yyn, int yylen, YYSTYPE *yyvalp, YYSTYPE *yyvsp)
{
  int i;
  mm_buf_region t, *dst = &yyvalp->t, *src;
  int verbose = 0;

  mm_buf_region_init(&t);
  if ( verbose >= 2 ) {
    fprintf(stderr, "  yyn=%d yylen=%d yyvalp=%p yyvsp=%p\n", yyn, yylen, yyvalp, yyvsp);
    fprintf(stderr, "    tokens: ");
  }
  for ( i = 1 - yylen; i <= 0; ++ i ) {
    src = &(yyvsp[i].t);
    if ( verbose >= 3 ) fprintf(stderr, "'%s' ", mm_buf_region_str(src));
    mm_buf_region_union(&t, &t, src);
  }
  if ( verbose >= 2 ) fprintf(stderr, "\n ");
  if ( verbose >= 1 ) {
    fprintf(stderr, "    = '%s'\n", mm_buf_region_str(&t));
  }
  *dst = t;
}

%}

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token GGHC_inline
%token GGHC___builtin_va_list
%token GGHC___attribute__
%token GGHC___asm

%start translation_unit

%type   <expr>          IDENTIFIER TYPE_NAME

%type   <u.decl_spec>   declaration_specifiers
                        specifier_qualifier_list

%type   <u.decl>        declarator direct_declarator
                        init_declarator init_declarator_list
                        declaration_list
                        struct_declarator struct_declarator_list

%type   <u.i>           storage_class_specifier
%type   <type>          type_specifier
%type   <type>          struct_or_union_specifier struct_or_union

%type   <type>          enum_specifier enumerator_list enumerator

%type   <type>          pointer

%type   <type>          parameter_type_list parameter_list
                        parameter_declaration

%type   <type>          abstract_declarator
                        direct_abstract_declarator

%%

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression
	: assignment_expression
	| expression ',' assignment_expression
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
        { gghc_declaration(&$1, 0); }
	| declaration_specifiers init_declarator_list ';'
        { gghc_declaration(&$1, $2); debug_stop_here(); }
	;

declaration_specifiers
	: storage_class_specifier
        { $$.storage = $1; $$.type = gghc_type($$.type_text = "int"); }

	| storage_class_specifier declaration_specifiers
        { $$.storage = $1; $$.type = $2.type; $$.type_text = $2.type_text; }

	| type_specifier
        { $$.storage = 0; $$.type = $$.type_text = TYPE($<u>1); }

	| type_specifier declaration_specifiers
        { $$.storage = $2.storage; $$.type = $$.type_text = TYPE($<u>1); }

	| type_qualifier
        { $$.storage = 0; $$.type = gghc_type($$.type_text = "int"); }

	| type_qualifier declaration_specifiers
        { $$ = $2; }
	;

init_declarator_list
	: init_declarator
        { $$ = $1; }
	| init_declarator_list ',' init_declarator
        { $3->next = $1; $$ = $3; }
	;

init_declarator
	: declarator
        { $$ = $1; }
	| declarator '=' initializer
        { $$ = $1; }
	;

storage_class_specifier
: storage_class_specifier_TOKEN { $$ = $<token>1; } ;

storage_class_specifier_TOKEN
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID
        { $$ = gghc_type("void"); }
/*
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
*/
	| char_specifier
        { $$ = gghc_type("char"); }
        | uchar_specifer
        { $$ = gghc_type("unsigned char"); }
	| sshort_specifer
        { $$ = gghc_type("short"); }
	| ushort_specifer
        { $$ = gghc_type("unsigned short"); }
	| int_specifier
        { $$ = gghc_type("int"); }
	| uint_specifier
        { $$ = gghc_type("unsigned int"); }
	| slong_specifier
        { $$ = gghc_type("long"); }
	| ulong_specifier
        { $$ = gghc_type("unsigned long"); }
	| slong_long_specifier
        { $$ = gghc_type("long long"); }
	| ulong_long_specifier
        { $$ = gghc_type("unsigned long long"); }
	| FLOAT
        { $$ = gghc_type("float"); }
	| DOUBLE
        { $$ = gghc_type("double"); }
        | ldouble_specifier
        { $$ = gghc_type("long double"); }
        | GGHC___builtin_va_list
        { $$ = gghc_type("__builtin_va_list"); }

	| struct_or_union_specifier
        { $$ = $1; }
	| enum_specifier
        { $$ = $1; }
	| TYPE_NAME
        { $$ = gghc_type(EXPR($<u>1)); }
	;

/* Reduce to unique types */
char_specifier : CHAR | SIGNED CHAR | CHAR SIGNED;
uchar_specifer : UNSIGNED CHAR | CHAR UNSIGNED;
short_specifer : SHORT | SHORT INT;
sshort_specifer : short_specifer | SIGNED short_specifer;
ushort_specifer : UNSIGNED short_specifer | short_specifer UNSIGNED;
int_specifier : INT | SIGNED | SIGNED INT;
uint_specifier : UNSIGNED INT | UNSIGNED | INT UNSIGNED;
long_specifier : LONG | INT LONG | LONG INT;
slong_specifier : long_specifier | SIGNED long_specifier;
ulong_specifier : UNSIGNED long_specifier | long_specifier UNSIGNED;
long_long_specifier : LONG LONG | LONG LONG INT | INT LONG LONG | LONG INT LONG;
slong_long_specifier : long_long_specifier | SIGNED long_long_specifier;
ulong_long_specifier : UNSIGNED long_long_specifier | long_long_specifier UNSIGNED;
ldouble_specifier : LONG DOUBLE | DOUBLE LONG;

struct_or_union_specifier
	: struct_or_union IDENTIFIER
        { gghc_struct_type($1, EXPR($<u>2)); debug_stop_here(); }
          '{' struct_declaration_list '}'
          { $$ = gghc_struct_type_end(); }
	| struct_or_union
        { gghc_struct_type($1, ""); }
          '{' struct_declaration_list '}'
          { $$ = gghc_struct_type_end(); }
	| struct_or_union IDENTIFIER
        { $$ = gghc_type(ssprintf("%s %s", $1, EXPR($<u>2))); }
	;

struct_or_union
	: STRUCT { $$ = "struct"; }
	| UNION  { $$ = "union"; }
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
        { gghc_struct_type_element(&$1, $2, EXPR($<u>2)); }
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
        { $$.type = $1; $$.type_text = $<text>2; }
	| type_specifier
        { $$.type = ""; $$.type_text = ""; }
	| type_qualifier specifier_qualifier_list
        { $$ = $2; }
	| type_qualifier
        { $$.type = ""; $$.type_text = ""; }
	;

struct_declarator_list
	: struct_declarator
        { $$ = $1; }
	| struct_declarator_list ',' struct_declarator
        { $3->next = $1; $$ = $3; }
	;

struct_declarator
	: declarator
        { $$ = $1; }
	| ':' constant_expression
        { $$ = make_decl(); $$->is_bit_field = 1; $$->bit_field_size = EXPR($<u>2); }
	| declarator ':' constant_expression
        { $$ = $1; $$->is_bit_field = 1; $$->bit_field_size = EXPR($<u>3); }
	;

enum_specifier
        : ENUM
        { $<type>$ = gghc_enum_type(0); }
            '{' enumerator_list '}'
            { gghc_enum_type_end(); }
	| ENUM IDENTIFIER
        { $<type>$ = gghc_enum_type(EXPR($<u>2)); }
            '{' enumerator_list '}'
            { gghc_enum_type_end(); }
	| ENUM IDENTIFIER
        { $$ = gghc_type(ssprintf("enum %s", EXPR($<u>2))); /* FIXME */ }
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
        { gghc_enum_type_element(EXPR($<u>1)); }
	| IDENTIFIER '=' constant_expression
        { gghc_enum_type_element(EXPR($<u>1)); }
	;

type_qualifier
	: CONST
	| VOLATILE
	;

declarator
	: pointer direct_declarator
        {
          $$ = $2;
          if ( $$->type[0] == 'f' || $$->type[0] == 'a' ) {
            $$->declarator = ssprintf($2->declarator, $1);
          } else {
            $$->declarator = ssprintf($1, $2->declarator);
          }
          $$->declarator_text = ssprintf("%s %s", $<text>2, $2->declarator_text);
        }
        | direct_declarator
	;

direct_declarator
	: IDENTIFIER
        { $$ = make_decl(); $$->identifier = EXPR($<u>1); }
	| '(' declarator ')'
        {
          $$ = $2;
          $$->declarator_text = ssprintf("(%s)", $2->declarator_text);
          $$->is_parenthised = 1;
        }
	| direct_declarator '[' constant_expression ']'
        {
          void *size = EXPR($<u>3);
          $$ = $1;
          if ( $1->is_parenthised ) {
            $$->declarator = ssprintf($1->declarator, gghc_array_type("%s", size));
          } else {
            $$->declarator = gghc_array_type($1->declarator, size);
          }
          $$->declarator_text = ssprintf("%s[%s]", $1->declarator_text, size);
          $$->type = "array";
        }
	| direct_declarator '[' ']'
        {
          $$ = $1;
          if ( $1->is_parenthised ) {
            $$->declarator = ssprintf($1->declarator, gghc_array_type("%s", ""));
          } else {
            $$->declarator = gghc_array_type($1->declarator, "");
          }
          $$->declarator_text = ssprintf("%s[%s]", $1->declarator_text, "");
          $$->type = "array";
        }
	| direct_declarator '(' parameter_type_list ')'
        {
          void *params = TYPE($<u>3);
          $$ = $1;
          if ( $1->is_parenthised ) {
            $$->declarator = ssprintf($1->declarator, gghc_function_type("%s", params));
          } else {
            $$->declarator = gghc_function_type($1->declarator, params);
          }
          $$->declarator_text = ssprintf("%s(%s)", $1->declarator_text, params);
          $$->type = "function";
        }
	| direct_declarator '(' identifier_list ')'
	| direct_declarator '(' ')'
        {
          $$ = $1;
          if ( $1->is_parenthised ) {
            $$->declarator = ssprintf($1->declarator, gghc_function_type("%s", ""));
          } else {
            $$->declarator = gghc_function_type($1->declarator, "");
          }
          $$->declarator_text = ssprintf("%s(%s)", $1->declarator_text, "");
          $$->type = "function";
        }
	;

pointer
	: '*'
        { $$ = gghc_pointer_type("%s"); }
	| '*' type_qualifier_list
        { $$ = gghc_pointer_type("%s"); }
	| '*' pointer
        { $$ = gghc_pointer_type($2); }
	| '*' type_qualifier_list pointer
        { $$ = gghc_pointer_type($3); }
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;

parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
        { $$ = ssprintf("%s%s GGHCT_VARARGS", $1, mode_c ? "," : ""); }
	;

parameter_list
	: parameter_declaration
        { $$ = $1; }
	| parameter_list ',' parameter_declaration
        { $$ = ssprintf("%s%s %s", $1, mode_c ? "," : "", $3); }
	;

parameter_declaration
	: declaration_specifiers declarator
        { $$ = ssprintf($2->declarator, TYPE($<u>1)); }
	| declaration_specifiers abstract_declarator
        { $$ = ssprintf($2, TYPE($<u>1)); }
	| declaration_specifiers
        { $$ = ssprintf("%s", TYPE($<u>1)); }
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
        { $$ = gghc_pointer_type("%s"); }
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
        { $$ = gghc_pointer_type($2); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
        { $$ = $2; }
	| '[' ']'
        { $$ = gghc_array_type("%s", ""); }
	| '[' constant_expression ']'
        { $$ = gghc_array_type("%s", EXPR($<u>2)); }
	| direct_abstract_declarator '[' ']'
        { $$ = gghc_array_type($1, ""); }
	| direct_abstract_declarator '[' constant_expression ']'
        { $$ = gghc_array_type("%s", EXPR($<u>3)); }
	| '(' ')'
        { $$ = gghc_function_type("%s", ""); }
	| '(' parameter_type_list ')'
        { $$ = gghc_function_type("%s", $2); }
	| direct_abstract_declarator '(' ')'
        { $$ = gghc_function_type($1, ""); }
	| direct_abstract_declarator '(' parameter_type_list ')'
        { $$ = gghc_function_type($1, $3); }
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	;

initializer_list
	: initializer
	| initializer_list ',' initializer
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{' statement_list '}'
	| '{' declaration_list '}'
	| '{' declaration_list statement_list '}'
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: ';'
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
        { gghc_declaration(&$<u.decl_spec>1, $<u.decl>2); }
	| declaration_specifiers declarator compound_statement
        { gghc_declaration(&$<u.decl_spec>1, $<u.decl>2); }
	| declarator declaration_list compound_statement
	| declarator compound_statement
	;

%%

int gghc_yyparse_y(mm_buf *mb)
{
  extern int yydebug;
  yydebug = 0;
  return yyparse();
}

