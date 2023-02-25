static char     sccsid[] = "@(#)74      1.9  src/bos/usr/sbin/install/inulib/inu_sop_func.c, cmdinstl, bos411, 9428A410j 6/17/93 16:15:52";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_del_op, inu_free_op, inu_free_bff
 *
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

#include <inulib.h>

extern Option_t *SopPtr;

/* NAME: inu_del_op ()
 *
 * FUNCTION:  The parameter is the first node of options link list. This
 *            function will delete the node from the linked list and set the
 *            node before it to the current nodes next. The space for the node
 *            will also be freed.
 *
 * RETURNS:   SUCCESS if the operation works.
 *            FAILURE if it does not work.
 *
 */

int inu_del_op (Option_t * sop,     /* pointer at the beginning of the
                                     * operation */
                Option_t *next_op, /* pointer at the beginning of the next
                                    * operation */
                Option_t *ptr)     /* pointer to node to be deleted. */
{
   Option_t       *current_op;  /* pointer to the first op in the list */
   Option_t       *last_op;     /* pointer to the last op in the list */

   /* start at the beginning of the list and loop through looking for ptr */

   for (current_op = SopPtr -> next, last_op = SopPtr;
        (current_op != NIL (Option_t)) && (current_op != ptr);
        current_op = current_op -> next)
   {
      last_op = current_op;
   }

   /* check that ptr was found */

   if (current_op != ptr)       /* then there was no match found */
      return (FAILURE);

   /* check if ptr = sop */

   if (current_op == sop)       /* then we have to move the sop pointer */
      sop = current_op -> next;

   /* check if ptr = next_op */

   if (current_op == next_op)   /* then we have to move the next_op pointer */
      next_op = current_op -> next;

   /* move the last_op->next pointer to ptr->next and free up ptr */

   last_op -> next = current_op -> next;

   ptr -> SelectedList = NIL (Option_t);

   inu_free_op (ptr);

   return (SUCCESS);

} /* end inu_del_op */


/* NAME:      inu_free_op ()
 *
 * FUNCTION:  The parameter is the first node of options link list. This
 *            function will free the size and then free the node itself.
 *
 * RETURNS:   No return value.
 *
 */

void inu_free_op (Option_t * optptr)
{
   /*------------------------------------------------------------*
    * If this option isn't on the selected options list then
    * we can free the prereq and supersede malloc'ed areas.
    *------------------------------------------------------------*/

/*   Yanked out code because sometimes SelectedList has already been
     freed.  Occurs in ckprereq when it frees the sop and recreates it.
     Proper solution is for sop to not share same memory as toc for prereq,
     etc.  Also, sop should have back pointer to toc entries.

   if (optptr -> SelectedList == NIL (Option_t))
   {
      if (optptr -> prereq != NIL (char))
      {
         free (optptr -> prereq);
         optptr -> prereq = NIL (char);
      }

      if (optptr -> supersedes != NIL (char))
      {
         free (optptr -> supersedes);
         optptr -> supersedes = NIL (char);
      }
   } */

   /*------------------------------------------------------------*
    * The size calculations have already been done so free the
    * size file malloc'ed area.
    *------------------------------------------------------------*/

   if ( (optptr -> desc != NIL (char) )
                   &&
        (optptr -> desc != NullString) )
   {
      free (optptr -> desc);
   }
   optptr -> desc = NIL (char);

   if (optptr -> size != NIL (char))
   {
      free (optptr -> size);
      optptr -> size = NIL (char);
   }

   /*------------------------------------------------------------*
    * Now free the Option_t structure...
    *------------------------------------------------------------*/

   free (optptr);

} /* end inu_free_op */

/* NAME:      inu_free_bff
 *
 * FUNCTION:  The parameter to this function is the beginning node of bff link
 *            list.
 *
 * RETURNS:   No return value.
 *
 */

void inu_free_bff (BffRef_t * bffptr)
{
   OptionRef_t * next_option;
   OptionRef_t * option;

   if (bffptr != NIL (BffRef_t))
   {
      if (bffptr -> action_string != NIL (char) )
      {
         free (bffptr -> action_string);
         bffptr -> action_string = NIL (char);  /* Out of habit. */
      }
      if (bffptr -> path != NIL (char) )
      {
         free (bffptr -> path);
         bffptr -> path = NIL (char);          /* Out of habit. */
      }
      option = bffptr -> options;
      while (option != NIL (OptionRef_t))
      {
        option -> option = NIL (Option_t);    /* Want to know if someone is
                                                 still using. */
        next_option = option -> next;
        free (option);
        option - next_option;
      }
      bffptr -> options = NIL (OptionRef_t);   /* Out of habit. */
      free (bffptr);
   }
} /* end inu_free_bff */
