static char sccsid[] = "@(#)16	1.5  src/tcpip/usr/bin/telnet/lookup.c, tcptelnet, tcpip411, GOLD410 11/4/93 08:32:52";
/* 
 * COMPONENT_NAME: TCPIP lookup.c
 * 
 * FUNCTIONS: get_token_type.
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* lookup table to get the corresponding id */

/*inludes BIDI and ARABIC support*/

#include <stdio.h>
#include "tokentype.h"
#define T_NOOP -2

struct table {
	char *token;
	int token_type ;
	};

#define TABSIZE   70 
struct table lookuptab[] = {
	"enter",	T_RETURN,
	"clear",	T_CLEAR,
	"nl",		T_ENTER,
	"tab",		T_TAB,
	"btab",		T_BACKTAB,
	"left",		T_LEFT,
	"right",	T_RIGHT,
	"up",		T_UP,
	"down",		T_DOWN,
	"home",		T_HOME,
	"delete",	T_DELETE,
	"eeof",		T_ERASEEOF,
	"einp",		T_ERASEINPUT,
	"insrt",	T_INSERTMODE,
	"dp",		T_DUP,
	"fm",		T_FIELDMARK,
	"attention",	T_ATTENTION,
	"pfk1",		T_PF1,
	"pfk2",		T_PF2,
	"pfk3",		T_PF3,
	"pfk4",		T_PF4,
	"pfk5",		T_PF5,
	"pfk6",		T_PF6,
	"pfk7",		T_PF7,
	"pfk8",		T_PF8,
	"pfk9",		T_PF9,
	"pfk10",	T_PF10,
	"pfk11",	T_PF11,
	"pfk12",	T_PF12,
	"pfk13",	T_PF13,
	"pfk14",	T_PF14,
	"pfk15",	T_PF15,
	"pfk16",	T_PF16,
	"pfk17",	T_PF17,
	"pfk18",	T_PF18,
	"pfk19",	T_PF19,
	"pfk20",	T_PF20,
	"pfk21",	T_PF21,
	"pfk22",	T_PF22,
	"pfk23",	T_PF23,
	"pfk24",	T_PF24,
	"pa1",		T_PA1,
	"pa2",		T_PA2,
	"pa3",		T_PA3,
	"reset",	T_RESET,
	"low_prot",	T_LOWPROT,
	"low_unprot",	T_LOWUNPROT,
	"high_prot",	T_HIGHPROT,
	"high_unprot",	T_HIGHUNPROT,
	"background",	T_BACKGROUND,

        "autopush",      T_AUTOPUSH,
        "scrrev",       T_SCRREV,
        "eng_lang",     T_ENG_LANG,
        "nls_lang",     T_NLS_LANG,
        "push",         T_PUSH,
        "endpush",      T_ENDPUSH,
        "fldrev",       T_FLDREV,

        "autoshape",    T_AUTOSHAPE,
        "isolated",     T_ISOLATED,
        "initial",      T_INITIAL,
        "middle",       T_MIDDLE,
        "final",        T_FINAL,
        "fieldshape",   T_FIELDSHAPE,
        "fieldbase",   T_FIELDBASE,

	"cursel",	T_NOOP,
	"master_reset",	T_NOOP,
	"flinp",	T_NOOP,
	"reshow",	T_NOOP,
	"escape",	T_NOOP,
	"xon",		T_NOOP,
	"xoff",		T_NOOP,
	"werase",	T_NOOP,
	"ferase",	T_NOOP,
	"synch",	T_NOOP,
	"settab",	T_NOOP,
	"clrtab",	T_NOOP,
	"deltab",	T_NOOP,
	"setmrg",	T_NOOP,
	"sethom",	T_NOOP,
	"coltab",	T_NOOP,
	"colbak",	T_NOOP,
	"indent",	T_NOOP,
	"undent",	T_NOOP,
	  ""              -1,
};

int get_token_type(tok_name)
char *tok_name;
{
	int index;
	for(index=0; index < TABSIZE; index++){
		if(strcmp(tok_name,lookuptab[index].token) == 0)
			return(lookuptab[index].token_type);
	}
	return(-1);
}
