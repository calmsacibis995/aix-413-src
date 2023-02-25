static char sccsid[] = "@(#)06  1.7  src/bos/usr/sbin/install/installp/inu_set_savdir.c, cmdinstl, bos411, 9428A410j 6/18/93 16:52:33";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_set_savdir (installp)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <installp.h>

char savdir[PATH_MAX]; /* pointer to INUSAVE env var */

/*--------------------------------------------------------------------------*
**
** NAME: inu_set_savdir()
**
** FUNCTION: First call inu_get_savdir() to have the save path built and
**           put into buf. then call inu_putenv() to put out to the environment
**
**--------------------------------------------------------------------------*/
void inu_set_savdir(
char     tree,      /* The tree that we're dealing with. */
Option_t *op)       /* Option where the info resides to determine the savdir */

{
    int rc;                 /* return code               */
    char *ptr;              /* generic character pointer */

    strcpy(savdir,"INUSAVEDIR=");
    ptr = savdir + strlen(savdir);
    (void) inu_get_savdir(tree, op, ptr);
    if ((rc = inu_putenv(savdir)) != SUCCESS)
	inu_quit(INUBADSC);

}
