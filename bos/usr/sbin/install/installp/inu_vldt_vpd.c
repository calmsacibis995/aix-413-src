static char     sccsid[] = "@(#)12  1.18  src/bos/usr/sbin/install/installp/inu_vldt_vpd.c, cmdinstl, bos411, 9428A410j 3/21/94 18:27:11";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: validate_vpd, go_through_vpd
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <installp.h>
#include <instl_options.h>

/* NAME: validate_vpd
 *
 * FUNCTION:    Call go_through_vpd function to verify product and history vpd
 *              database is OK. It passes "/etc", "/usr/lib" or
 *              "/usr/share/lib" to the called function based on the ROOT_PART, 
 *              USR_PART or SHARE_PART set or not.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0: no error found -1: any error found
 *
 */

static int go_through_vpd (char * direct);

int inu_validate_vpd (void)
{
   int             rc = 0;
   int             rc2;

   if (ROOT_PART)
      rc |= go_through_vpd (VPD_ROOT_PATH);

   if (USR_PART)
      rc |= go_through_vpd (VPD_USR_PATH);

   if (SHARE_PART)
   {
      /*------------------------------------------------------------*
       * Make sure the following directory exists prior to trying to
       * access the share swvpd database...This directory may not
       * exist due to the ability to over mount file systems...
       *------------------------------------------------------------*/

      if ((rc2 = inu_mk_dir ("/usr/share/lib/objrepos")) != SUCCESS)
         inu_quit (rc2);
      rc |= go_through_vpd (VPD_SHARE_PATH);
   }
   return (rc);

} /* end inu_validate_vpd */

/* NAME: go_through_vpd
 *
 * FUNCTION:   Go through whole product and history vpd data base and verify
 *             everything is correct. If it finds some incorrect records, then
 *             change those and give the error message. The entry point is
 *             passed from validate_vpd ().
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0: no error found -1: any error found
 *
 */

static int go_through_vpd (char * direct)
{
   prod_t          prod_rec;
   hist_t          hist_rec;
   lpp_t           lpp_rec;
   int             rc = 0;
   int             hdflg = 0;

   /* set up the database entry point according to the calling function */

   vpdremotepath (direct);
   vpdremote ();

   if (vpdget (PRODUCT_TABLE, VPD_ALL, &prod_rec) == VPD_OK)
   {
      do
      {
         switch (prod_rec.state)
         {
            case ST_APPLYING:
            case ST_COMMITTING:
               if ( ! C_FLAG)
               {
                  if ( ! hdflg)
                  {
                     hdflg = 1;
                     instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NEEDCLEANUP_E, 
                                              INP_NEEDCLEANUP_D));
                  }
                  instl_msg (FAIL_MSG, "\t%s\n", prod_rec.lpp_name);
                  rc = -1;
               } /* end if */
               break;
            case ST_REJECTING:
            case ST_DEINSTALLING:

               /* get rid of supercedes */

              /** -------------------------------------------------------- *
               **  If we're rejecting a 3.2 update, unset the sceded_by's.
               ** -------------------------------------------------------- */

               if (prod_rec.state == ST_REJECTING   &&
                   prod_rec.cp_flag & LPP_32_FMT)

               inu_vpd_ch_sceded_by (prod_rec.ptf, "", prod_rec.lpp_name);

               /* change the state to BROKEN */

               prod_rec.state = ST_BROKEN;
               if (vpdchgget (PRODUCT_TABLE, &prod_rec) != VPD_OK)
               {
                  instl_msg (FAIL_MSG, 
                           MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                           INU_INSTALLP_CMD);
                  rc = -1;
                  hdflg = 0;
               } /* end if */

               /* If this was an install image then set it broken in the LPP */
               /* database also */

               if (prod_rec.cp_flag & LPP_INSTAL)
               {
                  strcpy (lpp_rec.name, prod_rec.lpp_name);
                  if (vpdget (LPP_TABLE, LPP_NAME, &lpp_rec) == VPD_OK)
                  {
                     lpp_rec.state = ST_BROKEN;
                     if (vpdchgget (LPP_TABLE, &lpp_rec) != VPD_OK)
                     {
                        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E,
                                             CMN_VPDERR_D), INU_INSTALLP_CMD);
                        hdflg = 0;
                        rc = -1;
                     }
                     vpd_free_vchars (LPP_TABLE, &lpp_rec);
                  }
               } /* end if */
               break;

            default:
               break;
         } /* end switch (prod_rec.state) */

         vpd_free_vchars (PRODUCT_TABLE, &prod_rec);
      } while (vpdgetnxt (PRODUCT_TABLE, &prod_rec) == VPD_OK);
   } /* end if */

   hist_rec.state = HIST_PENDING;

   /* get the first HIST record with HIST_PENDING */

   if (vpdget (HISTORY_TABLE, HIST_STATE, &hist_rec) == VPD_OK)
   {
      do
      {
         if (hist_rec.state == HIST_PENDING)
         {
            hist_rec.state = HIST_BROKEN;
            if (vpdchgget (HISTORY_TABLE, &hist_rec) != VPD_OK)
            {
               instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                    CMN_VPDERR_D), INU_INSTALLP_CMD);
               rc = -1;
            }
         }

         vpd_free_vchars (HISTORY_TABLE, &hist_rec);
      } while (vpdgetnxt (HISTORY_TABLE, &hist_rec) == VPD_OK);
   } /* end if */

   return (rc);

} /* end inu_validate_vpd */
