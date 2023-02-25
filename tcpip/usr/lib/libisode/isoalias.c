static char sccsid[] = "@(#)83	1.3  src/tcpip/usr/lib/libisode/isoalias.c, isodelib7, tcpip411, GOLD410 4/5/93 13:53:06";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: PHASH add_alias alias2name read_aliases read_file
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/isoalias.c
 */

/* isoalias.c - application entity info --  directory service utilities */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/isoalias.c,v 1.2 93/02/05 17:04:51 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/isoalias.c,v 1.2 93/02/05 17:04:51 snmp Exp $
 *
 *
 * $Log:	isoalias.c,v $
 * Revision 1.2  93/02/05  17:04:51  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:14:41  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/01/11  18:35:01  mrose
 * real-sync
 * 
 * Revision 7.0  89/11/23  21:22:12  mrose
 * Release 6.0
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <stdio.h>
#include <string.h> 
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/tailor.h>

/*    DATA */

static char *isoaliases = "isoaliases";


#define	PBUCKETS	128
#define	PHASH(nm) \
    (((nm)[1]) ? (((chrcnv[((nm)[0])] - chrcnv[((nm)[1])]) & 0x1f) \
		  	+ ((chrcnv[(nm)[2]]) & 0x5f)) \
	       : (chrcnv[(nm)[0]]) & 0x7f)

struct pair {
    char   *p_name;
    char   *p_value;

    struct pair *p_chain;
};

static int inited = 0;
static struct pair *Pbuckets[PBUCKETS];

/*  */

char   *alias2name (name)
char   *name;
{
    register struct pair *p;

    if (!inited)
	read_aliases ();

    for (p = Pbuckets[PHASH (name)];
	     p && lexequ (p -> p_name, name);
	     p = p -> p_chain)
	continue;

    return (p ? p -> p_value : NULL);
}

/*  */

static int  read_aliases ()
{
    register char   *hp;
    char   buffer[BUFSIZ];

    if (inited)
	return(0);
    inited = 1;

    bzero ((char *) Pbuckets, sizeof Pbuckets);

    read_file (isodefile (isoaliases, 0));

    if ((hp = getenv ("HOME")) == NULL)
	hp = ".";
    (void) sprintf (buffer, "%s/.isode_aliases", hp);
    read_file (buffer);
}

/*  */

static int  read_file (file)
char   *file;
{
    register char *cp;
    char   buffer[BUFSIZ + 1],
	  *vec[NVEC + NSLACK + 1];
    register FILE *fp;

    if ((fp = fopen (file, "r")) == NULL)
	return(0);

    while (fgets (buffer, sizeof buffer, fp)) {
	if (*buffer == '#')
	    continue;
	if (cp = index (buffer, '\n'))
	    *cp = NULL;
	if (str2vec (buffer, vec) < 2)
	    continue;

	if (add_alias (vec[0], vec[1]) == NOTOK)
	    break;
    }

    (void) fclose (fp);
}

/*  */

int	add_alias (name, value)
char   *name,
       *value;
{
    int	    i;
    register struct pair *p;

    if (!inited)
	read_aliases ();

    if ((p = (struct pair *) calloc (1, sizeof *p)) == NULL) {
	SLOG (addr_log, LLOG_EXCEPTIONS, NULLCP,
	      ("calloc of alias structure failed"));
	return NOTOK;
    }

    if ((p -> p_name = (char *)malloc ((unsigned) (strlen (name) + 1))) == NULL
	|| (p -> p_value = (char *)malloc ((unsigned) (strlen (value) + 1)))
	    == NULL) {
	SLOG (addr_log, LLOG_EXCEPTIONS, NULLCP,
	      ("malloc of alias structure failed"));
	if (p -> p_name)
	    free (p -> p_name);
	free ((char *) p);
	return NOTOK;
    }
    (void) strcpy (p -> p_name, name);
    (void) strcpy (p -> p_value, value);

    p -> p_chain = Pbuckets[i = PHASH (p -> p_name)];
    Pbuckets[i] = p;

    return OK;
}
