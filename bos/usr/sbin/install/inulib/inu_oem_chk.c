static char sccsid[] = "@(#)38  1.1  src/bos/usr/sbin/install/inulib/inu_oem_chk.c, cmdinstl, bos41J, 9510A_all 2/28/95 13:31:39";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_oem_chk
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
 */

#include <sys/file.h>
#include <installp.h>
#include <instl_options.h>
#include <inuconvert.h>

#define ROOT_INV_FILE           1       /* constant identifying a root
                                         * filesystem inventory file.  */

static int    inu_check_product        (Option_t *);
static int    inu_check_update_image   (Option_t *);
static int    inu_get_prod             (Option_t *, prod_t *);


/*--------------------------------------------------------------------------*
**
** NAME: inu_oem_chk
**
** FUNCTION: Check to see whether there are conflicts between an OEM-propietary
**           fileset or fileset update and the system, or between the IBM
**           fileset update and the fileset
**
** RETURN VALUE DESCRIPTION:
**          SUCCESS:   no conflicts between the fileset and the system
**          FAILURE:  conflicts between the fileset and the system
**
**--------------------------------------------------------------------------*/


int inu_oem_chk (Option_t *op, char *oem_system_name) 
{
   lpp_t     lpp;            /* pointer to lpp info structure */
   int       rc;             /* return code */

   /*-----------------------------------------------------------------------*
    * Check for the following conditions:
    *   If this is an OEM-proprietary fileset or fileset update, is this an
    *      OEM-proprietary system for that OEM?
    *   If this is a non-OEM fileset update, is the fileset in question
    *      already modified to be OEM-proprietary?
    *-----------------------------------------------------------------------*/
    /* Check to see if this is an OEM-proprietary fileset */
    if ((op->lang[0] == 'o') && (!strncmp(op->lang,"oem_", 4))) {
	/* Check to see if the OEM system matches the OEM fileset supplier */
	if ((oem_system_name == NULL) || 
	    (strcmp(&op->lang[4], oem_system_name))) {
	      op->Status = STAT_OEM_MISMATCH;
	      return (FAILURE);
	}
	return (SUCCESS);
    }

    /* Allow a non-OEM fileset to apply on any type system */
    if (IF_INSTALL(op->op_type)) {
	return (SUCCESS);
    }

   /*-----------------------------------------------------------------------*
    * This must be a non-OEM fileset update, check to see if the fileset
    *  has already been replaced by an OEM fileset, either at the base level
    *  or with an update.
    *-----------------------------------------------------------------------*/

   /*-----------------------------------------------------------------------*
    * Read the lpp vpd database based on the lpp name.
    *-----------------------------------------------------------------------*/

   memset (&lpp, NULL, sizeof (lpp_t));

   strcpy (lpp.name, op->name);
   rc = vpdget (LPP_TABLE, LPP_NAME, &lpp);
   vpd_free_vchars (LPP_TABLE, &lpp); /* Don't need these fields. */
   if ((rc == VPD_SYS) || (rc == VPD_SQLMAX)) {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E,
       			            CMN_VPDERR_D), INU_INSTALLP_CMD);
       op->Status = -1;   /* Just mark as some unknown failure. */
       return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * If an entry isn't found, 
    * then return SUCCESS to mean it's OK to install the new one...
    *-----------------------------------------------------------------------*/

   if (rc == VPD_NOMATCH) 
   {
      return (SUCCESS);
   }

   if (lpp.cp_flag & LPP_OEM_SPECIFIC) {
       op->Status = STAT_OEM_REPLACED;
       return (FAILURE);
   }

   return (SUCCESS);  
} 
