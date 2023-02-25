static char sccsid[] = "@(#)03	1.2  src/bos/usr/sbin/adfutil/scan.c, cmdadf, bos411, 9428A410j 6/15/90 16:58:27";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (scan) main()
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
 *
 * NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE 
 *
 *    This code is included for debugging purposes only so that the 
 *  the developer of ADFutil code can make changes to the parser/scanner
 *  without the overhead of adfutil.
 *
 */                                                                   

#include "symbol.h"
#include "tn.h"

YYSTYPE yylval;
int yydebug;

#define fEOF 0		/* yylex returns 0 on EOF */
#include "global.h"

char yytext[2046];

extern FILE *yyin;

main(argc, argv)
int argc;
char **argv;
{
    char *sb;
    int token;

    yydebug = false;

    while (--argc > 0 && (*++argv)[0] == '-')
	for (sb = argv[0]+1; *sb != '\0'; sb++)
	    switch (*sb) {
	    case 'f':
		if (EOF == fclose(yyin)) {
		    fprintf(stderr, "scan : file error\n");
		    exit(1);
		}
		if (NULL == (yyin = fopen(*++argv, "r"))) {
		    fprintf(stderr, "scan : can't open file %s.\n", *argv);
		    exit(1);
		}
		strcpy(sbFile, *argv);
		break;
	    case 'd':
	    default:
		yydebug = true;
		break;
	    }

    tabinit();

    while (fEOF != (token = yylex())) {
	switch(token) {
	case LBRACK:
	    printf(" [ ");
	    break;
	case RBRACK:
	    printf(" ] ");
	    break;
	case EQ:
	    printf(" = ");
	    break;
	case DASH:
	    printf(" - ");
	    break;
	case ARB:
	    printf("\nARB ");
	    break;
	case ID:
	    printf("\nID ");
	    break;
	case NAME:
	    printf("\nNAME ");
	    break;
	case CHOICE:
	    printf("\nCHOICE ");
	    break;
	case FIXED:
	    printf("\nFIXED ");
	    break;
	case HELP:
	    printf("\nHELP ");
	    break;
	case INT:
	    printf("\nINT ");
	    break;
	case IO:
	    printf("\nIO ");
	    break;
	case MEM:
	    printf("\nMEM ");
	    break;
	case ITEM:
	    printf("\nITEM ");
	    break;
	case BYTES:
	    printf("\nBYTES ");
	    break;
	case POS:
	    printf("\nPOS ");
	    break;
	case PROMPT:
	    printf("\nPROMPT ");
	    break;
	case DECIMAL:
	    printf("%d ", yylval.w);
	    break;
	case HEX:
	    printf("%x ", yylval.ul);
	    break;
	case POSBYTE:
	    printf(" %s \n ", yylval.sb);
	    break;
	case STRING:
	    printf("%s \n", yylval.sb);
	    break;
	case IDENT:
	    printf("\n***** IDENT %s \n", yylval.phe->sb);
	    break;
	default:
	    printf("\n***** Unknown token value %d.\n", token);
	    break;
	}
    }

    switch (cErrors) {
    case 0:
	fprintf(stderr, "No errors detected\n");
	break;
    case 1:
	fprintf(stderr, "One error detected\n");
	break;
    default:
	fprintf(stderr, "%d errors detected\n", cErrors);
	break;
    }

    exit(cErrors);
}
