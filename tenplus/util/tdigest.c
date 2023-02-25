static char sccsid[] = "@(#)58	1.7  src/tenplus/util/tdigest.c, tenplus, tenplus411, GOLD410 11/4/93 12:23:24";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, convert, get_termname
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
 
/****************************************************************************/
/* File: tdigest.c - digest trm files into terms.bin form                   */
/****************************************************************************/

#include    <string.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <locale.h>

#include    "chardefs.h"
#include    "database.h"
#include    "libobj.h"
#include    "tdigest_msg.h"
#include    "util.h"

POINTER *g_outrec;
POINTER *g_inrec;
int g_recnum;
int g_nitems;

unsigned char *d_intran = NULL; /* default intran  */
POINTER       *d_multseq;       /* default multseq */

int MultibyteCodeSet;

nl_catd g_catd;    /* Catalog descriptor for MF_TDIGEST */

#define MSGSTR(num,str)		catgets(g_catd,MS_TDIGEST, num, str)

/****************************************************************************/
/* main: loop over input file, producing output file with the input         */
/* translation digested                                                     */
/****************************************************************************/

main (int argc, char *argv[])
    {
    int i;
    SFILE *isfp, *osfp;
    int noutput;
    char *termname;

    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    g_catd = catopen(MF_TDIGEST, NL_CAT_LOCALE);

    if (argc != 3)
	{
	(void) fprintf (stderr, MSGSTR(M_USAGE_MESSAGE,"Usage: tdigest InputFile OutputFile\n"));
	(void)exit (-1);
	}

    isfp = sf_open (argv [1], "r");
    if (isfp == (SFILE *) ERROR)
	{
	(void) fprintf (stderr, MSGSTR(M_READ,"tdigest: 0611-267 Cannot read %s.\n"), argv [1]);
	(void)exit (-1);
	}

    osfp = sf_open (argv [2], "c");

    if (osfp == (SFILE *) ERROR)
	{
	(void) fprintf (stderr, MSGSTR(M_CREATE,"tdigest: 0611-268 Cannot create %s.\n"), argv [2]);
	(void)exit (-1);
	}

    /* loop over the human-readable format, digesting it        */
    noutput = 0;
    for (i = 0; i < obj_count (sf_records (isfp)); i++)
	{
	g_inrec = (POINTER *) sf_read (isfp, i);
	if (g_inrec == NULL)
	    continue;

	if (g_inrec == (POINTER *) ERROR)
	    (void)fatal (MSGSTR(M_READ_RECORD,"0611-269 Internal Error (read error on %s, record %d)."), argv [1], i);

	termname = get_termname();

	if ((d_intran == NULL) && (strcmp(termname, "DEFAULT") != 0))
	    (void)fatal(MSGSTR(M_NO_DEFAULT, "Default keyboard definition must be first record in %s."), argv[1]);

	g_outrec = (POINTER *) s_alloc (3, T_POINTER, termname);

	g_recnum = i;

	g_nitems = 0;

	/*
	** ibm3101 keyboard definition must not include default keyboard subset
	*/

	if (strcmp(termname, "ibm3101") == 0)
	    doinseqs(TRUE);  /* exclude default subset from keyboard */
	else
	    doinseqs(FALSE); /* include default subset with keyboard */


	if (sf_write (osfp, noutput, (char *)g_outrec) == ERROR)
	    (void)fatal (MSGSTR(M_WRITE_RECORD,"0611-270 Internal Error (write error on %s, record %d)"), argv [2], noutput);

	noutput++;
	(void)s_free ((char *)g_outrec);
	(void)s_free ((char *)g_inrec);
	}

    (void) sf_close (isfp);

    if (sf_close (osfp) == ERROR)
	(void)fatal (MSGSTR(M_CLOSE,"0611-271 Internal Error (write error while closing %s)."), argv [2]);

    catclose(g_catd);
    return(0);
    /*NOTREACHED*/
    }

/****************************************************************************/
/* convert: convert escapes in a single output sequence to "internal form," */
/* honor escapes '^' (means map to control shift), and '\', which means     */
/* leave alone next character. Returns NULL if no value supplied, assumes   */
/* that a trailing \ means a space at the end of the line.  For input       */
/* sequences NULLs are converted to the sequence ESC ESC 0.                 */
/****************************************************************************/

unsigned char *convert (char *value, char *name, unsigned char inseq)
    {
    unsigned char cbuf [1024];
    char *icp;
    unsigned char *ocp;
    unsigned cvalue;
    unsigned char *thisvalue;

    icp = value;
    ocp = cbuf;

    /* if value wholly empty, no output sequence        */
    if (*icp == '\0')
	return (NULL);

    /* trim trailing whitespace                         */
    while (*icp)
	icp++;

    icp--;

    while (icp >= value && (*icp == SPACE || *icp == '\t'))
	icp--;

    /* if value wholly whitespace, no output sequence   */
    if (icp < value)
	return (NULL);

    *++icp = '\0';

    icp = value;

    while (*icp)
	{
	switch (*icp)
	    {
	    case '\\':
		    icp++;
		    if (*icp)
			{
			/* handle octal char defn       */
			if (*icp == '0')
			    {
			    cvalue = 0;
			    while (*icp && (*icp >= '0' && *icp <= '7'))
				cvalue = (cvalue << 3) + (*icp++ - '0');

			    *ocp = cvalue;
			    }
			else if (*icp == 'x')
			    {
			    icp++;
			    cvalue = 0;
			    while (*icp && ((*icp >= '0' && *icp <= '9')
					||  (*icp >= 'A' && *icp <= 'F')
					||  (*icp >= 'a' && *icp <= 'f')))
			       {
				if(*icp >= '0' && *icp <= '9')
				  cvalue = (cvalue << 4) + (*icp++ - '0');
				else if (*icp >= 'A' && *icp <= 'F')
				  cvalue = (cvalue << 4) +
					   ((*icp++ - 'A') + 10);
				else
				  cvalue = (cvalue << 4) +
					   ((*icp++ - 'a') + 10);
			       }
			    *ocp = cvalue;
			    }
			else
			    *ocp = *icp++;
			}
		    else /* handle elided space at end of line  */
			*ocp = SPACE;
		    break;

	    case '^':
		    icp++;
		    if (*icp >= '`')            /* machine independent */
			*ocp = *icp++ - 0140;
		    else if (*icp >= '@')       /* machine independent */
			*ocp = *icp++ - 0100;
		    else if (*icp)
			*ocp = *icp++;
		    break;

	    default:
		*ocp = *icp++;
	    }
	if (inseq && (*ocp == '\0'))
	    {
	    *ocp++ = '\033';
	    *ocp++ = '\033';
	    *ocp   = '0';
	    }
	ocp++;
	}

    *ocp = '\0';

    thisvalue = (unsigned char *) s_string (cbuf);

    if (name)
	(void)s_newname (thisvalue, name);
    return (thisvalue);
    }


/****************************************************************************/
/* get_termname: retrieve terminal name from input record g_inrec           */
/****************************************************************************/

char *get_termname(void)
    {

    char *termname;

    termname = s_findnode(g_inrec, "Name");

    if (termname == (char *) ERROR)
	(void)fatal (MSGSTR(M_NO_TERM_NAME,"Input record %d has no defined terminal name."), g_recnum);

    if (obj_type(termname) == T_POINTER && obj_count(termname) == 1)
	termname = *((POINTER *) termname);

    (void) s_typecheck (termname, "get_termname: terminal name", T_CHAR);

    return (termname);

    }
