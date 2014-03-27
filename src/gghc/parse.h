#ifndef __gghc_PARSE_H
#define __gghc_PARSE_H

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>	/* strstr() */
#include <assert.h>
#include "gghc.h"
#include "gghc/decl.h"
#include "gghc/output.h"

#define YYERROR_VERBOSE 1
#define YYDEBUG 1
#define YYPARSE_PARAM _ctx

#include "cy.h"

#define yywarning(X) gghc_yywarning(ctx, (X))
#define yyerror(X)   gghc_yyerror(ctx, (X))

#endif
