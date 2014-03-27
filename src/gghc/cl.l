%{ /* -*- flex-mode -*- */
/* From http://www.lysator.liu.se/c/ANSI-C-grammar-y.html. */

/*
** Changes: Copyright 1993, 1994, 2014 Kurt A. Stephens
*/

#include "parse.h"

static int yywrap();

#undef fgetc
#define fgetc(IN) mm_buf_getc(gghc_mb)
#undef ugetc
#define ungetc(IN,C) mm_buf_ungetc(gghc_mb, (C))

 static gghc_ctx _lex_ctx; // NOT THREAD SAFE!

#define YY_DECL int gghc_yylex_internal (yyscan_t yyscanner, gghc_ctx ctx, YYSTYPE *lvalp)
#define yylval (*ctx->_yylvalp)

static int _input(char *buf, size_t max_size)
{
  int c;
  if ( (c = mm_buf_getc(_lex_ctx->mb)) == '\n' )
    _lex_ctx->parse_lineno ++;
  return (c == EOF) ? YY_NULL : (buf[0] = c, 1);
}
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)                                   \
  do {                                                                  \
    result = _input(buf, max_size);                                     \
  } while(0)

static void _prepare_token(gghc_ctx ctx, YYSTYPE *lvalp, char *text, int leng)
{
   ctx->_yytext = text;
   ctx->_yyleng = leng;

     mm_buf_region *token = ctx->last_token;
     if ( ctx->_yydebug >= 2 ) fprintf(ctx->_stderr, "  gghc: lex: matched '%s'\n", text);

     mm_buf_region_begin(token, ctx->mb_token);
     mm_buf_read(ctx->mb_token, 0, leng);
     mm_buf_region_end(token, ctx->mb_token, leng);

     yylval.t = *token;
     yylval.leng = leng;

     if ( ctx->_yydebug >= 3 ) {
         char buf[64];

         fprintf(ctx->_stderr, "  gghc: lex:   len %d '%s'\n", (int) leng, text);

         memset(buf, 0, sizeof(buf));
         strncpy(buf, (char*) token->beg.pos, 60);
         buf[token->beg.size] = 0;
         if ( buf[60] ) strcat(buf, "...");
         fprintf(ctx->_stderr, "  gghc: token: len %d '%s'\n\n", (int) token->beg.size, buf);
     }
 }

#define YY_USER_ACTION _prepare_token(ctx, lvalp, yytext, yyleng);

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
  gghc_pragma(ctx, yytext);
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
  
  ctx->parse_lineno = _stod((const char**) &s);

  if ( (s = strchr(s, '"')) ) {
    s ++;
    e = strrchr(s, '"');

    filename = malloc(e - s + 1);
    memcpy(filename, s, e - s);
    filename[e - s] = '\0';

    free(ctx->parse_filename);
    ctx->parse_filename = filename;
    if ( ctx->parse_top_level_filename == 0 ) {
      ctx->parse_top_level_filename = strdup(ctx->parse_filename);
    }
  }

  ctx->mb->s.src.filename = strdup(ctx->parse_filename);
  ctx->mb->s.src.lineno = ctx->parse_lineno;
  ctx->mb->s.src.column = 0;

  ctx->mb_token->s.src = ctx->mb->s.src;
}

"/*"                    {
  char c, c1;

loop:
  while ((c = input(yyscanner)) != '*' && c != 0)
    putchar(c);

  if ((c1 = input(yyscanner)) != '/' && c != 0)
    {
      unput(c1);
      goto loop;
    }

  if (c != 0)
    putchar(c1);
}

"inline"                { _return(GGHC_inline); }
"__inline"              { _return(GGHC___inline); }
"__inline__"            { _return(GGHC___inline__); }
"__builtin_va_list"     { _return(GGHC___builtin_va_list); }
"__builtin_va_arg"      { _return(GGHC___builtin_va_arg); }
"__attribute__"         { _return(GGHC___attribute__); }
"__asm"                 { _return(GGHC___asm); }
"__asm__"               { _return(GGHC___asm__); }
"__restrict"            { _return(GGHC___restrict); }
"__extension__"         { _return(GGHC___extension__); }
"__typeof__"            { _return(GGHC___typeof__); }
"__alignof"             { _return(GGHC___alignof); } // GCC, CLANG
"__alignof__"           { _return(GGHC___alignof__); } // GCC
"__const"               { _return(GGHC___const); }

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

{L}({L}|{D})*		{
  int tok = IDENTIFIER;
  gghc_symbol *sym;
  yylval.u.cp = strdup(yytext);
  if ( (sym = gghc_symbol_get(yytext)) ) { // FIXME
    tok = TYPE_NAME;
  }
  _return(tok);
}

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
// "' emacs

static int yywrap()
{
	return(1);
}

int gghc_yylex(gghc_ctx ctx, YYSTYPE *lvalp)
{
  gghc_ctx save_ctx = _lex_ctx;
  int result;

  _lex_ctx = ctx;
  ctx->_yylvalp = lvalp;

  result = gghc_yylex_internal(ctx->scanner, ctx, lvalp);

  _lex_ctx = save_ctx;
  return result;
}

int gghc_yyparse(gghc_ctx ctx, mm_buf *mb)
{
    int result;
    yyscan_t scanner;
     
    gghc_yylex_init(&scanner);
    ctx->scanner = scanner;
 
    ctx->mb = mb;
    *ctx->mb_token = *ctx->mb;

    // calls yylex(scanner)
    result = gghc_yyparse_y(ctx, mb);

    gghc_yylex_destroy(scanner);

    return result;
}

