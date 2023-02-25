static char sccsid[] = "@(#)07  1.13  src/bos/usr/sbin/install/installp/inu_set_vpdtree.c, cmdinstl, bos411, 9428A410j 3/6/94 19:32:36";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_set_vpdtree (installp)
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

char odmdir[100]; /* buffer for ODMDIR  env var */
char inutree[15]; /* buffer for INUTREE env var */

/*--------------------------------------------------------------------------*
**
** NAME: inu_set_vpdtree ()
**
** FUNCTION: Based on the tree parameter, set the vpdremotepath, vpdremote, 
**           ODMDIR env var, and INUTREE env var.
**
** EXECUTION ENVIRONMENT: Part of user command.
**
** NOTES:
**
** ON ENTRY:
**
**--------------------------------------------------------------------------*/
void inu_set_vpdtree (
char    tree)       /* char to indicate the tree to switch to */

{
    int rc;                /* return code                  */

   /*-------------------------------------------------------------------*
    * Build the string for setting the ODMDIR env variable and the
    * vpdremote path for the tree we're dealing with for this operation.
    * Also, set the inu_libdir string to point to the library directory.
    * (The library directory is where the liblpp.a file resides.)
    *-------------------------------------------------------------------*/
    strcpy (odmdir, "ODMDIR=");
    switch (tree) {
        case VPDTREE_ROOT:
            strcat (odmdir, VPD_ROOT_PATH);
            break;
        case VPDTREE_USR:
            strcat (odmdir, VPD_USR_PATH);
            break;
        case VPDTREE_SHARE:
            strcat (odmdir, VPD_SHARE_PATH);
            break;
        default:
            break;

    } /* end switch (tree) */

   /*-------------------------------------------------------------------*
    * Set the path to the VPD database based on the tree to process.
    * We start at subscript 7 to skip the "ODMDIR=" part of the string...
    *-------------------------------------------------------------------*/
    if ((rc = vpdremotepath (&odmdir[7])) != VPD_OK) {
	instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                         INU_INSTALLP_CMD);
        inu_quit (rc);
    }

    vpdremote (); /* always returns VPD_OK */

   /*-------------------------------------------------------------------*
    * Do a putenv to set the environment variable ODMDIR="<odm path>"
    *-------------------------------------------------------------------*/
    if ((rc = inu_putenv (odmdir)) != SUCCESS)
	inu_quit (INUUNKWN);

   /*-------------------------------------------------------------------*
    * Do a putenv to set the environment variable INUTREE="<tree char>"
    * tree char = M (for root) U (for user) and S (for /usr/share)
    *-------------------------------------------------------------------*/
    strcpy (inutree, "INUTREE=");
    inutree[8] = tree;
    inutree[9] = '\0';
    if ((rc = inu_putenv (inutree)) != SUCCESS)
	inu_quit (INUUNKWN);
}
