%{
/*
** Copyright 1993, 1994, 2014 Kurt A. Stephens
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

char*	gghc_parse_last_text = "";

static int yylex ();

void	yywarning(const char* s)
{
	fprintf(stderr, "gghc: %s in file %s: line %d\n",
		s,
		gghc_parse_filename ? gghc_parse_filename : "UNKNOWN",
		gghc_parse_lineno
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

%token	AUTO REGISTER STATIC EXTERN TYPEDEF
%token	VOID CHAR SHORT INT LONG FLOAT DOUBLE SIGNED UNSIGNED
%token	CONST VOLATILE
%token	STRUCT UNION
%token	ENUM
%token	DDD
%token	CASE DEFAULT
%token	IF ELSE SWITCH
%token	WHILE DO FOR
%token	GOTO CONTINUE BREAK RETURN
%token	MULA DIVA MODA ADDA SUBA LSA RSA AA XA OA
%token	LO LA
%token	EQ NE
%token	LE GE
%token	LS RS
%token	INC DEC SIZEOF
%token	AR

/**************************************/

%token		CHAR_CONSTANT
%token		WCHAR_CONSTANT
%token		INT_CONSTANT
%token		UNSIGNED_INT_CONSTANT
%token		LONG_INT_CONSTANT
%token		UNSIGNED_LONG_INT_CONSTANT
%token		FLOAT_CONSTANT
%token		DOUBLE_CONSTANT
%token		LONG_DOUBLE_CONSTANT
%token		ENUM_CONSTANT
%token		STRING_CONSTANT
%token		WSTRING_CONSTANT
%token		IDENTIFIER

%token		TYPEDEF_NAME

/*******************************************************************************************/

%type	<u.cp>		IDENTIFIER IDENTIFIER_opt TYPEDEF_NAME

%type	<u.decl_spec>	declaration_specifiers declaration_specifiers_opt
			specifier_qualifier_list specifier_qualifier_list_opt

%type	<u.i>		storage_class_specifier
%type	<u.cp>		type_specifier
%type	<u.cp>		struct_or_union_specifier struct_or_union

%type	<u.cp>		enum_specifier enumerator_list enumerator

%type	<u.decl>	declarator direct_declarator declarator_opt
			init_declarator_list init_declarator_list_opt
			init_declarator
			declaration_list declaration_list_opt
			struct_declarator_list struct_declarator

%type	<u.cp>		pointer pointer_opt

%type	<u.cp>		parameter_type_list parameter_list parameter_type_list_opt
			parameter_declaration

%type	<u.cp>		abstract_declarator abstract_declarator_opt
			direct_abstract_declarator direct_abstract_declarator_opt

%%

translation_unit:
	  external_declaration
	| translation_unit external_declaration
	;

external_declaration:
	  top_level_declaration
	| function_definition
	;

function_definition:
	declaration_specifiers_opt declarator declaration_list_opt compound_statement 
{
	gghc_declaration(&$1, $2);	
}
	;

top_level_declaration:
	declaration_specifiers init_declarator_list_opt ';'
{
	gghc_declaration(&$1, $2);
}
	;

declaration:
	declaration_specifiers init_declarator_list_opt ';'
	;



declaration_list:
	  declaration						{ TEXT1(); }
	| declaration_list declaration				{ TEXT2(); }
	;

declaration_list_opt:
	/* EMPTY */						{ TEXT0(); }
	| declaration_list					{ TEXT1(); }
	;

declaration_specifiers:
	  storage_class_specifier declaration_specifiers_opt	{ $$.storage = $1; $$.type = $2.type; $$.type_text = $2.type_text; TEXT2(); }
	| type_specifier declaration_specifiers_opt		{ $$.storage = $2.storage; $$.type = $1; $$.type_text = yyvsp[-1].text; TEXT2(); }
	| type_qualifier declaration_specifiers_opt		{ $$ = $2; TEXT2(); }
	;

declaration_specifiers_opt:
	  /* EMPTY */						{ $$.storage = 0; $$.type = gghc_type("int"); $$.type_text = "int"; TEXT0(); }
	| declaration_specifiers				{ $$ = $1; TEXT1(); }
	;


storage_class_specifier:
	  AUTO							{ $$ = AUTO;		TEXT1(); }
	| REGISTER						{ $$ = REGISTER;	TEXT1(); }
	| STATIC						{ $$ = STATIC;		TEXT1(); }
	| EXTERN						{ $$ = EXTERN;		TEXT1(); }
	| TYPEDEF						{ $$ = TYPEDEF;		TEXT1(); }
	;

type_specifier:
	  VOID							{ $$ = gghc_type("void"); yyval.text = "void"; }
	| char_specifier					{ $$ = gghc_type("char"); yyval.text = "char"; }
	| uchar_specifer					{ $$ = gghc_type("unsigned char"); yyval.text = "unsigned char"; }
	| sshort_specifer					{ $$ = gghc_type("short"); yyval.text = "short"; }
	| ushort_specifer					{ $$ = gghc_type("unsigned short"); yyval.text = "unsigned short"; }
	| int_specifier						{ $$ = gghc_type("int"); yyval.text = "int"; }
	| uint_specifier					{ $$ = gghc_type("unsigned int"); yyval.text = "unsigned int"; }
	| slong_specifier					{ $$ = gghc_type("long"); yyval.text = "long"; }
	| ulong_specifier					{ $$ = gghc_type("unsigned long"); yyval.text = "unsigned long";  }
	| slong_long_specifier					{ $$ = gghc_type("long long"); yyval.text = "long long"; }
	| ulong_long_specifier					{ $$ = gghc_type("unsigned long long"); yyval.text = "unsigned long long";  }
	| FLOAT							{ $$ = gghc_type("float"); yyval.text = "float"; }
	| DOUBLE						{ $$ = gghc_type("double"); yyval.text = "double"; }
	| ldouble_specifier					{ $$ = gghc_type("long double"); yyval.text = "long double"; }
	| struct_or_union_specifier				{ $$ = $1; TEXT1(); }
	| enum_specifier					{ $$ = $1; TEXT1(); }
	| TYPEDEF_NAME						{ $$ = gghc_type($1); TEXT1(); }
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

type_qualifier:
	  CONST							{ TEXT1(); }
	| VOLATILE						{ TEXT1(); }
	;


/**************************************************************************************************************/


struct_or_union_specifier:
	  struct_or_union IDENTIFIER_opt			{ $<u.cp>$ = gghc_struct_type($1, $2); }
	  '{' struct_declaration_list '}'			{ $<u.cp>$ = gghc_struct_type_end(); }

	| struct_or_union IDENTIFIER				{ $$ = gghc_type(ssprintf("%s %s", $1, $2)); TEXT2(); }
	;

struct_or_union:
	  STRUCT						{ $$ = "struct"; TEXT1(); }
	| UNION							{ $$ = "union"; TEXT1(); }
	;

struct_declaration_list:
	  struct_declaration					{ TEXT1(); }
	| struct_declaration_list struct_declaration		{ TEXT2(); }
	;


struct_declaration:
	specifier_qualifier_list struct_declarator_list ';'	{ TEXT3(); gghc_struct_type_element(&$1, $2, yyval.text);  }
	;

specifier_qualifier_list:
	  type_specifier specifier_qualifier_list_opt		{ $$.type = $1; $$.type_text = yyvsp[-1].text; TEXT2(); }
	| type_qualifier specifier_qualifier_list_opt		{ $$ = $2; TEXT2(); }
	;

specifier_qualifier_list_opt:
	  /* EMPTY */						{ $$.type = ""; $$.type_text = ""; TEXT0(); }
	| specifier_qualifier_list				{ $$ = $1; TEXT1(); }
	;


struct_declarator_list:
	  struct_declarator					{ $$ = $1; TEXT1(); }
	| struct_declarator_list ',' struct_declarator		{ $3->next = $1; $$ = $3; TEXT3(); }
	;

struct_declarator:
	  declarator						{ $$ = $1; TEXT1(); }

| declarator_opt ':' constant_expression		{ $$ = $1; $$->is_bit_field = 1; $$->bit_field_size = $<text>3; TEXT3(); }
	;


/**************************************************************************************************************/


enum_specifier:
	  ENUM IDENTIFIER_opt 
{
	$<u.cp>$ = gghc_enum_type($2);
}
'{' enumerator_list '}'
{
	gghc_enum_type_end();
	TEXT4();
}

	| ENUM IDENTIFIER					{ $$ = gghc_type($2); TEXT2(); }
	;

enumerator_list:
	  enumerator						{ TEXT1(); }
	| enumerator_list ',' enumerator			{ TEXT3(); }
	;

enumerator:
	  IDENTIFIER						{ gghc_enum_type_element($1); TEXT1(); }
	| IDENTIFIER '=' constant_expression_no_comma		{ gghc_enum_type_element($1); TEXT3(); }
	;

/**************************************************************************************************************/


init_declarator_list:
	  init_declarator					{ $$ = $1; TEXT1(); }
	| init_declarator_list ',' init_declarator		{ $3->next = $1; $$ = $3; TEXT3(); }
	;

init_declarator_list_opt:
	  /* EMPTY */						{ $$ = 0; TEXT0(); }
	| init_declarator_list					{ $$ = $1; TEXT1(); }
	;


init_declarator:
	  declarator						{ $$ = $1; TEXT1(); }
	| declarator '=' initializer				{ $$ = $1; TEXT3(); }
	;




declarator:
	  direct_declarator					{ $$ = $1; TEXT1(); }
	| pointer direct_declarator
{
  TEXT2();
#if 0
  if ( gghc_debug ) {
    fprintf(stderr, "/* text = '%s' */\n", gghc_parse_last_text);
    fprintf(stderr, "/* pointer = '%s' */\n", $1);
    fprintf(stderr, "/* declarator = '%s' */\n", $2->declarator);
    fprintf(stderr, "\n");
  }
#endif
  $$ = $2;
  if ( $$->type[0] == 'f' || $$->type[0] == 'a' ) {
    $$->declarator = ssprintf($2->declarator, $1);
  } else {
    $$->declarator = ssprintf($1, $2->declarator);
  }
  $$->declarator_text = ssprintf("%s %s", yyvsp[-1].text, $2->declarator_text);
}
	;

declarator_opt:
	/* EMPTY */						{ $$ = 0; TEXT0(); }
	| declarator						{ $$ = $1; TEXT1(); }
	;

direct_declarator:
	  IDENTIFIER
{
  $$ = malloc(sizeof(gghc_decl));
  $$->identifier = $1;
  $$->declarator = "%s";
  $$->declarator_text = "%s";
  $$->type = "variable";
  $$->is_bit_field = 0;
  $$->is_parenthised = 0;
  $$->next = 0;
  TEXT1();
}

	| '(' declarator ')'
{
  $$ = $2;
  $$->declarator_text = ssprintf("(%s)", $2->declarator_text);
  $$->is_parenthised = 1;
  TEXT3();
}

	| direct_declarator '[' constant_expression_opt ']'
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
#if 0
  printf("array '%s': $1->declarator = '%s'\n$3.text = '%s'\n\n", yyval.text, $1->declarator, $<text>3);
#endif
}

	| direct_declarator '(' parameter_type_list ')'	
{
  $$ = $1;
  if ( $1->is_parenthised ) {
    $$->declarator = ssprintf($1->declarator, gghc_function_type("%s", $3));
  } else {
    $$->declarator = gghc_function_type($1->declarator, $3);
  }

  $$->declarator_text = ssprintf("%s(%s)", $1->declarator_text, yyvsp[-1].text);
  $$->type = "function";
  TEXT4();
}

	| direct_declarator '(' identifier_list_opt ')'		{ TEXT4(); }
	;


pointer:
	  '*' type_qualifier_list_opt				{ $$ = gghc_pointer_type("%s"); TEXT2(); }
	| '*' type_qualifier_list_opt pointer			{ $$ = gghc_pointer_type($3); TEXT3(); }
	;

pointer_opt:
	  /* EMPTY */						{ $$ = ""; TEXT0(); }
	| pointer						{ $$ = $1; TEXT1(); }
	;


type_qualifier_list:
	  type_qualifier					{ TEXT1(); }
	| type_qualifier_list type_qualifier			{ TEXT2(); }
	;

type_qualifier_list_opt:
	  /* EMPTY */						{ TEXT0(); }
	| type_qualifier_list					{ TEXT1(); }
	;


parameter_type_list:
	  parameter_list					{ $$ = $1; TEXT1(); }
| parameter_list ',' DDD				{ $$ = ssprintf("%s%s GGHCT_VARARGS", $1, mode_c ? "," : ""); TEXT3(); }
	;

parameter_type_list_opt:
	/* EMPTY */						{ $$ = ""; TEXT0(); }
	| parameter_type_list					{ $$ = $1; TEXT1(); }
	;


parameter_list:
	  parameter_declaration					{ $$ = $1; TEXT1(); }
| parameter_list ',' parameter_declaration		{ $$ = ssprintf("%s%s %s", $1, mode_c ? "," : "", $3); TEXT3(); }
	;

parameter_declaration:
	  declaration_specifiers declarator
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

	| declaration_specifiers abstract_declarator_opt	{ $$ = ssprintf($2, $1.type); TEXT2(); }
	;

identifier_list_opt:
	  /* EMPTY */						{ TEXT0(); }
	| identifier_list					{ TEXT1(); }
	;

identifier_list:
	  IDENTIFIER						{ TEXT1(); }
	| identifier_list ',' IDENTIFIER			{ TEXT3(); }
	;

initializer:
	  assignment_expression					{ TEXT1(); }
	| '{' initializer_list '}'				{ TEXT3(); }
	| '{' initializer_list ',' '}'				{ TEXT4(); }
	;

initializer_list:
	  initializer						{ TEXT1(); }
	| initializer_list ',' initializer			{ TEXT3(); }
	;

type_name:
	specifier_qualifier_list abstract_declarator_opt	{ TEXT2(); }
	;



abstract_declarator:
	  pointer						{ $$ = gghc_pointer_type("%s"); TEXT1(); }
	| pointer direct_abstract_declarator			{ $$ = gghc_pointer_type($2); TEXT2(); }
	| direct_abstract_declarator				{ $$ = $1; TEXT1(); }
	;

abstract_declarator_opt:
	  /* EMPTY */						{ $$ = "%s"; TEXT0(); }
	| abstract_declarator					{ $$ = $1; TEXT1(); }
	;

direct_abstract_declarator:
	  '(' abstract_declarator ')'				{ $$ = $2; TEXT3(); }
	| direct_abstract_declarator_opt '[' constant_expression_opt ']'
								{ $$ = gghc_array_type($1, $<text>3); TEXT4(); }
	| direct_abstract_declarator_opt '(' parameter_type_list_opt ')'
								{ $$ = gghc_function_type($1, $3); TEXT4(); }
	;

direct_abstract_declarator_opt:
	  /* EMPTY */						{ $$ = "%s"; TEXT0(); }
	| direct_abstract_declarator				{ $$ = $1; TEXT1(); }
	;


typedef_name:
	TYPEDEF_NAME						{ TEXT1(); }
	;

statement:
	  labeled_statement
	| expression_statement
	| compound_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement:
	  IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

expression_statement:
	| expression_opt ';'
	;

compound_statement:
	'{' declaration_list_opt statement_list_opt '}'
	;

statement_list:
	  statement
	| statement_list statement
	;

statement_list_opt:
	| statement_list
	;

selection_statement:
	  IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement:
	  WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')'
	| FOR '(' expression_opt ';' expression_opt ';' expression_opt ')' statement
	;

jump_statement:
	  GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN expression_opt ';'
	;


expression:
	  assignment_expression						{ TEXT1(); }
	| expression ',' assignment_expression				{ TEXT3(); }
	;

expression_opt:
	  /* EMPTY */							{ TEXT0(); }
	| expression							{ TEXT1(); }
	;

/* ADDED */
expression_no_comma:
	assignment_expression						{ TEXT1(); }
	;

constant_expression_no_comma:
	assignment_expression						{ TEXT1(); }
	;
/* END ADDED */

assignment_expression:
	  conditional_expression					{ TEXT1(); }
	| unary_expression assignment_operator assignment_expression
									{ TEXT3(); }
	;

assignment_operator:
	  '='								{ TEXT1(); }
	/* *= */	| MULA						{ TEXT1(); }
	/* /= */	| DIVA						{ TEXT1(); }
	/* %= */	| MODA						{ TEXT1(); }
	/* += */	| ADDA						{ TEXT1(); }
	/* -= */	| SUBA						{ TEXT1(); }
	/* <<= */	| LSA						{ TEXT1(); }
	/* >>= */	| RSA						{ TEXT1(); }
	/* &= */	| AA						{ TEXT1(); }
	/* ^= */	| XA						{ TEXT1(); }
	/* |= */	| OA						{ TEXT1(); }
	;

conditional_expression:
	  logical_OR_expression						{ TEXT1(); }
	| logical_OR_expression '?' expression ':' conditional_expression { TEXT5(); }
	;

logical_OR_expression:
	  logical_AND_expression					{ TEXT1(); }
	| logical_OR_expression /* || */ LO logical_AND_expression	{ TEXT3(); }
	;

logical_AND_expression:
	  inclusive_OR_expression					{ TEXT1(); }
	| logical_AND_expression /* && */ LA inclusive_OR_expression	{ TEXT3(); }
	;

inclusive_OR_expression:
	  exclusive_OR_expression					{ TEXT1(); }	
	| inclusive_OR_expression '|' exclusive_OR_expression		{ TEXT3(); }

exclusive_OR_expression:
	  AND_expression						{ TEXT1(); }
	| exclusive_OR_expression '^' AND_expression			{ TEXT3(); }

AND_expression:
	  equality_expression						{ TEXT1(); }
	| AND_expression '&' equality_expression			{ TEXT3(); }
	;

equality_expression:
	  relational_expression						{ TEXT1(); }
	| equality_expression /* == */ EQ relational_expression		{ TEXT3(); }
	| equality_expression /* != */ NE relational_expression		{ TEXT3(); }
	;

relational_expression:
	  shift_expression						{ TEXT1(); }
	| relational_expression '<' shift_expression			{ TEXT3(); }
	| relational_expression '>' shift_expression			{ TEXT3(); }
	| relational_expression /* <= */ LE shift_expression		{ TEXT3(); }
	| relational_expression /* >= */ GE shift_expression		{ TEXT3(); }
	;

shift_expression:
	  additive_expression						{ TEXT1(); }
	| shift_expression /* << */ LS additive_expression		{ TEXT3(); }
	| shift_expression /* >> */ RS additive_expression		{ TEXT3(); }
	;

additive_expression:
	  multiplicative_expression					{ TEXT1(); }
	| additive_expression '+' multiplicative_expression		{ TEXT3(); }
	| additive_expression '-' multiplicative_expression		{ TEXT3(); }
	;

multiplicative_expression:
	  cast_expression
	| multiplicative_expression '*' cast_expression			{ TEXT3(); }
	| multiplicative_expression '/' cast_expression			{ TEXT3(); }	
	| multiplicative_expression '%' cast_expression			{ TEXT3(); }
	;

cast_expression:
	  unary_expression						{ TEXT1(); }
	| '(' type_name ')' cast_expression				{ TEXT4(); }
	;

unary_expression:
	  postfix_expression						{ TEXT1(); }
	| /* ++ */ INC unary_expression					{ TEXT2(); }
	| /* -- */ DEC unary_expression					{ TEXT2(); }
	| unary_operator cast_expression				{ TEXT2(); }
	| SIZEOF unary_expression					{ TEXT2(); }
	| SIZEOF '(' type_name ')'					{ TEXT4(); }
	;

unary_operator:
	  '&'								{ TEXT1(); }
	| '*'								{ TEXT1(); }
	| '+'								{ TEXT1(); }
	| '-'								{ TEXT1(); }
	| '~'								{ TEXT1(); }
	| '!'								{ TEXT1(); }
	;

postfix_expression:
	primary_expression						{ TEXT1(); }
	| postfix_expression '[' expression ']'				{ TEXT4(); }
	| postfix_expression '(' argument_expression_list_opt ')'	{ TEXT4(); }
	| postfix_expression '.' IDENTIFIER				{ TEXT3(); }
	| postfix_expression /* -> */ AR  IDENTIFIER			{ TEXT3(); }
	| postfix_expression /* -- */ INC				{ TEXT2(); }
	| postfix_expression /* ++ */ DEC				{ TEXT2(); }
	;

primary_expression:
	  IDENTIFIER							{ TEXT1(); }
	| constant							{ TEXT1(); }
	| '(' expression ')'						{ TEXT3(); }
	;

argument_expression_list:
	  assignment_expression						{ TEXT1(); } 
	| argument_expression_list ',' assignment_expression		{ TEXT3(); }
	;

argument_expression_list_opt:
	 /* EMPTY */							{ TEXT0(); }
	| argument_expression_list					{ TEXT1(); }
	;

constant:
	  WCHAR_CONSTANT						{ TEXT1(); }
	| CHAR_CONSTANT							{ TEXT1(); }
	| INT_CONSTANT							{ TEXT1(); }
	| UNSIGNED_INT_CONSTANT						{ TEXT1(); }
	| LONG_INT_CONSTANT						{ TEXT1(); }
	| UNSIGNED_LONG_INT_CONSTANT					{ TEXT1(); }
	| FLOAT_CONSTANT						{ TEXT1(); }
	| DOUBLE_CONSTANT						{ TEXT1(); }
	| LONG_DOUBLE_CONSTANT						{ TEXT1(); }
	| ENUM_CONSTANT							{ TEXT1(); }
	| STRING_CONSTANT						{ TEXT1(); }
	| WSTRING_CONSTANT						{ TEXT1(); }
	;

/******* ADDED ********/

constant_expression:
	  expression							{ TEXT1(); }
	| constant							{ TEXT1(); }
	;

constant_expression_opt:
	 /* EMPTY */							{ TEXT0(); }
	| constant_expression						{ TEXT1(); }
	;

IDENTIFIER_opt:
	  /* EMPTY */							{ $$ = 0; TEXT0(); }
	| IDENTIFIER							{ $$ = $1; TEXT1(); }
	;

%%


