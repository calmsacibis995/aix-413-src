static char sccsid[] = "@(#)62	1.9  src/tenplus/filters/rpl.c, tenplus, tenplus411, GOLD410 3/3/94 15:15:26";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, rpl, err, getline, putline, escape,
 *              is_octal_digit, re_error
 * 
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include        <limits.h>
#include        <stdio.h>
#include	<stdlib.h>
#include        <string.h>
#include        <unistd.h>
#include        <ctype.h>
#include        "rpl_msg.h"
#include        <locale.h>
#include        <regex.h>

#include	"fdefines.h"

#define AMOUNT (100 * MB_LEN_MAX)


/* new rpl -- uses regcomp and regexp for expression matching     */
/* works line by line only (ie, no patterns across lines)       */

char *escape(char *);
void putline(char *, FILE *, int );
char *getline(FILE *);
void err(char *);
int is_octal_digit(char);
void rpl(regex_t *, char *);

int MultibyteCodeSet;

nl_catd g_catd;    /* Catalogue descriptor MF_RPL */

#define MSGSTR(num, str)    catgets (g_catd, MS_RPL, num, str)

main(int ac, char **av)
	{
	char *from, *to;
        regex_t refrom;
	int len, status = 0;

       /* set the local environment */
       (void) setlocale(LC_ALL, "");

       if (MB_CUR_MAX == 1)
            MultibyteCodeSet = 0;
       else MultibyteCodeSet = 1;
    
	g_catd = catopen (MF_RPL, NL_CAT_LOCALE);

	if (ac != 3)
	{
		(void) fprintf (stderr, MSGSTR(M_USAGE,"Usage:  rpl Expression Substitute\n"));
		exit(EXITBAD);
	}
	from = escape(av[1]);

	to = escape(av[2]);
        if((status = regcomp(&refrom, from, REG_EXTENDED)) != 0) 
           /*compile error */
           err (MSGSTR(M_ILLPAT,"rpl: 0611-287 The specified pattern is not a valid regular expression."));
	rpl(&refrom, to);
	(void)catclose(g_catd);
	exit(EXITOK);
        return(0);
        /*NOTREACHED*/
}

void rpl(regex_t *refrom, char *to)
{
	char *linp;
	char *linbuf;
        int eflag, status, count;
        regmatch_t pmatch;
        /*define eflag and pmatch array for regexec */
 
	while((linbuf=getline(stdin)) != NULL){
                eflag = 0;
                /*set flag for newline */
                linp = linbuf;
                /* value of 1 used for nmatch so pmatch value not ignored */
		while ((status = regexec(refrom, linp, (size_t)1, &pmatch, eflag)) == 0){
			for(count = 0; count < pmatch.rm_so; count++) 
                                /* before match */
				(void) putchar(*linp++);
			putline(to, stdout, 0);           /* replacement */
			if ((pmatch.rm_eo - pmatch.rm_so) == 0)
				/* pattern matches a null string - don't match it again */
				if ( *linp == '\0')
					break;	/* end of the line */
				else
					linp++;
			else 
                        	linp+= pmatch.rm_eo - pmatch.rm_so; 
                        /* length of matched substring */
                        eflag = REG_NOTBOL;
			/* we've done one substitute so cannot be the start of line */
		}
                if (status)     /* no match or error */
                   if ((status != REG_NOMATCH) && (status != REG_EBOL) && (status != REG_EEOL))
                       err (MSGSTR(M_ILLPAT,"rpl: 0611-287 The specified pattern is not a valid regular expression."));
		(void) putline(linp, stdout, 1);
		free (linbuf);
	}
}

void err(char *msg)
{
	if (ttyname (1) != NULL)
		(void) fputs(msg, stderr);
	exit(EXITBAD);
}

/***************************************************

GETLINE

Getline reads characters from the file whose file handle is
passed to it until it hits a newline or the end of file. The
characters that have been read in are put in a null terminated 
string on the heap and a pointer to this string is returned.

Room on the heap is not allocated per character, but in chunks
of AMOUNT bytes (AMOUNT being a constant defined somewhere in
INed). When AMOUNT characters have been read and there are still
more to read, an extra chunk is allocated, etc.

****************************************************/

char *getline(FILE *iop)
{
	register char *buf;
	register int c,indx;
	size_t	size;

	indx = 0;
	size = AMOUNT;
	buf = malloc(size);
	if (buf == 0)
		exit(EXITBAD);

	while (((c = getc(iop)) != EOF) && (c != '\n')) {
		if ((size_t)indx >= size) {
			size += AMOUNT;
			buf = realloc (buf, size);
			if (buf == 0)
				exit(EXITBAD);
		}
		buf[indx++] = c;
	}

	if ((c == EOF) && (indx == 0)) {
		free(buf);
		return((char *) 0);
	}

	buf[indx++] = '\0';
	return(buf);
}

/***************************************************************

PUTLINE

Putline writes the char string in buf to the file whose descriptor 
is passed in iop
(it doesn't write the terminating null character). If the nl flag is
TRUE, it then writes a newline char.

***************************************************************/

void putline(char *buf, FILE *iop, int nl)
	{

	while (*buf)
	{
		(void) putc(*buf, iop);
		buf++;
	}
	if (nl)
		(void) putc('\n', iop);
}

/***********************************************************

ESCAPE

Escape has a char string as parameter. Every \ followed by up to
three octal digits is replaced by a character whose ascii value
is equal to the octal number after the backslash. The resulting
string is returned by the function. A \ not followed by a 
octal digit is left alone.

***********************************************************/

char *escape(char *str)
{
	register char *r;
	register char *w;
	register c;
	int i, num, len_c;

	r = w = str;

	while (1)
		switch(c = *r) {
			default:
				if (c) {
                                     if (!MultibyteCodeSet) 
                                         *w++ = *r++;
                                     else {
                                         len_c = mblen(r, MB_CUR_MAX);
					 /* copy at least 1 char */
					 if (len_c < 1) len_c = 1;
                                         (void) strncpy (w, r, len_c);
                                         w += len_c;
                                         r += len_c;
                                     }
				     break;
                                }
				else {
                                     *w = '\0';      
                                     return str;
                                }

			case '\\':
                                /* r points to '\', which is just one
                                   byte long. So increment r by one.
                                */  
                                r++;
				if (*r == '\\') {
					r++;
					*w++ = '\\';
					*w++ = '\\';
					break;
				}
				for (i=0,num=0; is_octal_digit(*r) && i++<3; ) {
					c = *r++;
					num = num * 8 + c - '0';
				}
				if (i == 0)     /* nothing there */
					*w++ = '\\';
				else    *w++ = (num ? num : 0200);
				break;
		}

}

/********************************************************

IS_OCTAL_DIGIT

Returns 0 when c is not an octal digit (0 .. 7), otherwise
it returns a non-0 number.

*********************************************************/

int is_octal_digit(char c)
{
  return((c >= '0') && (c <= '7'));
}

