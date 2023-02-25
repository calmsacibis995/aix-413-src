/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)19	1.6  src/tenplus/lib/obj/s_typchk.c, tenplus, tenplus411, GOLD410 3/23/93 12:06:13";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_typecheck
 * 
 * ORIGINS:  9, 10, 27
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
/* s_typecheck:  checks type of structured allocation                       */
/* Takes a type to check against.  If this is T_ANY, any valid type is      */
/* accepted.  If a specific type is given, only that type is accepted.      */
/* Does fatal error exit if object type is not valid.  This routine is      */
/* used to make sure a structured allocation pointer is valid Returns RET_OK*/
/* if type is ok.  If type is invalid, does a fatal error if g_typefcn is   */
/* NULL (the default value).  If g_typefcn is not NULL, it is called and    */
/* if it returns, this routine returns ERROR with g_errno set to            */
/* E_TYPECHECK.  Treats cp values of NULL and ERROR as special cases.       */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

#include <libobj.h>

int s_typecheck (char *cp, char *name, int type)
    /* cp:                salloc data area pointer */
    /* name:              name of calling routine  */
    /* type:              type to check against    */
    {
    register objtype;       /* object datatype          */

#ifdef TRACE

    debug ("s_typecheck:  called w/ cp = 0%o, name = '%s'\n",
	   cp, name);
    debug ("              type to check against = %d\n", type);

#endif

    if ((cp == NULL) || (cp == (char *) ERROR))
	{
	if (g_typefcn == NULL) /* if no special type check failure routine */
	    fatal ("s_typecheck:  %s pointer in routine '%s'",
		   cp == NULL ? "NULL" : "ERROR", name);

	(*g_typefcn) (); /* call user's type check failure routine */
	g_errno = E_TYPECHECK;
	return (ERROR);
	}
    objtype = obj_type (cp);

    /* first make sure object has a valid type */

    if ((objtype <= 0) || (objtype >= MAX_TYPES) || 
      (dt_name (g_dtypes + objtype) == NULL))
	{
	if (g_typefcn == NULL) /* if no special type check failure routine */
	    fatal ("s_typecheck:  illegal type (%d) in routine '%s'\n",
		   objtype, name);

	(*g_typefcn) (); /* call user's type check failure routine */
	g_errno = E_TYPECHECK;
	return (ERROR);
	}
    /* if a particular type was specified, make sure it matches */

    if ((type != T_ANY) && (objtype != type))
	{
	if (g_typefcn == NULL) /* if no special type check failure routine */
	    fatal ("s_typecheck:  object type '%s' not '%s' in routine '%s'",
		   dt_name (&g_dtypes [objtype]), dt_name (&g_dtypes [type]),
		   name);

	(*g_typefcn) (); /* call user's type check failure routine */
	g_errno = E_TYPECHECK;
	return (ERROR);
	}
    return (RET_OK);
    }

