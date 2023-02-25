static char sccsid[] = "@(#)71	1.10  src/tcpip/usr/bin/telnet/teltoterm.c, tcptelnet, tcpip411, GOLD410 7/18/91 11:38:52";
/* 
 * COMPONENT_NAME: TCPIP teltoterm.c
 * 
 * FUNCTIONS: Mteltoterm, finishlookup, hyphenate, uppercase 
 *            initlookup, nextname, terminfolookup, terminfototelnet
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * teltoterm.c - stuff to convert between telnet (RFC 1010) and unix
 * (terminfo or termcap) naming conventions for terminals.
 * 	Mark Carson	11/25/87
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

/*
 * general form of telnet name:
 *	MANUFACTURER-MODEL-ETC
 * general form of terminfo/termcap name:
 *	manufacturermodel, or manmodel, or model
 * What we do to convert:
 *	1. check table. If it's present, use it.
 *	2. telnet to terminfo:
 *		a. lowercase the whole string.
 *		b. try dropping the hyphens	(gives manufacturermodel)
 *		c. try deleting the stuff before the first hyphen
 *					(gives model)
 *	3. terminfo to telnet
 *		a. uppercase the whole string.
 *		b. insert a hyphen before the first digit.
 * This could be made more sophisticated, or the table could be added to.
 * One useful innovation would be to add manufacturer abbreviations to the
 * table to help generate manmodel names.  (E.g. DATAMEDIA=dm, TELEVIDEO=tvi)
 *
 * Bugs: RFC 1010 neglects the area of terminals with multiple subtypes,
 * such as the Ambassador or VT100.  Nothing we can do about it.
 * Terminfo/termcap is wildly inconsistent on how terminals are named, so
 * the algorithm is probably not going to work too often.
 */

/* Table */
static char *CONFIG = "/etc/telnet.conf";

/* Table entries */
#define TERMINFOMAX	20	/* maximum no. of terminfo alternative names */
struct terminalnames {
	char *rfc1010;		/* telnet standard name */
	char *terminfo[TERMINFOMAX];	/* terminfo names */
};

/* Static functions */
static struct terminalnames *terminfolookup(char *);
static int initlookup(void);
static void finishlookup(void);
static struct terminalnames *nextname(void);
static void uppercase(register char *);
static void hyphenate(register char *);	

#ifdef TEST
#include <locale.h>
main()
{
	char input[256], *gets();
	char *telanswer, *termanswer[TERMINFOMAX];
	int i;

	setlocale(LC_ALL,"");
	while (1) {
		printf("terminal name? ");
		gets(input);
		printf("terminfo names");
		for (i=0; termanswer[i]; ++i)
			printf(" |%s|", termanswer[i]);
		printf("\n");
		printf("terminfototelnet returns %d\n",
			terminfototelnet(input, &telanswer));
		printf("telnet name |%s|\n",
			telanswer);
	}
}
#endif TEST
/*
 * terminfototelnet(terminfo, telnet)
 *
 * For a terminfo-style name, return the corresponding telnet-style name.
 * Again, this returns 1 if it's sure and 0 if it's not.  Maybe this could
 * be used to ask for help from the user?
 */

terminfototelnet(terminfo, telnet)
char *terminfo;		/* input */
char **telnet;		/* output */
{
	struct terminalnames *intable;
	static char workspace[41];

	if (!terminfo) {		/* defend against my stupid code */
		*telnet = NULL;
		return 0;
	}
	if (intable = terminfolookup(terminfo)) {
		*telnet = intable->rfc1010;
		return 1;
	}
	strcpy(workspace, terminfo);
	uppercase(workspace);
	if (!strchr(workspace, '-'))
		hyphenate(workspace);
	*telnet = workspace;
	return 0;
}

static struct terminalnames *
terminfolookup(char *terminfo)
{
	struct terminalnames *answer;
	register int i;

	if (!initlookup())
		return NULL;
	while (answer = nextname())
		for (i=0; answer->terminfo[i]; ++i)
			if (!strcmp(terminfo, answer->terminfo[i]))
				goto foundit;
foundit:
	finishlookup();
	return answer;
}

/* A basic question for these lookup functions is whether we should try to
 * cache the table here, or have indexes to it, etc. - that is, do anything
 * which requires any intelligence.  My answer is not for now. */

static FILE *table;

static int
initlookup(void)
{
	if (table)
		return 1;
	return (table = fopen(CONFIG, "r")) != NULL;
}

static void
finishlookup(void)
{
	if (table) {
		fclose(table);
		table = NULL;
	}
}

static struct terminalnames *
nextname(void)
{
	static char line[2048];
	static struct terminalnames answer;
	register char *linerun;
	register int i;

	answer.rfc1010 = answer.terminfo[0] = NULL;
	while (fgets(line, 256, table)) {
		if (isprint(line[0]) && line[0] != '#') {
			answer.rfc1010 = linerun = line;
			for (i=0; i < TERMINFOMAX; ++i) {
				while (*linerun && !isspace(*linerun))
					++linerun;
				while (isspace(*linerun))
					*linerun++ = '\0';
				if (*linerun)
					answer.terminfo[i] = linerun;
				else
					break;
			}
			answer.terminfo[i] = NULL;
			return &answer;
		}
	}
	return NULL;
}

/*
 * Miscellaneous conversion routines
 */

static void
uppercase(register char *string)	/* uppercase string in place */
{

	if (!string)
		return;
        if (MB_CUR_MAX > 1) {           /* multibyte code */
                wchar_t *wc, *tmpwc;
                int mbcount;
		size_t n;

                n = strlen(string)*sizeof(wchar_t) + 1;
                wc = (wchar_t *)malloc(n);
                /* convert multibyte string to widechar string */
                mbcount = mbstowcs(wc, string, n);
                if (mbcount >= 0) {
                        while (*wc) {
                                if (iswlower((wint_t) *wc))
                                        *wc = towupper((wint_t) *wc);
                                wc++;
                        }
                        /* convert widechar string back to multibyte */
			tmpwc = wc;
                        wcstombs(string, tmpwc, n);
                }
        }
        else {
                while (*string) {
                        if (islower(*string))
                                *string = toupper(*string);
                        string++;
                }
        }
}


/*
 * The following functions work for IBM terminal names at least ...
 */

static void
hyphenate(register char *string)	/* insert a hyphen before first digit */
{
	register char *end;

	if (!string)
		return;
	while (*string && !isdigit(*string))
		++string;
	if (!*string)
		return;
	end = string;
	while (*end)
		++end;
	while (end >= string) {
		*(end+1) = *end;
		--end;
	}
	*string = '-';
}

