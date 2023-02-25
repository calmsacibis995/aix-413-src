/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)24	1.5  src/tenplus/lib/obj/sf_cookie.c, tenplus, tenplus411, GOLD410 7/18/91 13:45:57";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_cookie
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include    <stdio.h>
#include    "database.h"
#include    <stdarg.h>

#include <libobj.h>

/****************************************************************************/
/* sf_cookie:  writes out a cookie to a delta file                          */
/* The argument to this routine depends on the cookie type.  The cookie     */
/* argument is passed in 'arg', which is the third argument to this         */
/* routine.  The C_INSERT C_DELETE and C_INDEX cookies take one integer     */
/* argument (in arg1).  The C_COMMENT cookie take one string argument (in   */
/* arg1).  The C_START cookie takes no arguments and takes all its          */
/* information from the sfile object itself.  The C_END cookie needs a long */
/* file position.  Since arguments to this routine must be treated as       */
/* integers (for portability), a pointer to the long is passed (as arg1).   */
/* The C_ARRAY cookie take no arguments.                                    */
/****************************************************************************/

void sf_cookie (SFILE *sfp, int type, ...)
    /* sfp:                    structured file pointer */
    /* type:                   cookie type             */
    /* value:                 cookie argument         */
    {
    register FILE *fp;      /* file pointer from sfp   */
    va_list ap;

#ifdef TRACE

    static char *names [] = /* cookie names for trace */
	{
	"insert",           /* insert records    */
	"delete",           /* delete records    */
	"index",            /* set current index */
	"start",            /* start information */
	"end",              /* end information   */
	"comment",          /* user comment      */
	"array",            /* start of array     */


	};

    debug ("sf_cookie:  type = '%s'\n", names [type - 1]);

#endif


    va_start(ap, type);


    fp = sf_fp (sfp);
    sf_seek (sfp, sf_eofloc (sfp), 'w'); /* seek to sfile write location */

    (void) fputc (COOKIE, fp);  /* put out cookie */
    (void) fputc ((char)type, fp);

    switch (type)
	{
	case C_INSERT:   /* insert lines      */
	case C_DELETE:   /* delete lines      */
	case C_INDEX:    /* set current index */

	    puti (va_arg( ap, int) , fp); /* all use one numeric argument */
	    break;

	case C_COMMENT:  /* user comment      */

	    (void) fprintf (fp, "%s\n", va_arg( ap, char *));
	    break;

	case C_START:    /* start information */

	    (void) fputc (sf_uid (sfp), fp);  /* output uids */
	    (void) fputc (sf_gid (sfp), fp);  /* output gids */
	    putl (sf_time (sfp), fp); /* output time */
	    break;


	case C_END:      /* end information   */

	    putl ( *va_arg( ap, long *), fp);
	    break;


	case C_ARRAY:    /* start of array         */
	    break;

#ifdef CAREFUL

	default:
	    fatal ("sf_cookie:  illegal cookie type (%d)", type);
#endif
	}
    va_end(ap);
    sfp->sf__eofloc = ftell (fp); /* reset write location for file */
    }

