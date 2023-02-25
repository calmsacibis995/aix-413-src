static char sccsid[] = "@(#)05  1.10  src/bos/usr/sbin/install/installp/inu_set_libdir.c, cmdinstl, bos411, 9428A410j 3/21/94 18:26:13";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_set_libdir (installp)
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
#include <instl_options.h>

char libdir[512]; /* buffer for INULIBDIR env var */

/*--------------------------------------------------------------------------*
**
** Function: inu_set_libdir()
**
** Purpose:  Form the libdir (the directory where the liblpp.a resides)
**           based on the information in the option structure.
**
** Notes:    INU_LIBDIR is a global variable.
**
**           libdir Dictionary
**           -----------------
**           4.1 Install  (Same as 3.1 and 3.2 Installs)
**              Usr    /usr/lpp/<prodname>
**              root   /usr/lpp/<prodname>/inst_root
**              share  /usr/share/lpp/<prodname>
**
**           4.1 Update
**              Usr    /usr/lpp/<prodname>/<option>/<v.r.m.f>
**              root   /usr/lpp/<prodname>/<option>/<v.r.m.f>/inst_root
**              share  /usr/share/lpp/<prodname>/<option>/<v.r.m.f>
**
**           3.2 Update
**              Usr    /usr/lpp/<prodname>/inst_<ptf>
**              root   /usr/lpp/<prodname>/inst_<ptf>/inst_root
**              share  /usr/share/lpp/<prodname>/inst_<ptf>
**
**           3.1 Update   strcat(INU_LIBDIR, "/inst_updt");
**              Usr    /usr/lpp/<prodname>/inst_updt
**              root   /usr/lpp/<prodname>/inst_updt/inst_root
**              share  /usr/share/lpp/<prodname>/inst_updt
**
**--------------------------------------------------------------------------*/

void inu_set_libdir(
char     tree,      /* the tree that we're dealing with */
Option_t *op)       /* Option where the info resides to determine the libdir */

{
    char lppname[MAX_LPP_NAME];          /* top level lpp name */
    char level_str [MAX_LEVEL_LEN+2];    /* to contain v.r.m.f */
    int  rc;

   /*-----------------------------------------------------------------------*
    * Build the string for setting the ODMDIR env variable and the
    * vpdremote path for the tree we're dealing with for this operation.
    * Also, set the INU_LIBDIR string to point to the library directory.
    * (The library directory is where the liblpp.a file resides.)
    *-----------------------------------------------------------------------*/

    switch (tree) {
        case VPDTREE_SHARE:
	    strcpy(INU_LIBDIR, "/usr/share/lpp/");
            break;
        case VPDTREE_ROOT:
        case VPDTREE_USR:
	default:
	    strcpy(INU_LIBDIR, "/usr/lpp/");
            break;

    } /* end switch (tree) */


   /*-----------------------------------------------------------------------*
    * Now finish the INU_LIBDIR string, the directory path at the lpp/ level
    * has been strcpy'ed, now we need to add on the top level lpp name,
    *-----------------------------------------------------------------------*/

    (void) inu_get_prodname(op, lppname);
    strcat(INU_LIBDIR, lppname);

    if (IF_UPDATE(op->op_type))
    {
       if (IF_3_1 (op->op_type))
	   strcat(INU_LIBDIR, "/inst_updt");
       else if (IF_3_2(op->op_type))
       {
          strcat(INU_LIBDIR, "/inst_");
          strcat(INU_LIBDIR, op->level.ptf);
       }
       else  /* 4.1 or later */
       {
          strcat(INU_LIBDIR, "/");
          strcat(INU_LIBDIR, op->name);
          sprintf (level_str, "/%d.%d.%d.%d", op->level.ver, op->level.rel,     
                                       op->level.mod, op->level.fix);
          strcat(INU_LIBDIR, level_str);
       }
    }

    if (tree == VPDTREE_ROOT)
	strcat(INU_LIBDIR, "/inst_root");

   /*-----------------------------------------------------------------------*
    * Now set the INULIBDIR environment variable,
    *-----------------------------------------------------------------------*/
    strcpy(libdir,"INULIBDIR=");
    strcat(libdir,INU_LIBDIR);
    if ((rc = inu_putenv(libdir)) != SUCCESS)
	inu_quit(INUBADSC);
}
