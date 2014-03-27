%{ /* -*- bison-mode -*- */
/* From http://www.lysator.liu.se/c/ANSI-C-grammar-y.html. */

/*
** Changes: Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#include "parse.h"

 void gghc_debug_stop_here()
 {
 }

void *gghc_malloc0(size_t size)
{
   void *ptr = malloc(size);
   memset(ptr, 0, size);
   return ptr;
}

gghc_decl *gghc_m_decl()
{
  gghc_decl *x = gghc_malloc0(sizeof(gghc_decl));
  x->identifier = "";
  x->declarator = "%s";
  x->declarator_text = "%s";
  x->type = "variable";
  return x;
}

gghc_decl *gghc_m_decl_array(gghc_ctx ctx, gghc_decl *decl, const char *size)
{
    if ( ! size ) size = "";
    if ( decl->is_parenthised ) {
        decl->declarator = ssprintf(decl->declarator, gghc_array_type(ctx, "%s", size));
    } else {
        decl->declarator = gghc_array_type(ctx, decl->declarator, size);
    }
    decl->declarator_text = ssprintf("%s[%s]", decl->declarator_text, size);
    decl->type = "array";
    return decl;
}

gghc_decl *gghc_m_decl_func(gghc_ctx ctx, gghc_decl *decl, const char *params)
{
    if ( ! params ) params  = "";
    if ( decl->is_parenthised ) {
        decl->declarator = ssprintf(decl->declarator, gghc_function_type(ctx, "%s", params));
    } else {
        decl->declarator = gghc_function_type(ctx, decl->declarator, params);
    }
    decl->declarator_text = ssprintf("%s(%s)", decl->declarator_text, params);
    decl->type = "function";
    return decl;
}

static void token_msg(gghc_ctx ctx, const char *desc, int indent, mm_buf_state *s)
{
  int size = s->size;
  if ( size > 120 ) size = 120;
  fprintf(ctx->_stderr, "gghc:%s%*s`", desc, (int) indent, "");
  fwrite(s->beg, size, 1, ctx->_stderr);
  fprintf(ctx->_stderr, "'\n");
}

static void parse_msg(gghc_ctx ctx, const char *desc, const char *s)
{
  mm_buf_region *t = ctx->last_token;
  if ( t ) {
    fprintf(ctx->_stderr, "gghc: %s%s:%d:%d %s\n",
            desc,
          t->beg.src.filename ? t->beg.src.filename : "UNKNOWN",
          t->beg.src.lineno,
          t->beg.src.column,
          s
          );
    {
      mm_buf_state s;
      s.beg = s.end = t->beg.beg;
      s.beg ++;
      while ( s.beg > t->mb->s.beg && *s.beg != '\n' ) s.beg --;
      s.beg ++;
      while ( s.end < t->mb->s.end && *s.end != '\n' ) s.end ++;
      s.size = s.end - s.beg;
      token_msg(ctx, " token: ", t->beg.beg - s.beg, &t->beg);
      token_msg(ctx, " line:  ", 0, &s);
    }
  } else {
    fprintf(ctx->_stderr, "gghc: 0:0:0 %s\n", s);
  }
}

 void gghc_yywarning(gghc_ctx ctx, const char* s)
{
  parse_msg(ctx, "warning: ", s);
}

 void gghc_yyerror(gghc_ctx ctx, const char* s)
{
  parse_msg(ctx, "ERROR: ", s);
  ctx->errors ++;
}

/****************************************************************************************/

static char *_to_expr(YYSTYPE *yyvsp)
{
  if ( yyvsp->expr ) return yyvsp->expr;
  if ( yyvsp->type ) return yyvsp->type;
  return yyvsp->expr = mm_buf_region_cstr(&(yyvsp->t));
}
#define EXPR(YYV) _to_expr((void*)&(YYV))

static char *_to_type(YYSTYPE *yyvsp)
{
  if ( yyvsp->type ) return yyvsp->type;
  if ( yyvsp->expr ) return yyvsp->expr;
  return yyvsp->type = mm_buf_region_cstr(&(yyvsp->t));
}
#define TYPE(YYV) _to_type((void*)&(YYV))

#define YY_USER_ACTION(yyn) token_merge(_ctx, yyn, yylen, &yyval, yyvsp);
static void token_merge(gghc_ctx ctx, int yyn, int yylen, YYSTYPE *yyvalp, YYSTYPE *yyvsp)
{
  int i;
  mm_buf_region t, *dst = &yyvalp->t, *src;
  int verbose = ctx->_yydebug;

  mm_buf_region_init(&t);
  if ( verbose >= 2 ) {
    fprintf(ctx->_stderr, "  yyn=%d yylen=%d yyvalp=%p yyvsp=%p\n", yyn, yylen, yyvalp, yyvsp);
    fprintf(ctx->_stderr, "    tokens: ");
  }
  for ( i = 1 - yylen; i <= 0; ++ i ) {
    src = &(yyvsp[i].t);
    if ( verbose >= 3 ) fprintf(ctx->_stderr, "'%s' ", mm_buf_region_cstr(src));
    mm_buf_region_union(&t, &t, src);
  }
  if ( verbose >= 2 ) fprintf(ctx->_stderr, "\n ");
  if ( verbose >= 1 ) {
    fprintf(ctx->_stderr, "    = '%s'\n", mm_buf_region_cstr(&t));
  }
  *dst = t;
}

#define yylex(arg) gghc_yylex(_ctx, (arg))
#define ctx ((gghc_ctx) _ctx)

%}
%define api.pure full
 /*
%lex-param   {gghc_ctx *ctx}
%parse-param {gghc_ctx *ctx}
%param       {gghc_ctx *ctx}
 */
%verbose
 /*
%define parse.trace
 */

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
%token GGHC___inline
%token GGHC___inline__
%token GGHC___alignof
%token GGHC___alignof__
%token GGHC___builtin_va_list
%token GGHC___builtin_va_arg
%token GGHC___attribute__
%token GGHC___asm
%token GGHC___asm__
%token GGHC___restrict
%token GGHC___extension__
%token GGHC___typeof__
%token GGHC___const

%start translation_unit

%type   <expr>          IDENTIFIER TYPE_NAME identifier

%type   <expr>          declaration
                        declaration_ANSI

%type   <u.decl_spec>   declaration_specifiers
                        specifier_qualifier_list

%type   <u.decl>        declarator
                        direct_declarator
                        direct_declarator_ANSI
                        init_declarator init_declarator_list
                        declaration_list
                        struct_declarator
                        struct_declarator_list

%type   <u.i>           storage_class_specifier
                        storage_class_specifier_ANSI
                        storage_class_specifier_EXT
%type   <type>          type_specifier
%type   <type>          struct_or_union_specifier
                        struct_or_union_specifier_ANSI
                        struct_or_union_specifier_ANSI_def
                        struct_or_union

%type   <type>          enum_specifier enumerator_list enumerator

%type   <type>          pointer

%type   <type>          parameter_type_list parameter_list
                        parameter_declaration

%type   <type>          abstract_declarator
                        direct_abstract_declarator
                        direct_abstract_declarator_EXT

%%

primary_expression
	: IDENTIFIER
	| CONSTANT
	| string_constant
	| '(' expression ')'
        | primary_expression_EXT
	;

primary_expression_EXT
        : '(' compound_statement ')'
        | GGHC___builtin_va_arg '(' expression ',' type_name ')'
        | __alignof__ '(' type_name ')'
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
        : __attribute__ declaration_ANSI { $$ = $2; }
        |               declaration_ANSI { $$ = $1; }
        ;

declaration_ANSI
	: declaration_specifiers ';'
        { gghc_declaration(ctx, &$1, 0); }
	| declaration_specifiers init_declarator_list ';'
        { gghc_declaration(ctx, &$1, $2); }
	;

declaration_specifiers
	: storage_class_specifier
        { $$.storage = $1; $$.type = gghc_type(ctx, $$.type_text = "int"); }

	| storage_class_specifier declaration_specifiers
        { $$.storage = $1; $$.type = $2.type; $$.type_text = $2.type_text; }

	| type_specifier
        { $$.storage = 0; $$.type = $$.type_text = TYPE($<u>1); }

	| type_specifier declaration_specifiers
        { $$.storage = $2.storage; $$.type = $$.type_text = TYPE($<u>1); }
/*
        { $$.storage = $2.storage; $$.type = $2.type; $$.type_text = $2.type_text; }
*/

	| type_qualifier
        { $$.storage = 0; $$.type = gghc_type(ctx, $$.type_text = "int"); }

	| type_qualifier declaration_specifiers
        { $$.storage = $2.storage; $$.type = $2.type; $$.type_text = $2.type_text; }
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
  : storage_class_specifier_EXT { $$ = $1; }
  ;

storage_class_specifier_EXT
  : storage_class_specifier_ANSI
    { $$ = $1; }
  | storage_class_specifier_OTHER
    { $$ = $<token>1; }
  | storage_class_specifier_EXT storage_class_specifier_ANSI
    { $$ = $2; }
  | storage_class_specifier_EXT storage_class_specifier_OTHER
    { $$ = $1; }
  ;

storage_class_specifier_OTHER
  : __inline__
  | __attribute__
  | __extension__
  ;

storage_class_specifier_ANSI
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
        { $$ = gghc_type(ctx, "void"); }
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
        { $$ = gghc_type(ctx, "char"); }
        | uchar_specifer
        { $$ = gghc_type(ctx, "unsigned char"); }
	| sshort_specifer
        { $$ = gghc_type(ctx, "short"); }
	| ushort_specifer
        { $$ = gghc_type(ctx, "unsigned short"); }
	| int_specifier
        { $$ = gghc_type(ctx, "int"); }
	| uint_specifier
        { $$ = gghc_type(ctx, "unsigned int"); }
	| slong_specifier
        { $$ = gghc_type(ctx, "long"); }
	| ulong_specifier
        { $$ = gghc_type(ctx, "unsigned long"); }
	| slong_long_specifier
        { $$ = gghc_type(ctx, "long long"); }
	| ulong_long_specifier
        { $$ = gghc_type(ctx, "unsigned long long"); }
	| FLOAT
        { $$ = gghc_type(ctx, "float"); }
	| DOUBLE
        { $$ = gghc_type(ctx, "double"); }
        | ldouble_specifier
        { $$ = gghc_type(ctx, "long double"); }
        | GGHC___builtin_va_list
        { $$ = gghc_type(ctx, "__builtin_va_list"); }

	| struct_or_union_specifier
        { $$ = $1; }
	| enum_specifier
        { $$ = $1; }
	| TYPE_NAME
        { $$ = gghc_type(ctx, EXPR($<u>1)); }
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
:                struct_or_union_specifier_ANSI
;

struct_or_union_specifier_ANSI
        : struct_or_union_specifier_ANSI_def
	| struct_or_union identifier
        { $$ = gghc_struct_type_forward(ctx, $1, EXPR($<u>2)); }
        ;

struct_or_union_specifier_ANSI_def
	: struct_or_union identifier
        { gghc_struct_type(ctx, $1, EXPR($<u>2)); }
          '{' struct_declaration_list '}'
          { $$ = gghc_struct_type_end(ctx); }
	| struct_or_union
        { gghc_struct_type(ctx, $1, ""); }
          '{' struct_declaration_list '}'
          { $$ = gghc_struct_type_end(ctx); }
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
: __extension__ struct_or_union_specifier_ANSI_def ';'
{ gghc_debug_stop_here(); /* inline union */ }
| struct_declaration_ANSI
;

struct_declaration_ANSI
	: specifier_qualifier_list struct_declarator_list ';'
        { gghc_struct_type_element(ctx, &$1, $2, EXPR($<u>2)); }
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
        { $$.type = $1; $$.type_text = $<text>2; }
	| type_specifier
        { $$.type = $1; $$.type_text = ""; }
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
	| ':' constant_expression
        { $$ = gghc_m_decl(); $$->is_bit_field = 1; $$->bit_field_size = EXPR($<u>2); }
	| declarator ':' constant_expression
        { $$ = $1; $$->is_bit_field = 1; $$->bit_field_size = EXPR($<u>3); }
	;

enum_specifier
        : ENUM
        { gghc_enum_type(ctx, 0); }
            '{' enumerator_list '}'
            { $$ = gghc_enum_type_end(ctx); }
	| ENUM identifier
        { gghc_enum_type(ctx, EXPR($<u>2)); }
            '{' enumerator_list '}'
            { $$ = gghc_enum_type_end(ctx); }
	| ENUM identifier
        { $$ = gghc_enum_type_forward(ctx, EXPR($<u>2)); }
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
        { gghc_enum_type_element(ctx, EXPR($<u>1)); }
	| IDENTIFIER '=' constant_expression
        { gghc_enum_type_element(ctx, EXPR($<u>1)); }
	;

type_qualifier
        : CONST
        | GGHC___const
        | VOLATILE
        | GGHC___restrict
        | __extension__
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
          $$->declarator_text = ssprintf("%s %s", TYPE($<u>2), $2->declarator_text);
        }
        | direct_declarator
	;

direct_declarator:
      direct_declarator_ANSI                        { $$ = $1; }
    | direct_declarator_ANSI direct_declarator_EXTs { $$ = $1; }
    ;

direct_declarator_ANSI
	: IDENTIFIER
        { $$ = gghc_m_decl(); $$->identifier = EXPR($<u>1); }
	| '(' declarator ')'
        {
          $$ = $2;
          $$->declarator_text = ssprintf("(%s)", $$->declarator_text);
          $$->is_parenthised = 1;
        }
	| direct_declarator_ANSI '[' constant_expression ']'
        { $$ = gghc_m_decl_array(ctx, $1, EXPR($<u>3)); }
	| direct_declarator_ANSI '[' ']'
        { $$ = gghc_m_decl_array(ctx, $1, ""); }
	| direct_declarator_ANSI '(' parameter_type_list ')'
        { $$ = gghc_m_decl_func(ctx, $1, TYPE($<u>3)); }
	| direct_declarator_ANSI '(' identifier_list ')'
        { $$ = gghc_m_decl_func(ctx, $1, ""); /* FIXME */}
	| direct_declarator_ANSI '(' ')'
        { $$ = gghc_m_decl_func(ctx, $1, ""); }
	;

pointer
	: '*'
        { $$ = gghc_pointer_type(ctx, "%s"); }
	| '*' type_qualifier_list
        { $$ = gghc_pointer_type(ctx, "%s"); }
	| '*' pointer
        { $$ = gghc_pointer_type(ctx, $2); }
	| '*' type_qualifier_list pointer
        { $$ = gghc_pointer_type(ctx, $3); }
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
        : type_name_ANSI
        | GGHC___typeof__ '(' expression ')' { $<type>$ = gghc_type(ctx, "void"); }
        ;

type_name_ANSI
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
        { $$ = gghc_pointer_type(ctx, "%s"); }
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
        { $$ = gghc_pointer_type(ctx, $2); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
        { $$ = $2; }
	| '[' ']'
        { $$ = gghc_array_type(ctx, "%s", ""); }
	| '[' constant_expression ']'
        { $$ = gghc_array_type(ctx, "%s", EXPR($<u>2)); }
	| direct_abstract_declarator '[' ']'
        { $$ = gghc_array_type(ctx, $1, ""); }
	| direct_abstract_declarator '[' constant_expression ']'
        { $$ = gghc_array_type(ctx, "%s", EXPR($<u>3)); }
	| '(' ')'
        { $$ = gghc_function_type(ctx, "%s", ""); }
	| '(' parameter_type_list ')'
        { $$ = gghc_function_type(ctx, "%s", $2); }
	| direct_abstract_declarator '(' ')'
        { $$ = gghc_function_type(ctx, $1, ""); }
	| direct_abstract_declarator '(' parameter_type_list ')'
        { $$ = gghc_function_type(ctx, $1, $3); }
        | direct_abstract_declarator_EXT
	;

direct_abstract_declarator_EXT
        : '(' '^' ')' '(' parameter_type_list ')'
        { $$ = gghc_block_type(ctx, "%s", $5); }
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
	: declaration_specifiers declarator declaration_list function_body
        { gghc_declaration(ctx, &$<u.decl_spec>1, $<u.decl>2); }
	| declaration_specifiers declarator function_body
        { gghc_declaration(ctx, &$<u.decl_spec>1, $<u.decl>2); }
	| declarator declaration_list function_body
	| declarator function_body
	;

function_body
  : { gghc_emit_control(ctx, -1); } compound_statement { gghc_emit_control(ctx, 1); }
  ;

/* EXTENSIONS */

identifier
  : IDENTIFIER
  | TYPE_NAME
  ;

/* string concat "A" "B" */
string_constant
  : STRING_LITERAL
  | string_constant STRING_LITERAL
  ;

__inline__
  : GGHC_inline
  | GGHC___inline
  | GGHC___inline__
  ;

__alignof__
  : GGHC___alignof
  | GGHC___alignof__
  ;

__asm
  : __asm_ '(' string_constant ')' ;

__asm_
  : GGHC___asm
  | GGHC___asm__
  ;

__attribute__
  : GGHC___attribute__ '(' '(' attr_list ')' ')' ;

__extension__ : GGHC___extension__ ;

KEYWORD_EXT :
  | __asm_
  | __inline__
  | __extension__
  ;

attr_list
  : attr
  | attr_list ',' attr
  ;

attr
  : attr_ident '(' attr_arg_list ')'
  | attr_ident '(' ')'
  | attr_ident
  ;

attr_arg_list
  : attr_arg
  | attr_arg_list ',' attr_arg
  ;

attr_arg
  : attr_ident
  | attr_ident '=' constant_expression
  | constant_expression
  ;

attr_ident
  : IDENTIFIER
  | CONST
  | KEYWORD_EXT
  ;

direct_declarator_EXTs
  : direct_declarator_EXT
  | direct_declarator_EXTs direct_declarator_EXT
  ;

direct_declarator_EXT
  : __attribute__
  | __asm
  ;

%%

#undef ctx
int gghc_yyparse_y(gghc_ctx ctx, mm_buf *mb)
{
  /* EXT: NATIVE TYPES */
  // gghc_typedef("__uint16_t", gghc_type("unsigned short"));
  gghc_typedef(ctx, "_Bool", gghc_type(ctx, "int"));

  return yyparse(ctx);
}
