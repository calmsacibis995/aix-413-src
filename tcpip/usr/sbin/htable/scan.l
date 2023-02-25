/* @(#)29	1.5  src/tcpip/usr/sbin/htable/scan.l, tcpadmin, tcpip411, GOLD410 12/7/92 15:08:28 */
%{
/* 
 * COMPONENT_NAME: TCPIP scan.l
 * 
 * FUNCTIONS: yywrap 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
static char sccsid[] = "scan.l	5.4 (Berkeley) 4/19/86";
#endif not lint
*/
#include "y.tab.h"
#include "htable.h"
#include <nl_types.h>
#include "htable_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s)  catgets(catd,MS_HTABLE,n,s)
%}

BLANK	[ \t]
DIGIT	[0-9]
ALPHA	[A-Za-z]
ANUM	[0-9A-Za-z]
NAMECHR	[0-9A-Za-z./-]

%%
"NET"		{
			yylval.number = KW_NET;
			return (KEYWORD);
		}

"GATEWAY"	{
			yylval.number = KW_GATEWAY;
			return (KEYWORD);
		}

"HOST"		{
			yylval.number = KW_HOST;
			return (KEYWORD);
		}

{ALPHA}{NAMECHR}*{ANUM}	{
			yylval.namelist = newname(yytext);
			return (NAME);
		}

{ALPHA}		{
			yylval.namelist = newname(yytext);
			return (NAME);
		}

{DIGIT}+{ALPHA}{NAMECHR}* {
			fprintf(stderr, MSGSTR(NONSTANDARD,"Warning: nonstandard name \"%s\"\n"), 
				yytext);
			yylval.namelist = newname(yytext);
			return (NAME);
		}

{DIGIT}+	{
			yylval.number = atoi(yytext);
			return (NUMBER);
		}

"."		return ('.');
":"		return (':');
","		return (',');
"/"		return ('/');
";".*		;
"\n"{BLANK}+	;
{BLANK}+	;
"\n"		return (END);
.		fprintf(stderr, MSGSTR(ILLEGAL, "Illegal char: '%s'\n"), yytext);

%%

yywrap()
{
	return (1);
}
