static char     sccsid[] = "@(#)50      1.7  src/bos/usr/sbin/install/inulib/inu_deinst_chk.c, cmdinstl, bos41J, 9513A_all 3/23/95 12:41:26";

/*------------------------------------------------------------------------
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: inu_deinst_chk
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *------------------------------------------------------------------------*/

#include <inulib.h>

/*==========================================================================
**
** NAME: inu_deinst_chk
**
** FUNCTION: Given a product name, a product option name, and the vpd_tree, 
**           this function will see if there's a <prod_option>.pre_d execu-
**           table, and execute it if it exists.
**
** RETURNS:  If <prod_option>.pre_d does not exist, or it exists and executes
**           successfully then inu_deinst_chk will return 0.  Otherwise it
**           will return the status code STAT_FAILED_PRE_D.
**
**=========================================================================*/

int inu_deinst_chk (char * prod_name, 
                    char * op_name, 
                    char vpd_tree,
                    int display_pre_d_output)
{
   int    rc;
   int    failcode = 0;
   char   cur_dir [PATH_MAX];
   char   deinst_dir [2 * MAX_LPP_FULLNAME_LEN];
   char   pre_d [2 * MAX_LPP_FULLNAME_LEN];
   struct stat stat_buf;

  /*-----------------------------------*
   * Get the current working directory
   *-----------------------------------*/
   if ((getcwd (cur_dir, (int) sizeof (cur_dir))) == NIL (char))
      return (STAT_FAILED_PRE_D);

  /*-----------------------------------*
   * Determine the packaging directory 
   *-----------------------------------*/
   switch (vpd_tree)
   {
      case VPDTREE_USR:
      case VPDTREE_ROOT:
         sprintf (deinst_dir, "/usr/lpp/%s/deinstl", prod_name);
         break;
      case VPDTREE_SHARE:
         sprintf (deinst_dir, "/usr/share/lpp/%s/deinstl", prod_name);
         break;
      default:
         return (STAT_FAILED_PRE_D);
         break;
   }

  /*------------------------------------------------------------*
   * Return if can't change directory to the packaging directory 
   *------------------------------------------------------------*/
   if (chdir (deinst_dir) != SUCCESS)
      return (0);

   sprintf (pre_d, "./%s.pre_d", op_name);

  /*-------------------------------------------*
   * Execute the pre_d script if its executable 
   *-------------------------------------------*/
   if (stat (pre_d, &stat_buf) == 0  &&  (stat_buf.st_mode & S_IFREG)  &&  
       (stat_buf.st_mode & S_IXUSR))
   {
      if (! display_pre_d_output)
         strcat (pre_d, " >/dev/null 2>&1");

      inu_before_system_sigs ();
      rc = inu_do_exec (pre_d);
      inu_init_sigs ();

      if (rc != 0)
         failcode = STAT_FAILED_PRE_D;
   }

  /*-----------------------------------------------*
   * Change back to the current working directory
   *-----------------------------------------------*/
   if (chdir (cur_dir) != SUCCESS)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                           CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, cur_dir);
   }

   return (failcode);
 
} /* inu_deinst_chk */
