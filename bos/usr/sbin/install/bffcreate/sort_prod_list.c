static char sccsid[] = "@(#)33  1.2  src/bos/usr/sbin/install/bffcreate/sort_prod_list.c, cmdinstl, bos411, 9428A410j 1/22/94 14:49:19";

/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 *   FUNCTIONS:      sort_prod_list, comp_func
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

#include <bffcreate.h>


/**---------------------------------------------------------------------- *
 **
 ** Function:    sort_prod_list
 **
 ** Purpose:     Sorts the array passed in.
 **
 ** Returns:     None
 **
 **---------------------------------------------------------------------- */


int sort_prod_list (prod_lev ** sort_array, int num_nodes)
{
   qsort (sort_array, num_nodes, sizeof(prod_lev *), comp_func);
} 

/**---------------------------------------------------------------------- *
 **
 ** Function:    comp_func
 **
 ** Purpose:     Compares two product names/levels. 
 **
 ** Returns:     Returns an int indicating which record is greater.
 **              -1 if first_arg is less, otherwise 1.
 **
 **---------------------------------------------------------------------- */

int comp_func (const void * first_arg, const void * second_arg)
{
   prod_lev *node1, *node2;
   int rc;

   node1 = * ((prod_lev **) first_arg);
   node2 = * ((prod_lev **) second_arg);

  /** ------------------------------------------------ *
   **  Increasing alphabetic product names ... 
   ** ------------------------------------------------ */

   if ((rc=strcmp(node1->prodname, node2->prodname)) < 0)
      return -1;

   else if (rc > 0)
      return 1;

  /** ----------------------------------------------------------- *
   **  Decreasing level ...  4.1, 3.2, ..., and decreasing ptf ...
   **  U5, U4, U3, ...
   ** ----------------------------------------------------------- */

   rc = inu_level_compare(&(node1->level), &(node2->level) );

   switch (rc)
   {
      case -1:
           return  1;
           break;
      case  0:
           return -1;
           break;
      case  1:
           return -1;
           break;
      deafult:
           return -1;
   }
}
