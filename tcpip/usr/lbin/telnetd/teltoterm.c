static char sccsid[] = "@(#)54        1.1  src/tcpip/usr/lbin/telnetd/teltoterm.c, tcptelnet, tcpip411, GOLD410 5/14/91 11:08:01";
/* 
 * COMPONENT_NAME: TCPIP teltoterm.c
 * 
 * FUNCTIONS: Mteltoterm, dehyphen, finishlookup, hyphenate, 
 *            initlookup, lowercase, nextname, telnetlookup, 
 *            telnettoterminfo, terminfolookup, terminfototelnet, 
 *            uppercase 
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

#if	defined(NLS) || defined(KJI)
#include	<NLctype.h>
#endif	NLS || KJI

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
static struct terminalnames *telnetlookup(char *); 
static struct terminalnames *terminfolookup(char *);
static int initlookup(void);
static void finishlookup(void);
static struct terminalnames *nextname(void);
static void uppercase(register char *);
static void lowercase(register char *);  
static void hyphenate(register char *);	
static void dehyphen(register char *);	

#ifdef TEST
#if defined(NLS) || defined(KJI)
#include <locale.h>
#endif
main()
{
	char input[256], *gets();
	char *telanswer, *termanswer[TERMINFOMAX];
	int i;

#if defined(NLS) || defined(KJI)
	setlocale(LC_ALL,"");
#endif
	while (1) {
		printf("terminal name? ");
		gets(input);
		printf("telnettoterminfo returns %d\n",
			telnettoterminfo(input, termanswer)); 
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
 * telnettoterminfo(telnet, terminfo)
 *
 * For a telnet-style name, return possible terminfo-style names.
 * This function returns 1 if it found the answer in the table, and 0 if
 * it made it up.  Telnet currently doesn't do anything with this.
 *
*/
telnettoterminfo(telnet, terminfo)
char *telnet;		/* input */
register char *terminfo[];	/* output */
{
	register struct terminalnames *intable;
	static char workspace1[41], workspace2[41];
	char *strchr(), *temp;
	register int i;

	if (!telnet) {		/* defend against my stupid code */
		terminfo[0] = NULL;
		return 0;
	}
	if (intable = telnetlookup(telnet)) {
		for (i=0; i < TERMINFOMAX; ++i)
			terminfo[i] = intable->terminfo[i];
		return 1;/* to indicate we're sure */
	}
	strcpy(workspace1, telnet);
	lowercase(workspace1);
	strcpy(workspace2, workspace1);
	dehyphen(workspace1);
	terminfo[0] = workspace1;
	if (temp = strchr(workspace2, '-'))
		terminfo[1] = temp+1;
	else
		terminfo[1] = NULL;
	terminfo[2] = NULL;
	return 0;	/* to indicate it's a guess */
} 

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

/*
 * Local table lookup functions
 *
*/
static struct terminalnames *
telnetlookup(char *telnet)
{
	struct terminalnames *answer;

	if (!initlookup())
		return NULL;
	while (answer = nextname())
		if (!strcmp(telnet, answer->rfc1010))
			break;
	finishlookup();
	return answer;
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
#if	defined(NLS) || defined(KJI)
	register NLchar	nlc;
#endif	NLS || KJI

	if (!string)
		return;
	while (*string) {
#if	defined(NLS) || defined(KJI)
		_NCdec2(*string, *(string + 1), nlc);
		if (NCislower(nlc))
			nlc = _NCtoupper(nlc);
		string += _NCe2(nlc, *string, *(string + 1));
#else	NLS || KJI
		if (islower(*string))
			*string = toupper(*string);
		++string;
#endif	NLS || KJI
	}
}


static void
lowercase(register char *string)	/* lowercase string in place */
{
#if	defined(NLS) || defined(KJI)
	register NLchar	nlc;
#endif	NLS || KJI

	if (!string)
		return;
	while (*string) {
#if	defined(NLS) || defined(KJI)
		_NCdec2(*string, *(string + 1), nlc);
		if (NCisupper(nlc))
			nlc = _NCtolower(nlc);
		string += _NCe2(nlc, *string, *(string + 1));
#else	NLS || KJI
		if (isupper(*string))
			*string = tolower(*string);
		++string;
#endif	NLS || KJI
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

 
static void
dehyphen(register char *string)	/* delete the hyphens from a string */
{
	register char *scout;

	if (!string)
		return;
	scout = string;
	while (*string++ = *scout++) {
		while (*scout == '-')
			++scout;
	}
}


