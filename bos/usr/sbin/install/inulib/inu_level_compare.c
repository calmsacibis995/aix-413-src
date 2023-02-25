static char sccsid[] = "@(#)68  1.9  src/bos/usr/sbin/install/inulib/inu_level_compare.c, cmdinstl, bos411, 9428A410j 4/28/94 17:34:23";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_level_compare,
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
#include <sys/file.h>
#include <installp.h>
#include <instl_options.h>

/*--------------------------------------------------------------------------*
**
**   Function:    inu_level_compare
**
**   Purpose:     Compare the level of two options.
**
**   Notes:       Both parameters are *ptrs* to Level_t structures. 
**
**   Returns:
**                LESSER_LEVEL  if  a < b
**                LEVEL_MATCH   if  a = b
**                GREATER_LEVEL if  a > b
**                *These are defined in inudef.h
**
**--------------------------------------------------------------------------*/

int inu_level_compare
  (Level_t     *a,     /* Level_t structure first compare  */
   Level_t     *b)     /* Level_t structure second compare */
{
    int rc;

    if (a->ver != b->ver)
       return (a->ver < b->ver)  ?  LESSER_LEVEL : GREATER_LEVEL;

    else if (a->rel != b->rel)
       return (a->rel < b->rel)  ?  LESSER_LEVEL : GREATER_LEVEL;

    else if (a->mod != b->mod)
       return (a->mod < b->mod)  ?  LESSER_LEVEL : GREATER_LEVEL;

    else if (a->fix != b->fix)
       return (a->fix < b->fix)  ?  LESSER_LEVEL : GREATER_LEVEL;

   /** ------------------------------------------------------ *
    **  Check if a ptf id is involved.  If not, then the two
    **  levels are identical.
    ** ------------------------------------------------------ */

    else if (a->ptf != NIL (char) || b->ptf != NIL (char))
    {
       rc = strcmp (a->ptf, b->ptf);
       if (rc != 0)
          return (rc > 0) ? GREATER_LEVEL : LESSER_LEVEL;
    }

    return (LEVEL_MATCH);
}
