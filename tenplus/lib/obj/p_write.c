/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)79	1.6  src/tenplus/lib/obj/p_write.c, tenplus, tenplus411, GOLD410 3/23/93 12:05:20";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	p_write
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* p_write:  writes a pointer object to a file                              */
/* Does falal error exit if called with non-POINTER object.                 */
/* Returns ERROR on I/O error, else RET_OK.                                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

int p_write (register POINTER *pointers, register FILE *fp)
    /*pointers:	 data pointer for pointer array   */
    /*fp:           file pointer to write array to   */
    {
    register i;                 /* index into the pointer array     */

#ifdef CAREFUL

    s_typecheck ((char *)pointers, "p_write", T_POINTER);

#endif

    for (i = 0; i < obj_count (pointers); i++)
	if (s_write (pointers [i], fp) == ERROR)
	    return (ERROR); /* Note:  g_errno already set */

    return (RET_OK);
    }

