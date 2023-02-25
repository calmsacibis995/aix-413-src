%{

static char sccsid[] = "@(#)08	1.2  src/bos/usr/sbin/adfutil/scanner.l, cmdadf, bos411, 9428A410j 6/15/90 16:58:31";

/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (scanner) yylex()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "symbol.h"
#include "tn.h"
#include "externs.h"

/*
 * NAME: scanner (yylex)
 *                                                                    
 * FUNCTION: scan input (yyin) and seperate into TOKENS
 *                                                                    
 * NOTES: compile with "lex scanner.l" to make a lex.yy.c
 *
 * DATA STRUCTURES: dependent on tn.h made by parser
 *
 * RETURN: yylval - token value declared in tn.h
 *	
 */  

%}

%o 15000

%START O C S 

DIGIT	[0-9]
LET	[A-Za-z]
HX	[0-9A-Fa-f]
BIT	[xX01]

%%

%{
BEGIN O;
%}

<O>[\n]				cLines++;
<O>[ \t\f\r]+			/* eat white space */;

%{
/*
 *		keywords
 */
%}

<O>adapterid			return(ID);
<O>adaptername			return(NAME);
<O>address			return(ADDRESS);
<O>arb				return(ARB);
<O>begin			return(BBEGIN);
<O>choice			return(CHOICE);
<O>device			return(DEVICE);
<O>dynarballoc			return(DYNARB);
<O>end				return(END);
<O>fixedresources		return(FIXED);
<O>function			return(FUNCTION);
<O>help				return(HELP);
<O>initprog			return(INITPROG);
<O>input			return(INPUT);
<O>int				return(INT);
<O>io				return(IO);
<O>lock				return(LOCK);
<O>mem				return(MEM);
<O>microcode			return(MICROCODE);
<O>nameditem			return(ITEM);
<O>numbytes			return(BYTES);
<O>nodma			return(NODMA);
<O>pos				return(POS);
<O>prompt			return(PROMPT);
<O>rs422			return(RS422);
<O>scsi				return(SCSI);
<O>sharearb			return(SHAREDARB);
<O>sysmem			return(SYSMEM);
<O>vcheck			return(VCHECK);

%{
/*
*		operators
*/
%}

<O>"-"				return(DASH);
<O>"="				return(EQ);
<O>"["				return(LBRACK);
<O>"]"				return(RBRACK);

<O>{DIGIT}+			{
				yylval.w = atoi(yytext);
				return(DECIMAL);
				}
<O>{DIGIT}+d			{
				yylval.w = atoi(yytext);
				return(DECIMAL);
				}
<O>{HX}+[hH]			{
				yylval.ul = (unsigned long) 
				    strtol(yytext, (char **) NULL, 16);
				return(HEX);
				}

<O>{BIT}{8}b			{
				yytext[yyleng] = '\0';
				yylval.sb = MakeString(yytext);
				return(POSBYTE);
				}

<O>{LET}({LET}|{DIGIT}|_)*	{
				yytext[yyleng] = '\0';
				yylval.phe = Lookup(yytext);
				if (yylval.phe->tokid != -1)
				    return yylval.phe->tokid;
				return(IDENT);
				}

<O>\"				BEGIN S;
<O>;				BEGIN C;
<O>.				{
				if (26 != yytext[0])
				    yyerror("Unknown \\%03o ", yytext[0]);
				}

%{
/*
 *		comments
 */
%}

<C>\n				BEGIN O;
<C>.				yymore();

%{
/*
 *		strings
 */
%}
<S>\n				{
				cLines++;
	/*			--yyleng; /* remove newlines from strings */
				yymore();
				}
<S>\r				{
				--yyleng; /* remove newlines from strings */
				yymore();
				}
<S>\"				{
				yytext[--yyleng] = '\0';
				yylval.sb = MakeString(yytext);
				return(STRING);
				}
<S>.				yymore();
