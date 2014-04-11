%{ /* -*- bison-mode -*- */
/* From http://www.lysator.liu.se/c/ANSI-C-grammar-y.html. */

/*
** Changes: Copyright 1993, 1994, 2014 Kurt A. Stephens
*/
#include "parse.h"

 void gghc_debug_stop_here()
 {
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

static char *_to_expr(gghc_ctx ctx, YYSTYPE *yyvsp)
{
  if ( yyvsp->expr ) return yyvsp->expr;
  // if ( yyvsp->type ) return yyvsp->type;
  return yyvsp->expr = mm_buf_region_cstr(&(yyvsp->t));
}
#define EXPR(YYV) _to_expr(ctx, (void*)&(YYV))

static ggrt_type_t *_to_type(gghc_ctx ctx, YYSTYPE *yyvsp)
{
  if ( yyvsp->type ) return yyvsp->type;
  return yyvsp->type = ggrt_type(ctx->rt, mm_buf_region_cstr(&(yyvsp->t)));
}
#define TYPE(YYV) _to_type(ctx, (void*)&(YYV))

#define YY_USER_ACTION(yyn) token_merge(_ctx, yyn, yylen, &yyval, yyvsp);
static void token_merge(gghc_ctx ctx, int yyn, int yylen, YYSTYPE *yyvalp, YYSTYPE *yyvsp)
{
  int i;
  mm_buf_region t, *dst = &yyvalp->t, *src;
  int verbose = ctx->_yydebug;

  mm_buf_region_init(&t);
  if ( verbose >= 4 ) {
    fprintf(ctx->_stderr, "  yyn=%d yylen=%d yyvalp=%p yyvsp=%p\n", yyn, yylen, yyvalp, yyvsp);
  }
  if ( verbose >= 2 ) {
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

  if ( 0 && verbose ) {
    fprintf(stderr, "  ctx->current_declarator = %p\n", ctx->current_declarator);
    if ( ctx->current_declarator ) {
      fprintf(stderr, "    ->type = %p\n", ctx->current_declarator->type);
      if ( ctx->current_declarator->type ) {
        fprintf(stderr, "      ->type = %s\n", ctx->current_declarator->type->type);
        fprintf(stderr, "      ->name = %s\n", ctx->current_declarator->type->name);
      }
    }
  }

}

#define yylex(arg) gghc_yylex(_ctx, (arg))
#define ctx ((gghc_ctx) _ctx)
#define c_declaration (ctx)->current_declaration
#define c_declarator  (ctx)->current_declarator

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

%type <expr>
  IDENTIFIER TYPE_NAME identifier
  struct_or_union

%type <type>
  type_specifier type_specifier_CTX
  struct_or_union_specifier struct_or_union_specifier_ANSI struct_or_union_specifier_ANSI_def
  enum_specifier

%type <param>
  parameter_type_list parameter_list
  parameter_declaration

%type <decl>
  array_declarators array_declarator
  declarator
  init_declarator init_declarator_CTX
  abstract_declarator
  parameter_declaration_CTX

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
  : {
      gghc_declaration_begin(ctx);
    }
    declaration_CTX
    {
      gghc_emit_declaration(ctx, gghc_declaration_end(ctx));
    } 
    ;

declaration_CTX
        : __attribute__ declaration_ANSI
        |               declaration_ANSI
        ;

declaration_ANSI
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
 	;

declaration_specifiers
	: storage_class_specifier
	| storage_class_specifier declaration_specifiers
	| type_specifier
	| type_specifier declaration_specifiers
	| type_qualifier
	| type_qualifier declaration_specifiers
	;

init_declarator_list
        : init_declarator
        | init_declarator_list ',' init_declarator
        ;

init_declarator
  : init_declarator_CTX
      { gghc_add_declarator(ctx, $1); }
  ;

init_declarator_CTX
	: declarator
	| declarator '=' initializer
            { $$ = $1; }
	;

storage_class_specifier
  : storage_class_specifier_EXT
  ;

storage_class_specifier_EXT
  : storage_class_specifier_ANSI
  | storage_class_specifier_OTHER
  | storage_class_specifier_EXT storage_class_specifier_ANSI
  | storage_class_specifier_EXT storage_class_specifier_OTHER
  ;

storage_class_specifier_OTHER
  : __inline__
  | __attribute__
  | __extension__
  ;

storage_class_specifier_ANSI
  : storage_class_specifier_TOKEN { c_declaration->storage = $<token>1; } ;

storage_class_specifier_TOKEN
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
  : type_specifier_CTX
    {
      c_declaration->type = $1;
      if ( c_declarator ) c_declarator->type = $1;
      // fprintf(stderr, "  declaration %p, declarator %p type => %p\n", c_declaration, c_declarator, $1);
    }
  ;

type_specifier_CTX
	: VOID
        { $$ = ggrt_type(ctx->rt, "void"); }
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
        { $$ = ggrt_type(ctx->rt, "char"); }
        | uchar_specifer
        { $$ = ggrt_type(ctx->rt, "unsigned char"); }
	| sshort_specifer
        { $$ = ggrt_type(ctx->rt, "short"); }
	| ushort_specifer
        { $$ = ggrt_type(ctx->rt, "unsigned short"); }
	| int_specifier
        { $$ = ggrt_type(ctx->rt, "int"); }
	| uint_specifier
        { $$ = ggrt_type(ctx->rt, "unsigned int"); }
	| slong_specifier
        { $$ = ggrt_type(ctx->rt, "long"); }
	| ulong_specifier
        { $$ = ggrt_type(ctx->rt, "unsigned long"); }
	| slong_long_specifier
        { $$ = ggrt_type(ctx->rt, "long long"); }
	| ulong_long_specifier
        { $$ = ggrt_type(ctx->rt, "unsigned long long"); }
	| FLOAT
        { $$ = ggrt_type(ctx->rt, "float"); }
	| DOUBLE
        { $$ = ggrt_type(ctx->rt, "double"); }
        | ldouble_specifier
        { $$ = ggrt_type(ctx->rt, "long double"); }
        | GGHC___builtin_va_list
        { $$ = ggrt_type(ctx->rt, "__builtin_va_list"); }

	| struct_or_union_specifier
        { $$ = $1; }
	| enum_specifier
        { $$ = $1; }
	| TYPE_NAME
        { $$ = ggrt_type(ctx->rt, EXPR($<u>1)); }
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
  : struct_or_union_specifier_ANSI
  ;

struct_or_union_specifier_ANSI
        : struct_or_union_specifier_ANSI_def
	| struct_or_union identifier
        { $$ = ggrt_struct_forward(ctx->rt, $1, EXPR($<u>2)); }
        ;

struct_or_union_specifier_ANSI_def
	: struct_or_union identifier
            { ggrt_struct(ctx->rt, $1, EXPR($<u>2)); }
          '{' struct_declaration_list '}'
            { $$ = ggrt_struct_end(ctx->rt, 0); }
	| struct_or_union
            { ggrt_struct(ctx->rt, $1, ""); }
          '{' struct_declaration_list '}'
            { $$ = ggrt_struct_end(ctx->rt, 0); }
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
  : __extension__ struct_or_union_specifier_ANSI_def ';' /* inline union */
  | struct_declaration_ANSI_CTX
  ;

struct_declaration_ANSI_CTX
  :   { gghc_declaration_begin(ctx); }
    struct_declaration_ANSI
      { gghc_declaration_end(ctx); }
  ;

struct_declaration_ANSI
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
           { c_declaration->type = $1; }
	| type_specifier
           { c_declaration->type = $1; }
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
  :   { gghc_declarator_begin(ctx); }
    struct_declarator_CTX
    {
      gghc_struct_elem_decl(ctx, gghc_declarator_end(ctx));
    }
  ;

struct_declarator_CTX
	: declarator_CTX
	| ':' constant_expression
          { c_declarator->bit_field_size = EXPR($<u>2); }
	| declarator_CTX ':' constant_expression
          { c_declarator->bit_field_size = EXPR($<u>3); }
	;

enum_specifier
        : ENUM
            { ggrt_enum(ctx->rt, 0, 0, 0, 0); }
          '{' enumerator_list '}'
            { $$ = ggrt_enum_end(ctx->rt, 0, 0); }
	| ENUM identifier
            { ggrt_enum(ctx->rt, EXPR($<u>2), 0, 0, 0); }
          '{' enumerator_list '}'
            { $$ = ggrt_enum_end(ctx->rt, 0, 0); }
	| ENUM identifier
            { $$ = ggrt_enum_forward(ctx->rt, EXPR($<u>2)); }
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
          { ggrt_enum_elem(ctx->rt, 0, EXPR($<u>1), 0); }
	| IDENTIFIER '=' constant_expression
          { ggrt_enum_elem(ctx->rt, 0, EXPR($<u>1), 0); }
	;

type_qualifier
        : CONST
        | GGHC___const
        | VOLATILE
        | GGHC___restrict
        | __extension__
        ;

declarator 
  :   { gghc_declarator_begin(ctx); }
    declarator_CTX
      { $$ = gghc_declarator_end(ctx); }
  ;

declarator_CTX
	: pointer direct_declarator
        | direct_declarator
	;

direct_declarator
  : direct_declarator_ANSI
  | direct_declarator_ANSI direct_declarator_EXTs
  ;

direct_declarator_ANSI
	: IDENTIFIER
            { c_declarator->identifier = EXPR($<u>1); }
	| '(' declarator_CTX ')'
            { c_declarator->is_parenthised ++; }
	| direct_declarator_ANSI array_declarators
            { gghc_array_decl(ctx, $2); }
	| direct_declarator_ANSI '(' parameter_type_list ')'
            { gghc_function_decl(ctx, $3); }
	| direct_declarator_ANSI '(' identifier_list ')'
            { gghc_function_decl(ctx, 0); /* FIXME */}
	| direct_declarator_ANSI '(' ')'
            { gghc_function_decl(ctx, 0); }
	;

array_declarators
  :
    array_declarator
  | array_declarators array_declarator
      { $$ = $2; $2->prev = $1; }
  ;

array_declarator
  : '[' constant_expression ']'
      { $$ = gghc_m_array_decl(ctx, EXPR($<u>2)); }
  | '[' ']'
      { $$ = gghc_m_array_decl(ctx, ""); }
  ;

pointer
  : pointer_CTX
      { c_declarator->type = ggrt_pointer(ctx->rt, c_declarator->type); }
  ;

pointer_CTX
	: '*'
	| '*' type_qualifier_list
	| '*' pointer
	| '*' type_qualifier_list pointer
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;

parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
            { $$ = ggrt_parameter(ctx->rt, ggrt_varargs(ctx->rt), "..."); $$->prev = $1; }
	;

parameter_list
	: parameter_declaration
            { $$ = $1; $$->prev = 0; }
	| parameter_list ',' parameter_declaration
            { $$ = $3; $$->prev = $1; }
	;

parameter_declaration
:   { gghc_declaration_begin(ctx); gghc_declarator_begin(ctx); }
    parameter_declaration_CTX
      {
        $$ = gghc_parameter_decl(ctx, c_declarator);
        gghc_declarator_end(ctx);
        gghc_declaration_end(ctx);
      }
  ;

parameter_declaration_CTX
        : declaration_specifiers declarator_CTX
        | declaration_specifiers abstract_declarator_CTX
        | declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
        : type_name_ANSI
        | GGHC___typeof__ '(' expression ')' { $<type>$ = ggrt_type(ctx->rt, "void"); }
        ;

type_name_ANSI
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
  :   { gghc_declarator_begin(ctx); }
    abstract_declarator_CTX
      { $$ = gghc_declarator_end(ctx); }
  ;

abstract_declarator_CTX
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator_CTX ')'
            { c_declarator->is_parenthised ++; }
	| array_declarators
            { gghc_array_decl(ctx, $1); }
	| direct_abstract_declarator array_declarators
            { gghc_array_decl(ctx, $2); }
	| '(' ')'
            { gghc_function_decl(ctx, 0); }
	| '(' parameter_type_list ')'
            { gghc_function_decl(ctx, $2); }
	| direct_abstract_declarator '(' ')'
            { gghc_function_decl(ctx, 0); }
	| direct_abstract_declarator '(' parameter_type_list ')'
            { gghc_function_decl(ctx, $3); }
        | direct_abstract_declarator_EXT
	;

direct_abstract_declarator_EXT
        : '(' '^' ')' '(' parameter_type_list ')'
            { gghc_block_decl(ctx, $5); }
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
  : { gghc_declaration_begin(ctx); }
  function_definition_CTX
    { gghc_emit_declaration(ctx, gghc_declaration_end(ctx)); }
  ;

function_definition_CTX
	: declaration_specifiers declarator declaration_list function_body
	| declaration_specifiers declarator function_body
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
  return yyparse(ctx);
}

