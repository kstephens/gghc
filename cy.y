%{ /* -*- c -*- */
  /* From http://www.lysator.liu.se/c/ANSI-C-grammar-y.html. */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>	/* strstr() */
#include "gghc.h"
#include "gghc_sym.h"
#include "gghc_o.h"

char*	gghc_parse_last_text = "";
extern char yytext[];
extern int yylex_column;

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
  fprintf(stderr, "gghc: %s:%d:%d %s\n",
          gghc_parse_filename ? gghc_parse_filename : "UNKNOWN",
          gghc_parse_lineno,
          yylex_column,
          s
          );
  fprintf(stderr, "gghc: near '%s'\n", gghc_parse_last_text);
}

void	yyerror(const char* s)
{
  yywarning(s);
  gghc_error_code ++;
}

/****************************************************************************************/

int	yydebug = 0;

#if 0
#define _DEBUG
#endif
#ifdef _DEBUG
#define	TEXT_PRINT() printf("TEXT:%s\n", yyval.text)
#else
#define	TEXT_PRINT() while ( 0 )
#endif

#define TEXT0()	{ gghc_parse_last_text = yyval.text = ""; TEXT_PRINT(); }
#define	TEXT1()	{ gghc_parse_last_text = yyval.text = yyvsp[0].text; TEXT_PRINT(); }
#define	TEXT2()	{ gghc_parse_last_text = yyval.text = ssprintf("%s %s", yyvsp[-1].text, yyvsp[0].text); TEXT_PRINT(); }
#define	TEXT3() { gghc_parse_last_text = yyval.text = ssprintf("%s %s %s", yyvsp[-2].text, yyvsp[-1].text, yyvsp[0].text); TEXT_PRINT(); }
#define	TEXT4() { gghc_parse_last_text = yyval.text = ssprintf("%s %s %s %s", yyvsp[-3].text, yyvsp[-2].text, yyvsp[-1].text, yyvsp[0].text); TEXT_PRINT(); }
#define	TEXT5() { gghc_parse_last_text = yyval.text = ssprintf("%s %s %s %s %s", yyvsp[-4].text, yyvsp[-3].text, yyvsp[-2].text, yyvsp[-1].text, yyvsp[0].text); TEXT_PRINT(); }
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

%type	<u.cp>		IDENTIFIER TYPE_NAME

%type	<u.decl_spec>	declaration_specifiers
			specifier_qualifier_list

%type	<u.decl>	declarator direct_declarator
			init_declarator init_declarator_list
			declaration_list
			struct_declarator struct_declarator_list

%type	<u.i>		storage_class_specifier
%type	<u.cp>		type_specifier
%type	<u.cp>		struct_or_union_specifier struct_or_union

%type	<u.cp>		enum_specifier enumerator_list enumerator

%type	<u.cp>		pointer

%type	<u.cp>		parameter_type_list parameter_list
			parameter_declaration

%type	<u.cp>		abstract_declarator
			direct_abstract_declarator


%%

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
        { TEXT3(); }
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
        { TEXT4(); }
	| postfix_expression '(' ')'
        { TEXT3(); }
	| postfix_expression '(' argument_expression_list ')'
        { TEXT4(); }
	| postfix_expression '.' IDENTIFIER
        { TEXT3(); }
	| postfix_expression PTR_OP IDENTIFIER
        { TEXT3(); }
	| postfix_expression INC_OP
        { TEXT2(); }
	| postfix_expression DEC_OP
        { TEXT2(); }
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
        { TEXT3(); }
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
        { TEXT2(); }
	| DEC_OP unary_expression
        { TEXT2(); }
	| unary_operator cast_expression
        { TEXT2(); }
	| SIZEOF unary_expression
        { TEXT2(); }
	| SIZEOF '(' type_name ')'
        { TEXT4(); }
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
        { TEXT4(); }
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
        { TEXT3(); }
	| multiplicative_expression '/' cast_expression
        { TEXT3(); }
	| multiplicative_expression '%' cast_expression
        { TEXT3(); }
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
        { TEXT3(); }
	| additive_expression '-' multiplicative_expression
        { TEXT3(); }
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
        { TEXT3(); }
	| shift_expression RIGHT_OP additive_expression
        { TEXT3(); }
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
        { TEXT3(); }
	| relational_expression '>' shift_expression
        { TEXT3(); }
	| relational_expression LE_OP shift_expression
        { TEXT3(); }
	| relational_expression GE_OP shift_expression
        { TEXT3(); }
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
        { TEXT3(); }
	| equality_expression NE_OP relational_expression
        { TEXT3(); }
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
        { TEXT3(); }
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
        { TEXT3(); }
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
        { TEXT3(); }
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
        { TEXT3(); }
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
        { TEXT3(); }
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
        { TEXT5(); }
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
        { TEXT3(); }
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
        { TEXT3(); }
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
        { gghc_declaration(&$1, 0); TEXT2(); }
	| declaration_specifiers init_declarator_list ';'
        { gghc_declaration(&$1, $2); debug_stop_here(); TEXT3(); }
	;

declaration_specifiers
	: storage_class_specifier
        { $$.storage = $1; $$.type = gghc_type($$.type_text = "int"); TEXT1(); }

	| storage_class_specifier declaration_specifiers
        { $$.storage = $1; $$.type = $2.type; $$.type_text = $2.type_text; TEXT2(); }

	| type_specifier
        { $$.storage = 0; $$.type = $1; $$.type_text = $<text>1; TEXT1(); }

	| type_specifier declaration_specifiers
        { $$.storage = $2.storage; $$.type = $1; $$.type_text = $<text>1; TEXT2(); }

	| type_qualifier
        { $$.storage = 0; $$.type = gghc_type($$.type_text = "int"); TEXT1(); }

	| type_qualifier declaration_specifiers
        { $$ = $2; TEXT2(); }
	;

init_declarator_list
	: init_declarator
        { $$ = $1; TEXT1(); }
	| init_declarator_list ',' init_declarator
        { $3->next = $1; $$ = $3; TEXT3(); }
	;

init_declarator
	: declarator
        { $$ = $1; TEXT1(); }
	| declarator '=' initializer
        { $$ = $1; TEXT3(); }
	;

storage_class_specifier
: storage_class_specifier_TOKEN { $$ = $<token>1; TEXT1(); } ;

storage_class_specifier_TOKEN
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID
        { $$ = gghc_type(yyval.text = "void"); }
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
        { $$ = gghc_type(yyval.text = "char"); }
        | uchar_specifer
        { $$ = gghc_type(yyval.text = "unsigned char"); }
	| sshort_specifer
        { $$ = gghc_type(yyval.text = "short"); }
	| ushort_specifer
        { $$ = gghc_type(yyval.text = "unsigned short"); }
	| int_specifier
        { $$ = gghc_type(yyval.text = "int"); }
	| uint_specifier
        { $$ = gghc_type(yyval.text = "unsigned int"); }
	| slong_specifier
        { $$ = gghc_type(yyval.text = "long"); }
	| ulong_specifier
        { $$ = gghc_type(yyval.text = "unsigned long"); }
	| slong_long_specifier
        { $$ = gghc_type(yyval.text = "long long"); }
	| ulong_long_specifier
        { $$ = gghc_type(yyval.text = "unsigned long long"); }
	| FLOAT
        { $$ = gghc_type(yyval.text = "float"); }
	| DOUBLE
        { $$ = gghc_type(yyval.text = "double"); }
        | ldouble_specifier
        { $$ = gghc_type(yyval.text = "long double"); }
        | GGHC___builtin_va_list
        { $$ = gghc_type(yyval.text = "__builtin_va_list"); TEXT1(); }

	| struct_or_union_specifier
        { $$ = $1; TEXT1(); }
	| enum_specifier
        { $$ = $1; TEXT1(); }
	| TYPE_NAME
        { $$ = gghc_type($<text>1); TEXT1(); }
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
        { gghc_struct_type($1, $2); debug_stop_here(); }
          '{' struct_declaration_list '}'
          { $$ = gghc_struct_type_end(); }
	| struct_or_union
        { gghc_struct_type($1, ""); }
          '{' struct_declaration_list '}'
          { $$ = gghc_struct_type_end(); }
	| struct_or_union IDENTIFIER
        { $$ = gghc_type(ssprintf("%s %s", $1, $2)); TEXT2(); }
	;

struct_or_union
        : struct_or_union_TOKEN
        { $$ = $<text>1; TEXT1(); }
        ;
struct_or_union_TOKEN
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
        { TEXT1(); }
	| struct_declaration_list struct_declaration
        { TEXT2(); }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
        { TEXT3(); gghc_struct_type_element(&$1, $2, yyval.text); }
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
        { $$.type = $1; $$.type_text = $<text>2; TEXT2(); }
	| type_specifier
        { $$.type = ""; $$.type_text = ""; TEXT2(); }
	| type_qualifier specifier_qualifier_list
        { $$ = $2; TEXT2(); }
	| type_qualifier
        { $$.type = ""; $$.type_text = ""; TEXT1(); }
	;

struct_declarator_list
	: struct_declarator
        { $$ = $1; TEXT1(); }
	| struct_declarator_list ',' struct_declarator
        { $3->next = $1; $$ = $3; TEXT3(); }
	;

struct_declarator
	: declarator
        { $$ = $1; TEXT1(); }
	| ':' constant_expression
        { $$ = make_decl(); $$->is_bit_field = 1; $$->bit_field_size = $<text>2; TEXT2(); }
	| declarator ':' constant_expression
        { $$ = $1; $$->is_bit_field = 1; $$->bit_field_size = $<text>3; TEXT3(); }
	;

enum_specifier
        : ENUM
        { $<u.cp>$ = gghc_enum_type(0); }
            '{' enumerator_list '}'
             { gghc_enum_type_end(); TEXT4(); }
	| ENUM IDENTIFIER
        { $<u.cp>$ = gghc_enum_type($<text>2); }
            '{' enumerator_list '}'
             { gghc_enum_type_end(); TEXT5(); }
	| ENUM IDENTIFIER
        { $$ = gghc_type(ssprintf("enum %s", $2)); TEXT2(); }
	;

enumerator_list
	: enumerator
        { TEXT1(); }
	| enumerator_list ',' enumerator
        { TEXT3(); }
	;

enumerator
	: IDENTIFIER
        { gghc_enum_type_element($<text>1); TEXT1(); }
	| IDENTIFIER '=' constant_expression
        { gghc_enum_type_element($<text>1); TEXT3(); }
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
          TEXT2();
        }
        | direct_declarator
        { $$ = $1; TEXT1(); }
	;

direct_declarator
	: IDENTIFIER
        { $$ = make_decl(); $$->identifier = $1; TEXT1(); }
	| '(' declarator ')'
        {
          $$ = $2;
          $$->declarator_text = ssprintf("(%s)", $2->declarator_text);
          $$->is_parenthised = 1;
          TEXT3();
        }
	| direct_declarator '[' constant_expression ']'
        {
          $$ = $1;
          if ( $1->is_parenthised ) {
            $$->declarator = ssprintf($1->declarator, gghc_array_type("%s", $<text>3));
          } else {
            $$->declarator = gghc_array_type($1->declarator, $<text>3);
          }
          $$->declarator_text = ssprintf("%s[%s]", $1->declarator_text, $<text>3);
          $$->type = "array";
          TEXT4();
        }
	| direct_declarator '[' ']'
        {
          $$ = $1;
          if ( $1->is_parenthised ) {
            $$->declarator = ssprintf($1->declarator, gghc_array_type("%s", $<text>3));
          } else {
            $$->declarator = gghc_array_type($1->declarator, "");
          }
          $$->declarator_text = ssprintf("%s[%s]", $1->declarator_text, "");
          $$->type = "array";
          TEXT3();
        }
	| direct_declarator '(' parameter_type_list ')'
        {
          $$ = $1;
          if ( $1->is_parenthised ) {
            $$->declarator = ssprintf($1->declarator, gghc_function_type("%s", $3));
          } else {
            $$->declarator = gghc_function_type($1->declarator, $3);
          }
          $$->declarator_text = ssprintf("%s(%s)", $1->declarator_text, $3);
          $$->type = "function";
          TEXT4();
        }
	| direct_declarator '(' identifier_list ')'
        { TEXT4(); }
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
          TEXT3();
        }
	;

pointer
	: '*'
        { $$ = gghc_pointer_type("%s"); TEXT1(); }
	| '*' type_qualifier_list
        { $$ = gghc_pointer_type("%s"); TEXT2(); }
	| '*' pointer
        { $$ = gghc_pointer_type($2); TEXT2(); }
	| '*' type_qualifier_list pointer
        { $$ = gghc_pointer_type($3); TEXT3(); }
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list
        { $$ = $1; TEXT1(); }
	| parameter_list ',' ELLIPSIS
        { $$ = ssprintf("%s%s GGHCT_VARARGS", $1, mode_c ? "," : ""); TEXT3(); }
	;

parameter_list
	: parameter_declaration
        { $$ = $1; TEXT1(); }
	| parameter_list ',' parameter_declaration
        { $$ = ssprintf("%s%s %s", $1, mode_c ? "," : "", $3); TEXT3(); }
	;

parameter_declaration
	: declaration_specifiers declarator
        {
          char* output = ssprintf($2->declarator, $1.type);
#ifdef HOLD_AND_REST_VARS
          if ( $2->identifier ) {
            if ( strstr($2->identifier, "_HOLD_REST_") ) {
              output = gghc_hold_rest();
            } else
              if ( strstr($2->identifier, "_REST_") ) {
                output = gghc_rest();
              } else 
                if ( strstr($2->identifier, "_HOLD_") ) {
                  output = gghc_rest();
                }
          }
#endif
          $$ = output;
          TEXT2();
        }
	| declaration_specifiers abstract_declarator
        { $$ = ssprintf($2, $1.type); TEXT2(); }
	| declaration_specifiers
        { $$ = ssprintf("%s", $1.type); TEXT2(); }
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
        { TEXT2(); }
	;

abstract_declarator
	: pointer
        { $$ = gghc_pointer_type("%s"); TEXT1(); }
	| direct_abstract_declarator
        { $$ = $1; TEXT1(); }
	| pointer direct_abstract_declarator
        { $$ = gghc_pointer_type($2); TEXT2(); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
        { $$ = $2; TEXT3(); }
	| '[' ']'
        { $$ = gghc_array_type("%s", ""); TEXT2(); }
	| '[' constant_expression ']'
        { $$ = gghc_array_type("%s", $<text>2); TEXT3(); }
	| direct_abstract_declarator '[' ']'
        { $$ = gghc_array_type($1, ""); TEXT3(); }
	| direct_abstract_declarator '[' constant_expression ']'
        { $$ = gghc_array_type("%s", $<text>3); TEXT4(); }
	| '(' ')'
        { $$ = gghc_function_type("%s", ""); TEXT3(); }
	| '(' parameter_type_list ')'
        { $$ = gghc_function_type("%s", $2); TEXT3(); }
	| direct_abstract_declarator '(' ')'
        { $$ = gghc_function_type($1, ""); TEXT3(); }
	| direct_abstract_declarator '(' parameter_type_list ')'
        { $$ = gghc_function_type($1, $3); TEXT4(); }
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
        { TEXT1(); }
	| declaration_list declaration
        { TEXT2(); }
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

