static char sccsid[] = "@(#)06  1.5  src/bos/usr/sbin/install/installp/inu_sort_sop.c, cmdinstl, bos411, 9428A410j 6/16/94 17:36:54";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_sort_sop, comparison_function
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <installp.h>
#include <instl_options.h>
#include <inu_string.h>

#define i_cmp(a,b) (a == b) ? 0 : (a < b) ? -1 : 1

static int comparison_function (const void * first_arg,
                                const void * second_arg);

/*---------------------------------------------------------------------------
 * NAME:      inu_sort_sop
 *
 * FUNCTION:  Sort the given sop.
 *
 * RETURNS:   None.
 *--------------------------------------------------------------------------*/

void inu_sort_sop (Option_t * sop, boolean * error)
{
   Option_t ** sort_array;
   Option_t  * option;
   int         option_number;
   int         number_of_options;

   if (sop->next == NIL (Option_t))
      return;

   /* Count the number of elements that need to be sorted. */

   number_of_options = 0;
   for (option = sop->next; option != NIL (Option_t); option = option->next)
       number_of_options++;

   sort_array = (Option_t **) my_malloc (
                             number_of_options * (sizeof (Option_t *)), error);
   if (* error)
      return;

   /* Now copy pointers to corresponding elements in array. */

   option_number = 0;
   for (option = sop->next; option != NIL (Option_t); option = option->next)
      sort_array[option_number++] = option;

   /* Now call qsort to get the pointers in the correct sort order. */

   qsort (sort_array, number_of_options, sizeof (Option_t *),
          comparison_function);

   /* Now rebuild the linked list so that they are in sorted order. */

   option = sop;
   for (option_number = 0; option_number < number_of_options; option_number++)
    {
      option->next = sort_array[option_number];
      option = option->next;
    }

   option->next = NIL (Option_t);

   my_free (sort_array);

} /* inu_sort_sop */


/*----------------------------------------------------------------------*
 *
 * Function:  comparison_function
 *
 * Purpose:   Compares two sop records and determines their relative order.
 *
 * Returns:   Returns an int indicating which record is greater.
 *            -1 if first_arg is less, otherwise 1.
 *
 *----------------------------------------------------------------------*/

static int comparison_function (const void * first_arg,
                                const void * second_arg)
{
  Option_t * opt_1;
  Option_t * opt_2;
  int	i;

 /*-----------------------------------------------------------------------
  * We want to sort so that lpps are bundled together by version.  Within
  * an lpp, we want things bundled by products, and then by update.
  *----------------------------------------------------------------------*/

  opt_1 = * ((Option_t **) first_arg);
  opt_2 = * ((Option_t **) second_arg);

 /*--------------------------------------------------------------------------
  * If we are about to list APAR or fix info from a tape, then sort according
  * to tape order for performance reasons.
  *------------------------------------------------------------------------*/

  if ((i_FLAG || A_FLAG)
              &&
      (TocPtr->type != TYPE_DISK))
  {
     if ((i = i_cmp (opt_1->bff->vol, opt_2->bff->vol)) != 0)
        return (i);

     if ((i = i_cmp (opt_1->bff->off, opt_2->bff->off)) != 0)
        return (i);
  }

 /*-------------------------------------------------------------------------
  * Compare the <option>.prodname field of each.
  *------------------------------------------------------------------------*/

  if ((i = strcmp (opt_1->prodname, opt_2->prodname)) != 0)
     return (i);

 /*-------------------------------------------------------------------------
  * If no difference in prodnames, compare the <option>.name.
  *------------------------------------------------------------------------*/

  else if ((i = strcmp (opt_1->name, opt_2->name)) != 0)
     return (i);

 /*-------------------------------------------------------------------------
  * if still no difference, compare the <option>.level.
  *------------------------------------------------------------------------*/

  else if ((i = inu_level_compare (&(opt_1->level), &(opt_2->level))) != 0)
     return (i);
  else
     return (-1);  /* Must be a tie, just pick the first option. */

} /* comparison_function */
