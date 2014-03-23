%{
/*
** Copyright 1993, 1994 Kurt A. Stephens
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include <limits.h>
#include "gghc_sym.h"
#include "gghc_o.h"
#include "cy.h"

extern	FILE*	kshc_input;

void	yywarning(const char* s);

char *kshc_parse_filename = 0;
char *kshc_parse_top_level_filename = 0;
int  kshc_parse_lineno = 1;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) {\
  int	c; \
  if ( (c = fgetc(kshc_input)) == '\n' ) kshc_parse_lineno ++; \
  result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
}

static
const char*	_eatwhitespace(const char* s)
{
  while ( isspace(*s) ) s ++;
  return s;
}

static	
unsigned long	_stoo(const char** _s)
{
  unsigned long	i = 0;
  const char*	s = *_s;
  while ( *s && *s >= '0' && *s <= '7' )
    i = (i << 3) + *(s ++) - '0';
  *_s = s;
  return i;
}

static
unsigned long	_stoh(const char** _s)
{
  unsigned long	i = 0;
  int	t;
  const char *s = *_s;

  while ( ((t = isdigit(*s)) ? 1 : 0)
  || ((t = (*s >= 'a' && *s <= 'f')) ? 2 : 0)
  || ((t = (*s >= 'A' && *s <= 'F')) ? 3 : 0) ) {
    i = (i << 4) +
      t == 1 ? *(s ++) - '0' :
      t == 2 ? *(s ++) - 'a' + 10 :
		*(s ++) - 'A' + 10;
  }
  *_s = s;
  return i;
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

static
int	escape(int L, const char* s, int i, char* o)
{
  while ( i -- > 0 ) {
    int	c;

    if ( *s == '\\' ) {
      static	char	e[] = "n\nt\tv\vb\br\rf\fa\a\\\\??''\"\"";
      char* i;
      wchar_t v;

      s ++;
      if ( *s == '0' ) {
	s ++;
	v = _stoo(&s);
      } else
      if ( *s == 'x' ) {
	s ++;
	v = _stoh(&s);
      } else
      if ( (i = strchr(e, *s)) ) {
	v = i[1];
	s ++;
      } else {
	return 0;
      }

      c = v;
    } else {
      c = *(s ++);
    }

    if ( L ) {
      *((wchar_t*)o) = c;
      o += sizeof(wchar_t);
    } else {
      *(o ++) = c;
    }
  }
  
  return 1;
}

static
int	flushtype(YYSTYPE* v, const char* str, int len)
{
/*
  int	o =
	  str[0] == '0';
  int	l =
	  str[len - 1] == 'L' || str[len - 1] == 'l' ||
	  str[len - 2] == 'L' || str[len - 1] == 'l';
  int	u =
	  str[len - 1] == 'U' || str[len - 1] == 'u' ||
	  str[len - 2] == 'U' || str[len - 1] == 'u';
  int	s = l || u;
*/
  int	t;

  /* IMPLEMENT: rules for [lLuU] suffixes */
  if ( v->u.ul <= (unsigned long) INT_MAX ) {
    v->u.i = v->u.ul;
    t = INT_CONSTANT;
  } else
  if ( v->u.ul <= (unsigned long) UINT_MAX ) {
    v->u.l = v->u.ul;
    t = UNSIGNED_INT_CONSTANT;
  } else
  if ( v->u.ul <= (unsigned long) LONG_MAX ) {
    v->u.l = v->u.ul;
    t = LONG_INT_CONSTANT;
  }/* else
  if ( v->u.ul <= (unsigned long) ULONG_MAX ) */ {
    t = UNSIGNED_LONG_INT_CONSTANT;
  }

/*
  done:
*/
  return t;
}

typedef	struct	Symbol Symbol;
struct Symbol {
	char*	name;
	short	t;
	YYSTYPE	u;
	Symbol*	next;
};

#define	emalloc(x) malloc(x)

static	Symbol *symlist = 0;
static
Symbol *lookup(char* s)
{
  Symbol *sp;

  for ( sp = symlist; sp != 0; sp = sp->next )
    if ( ! strcmp(sp->name, s) ) return sp;
  return 0;
}

static
Symbol *install(char* s)
{
  Symbol *sp;

  sp = (Symbol*) emalloc(sizeof(Symbol));
  memset(sp, 0, sizeof(Symbol));

  sp->name = strcpy(emalloc(strlen(s) + 1), s);
  sp->next = symlist;
  symlist = sp;
  return sp;
}

#define	TEXT() 	{ extern int yydebug; yylval.text = strdup(yytext); if ( yydebug ) fprintf(stderr, "let: matched '%s'\n", yylval.text); }
%}

DIGIT	([0-9])
EXP	([Ee][+-]?[0-9]+)
LINE	(line)
PRAGMA	(pragma)
%%

"/*"	{
  int	c;

  do {
    while ( (c = input()) != '*' && c != EOF ) ;
    if ( c == '*' ) {
      if ( (c = input()) == '/' ) break;
      else unput(c);
    }
    if ( c == EOF ) {
      fprintf(stderr, "EOF in comment\n");
      break;
    }
  } while ( 1 );
}

\#[ \t]*{PRAGMA}.*\n {
  yywarning(yytext);
}

\#[ \t]*{LINE}?[ \t]*[0-9]+([ \t]*\".*\")?.*\n {
  char*	s;
  char*	e;

  /* skip over #[ \t]* */
  s = (char*) yytext + 1; s = (char*) _eatwhitespace(s); /* skip over '#[ \t]+' */
  
  /* skip over 'line' */
  if ( *s == 'l' )
    s += 4;
  
  kshc_parse_lineno = _stod((const char**) &s);

  if ( (s = strchr(s, '"')) ) {
    s ++;
    e = strrchr(s, '"');

    free(kshc_parse_filename);
    kshc_parse_filename = malloc(e - s + 1);
    memcpy(kshc_parse_filename, s, e - s);
    kshc_parse_filename[e - s] = '\0';
    if ( kshc_parse_top_level_filename == 0 ) {
      kshc_parse_top_level_filename =  strdup(kshc_parse_filename);
    }
  }
}

[ \t\n]+ {
}

(({DIGIT}+"."{DIGIT}*{EXP}?)|([+-]?{DIGIT}*"."{DIGIT}+{EXP}?)|({DIGIT}+{EXP}))[fFlL]? {                   /* a floating constant */
#define return(X) { TEXT(); return(X); }
  int	t;

  t = DOUBLE_CONSTANT;
  yylval.u.d = (double) atof(yytext);
  if ( yyleng >= 1 ) {
  if ( yytext[yyleng - 1] == 'f' || yytext[yyleng - 1] == 'F' ) {
    t = FLOAT_CONSTANT;
    yylval.u.f = (float) yylval.u.d;
  } else
  if ( yytext[yyleng - 1] == 'l' || yytext[yyleng - 1] == 'L' ) {
    t = LONG_DOUBLE_CONSTANT;
    yylval.u.ld = (long double) yylval.u.d;
  }
  }

  return(t);
}

0[0-7]*[uUlL]? {
  const char *s = yytext + 1;
  yylval.u.ul = _stoo(&s);
  return(flushtype(&yylval, yytext, yyleng));
}

0x[0-9a-fA-F]+[uUlL]? {
  const char *s = yytext + 2;
  yylval.u.ul = _stoh(&s);
  return(flushtype(&yylval, yytext, yyleng));
}

[1-9][0-9]*[uUlL]?+ {
  const char *s = yytext;
  yylval.u.ul = _stod(&s);
  return(flushtype(&yylval, yytext, yyleng));
}


"*="	{ return(MULA); }
"/="	{ return(DIVA); }
"%="	{ return(MODA); }
"+="	{ return(ADDA); }
"-="	{ return(SUBA); }
"<<="	{ return(LSA); }
">>="	{ return(RSA); }
"&="	{ return(AA); }
"^="	{ return(XA); }
"|="	{ return(OA); }
"||"	{ return(LO); }
"&&"	{ return(LA); }
"=="	{ return(EQ); }
"!="	{ return(NE); }
"<="	{ return(LE); }
">="	{ return(GE); }
"<<"	{ return(LS); }
">>"	{ return(RS); }
"++"	{ return(INC); }
"--"	{ return(DEC); }
"->"	{ return(AR); }
"..."	{ return(DDD); }


"auto"		{ return(AUTO); }
"break"		{ return(BREAK); }
"case"		{ return(CASE); }
"char"		{ return(CHAR); }
"const"		{ return(CONST); }
"continue"	{ return(CONTINUE); }
"default"	{ return(DEFAULT); }
"do"		{ return(DO); }
"double"	{ return(DOUBLE); }
"else"		{ return(ELSE); }
"enum"		{ return(ENUM); }
"extern"	{ return(EXTERN); }
"float"		{ return(FLOAT); }
"for"		{ return(FOR); }
"goto"		{ return(GOTO); }
"if"		{ return(IF); }
"int"		{ return(INT); }
"long"		{ return(LONG); }
"register"	{ return(REGISTER); }
"return"	{ return(RETURN); }
"short"		{ return(SHORT); }
"signed"	{ return(SIGNED); }
"static"	{ return(STATIC); }
"struct"	{ return(STRUCT); }
"typedef"	{ return(TYPEDEF); }
"union"		{ return(UNION); }
"unsigned"	{ return(UNSIGNED); }
"void"		{ return(VOID); }
"volatile"	{ return(VOLATILE); }
"while"		{ return(WHILE); }

[A-Za-z_][A-Za-z_0-9]* {
  Symbol *s;

  if ( ! (s = lookup(yytext)) ) {
    s = install(yytext);
    s->t = 0;
  }

  yylval.u.cp = s->name; 

  /*
  ** Is it a typedef name?
  */
  {
  kshc_symbol* s = kshc_symbol_get(yylval.u.cp);
  if ( s && strcmp(s->value, "typedef") == 0 )
    return TYPEDEF_NAME;
  }

  return (s->t ? s->t : IDENTIFIER);	
}

L?\'([^\\']|\\([ntvbrfa\\?\'\"]|0[0-7]+|x[0-9a-f]+)+)\' {
  int	L = yytext[0] == 'L';
  if ( ! (
    L ?
      escape('L', yytext + 2, yyleng - 3, (char*) &yylval.u.wc) :
      escape(0, yytext + 1, yyleng - 2, &yylval.u.c )
    ) ) {
    fprintf(stderr, "bad escape sequence in %s\n", yytext);
    return 0;
  }
  return(L ? WCHAR_CONSTANT : CHAR_CONSTANT);
}

L?\"[^\\\"]*\" {		/* a literal string */
  int	L = yytext[0] == 'L';
  char *s = L ? yytext + 2 : yytext + 1;
  int	l = L ? yyleng - 3 : yyleng - 2;

  yylval.u.cp = (char*) malloc((l + 1 ) * (L ? sizeof(wchar_t) : sizeof(char)) ); 

  memset(yylval.u.cp + l * (L ? sizeof(wchar_t) : sizeof(char)), 0, L ? sizeof(wchar_t) : sizeof(char));

  if ( ! escape(L, s, l, yylval.u.cp) ) {
    fprintf(stderr, "bad escape sequence in %s\n", yytext);
    return 0;
  }

  return(L ? WSTRING_CONSTANT : STRING_CONSTANT);
}



.	{
  return(yytext[0]);

#undef	return
}

%%

#undef yywrap
int yywrap() { return 1; }

