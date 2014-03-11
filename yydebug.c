/*
** Copyright 1993, 1994 Kurt A. Stephens
** yydebug.c - debug routines for yacc.
** Don't need this for bison, it provides it.
*/
#include <stdio.h>
#include <ctype.h>	/* isprint() */

static	char	_y_tab_h[] = "obj/cy.h";

int	yyprintdefine(int s)
{
	static	FILE*	y_tab_h = NULL;
	char	buf[1024];
	int	found = -1;
	int	ok;

	if ( ! y_tab_h ) if ( ! (y_tab_h = fopen(_y_tab_h, "r")) ) return s;

	rewind(y_tab_h);
	while ( (ok = fscanf(y_tab_h, " # define %s %d", buf, &found)) != EOF ) {
		if ( found == s ) {
			fprintf(stderr, "%s\n", buf);
			return s;
		}
		while ( (ok = fgetc(y_tab_h)) != EOF && ok != '\n' ) ;
	}

	fprintf(stderr, isprint(s) ? "'%c'\n" : "%d\n", s);

	return s;
}

