/* @(#)69	1.6  src/tcpip/usr/bin/telnet/scan.l, tcptelnet, tcpip411, GOLD410 11/4/93 08:34:30 */
%{
/* 
 * COMPONENT_NAME: TCPIP scan.l
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  27  38 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *	Copyright 1985 by Paul Milazzo - all rights reserved
 */

/*
 *  lexical scanner for the IBM 3278 emulator keyboard profile file
 */

#include	<stdio.h>
#include	"tokentype.h"

#include <nl_types.h>
#include "telnet_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TELNET,n,s)

#define		yywrap()	1
#undef		input
#undef		unput

extern int	input ();
extern int	unput ();
%}
%%
bind		return T_BIND;
else		return T_ELSE;
exit		return T_EXIT;
if		return T_IF;
load		return T_LOAD;
or		return T_OR;
print		return T_PRINT;
quit		return T_QUIT;

character	return T_CHARACTER;
insertmode	return T_INSERTMODE;
delete		return T_DELETE;
backspace	return T_BACKSPACE;
tab		return T_TAB;
backtab		return T_BACKTAB;
return		return T_RETURN;
up		return T_UP;
down		return T_DOWN;
left		return T_LEFT;
right		return T_RIGHT;
home		return T_HOME;
eraseinput	return T_ERASEINPUT;
eraseeof	return T_ERASEEOF;
dup		return T_DUP;
fieldmark	return T_FIELDMARK;
reset		return T_RESET;
wordnext	return T_WORDNEXT;
wordprev	return T_WORDPREV;
attention	return T_ATTENTION; 
autopush        return T_AUTOPUSH;      /* BIDI support */
scrrev          return T_SCRREV;        /* BIDI support */
eng_lang        return T_ENG_LANG;      /* BIDI support */
nls_lang        return T_NLS_LANG;      /* BIDI support */
push            return T_PUSH;          /* BIDI support */
endpush         return T_ENDPUSH;       /* BIDI support */
fldrev          return T_FLDREV;        /* BIDI support */
autorev         return T_AUTOREV;       /* BIDI support */
autoshape      return T_AUTOSHAPE;     /* ARABIC support */
isolated       return T_ISOLATED;      /* ARABIC support */
initial        return T_INITIAL;       /* ARABIC support */
middle         return T_MIDDLE;        /* ARABIC support */
final          return T_FINAL;         /* ARABIC support */
fieldshape     return T_FIELDSHAPE;    /* ARABIC support */
fieldbase      return T_FIELDBASE;     /* ARABIC support */


clear		return T_CLEAR;
enter		return T_ENTER;
pa1		return T_PA1;
pa2		return T_PA2;
pa3		return T_PA3;
penselect	return T_PENSELECT;
pf1		return T_PF1;
pf2		return T_PF2;
pf3		return T_PF3;
pf4		return T_PF4;
pf5		return T_PF5;
pf6		return T_PF6;
pf7		return T_PF7;
pf8		return T_PF8;
pf9		return T_PF9;
pf10		return T_PF10;
pf11		return T_PF11;
pf12		return T_PF12;
pf13		return T_PF13;
pf14		return T_PF14;
pf15		return T_PF15;
pf16		return T_PF16;
pf17		return T_PF17;
pf18		return T_PF18;
pf19		return T_PF19;
pf20		return T_PF20;
pf21		return T_PF21;
pf22		return T_PF22;
pf23		return T_PF23;
pf24		return T_PF24;
sysreq		return T_SYSREQ;

low_prot	return T_LOWPROT;
low_unprot	return T_LOWUNPROT;
high_prot	return T_HIGHPROT;
high_unprot	return T_HIGHUNPROT;
background	return T_BACKGROUND;

illegal		return T_ILLEGAL;

\"([^"\n]|\\\"|\\\n)+\"	return T_STRING;
#.*$		;
[^ \n\t#"]+	return T_IDENT;
[ \n\t]		;
.		{
		    fprintf (stderr,
                             MSGSTR(ILL_CHAR, "Illegal character '%c' (%02x) ignored.\n"), /*MSG*/
			     yytext[0], (unsigned)(yytext[0]));
		}
