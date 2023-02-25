static char     sccsid[] = "@(#)86  1.7  src/bos/usr/sbin/install/installp/inu_swvpd_check.c, cmdinstl, bos411, 9428A410j 5/25/94 12:56:58";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_swvpd_check
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* include files */

#include <installp.h>
#include <inu_swvpd_check.h>

/* external variable */

extern char    *inu_cmdname;    /* command name */

/* FUNCTION:  Checks whether the specfied option has both a PRODUCT and LPP
 *            record in the SWVPD.
 *
 * RETURNS:   SUCCESS (1) if both records exist.
 *            FAILURE (0) otherwise.  If only one record exists it is marked
 *            BROKEN.
 *
 * The caller is responsible for setting up the path to the appropriate SWVPD
 * repository (i.e., root, /usr, or /usr/share).
 *
 * Input parameters:  op:    pointer to an lpp-option structure. The data type
 *                           of this structure is Option_t, which is declared
 *                           in inu_toc.h.
 *
 * Output parameters: None.
 *
 * lpp-option structure fields used/modified:
 *
 *      name:  contains the name of the lpp-option (input only).
 *
 *  level.ver: contains the version of the lpp-option (input only)
 *
 *  level.rel: contains the release of the lpp-option (input only)
 *
 *  level.mod: contains the modification level of the lpp-option (input only)
 *
 *  level.fix: contains the fix level of the lpp-option (input only)
 */

int inu_swvpd_check (Option_t * lppopt)
{
   lpp_t           lpp_row;     /* buffer used to read a row from the LPP VPD */
   prod_t          prod_row;    /* buffer used to read a row from the PRODUCT
                                 * VPD  */
   int             rc;          /* return code */

   uchar           lpp_entry;   /* Flag indicating whether there is a LPP
                                 * entry for the lpp-option in the SWVPD. */
   uchar           prod_entry;  /* Flag indicating whether there is a product
                                 * entry for the lpp-option in the SWVPD. */

   /* Initialize lpp_entry and prod_entry */

   memset (&prod_row, NULL, sizeof(prod_t));
   memset (&lpp_row, NULL, sizeof(lpp_t));

   lpp_entry = TRUE;
   prod_entry = TRUE;

   /* Check if the option's LPP record exists. */

   strcpy (lpp_row.name, lppopt -> name);
   if ((rc = vpdget (LPP_TABLE, LPP_NAME, (char *) &lpp_row)) != VPD_OK)
   {
      /* the option does not have an LPP record */

      lpp_entry = FALSE;
   }

   /* Check if the option's PRODUCT record exists */

   strcpy (prod_row.lpp_name, lppopt -> name);
   prod_row.ver = lppopt -> level.ver;
   prod_row.rel = lppopt -> level.rel;
   prod_row.mod = lppopt -> level.mod;
   prod_row.fix = lppopt -> level.fix;
   strcpy (prod_row.ptf, lppopt -> level.ptf);

   if ((rc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                     PROD_MOD | PROD_FIX | PROD_PTF),
                     &prod_row)) != VPD_OK)
   {
      prod_entry = FALSE;
   } /* end if */

   if (prod_entry && lpp_entry)
   {
      /* both records exist... return success */

      vpd_free_vchars (PRODUCT_TABLE, &prod_row);
      vpd_free_vchars (LPP_TABLE, &lpp_row);

      return (SUCCESS);
   } /* end if */

   /* one or both of the records don't exist */

   if (lpp_entry)
   { /* the LPP record exists... mark it broken */

      lpp_row.state = ST_BROKEN;

      if ((rc = vpdchg (LPP_TABLE, LPP_NAME, (char *) &lpp_row)) != VPD_OK)
      {
         /* can't update a row in the VPD's LPP database */

         instl_msg (SCREEN_LOG, MSGSTR(MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D),
                  inu_cmdname);
      }
   } /* end if */

   if (prod_entry)
   { /* the PRODUCT record exists... mark it broken */

      prod_row.state = ST_BROKEN;

      if ((rc = vpdchg (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                        PROD_MOD | PROD_FIX | PROD_PTF),
                        (char *) &prod_row)) != VPD_OK)
      {
         /* can't update a row in the VPD's PRODUCT database */
         instl_msg (SCREEN_LOG, MSGSTR(MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      }

   } /* end if */

   vpd_free_vchars (PRODUCT_TABLE, &prod_row);
   vpd_free_vchars (LPP_TABLE, &lpp_row);
   return (FAILURE);

} /* end inu_swvpd_check */
