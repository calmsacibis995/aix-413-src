/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)89	1.6  src/tenplus/lib/obj/s_alloc.c, tenplus, tenplus411, GOLD410 7/30/91 14:13:30";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_alloc
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
 
/****************************************************************************/
/* s_alloc: Takes count, type, and name and returns a pointer to the        */
/* first data item of the allocated object. Returns ERROR if no more heap   */
/* space is available.  The name is copied (if not NULL) and later freed    */
/* by the s_free routine                                                    */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include <stdlib.h>
#include <libobj.h>

char *s_alloc (int count, int type, char *name)
    /*count:             number of items to allocate             */
    /*type:              object datatype                         */
    /*name:              object name                             */
    {
    register SHEADER *shp; /* structured allocation                   */
    register i;            /* used in loop to clear data              */
    register char *cp;     /* used in loop to clear data              */
    register unsigned dlength;      /* number of data bytes in this allocation */

#ifdef TRACE

    debug ("s_alloc:  called w/ count = %d, type = %d, ", count, type);
    debug ("name = '%s'\n", (name == NULL) ? "NULL" : name);

#endif

#ifdef CAREFUL

    if ((type <= 0) || (type >= MAX_TYPES) || 
      (dt_name (g_dtypes + type) == NULL)) /* if type invalid */
	{
	if (g_typefcn == NULL) /* if no special type check failure routine */
	    fatal ("s_alloc:  illegal type (%d)", type);

	(*g_typefcn) (); /* call user's type check failure routine */
	g_errno = E_TYPECHECK;
	return ((char *) ERROR);
	}
    if (count < 0)
	fatal ("s_alloc:  negative count (%d)", count);

#endif

    dlength = dt_size (&g_dtypes [type]) * count; /* get data length */

    /* add size of header to request plus an extra byte     */
    /* the extra byte at the end of the data is set to NULL */

    shp = (SHEADER *) malloc (dlength + HEADERSIZE + 1);

    if (shp == NULL) /* return ERROR on allocation failure */
	{
	if (g_allocfcn == NULL) /* if no special alloc failure routine */
	    fatal ("s_alloc:  allocation failure");

	(*g_allocfcn) (); /* call user's allocation failure routine */
	g_errno = E_ALLOC;
	return ((char *) ERROR);
	}
    shp->sh__type = type;
    shp->sh__flag = '\0';
    shp->sh__name = (name == NULL) ? NULL : scopy (name);
    shp->sh__count = count;

    i = dlength + 1; /* Note:  there is an extra NULL byte after the data */
    cp = sh_data (shp);

    while (i--) /* zero data area and extra byte */
	*cp++ = '\0';

    return (sh_data (shp));
    }

