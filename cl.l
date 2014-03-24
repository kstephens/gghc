%{ /* -*- flex-mode -*- */
/* From http://www.lysator.liu.se/c/ANSI-C-grammar-y.html. */

/*
** Changes: Copyright 1993, 1994, 2014 Kurt A. Stephens
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include <limits.h>
#include "gghc.h"
#include "gghc_t.h"
#include "gghc_sym.h"
#include "gghc_o.h"
#include "cy.h"

 void	yywarning(const char* s);
 void	yyerror(const char* s);
static int yywrap();
static int comment();
static int check_type();

#undef fgetc
#define fgetc(IN) mm_buf_getc(gghc_mb)
#undef ugetc
#define ungetc(IN,C) mm_buf_ungetc(gghc_mb, (C))

 extern int yydebug;
int yyparse();
int gghc_yyparse(mm_buf *mb)
{
    int result;
    mm_buf mb_token;

    gghc_mb = mb;
    gghc_mb_token = &mb_token;
    *gghc_mb_token = *gghc_mb;

    result = gghc_yyparse_y(mb);
    gghc_mb = gghc_mb_token = 0;

    return result;
}

static int _input(char *buf, size_t max_size)
{
  int c;
  if ( (c = mm_buf_getc(gghc_mb)) == '\n' )
    gghc_parse_lineno ++;
  return (c == EOF) ? YY_NULL : (buf[0] = c, 1);
}
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)                                   \
  do {                                                                  \
    result = _input(buf, max_size);                                     \
  } while(0)

 static void _prepare_token(char *yytext, int yyleng)
 {
     static mm_buf_region token;

     if ( yydebug ) fprintf(stderr, "  gghc: lex: matched '%s'\n", yytext);

     mm_buf_region_begin(&token, gghc_mb_token);
     mm_buf_read(gghc_mb_token, 0, yyleng);
     mm_buf_region_end(&token, gghc_mb_token, yyleng);
     gghc_last_token = &token;

     yylval.t = token;
     yylval.leng = yyleng;

     if ( 0 ) {
         char buf[64];

         fprintf(stderr, "  gghc: lex:   len %d '%s'\n", (int) yyleng, yytext);

         memset(buf, 0, sizeof(buf));
         strncpy(buf, (char*) token.beg.pos, 60);
         buf[token.beg.size] = 0;
         if ( buf[60] ) strcat(buf, "...");
         fprintf(stderr, "  gghc: token: len %d '%s'\n\n", (int) token.beg.size, buf);
     }
 }

#define YY_USER_ACTION _prepare_token(yytext, yyleng);

#define _return(X)               \
  do {                           \
    return (yylval.token = (X)); \
  } while(0)

static
const char*	_eatwhitespace(const char* s)
{
  while ( isspace(*s) ) s ++;
  return s;
}

static
unsigned long	_stod(const char** _s)
{
  unsigned long	i = 0;
  const char *s = *_s;
  while ( isdigit(*s) )
    i = (i * 10) + *(s ++) - '0';
  *_s = s;
  return i;
}

%}

%option yylineno

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

LINE	(line)
PRAGMA	(pragma)

%%

\#[ \t]*{PRAGMA}.*\n {
  yywarning(yytext);
}

\#[ \t]*{LINE}?[ \t]*[0-9]+([ \t]*\".*\")?.*\n {
  char*	s;
  char*	e;
  char* filename;

  /* skip over #[ \t]* */
  s = (char*) yytext + 1; s = (char*) _eatwhitespace(s); /* skip over '#[ \t]+' */
  
  /* skip over 'line' */
  if ( *s == 'l' )
    s += 4;
  
  gghc_parse_lineno = _stod((const char**) &s);

  if ( (s = strchr(s, '"')) ) {
    s ++;
    e = strrchr(s, '"');

    filename = malloc(e - s + 1);
    memcpy(filename, s, e - s);
    filename[e - s] = '\0';

    free(gghc_parse_filename);
    gghc_parse_filename = filename;
    if ( gghc_parse_top_level_filename == 0 ) {
      gghc_parse_top_level_filename =  strdup(gghc_parse_filename);
    }
  }

  gghc_mb->s.src.filename = strdup(gghc_parse_filename);
  gghc_mb->s.src.lineno = gghc_parse_lineno;
  gghc_mb->s.src.column = 0;

  gghc_mb_token->s.src = gghc_mb->s.src;
}

"/*"			{ comment(); }

"inline"                { _return(GGHC_inline); }
"__builtin_va_list"     { _return(GGHC___builtin_va_list); }
"__attribute__"         { _return(GGHC___attribute__); }
"__asm"                 { _return(GGHC___asm); }

"auto"			{ _return(AUTO); }
"break"			{ _return(BREAK); }
"case"			{ _return(CASE); }
"char"			{ _return(CHAR); }
"const"			{ _return(CONST); }
"continue"		{ _return(CONTINUE); }
"default"		{ _return(DEFAULT); }
"do"			{ _return(DO); }
"double"		{ _return(DOUBLE); }
"else"			{ _return(ELSE); }
"enum"			{ _return(ENUM); }
"extern"		{ _return(EXTERN); }
"float"			{ _return(FLOAT); }
"for"			{ _return(FOR); }
"goto"			{ _return(GOTO); }
"if"			{ _return(IF); }
"int"			{ _return(INT); }
"long"			{ _return(LONG); }
"register"		{ _return(REGISTER); }
"return"		{ _return(RETURN); }
"short"			{ _return(SHORT); }
"signed"		{ _return(SIGNED); }
"sizeof"		{ _return(SIZEOF); }
"static"		{ _return(STATIC); }
"struct"		{ _return(STRUCT); }
"switch"		{ _return(SWITCH); }
"typedef"		{ _return(TYPEDEF); }
"union"			{ _return(UNION); }
"unsigned"		{ _return(UNSIGNED); }
"void"			{ _return(VOID); }
"volatile"		{ _return(VOLATILE); }
"while"			{ _return(WHILE); }

{L}({L}|{D})*		{ _return(check_type()); }

0[xX]{H}+{IS}?		{ _return(CONSTANT); }
0{D}+{IS}?		{ _return(CONSTANT); }
{D}+{IS}?		{ _return(CONSTANT); }
L?'(\\.|[^\\'])+'	{ _return(CONSTANT); }

{D}+{E}{FS}?		{ _return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ _return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ _return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ _return(STRING_LITERAL); }

"..."			{ _return(ELLIPSIS); }
">>="			{ _return(RIGHT_ASSIGN); }
"<<="			{ _return(LEFT_ASSIGN); }
"+="			{ _return(ADD_ASSIGN); }
"-="			{ _return(SUB_ASSIGN); }
"*="			{ _return(MUL_ASSIGN); }
"/="			{ _return(DIV_ASSIGN); }
"%="			{ _return(MOD_ASSIGN); }
"&="			{ _return(AND_ASSIGN); }
"^="			{ _return(XOR_ASSIGN); }
"|="			{ _return(OR_ASSIGN); }
">>"			{ _return(RIGHT_OP); }
"<<"			{ _return(LEFT_OP); }
"++"			{ _return(INC_OP); }
"--"			{ _return(DEC_OP); }
"->"			{ _return(PTR_OP); }
"&&"			{ _return(AND_OP); }
"||"			{ _return(OR_OP); }
"<="			{ _return(LE_OP); }
">="			{ _return(GE_OP); }
"=="			{ _return(EQ_OP); }
"!="			{ _return(NE_OP); }
";"			{ _return(';'); }
("{"|"<%")		{ _return('{'); }
("}"|"%>")		{ _return('}'); }
","			{ _return(','); }
":"			{ _return(':'); }
"="			{ _return('='); }
"("			{ _return('('); }
")"			{ _return(')'); }
("["|"<:")		{ _return('['); }
("]"|":>")		{ _return(']'); }
"."			{ _return('.'); }
"&"			{ _return('&'); }
"!"			{ _return('!'); }
"~"			{ _return('~'); }
"-"			{ _return('-'); }
"+"			{ _return('+'); }
"*"			{ _return('*'); }
"/"			{ _return('/'); }
"%"			{ _return('%'); }
"<"			{ _return('<'); }
">"			{ _return('>'); }
"^"			{ _return('^'); }
"|"			{ _return('|'); }
"?"			{ _return('?'); }

[ \t\v\n\f]		{ }
.			{ /* ignore bad characters */ }

%%
// ' emacs

static int yywrap()
{
	return(1);
}


static int comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
		putchar(c);

	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}

	if (c != 0)
		putchar(c1);
return 0;
}

static int check_type()
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return(TYPE_NAME);
*
*	return(IDENTIFIER);
*/

  gghc_symbol *sym;
  yylval.u.cp = strdup(yytext);
  if ( (sym = gghc_symbol_get(yytext)) && strcmp(sym->value, "typedef") == 0 ) {
    return TYPE_NAME;
  }

/*
*	it actually will only return IDENTIFIER
*/

	return(IDENTIFIER);
}
