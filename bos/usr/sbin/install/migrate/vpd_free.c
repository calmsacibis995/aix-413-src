static char sccsid[] = "@(#)10  1.1  src/bos/usr/sbin/install/migrate/vpd_free.c, cmdinstl, bos411, 9428A410j 1/5/94 16:33:32";

/*
 *   COMPONENT_NAME: LIBSWVPD
 *
 * FUNCTIONS: vpd_free_vchars
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Includes
 */

#include "swvpd0.h"


/*--------------------------------------------------------------------------*
**
** NAME: vpd_free_vchars
**
** FUNCTION:  This routine frees every vchar field allocated by odmget_obj
**            for this product structure.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void vpd_free_vchars (int    tbl_id,
                      void * tbl_ptr)
 {
   lpp_t  * lpp;
   prod_t * product;
   hist_t * history;
   inv_t  * inventory;


   switch (tbl_id)
   {
      case LPP_TABLE :
        lpp = (lpp_t *) tbl_ptr;
        if (lpp -> group != NULL )
        {
           free (lpp -> group);
           lpp -> group = NULL;
        }
        if (lpp -> description != NULL )
        {
           free (lpp -> description);
           lpp -> description = NULL;
        }
        break;

      case PRODUCT_TABLE :
        product = (prod_t *) tbl_ptr;
        if (product -> name != NULL )
        {
           free (product -> name);
           product -> name = NULL;
        }
        if (product -> fixinfo != NULL )
        {
           free (product -> fixinfo);
           product -> fixinfo = NULL;
        }
        if (product -> prereq != NULL )
        {
           free (product -> prereq);
           product -> prereq = NULL;
        }
        break;

      case HISTORY_TABLE :
        history = (hist_t *) tbl_ptr;
        if (history -> comment != NULL )
        {
           free (history -> comment);
           history -> comment = NULL;
        }
        break;

      case INVENTORY_TABLE :
        inventory = (inv_t *) tbl_ptr;
        if (inventory -> loc1 != NULL )
        {
           free (inventory -> loc1);
           inventory -> loc1 = NULL;
        }
        if (inventory -> loc2 != NULL )
        {
           free (inventory -> loc1);
           inventory -> loc2 = NULL;
        }
        break;
   } /* end switch */

   return;
 } /* vpd_free_vchars */
