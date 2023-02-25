static char sccsid[] = "@(#)25	1.6  src/bos/usr/sbin/audit/cfgcutil.c, cmdsaud, bos411, 9428A410j 6/15/90 20:21:54";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: cfgcutil support functions for audit commands
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
/* 
 * stanza file utility routine for audit
 */

#include	"audutil.h"
#include	<stdio.h>
#include	"IN/AFdefs.h"
#include	<sys/errno.h>

#define	MAXREC	4096
#define	MAXATR	512
extern	char	*xalloc ();
extern	errno;

/*
 * NOTES:
 *	this does NOT handle "default" stanzas
 *	this does NOT handle attributes with a "list" for a value
 */

char *
getattr (file, stanza, attr)
char	*file;
char	*stanza;
char	*attr;
{
AFILE_t	af;
ATTR_t	stz;
ATTR_t	at;
char	*p;

	if ((af = AFopen (PATH_CONFIG, MAXREC, MAXATR)) == NULL)
	{
		errno = EACCES;
		return (NULL);
	}

	/* get the specified stanza */
	if ((stz = AFgetrec (af, stanza)) == NULL)
		goto error;

	/* get the specified attribute */
	for (at = stz; at -> AT_name; at++)
	{
		if (strcmp (at -> AT_name, attr) == 0)
		{
			char	*p;

			p = xalloc (strlen (at -> AT_value) + 1);
			strcpy (p, at -> AT_value);
			AFclose (af);
			return (p);
		}
	}
error:
	AFclose (af);
	return (NULL);
}
