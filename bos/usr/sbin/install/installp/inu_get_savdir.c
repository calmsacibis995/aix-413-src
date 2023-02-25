static char sccsid[] = "@(#)95  1.8  src/bos/usr/sbin/install/installp/inu_get_savdir.c, cmdinstl, bos411, 9428A410j 3/21/94 18:25:36";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_get_savdir (installp)
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

/*--------------------------------------------------------------------------*
**
** Function: inu_get_savdir()
**
** Purpose:  Form the string containing the full path to the save directory
**           (the directory where files are saved) based on the information 
**           in the option structure passed in.
**
** Notes:    Save dirs are defined to be:
**
**           Save directory dictionary
**           -------------------------
**           4.1 Update
**              Usr    /usr/lpp/<prodname>/<option>/<v.r.m.f>.save
**              root   /lpp/<prodname>/<option>/<v.r.m.f>.save
**              share  /usr/share/lpp/<prodname>/<option>/<v.r.m.f>.save
**
**           3.2 Update
**              Usr    /usr/lpp/<prodname>/inst_<ptf>.save
**              root   /lpp/<prodname>/inst_<ptf>.save
**              share  /usr/share/lpp/<prodname>/inst_<ptf>.save
**           
**           3.1 Update
**              Usr    /usr/lpp/<prodname>/inst_updt.save
**              root   /lpp/<prodname>/inst_updt.save
**              share  /usr/share/lpp/<prodname>/inst_updt.save
**
** Returns:  None
**
**--------------------------------------------------------------------------*/
void inu_get_savdir(
char     tree,      /*  Root, usr, or share tree this option belongs to      */
Option_t *op,       /* Option where the info resides to determine the savdir */
char     *buf)      /* Ptr passed in where we'll put the save directory path */

{
    char lppname   [MAX_LPP_NAME];     /* top level lpp name */
    char level_str [MAX_LEVEL_LEN+2];    /* v.r.m.f level      */
    
   /*-----------------------------------------------------------------------*
    * We don't save anything for non-updates.
    *-----------------------------------------------------------------------*/

    if (! IF_UPDATE(op->op_type))
    {
       buf[0] = '\0';
       return;
    }

   /*-----------------------------------------------------------------------*
    * Set the save directory string to the correct "lpp/" directory based on
    * the vpd tree that we're dealing with.
    *-----------------------------------------------------------------------*/

    switch (tree) 
    {
        case VPDTREE_ROOT:
            strcpy(buf, "/lpp/");
            break;
        case VPDTREE_SHARE:
            strcpy(buf, "/usr/share/lpp/");
            break;
        case VPDTREE_USR:
        default:
            strcpy(buf, "/usr/lpp/");
            break;

    } /* end switch (tree) */


   /*-----------------------------------------------------------------------*
    * Now finish the savdir string, the directory path at the lpp/ level has
    * already been strcpy'ed, now we need to add on the top level lpp name,
    *-----------------------------------------------------------------------*/
    (void) inu_get_prodname(op, lppname);
    strcat(buf, lppname);

   /*-------------------------------------------------------------------*
    * IF this is a 3.1 update the save directory is in the form of:
    *      ..../<lpp_name>/inst_updt.save
    *-------------------------------------------------------------------*/

    if (IF_3_1(op->op_type))
        strcat(buf, "/inst_updt");

   /*---------------------------------------------------------------*
    * IF this is a 3.2 update the save directory is in the form of:
    *      ..../<lpp_name>/inst_<pftnum>.save
    * where the <ptfnum> is the name of this ptf.
    *---------------------------------------------------------------*/

    else if (IF_3_2(op->op_type)) 
    {
        strcat(buf, "/inst_");
        strcat(buf, op->level.ptf);
    }

   /*---------------------------------------------------------------*
    * 4.1 or later update, ...
    *---------------------------------------------------------------*/

    else
    {
       strcat (buf, "/");
       strcat (buf, op->name);
       sprintf (level_str, "/%d.%d.%d.%d", op->level.ver, op->level.rel,
                                          op->level.mod, op->level.fix);
       strcat (buf, level_str); 
    }

   /*---------------------------------------------------------------*
    * All save directories end in ".save".
    *---------------------------------------------------------------*/

    strcat(buf, ".save");
}
