static char sccsid[] = "@(#)69  1.35.3.1  src/bos/usr/sbin/install/ckprereq/determine_order.c, cmdinstl, bos411, 9434B411a 8/9/94 08:51:32";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            add_dependant 
 *            add_fix_to_set 
 *            apply_fix
 *            apply_medialess_fix 
 *            apply_some_medialess_fixes
 *            create_sop_list 
 *            create_sop_list_for_one_part
 *            determine_order_of_fixes
 *            order_apply_options 
 *            order_commit_or_reject_options
 *            reduce_dependants
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

/*
 * Includes
 */

#include <check_prereq.h>

static void add_dependant (
fix_info_type * prereq,
fix_info_type * parent,
boolean       * error);

static void add_fix_to_set (
fix_set_type  * fix_header,
fix_set_type  * fix_set_trailer,
fix_info_type * entry,
boolean       * error);

static int add_medialess_fix (
fix_set_type  * fix,
Option_t     ** last_option,
boolean       * error);

static void apply_fix (
fix_set_type  * fix,
Option_t     ** last_option,
boolean       * error);

static void apply_some_medialess_fixes (
fix_set_type  * medialess_fixes_header,
fix_set_type  * medialess_fixes_trailer,
Option_t     ** last_option,
boolean         ignore_C_updates,
boolean       * error);

static boolean bff_has_more_options_to_apply (
fix_info_type * source_fix,
fix_set_type  * fix_set_header,
fix_set_type  * fix_set_trailer);

static void create_sop_list (
Option_t     ** last_option,
fix_set_type  * fix_set_header,
fix_set_type  * fix_set_trailer);

static void create_sop_list_for_one_part (
short            part_to_operate_on,
fix_info_type  * fix_info,
Option_t      ** last_option);

static void order_apply_options (
Option_t * option_list,
boolean  * error);

static void order_commit_or_reject_options (
Option_t * option_list,
boolean  * error);

static void reduce_dependants (
fix_info_type * fix_ptr);

static void set_report_attributes (
fix_info_type * fix_ptr,
Option_t      * new_option);

/*--------------------------------------------------------------------------*
**
** NAME: determine_order_of_fixes
**
** FUNCTION:  determine_order_of_fixes is the only external function in
**            this file.  It is called after all of the prereq files have
**            been processed and errors reported.  Here we are only
**            concerned with ordering the fixes so that they can be
**            installed 1) without violating any prereq requirements, and
**            2) keeping media swaps and tape rewinds to a minimum.
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
determine_order_of_fixes 
(
Option_t * option_list,
boolean  * error
)
{

   /* In order to easily determine the order that fixes should be applied,
      we need to massage the information stored in the prereq_info table.

      This is accomplished by:

         *) recognizing that a coreq for a fix is really just a prereq for
            ALL of it's parents fixes.

         *) transforming the data structure from a fix -> list_of_prereqs
            model to a directed graph representing an activity network.

            The basic data structure used here is:
                     fix.number_of_requisites
                     fix -> list_of_dependants

            A list_of_dependants is the inverse of a list_of_prereqs.

            Once the activity network is built, it is easy to come up with
            an ordered list of fixes.  This can be done by the algorithm:

               Until all fixes are applied
                {
                  find a fix with number_of_requisites == 0
                  We can apply that fix now.
                  for every fix that is on this fix's list_of_dependants
                     decrement it's number_of_requisites
                }

       In addition to coming up with a list of fixes that are ordered such
       that no prerequisite rules are broken, we need to attempt to order
       fixes to minimize media swaps and tape travel.  This is accomplished
       by recognizing that when we put fixes in prereq order, we will often
       have several fixes that don't depend upon one another, but may as a set
       require to be put on before some other fix.

       If we order each set to minimize media swaps and tape travel, then we
       will have done the best that we can.
   */

   boolean               do_this_entry;
   fix_info_type       * entry;
   fix_info_type       * next_entry;
   requisite_list_type * requisite;
   fix_info_type       * requisite_node;
   requisite_list_type * temp_requisite;


   /* Go through all of our fixes and transform the requisite lists
      into an activity network.  PREREQ's (and INSTREQ's) are the 
      key requisite to look for since they will determine the order of
      installation.  */

   if (check_prereq.debug_mode)
      graph_dump ("determine_order_of_fixes:1");

   for (entry = check_prereq.fix_info_anchor -> next_fix;
        entry != NIL (fix_info_type);
        entry = entry -> next_fix)
    {
      requisite = entry -> requisites;
      while (requisite != NIL (requisite_list_type) )
       {
         if (requisite -> type == A_PREREQ || requisite->type == AN_INSTREQ)
          {
            requisite_node = requisite -> fix_ptr;
            add_dependant (requisite_node, entry, error);
            RETURN_ON_ERROR;
          } /* end if */

         requisite = requisite -> next_requisite;

       } /* end while */

    } /* end for */

    /* Now with that all done, we can proceed to crank out an ordered list
       of fixes. */

   if (check_prereq.mode == OP_APPLY)
    {
      order_apply_options (option_list, error);
      RETURN_ON_ERROR;
    }
   else
    {
      order_commit_or_reject_options (option_list, error);
      RETURN_ON_ERROR;
    } /* end if */

} /* end determine_order_of_fixes */

/*--------------------------------------------------------------------------*
**
** NAME: add_dependant
**
** FUNCTION:  Here we are converting or prereq information into dependant
**            information.  We are given a prereq and a node that I am a
**            prereq of.  We need to put the node that I am a dependant of
**            (parent) on my (prereq's) dependants list.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void 
add_dependant 
(
fix_info_type * prereq,
fix_info_type * parent,
boolean       * error
)
{
   /*  We need to record what fixes I am a prerequisite for. */

   dependants_type * dependant;

   dependant = (dependants_type *) my_malloc (sizeof (dependants_type),
                                              error);
   RETURN_ON_ERROR;

   dependant -> next_dependant = prereq -> dependants;
   dependant -> fix_ptr = parent;
   prereq -> dependants = dependant;

   parent -> number_of_requisites++;

} /* end add_dependant */

/*--------------------------------------------------------------------------*
**
** NAME: reduce_dependants
**
** FUNCTION:  Decrements the prereq count for every dependant of the given
**            fix.  Also frees the dependant list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
reduce_dependants 
(
fix_info_type * fix_ptr
)
{
   dependants_type * dependant;
   dependants_type * next_dependant;

   dependant = fix_ptr -> dependants;
   while (dependant != NIL (dependants_type))
    {
      dependant -> fix_ptr -> number_of_requisites --;
      next_dependant = dependant -> next_dependant;
      my_free (dependant);
      dependant = next_dependant;
    } /* end while */
   fix_ptr -> dependants = NIL (dependants_type);
} /* end reduce_dependants */

/*--------------------------------------------------------------------------*
**
** NAME: add_fix_to_set
**
** FUNCTION:  We are given a set of fixes that are ordered according to
**            Volume and Offset within that volume.  We are also given a
**            fix to add to that set, while preserving the ordering.
**
**            If the fix is not on the media, we put it at the head of the
**            list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
add_fix_to_set 
(
fix_set_type  * fix_header,
fix_set_type  * fix_trailer,
fix_info_type * entry,
boolean       * error
)
{
   fix_set_type * fix;
   fix_set_type * new_entry;

   /* We keep our fix set ordered according to 1) Volume and 2) offset */
   /* If the incomming fix is NOT on the media (TOC_Ptr == NIL), and we are
      doing an APPLY, then the ordering does not matter.  BUT: If we are doing
      an COMMIT or REJECT, then ordering DOES matter.

      Entery all fixes that are not on the media (ROOT part applys, all commits
      and rejects) at the END of the fix list. */

   new_entry = (fix_set_type *) my_malloc (sizeof (fix_set_type), error);
   RETURN_ON_ERROR;

   new_entry -> fix_ptr = entry;

   if ( (check_prereq.mode != OP_APPLY)  /* If we do an apply-commit, then */
                    ||                   /* the TOC_Ptr might not be NIL on */
        (entry -> TOC_Ptr == NIL (Option_t) ) )  /* COMMIT phase. */
    {
      fix = fix_trailer;
    }
   else
    {
      fix = fix_header -> next_fix;
      while (fix -> next_fix != NIL (fix_set_type))
       {
         if (fix->fix_ptr->apply_state == ALL_PARTS_APPLIED)
            continue;

         if (fix -> fix_ptr -> TOC_Ptr -> bff -> vol >
                               new_entry -> fix_ptr -> TOC_Ptr -> bff -> vol)
          {
            /* Must enter this one here */
            break;
          } /* end if */
         if ( (fix -> fix_ptr -> TOC_Ptr -> bff -> vol ==
                              new_entry -> fix_ptr -> TOC_Ptr -> bff -> vol)
                                   &&
              (fix -> fix_ptr -> TOC_Ptr -> bff -> off >=
                              new_entry -> fix_ptr -> TOC_Ptr -> bff -> off) )
          {
             /* Must enter this one here */
             break;
          } /* end if */
         fix = fix -> next_fix;
       } /* end while */
    } /* end if */

   new_entry -> next_fix = fix;
   new_entry -> previous_fix = fix -> previous_fix;
   fix -> previous_fix = new_entry;
   new_entry -> previous_fix -> next_fix = new_entry;
   return;

} /* end add_fix_to_set */

/*--------------------------------------------------------------------------*
**
** NAME: apply_medialess_fix
**
** FUNCTION:  This routine is called only by apply_some_medialess_fixes
**
**            We are given a pointer to a fix that does not require any media.
**            This can be a ROOT part apply, or a maintenance update where
**            we are using the AVAILABLE record info instead of media.
**
**            We delete the fix out of the doubly linked list before returning.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static int 
apply_medialess_fix 
(
fix_set_type  * fix,
Option_t     ** last_option,
boolean       * error
)
{
   Option_t * new_option;
   short      parts_to_apply;

   if (fix -> fix_ptr -> apply_state != ALL_PARTS_APPLIED)
    {
      new_option = create_option (fix -> fix_ptr -> name, FALSE,
                                  QUIESCE_NO,
                                  convert_cp_flag_type_to_content_type (
                                               fix -> fix_ptr -> cp_flag),
                                  "",
                                  & fix -> fix_ptr -> level, "",
                                  NIL (BffRef_t));
      
      /*
       * Set bits and/or the Status field to aid in reporting successes.
       */
      set_report_attributes (fix->fix_ptr, new_option);

      strcpy (new_option -> prodname, fix -> fix_ptr -> product_name);
      new_option -> operation = check_prereq.mode;
      new_option -> op_type = fix -> fix_ptr -> op_type;
      if (IF_3_2 (fix->fix_ptr->op_type) && IF_C_UPDT (fix->fix_ptr->op_type))
       {
         parts_to_apply = check_prereq.parts_to_operate_on &
                          (fix -> fix_ptr -> parts_applied ^
                                              fix -> fix_ptr -> parts_in_fix);
         switch (parts_to_apply)
          {
             case LPP_ROOT :
                new_option -> vpd_tree = VPDTREE_ROOT;
                break;

             case LPP_USER :
             case (LPP_USER | LPP_ROOT) :
                new_option -> vpd_tree = VPDTREE_USR;
                break;

             case LPP_SHARE :
                new_option -> vpd_tree = VPDTREE_SHARE;
                break;

          } /* end switch */
         if (fix -> fix_ptr -> supersedes_string != NIL (char))
          {
            new_option -> supersedes = dupe_string (fix -> fix_ptr ->
                                                     supersedes_string, error);
            RETURN_ON_ERROR;
          }

       }
      else
         new_option -> vpd_tree = VPDTREE_ROOT;

      new_option -> prereq = dupe_string (fix -> fix_ptr -> prereq_file, error);
      RETURN_ON_ERROR;

      if (check_prereq.debug_mode)
       {
         dump_option (new_option);
       }

      (* last_option) -> next = new_option;
      (* last_option) = new_option;
      (* last_option) -> next = NIL (Option_t);

      fix -> fix_ptr -> apply_state = ALL_PARTS_APPLIED;
   }

   fix -> next_fix -> previous_fix = fix -> previous_fix;
   fix -> previous_fix -> next_fix = fix -> next_fix;
   my_free (fix);

} /* end apply_medialess_fix */

/*--------------------------------------------------------------------------*
**
** NAME: apply_fix
**
** FUNCTION:  This routine is called only by order_apply_options.
**            We are given a pointer to a fix to add to the sop.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
apply_fix 
(
fix_set_type  * fix,
Option_t     ** last_option,
boolean       * error
)
{
   Option_t     * new_option;
   short          parts_to_apply;
   fix_set_type * save;


   new_option = copy_option (fix -> fix_ptr -> TOC_Ptr);
   fix -> fix_ptr -> TOC_Ptr -> SelectedList = new_option;
   (* last_option) -> next = new_option;
   (* last_option) = new_option;
   (* last_option) -> next = NIL (Option_t);

   /*
    * Set bits and/or the Status field to aid in reporting successes.
    */
   set_report_attributes (fix->fix_ptr, new_option);

   parts_to_apply = check_prereq.parts_to_operate_on &
                                                fix -> fix_ptr -> parts_in_fix;
   switch (parts_to_apply)
    {
       case LPP_ROOT :
          new_option -> vpd_tree = VPDTREE_ROOT;
          break;

       case LPP_USER :
       case (LPP_USER | LPP_ROOT) :
          new_option -> vpd_tree = VPDTREE_USR;
          break;

       case LPP_SHARE :
          new_option -> vpd_tree = VPDTREE_SHARE;
          break;

    } /* end switch */

   if (check_prereq.debug_mode)
    {
      dump_option (new_option);
    }

   save = fix -> next_fix;
   fix -> next_fix -> previous_fix = fix -> previous_fix;
   fix -> previous_fix -> next_fix = fix -> next_fix;
   my_free (fix);
   fix = save;

} /* end apply_fix */

/*--------------------------------------------------------------------------*
**
** NAME: apply_some_medialess_fixes
**
** FUNCTION:  Apply as many fixes as we can from the root part.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
apply_some_medialess_fixes 
(
fix_set_type  * medialess_fixes_header,
fix_set_type  * medialess_fixes_trailer,
Option_t     ** last_option,
boolean         ignore_C_updates,
boolean       * error
)
{
   boolean         at_least_one_fix_was_applied;
   fix_set_type  * fix;
   fix_set_type  * previous_fix;

   at_least_one_fix_was_applied = TRUE;
   while (at_least_one_fix_was_applied)
    {
      at_least_one_fix_was_applied = FALSE;
      fix = medialess_fixes_header -> next_fix;
      while (fix != medialess_fixes_trailer)
      {
          if ((fix -> fix_ptr -> number_of_requisites == 0) &&
              ((! IF_C_UPDT (fix->fix_ptr->op_type)) ||
               (! ignore_C_updates)))
           {
             previous_fix = fix -> previous_fix;
             reduce_dependants (fix -> fix_ptr);

             /* apply_medialess_fix deletes the fix. */

             apply_medialess_fix (fix, last_option, error);
             RETURN_ON_ERROR;

             at_least_one_fix_was_applied = TRUE;
             fix = previous_fix -> next_fix;
           }
          else
           {
             fix = fix -> next_fix;
           } /* end if */

       } /* end while */

    } /* end while */
  return;

} /* end apply_some_medialess_fixes */

/*--------------------------------------------------------------------------*
**
** NAME: order_apply_options
**
** FUNCTION:  This is where we actually create a sop for all of the options
**            that we are going to apply.  The order that we apply them is
**            influenced by the position they are on the media.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
order_apply_options 
(
Option_t * option_list,
boolean  * error
)
{
   boolean         at_least_one_fix_was_applied;
   boolean         fix_applied_from_this_volume;
   boolean         ignore_C_updates;
   fix_set_type  * first_fix_in_current_bff;
   fix_set_type  * fix;
   fix_info_type * fix_info;
   fix_set_type    fixes_on_media_header;
   fix_set_type    fixes_on_media_trailer;
   fix_set_type  * last_fix_on_previous_volume;
   Option_t      * last_option;
   fix_set_type    medialess_fixes_header;
   fix_set_type    medialess_fixes_trailer;
   fix_set_type  * next_fix;
   int             volume;

   /* A fix set is a set of fixes that can be applied in any order, they do
      not depend upon one another in terms of prereqs.

      Since we do so many inserts/deletes from a fix set, we utilize header
      and trailer nodes to make life so much easier. */

   medialess_fixes_header.next_fix = & medialess_fixes_trailer;
   medialess_fixes_trailer.previous_fix = & medialess_fixes_header;
   medialess_fixes_header.previous_fix = NIL (fix_set_type);
   medialess_fixes_trailer.next_fix = NIL (fix_set_type);
   medialess_fixes_header.fix_ptr = NIL (fix_info_type);
   medialess_fixes_trailer.fix_ptr = NIL (fix_info_type);

   fixes_on_media_header.next_fix = & fixes_on_media_trailer;
   fixes_on_media_trailer.previous_fix = & fixes_on_media_header;
   fixes_on_media_header.previous_fix = NIL (fix_set_type);
   fixes_on_media_trailer.next_fix = NIL (fix_set_type);
   fixes_on_media_header.fix_ptr = NIL (fix_info_type);
   fixes_on_media_trailer.fix_ptr = NIL (fix_info_type);

   /* We are worried about reducing the amount of disk/tape swaps and tape
      rewinds.

      Pretend that we are the media (tape or diskette, disk doesn't matter).
      For each volume, we start at the beginning and move to the end.
      If there is a fix that we can apply as we get to it, then do it.
      When we get to the end of the volume, we rewind and repeat the process
      until we can go no further with this volume.

      We then go to the next volume and repeat the process.

      After we have tackled all volumes, we start over.

      When we have applied everything, then we are finished. */


   /* To accomplish all of this, we need to proceed as follows:

         1)  Build a list of all of the fixes that just need ROOT parts applied.
             Also include all of the fixes that have already been applied since
             we may have a USER part only fix in the middle of a list of
             USER/ROOT fixes and we have to preserve the ordering.

         2)  Build a list of all of the fixes that need media.  They are ordered
             according to the order given by the TOC.

         3)  Apply as many fixes as possible from the medialess list.

         4)  Apply the fixes from the media.  After a fix (or set from the
             same bff) is applied, try to apply any remaining ones from the
             medialess list. */


   /* Okay, Build the list off all fixes that just need ROOT parts applied.
      Also include any fixes that have ALL_PARTS_APPLIED because they act
      to keep the requisites in order for fixes that actually need something
      applied. */

   for (fix_info = check_prereq.fix_info_anchor -> next_fix;
        fix_info != NIL (fix_info_type);
        fix_info = fix_info -> next_fix)
    {
      if (fix_info -> apply_state != ALL_PARTS_APPLIED)
       {
         if (fix_info -> parts_applied == LPP_USER)
          {
            fix_info -> apply_state = APPLY_TO_BE_COMPLETED;
          }

         /* If this is a C type fix, we may not have media (its from an
            AVAILABLE record).  Treat like a root part apply. */

         if ( IF_C_UPDT (fix_info->op_type)
                                &&
             (fix_info -> TOC_Ptr == NIL (Option_t) ) )
          {
            fix_info -> apply_state = APPLY_TO_BE_COMPLETED;
          } /* end if */
       }

      if ( (fix_info -> apply_state == APPLY_TO_BE_COMPLETED)
                                     ||
           (fix_info -> apply_state == ALL_PARTS_APPLIED) )
       {
         add_fix_to_set (& medialess_fixes_header, & medialess_fixes_trailer,
                         fix_info, error);
         RETURN_ON_ERROR;

         fix_info -> flags &= ~ SUCCESSFUL_NODE;
       } /* end if */

    } /* end for */

   last_option = option_list;
   apply_some_medialess_fixes (& medialess_fixes_header,
                               & medialess_fixes_trailer, & last_option,
                               ignore_C_updates = FALSE, error);
   RETURN_ON_ERROR;


   /* Now, do all of the fixes that are on the media.

      Put all of the fixes into the fixes_on_media in TOC order. */

   for (fix_info = check_prereq.fix_info_anchor -> next_fix;
        fix_info != NIL (fix_info_type);
        fix_info = fix_info -> next_fix)
    {
      if (fix_info -> flags & SUCCESSFUL_NODE)
       {
         #pragma covcc !instr
         if ( (fix_info -> TOC_Ptr == NIL (Option_t) )
                                ||
              (fix_info -> TOC_Ptr -> bff == NIL (BffRef_t) ) )
          {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                     CKP_INT_CODED_MSG_D), inu_cmdname,
                     INTERNAL_ERROR_FIX_NOT_ON_MEDIA);
            * error = TRUE;
            check_prereq.function_return_code = INTERNAL_ERROR_RC;
            return;
          }
         #pragma covcc instr

         add_fix_to_set (& fixes_on_media_header, & fixes_on_media_trailer,
                         fix_info, error);
         RETURN_ON_ERROR;
       }
    } /* end for */

   /* Now apply all of these fixes. */

   while (fixes_on_media_header.next_fix != & fixes_on_media_trailer)
    {
      at_least_one_fix_was_applied = FALSE;
      fix = fixes_on_media_header.next_fix;
      last_fix_on_previous_volume = & fixes_on_media_header;
      volume = fix -> fix_ptr -> TOC_Ptr -> bff -> vol;
      while (fix != & fixes_on_media_trailer)
       {
         fix_applied_from_this_volume = FALSE;
         first_fix_in_current_bff = fix;
         while (             (fix != & fixes_on_media_trailer)
                                      &&
                 (fix -> fix_ptr -> TOC_Ptr -> bff -> vol == volume) )
          {
             if (first_fix_in_current_bff -> fix_ptr -> TOC_Ptr -> bff -> off
                                   != fix -> fix_ptr -> TOC_Ptr -> bff -> off)
                 first_fix_in_current_bff = fix;

             if (fix -> fix_ptr -> number_of_requisites == 0)
              {
                reduce_dependants (fix -> fix_ptr);

                if (fix != first_fix_in_current_bff)
                   next_fix = first_fix_in_current_bff;
                 else
                   next_fix = fix -> next_fix;

                /* Set a flag that tells apply_some_medialess_fixes if C_type
                   updates should be ignored.  We need to do this so that
                   we don't cause a C type update to be applied in the middle
                   of applying options from the same bff.  This can cause
                   tape positioning problems.  (See cmvc defect 90387). */

                if (check_prereq.media_type == TYPE_TAPE_SKD)
                 {
                     ignore_C_updates = bff_has_more_options_to_apply (
                                                     fix -> fix_ptr,
                                                   & fixes_on_media_header,
                                                   & fixes_on_media_trailer);
                 }
                else
                 {
                     ignore_C_updates = FALSE;
                 }

                apply_fix (fix, &last_option, error); /* fix will be deleted. */
                RETURN_ON_ERROR;

                fix_applied_from_this_volume = TRUE;

                /* We can apply all fixes from the same bff at once, even if
                   there are some requisite interdependencies.  So... if we
                   just applied a fix, and it wasn't the first one in the bff,
                   reconsider the earlier fixes in this bff. */

                fix = next_fix;
                first_fix_in_current_bff = fix;

                /* Now see if there are any ROOT parts that we need to apply. */

                apply_some_medialess_fixes (& medialess_fixes_header,
                                            & medialess_fixes_trailer,
                                            & last_option, 
                                            ignore_C_updates, error);
                RETURN_ON_ERROR;
              }
             else
              {
                fix = fix -> next_fix;
              } /* end if */
          } /* end while */

         if (fix_applied_from_this_volume)
          {
            /* Keep trying to apply fixes from this volume.  Rewind and start
               over. */

            at_least_one_fix_was_applied = TRUE;
            fix = last_fix_on_previous_volume -> next_fix;
          }
         else
          {  /* We have exhausted this volume, go on to the next (if there is
                one, otherwise, start over). */

            if (fix != & fixes_on_media_trailer)
             {
               volume = fix -> fix_ptr -> TOC_Ptr -> bff -> vol;
               last_fix_on_previous_volume = fix -> previous_fix;
             }
          } /* end if */
       } /* end while */

      #pragma covcc !instr
      if (! at_least_one_fix_was_applied)
       {
         /*"Internal error: Loop detected.  Can not apply fix from the TOC.\n"*/

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                  CKP_INT_CODED_MSG_D), inu_cmdname, ORDER_APPLY_LOOP2_CODE);

         * error = TRUE;
         check_prereq.function_return_code = INTERNAL_ERROR_RC;
         return;
       }
      #pragma covcc instr
    } /* end while */


   /* If we could not apply all ROOT part fixes, then we is gots a problemo. */

   #pragma covcc !instr
   if (medialess_fixes_header.next_fix != & medialess_fixes_trailer)
    {
      /*"Internal error: Loop detected.  Can not apply fix in
         APPLY_TO_BE_COMPLETED state\n"*/

      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
               CKP_INT_CODED_MSG_D), inu_cmdname, ORDER_APPLY_LOOP_CODE);

      * error = TRUE;
      check_prereq.function_return_code = INTERNAL_ERROR_RC;
      return;
    }
   #pragma covcc instr

} /* end order_apply_options */

/*--------------------------------------------------------------------------*
**
** NAME: order_commit_or_reject_options
**
** FUNCTION:  This is where we actually create a sop for all of the options
**            that we are going to commit or reject.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
order_commit_or_reject_options 
(
Option_t * option_list,
boolean  * error
)
{
   boolean            at_least_one_fix_done;
   fix_info_type    * entry;
   fix_set_type     * fix;
   fix_set_type       fix_set_header;  /* to make insert/delete operations */
   fix_set_type       fix_set_trailer; /*   easy. */
   Option_t         * last_option;

   /* A fix set is a set of fixes that can be applied in any order, they do
      not depend upon one another in terms of prereqs.

      Since we do so many inserts/deletes from a fix set, we utilize header
      and trailer nodes to make life so much easier. */

   fix_set_header.next_fix = & fix_set_trailer;
   fix_set_trailer.previous_fix = & fix_set_header;
   fix_set_header.previous_fix = NIL (fix_set_type);
   fix_set_trailer.next_fix = NIL (fix_set_type);
   fix_set_header.fix_ptr = NIL (fix_info_type);
   fix_set_trailer.fix_ptr = NIL (fix_info_type);

   last_option = option_list;
   do
    {
      at_least_one_fix_done = FALSE;
      for (entry = check_prereq.fix_info_anchor -> next_fix;
           entry != NIL (fix_info_type);
           entry = entry -> next_fix)
       {
          if (entry -> number_of_requisites == 0)
           {
             at_least_one_fix_done = TRUE;
             add_fix_to_set (& fix_set_header, & fix_set_trailer, entry,
                             error);
             #pragma covcc !instr
             if (* error)
                return;
             #pragma covcc instr
           }
       } /* end for */

      /* Now that we have a batch of fixes that do not depend upon each other
         we need to decrement the prereq count of all of their dependants. */

      for (fix = fix_set_header.next_fix;
           fix != & fix_set_trailer;
           fix = fix -> next_fix)
       {
         reduce_dependants (fix -> fix_ptr);
       }

      create_sop_list (& last_option, & fix_set_header, & fix_set_trailer);

    } while (at_least_one_fix_done);

   for (entry = check_prereq.fix_info_anchor -> next_fix;
        entry != NIL (fix_info_type);
        entry = entry -> next_fix)
    {
       #pragma covcc !instr
       if (entry -> number_of_requisites >= 0)
        {
          inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                   CKP_INT_CODED_MSG_D), inu_cmdname, ORDER_COMT_REJ_LOOP_CODE);
          * error = TRUE;
          check_prereq.function_return_code = INTERNAL_ERROR_RC;
          return;
        }
       #pragma covcc instr
    } /* end for */

} /* end order_commit_or_reject_options */

/*--------------------------------------------------------------------------*
**
** NAME: create_sop_list
**
** FUNCTION:  This routine is not called during an APPLY operation.
**
**            Builds a SOP list to be passed back to installp.  This SOP list
**            indicates what options are to be COMMITTED/REJECTED and the order
**            in which they can be done.
**
**            If we are doing a REJECT, we will have to invert the SOP list
**            after it is completed.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
create_sop_list 
(
Option_t     ** last_option,
fix_set_type  * fix_set_header,
fix_set_type  * fix_set_trailer
)
{
   fix_set_type  * fix;
   fix_set_type  * save;

   fix = fix_set_header -> next_fix;
   while (fix != fix_set_trailer)
   {
      create_sop_list_for_one_part (LPP_SHARE, fix->fix_ptr, last_option);
      /* 
       * NOTE:
       * We're making a change from the old strategy of "clumping" usr parts
       * and root parts for commit and reject ops.  Apparently, this only used
       * to be done for message reduction.  Now, we're doing usr-root, usr-root,
       * so that progress messages from installp can be printed accurately.
       */ 
      if (check_prereq.mode == OP_COMMIT) {
       
         create_sop_list_for_one_part (LPP_USER, fix->fix_ptr, last_option);
         create_sop_list_for_one_part (LPP_ROOT, fix->fix_ptr, last_option);
         fix -> fix_ptr -> commit_state = ALL_PARTS_COMMITTED;

      } else {

         create_sop_list_for_one_part (LPP_ROOT, fix->fix_ptr, last_option);
         create_sop_list_for_one_part (LPP_USER, fix->fix_ptr, last_option);
         fix -> fix_ptr -> reject_state = REJECTED;

      }
      fix -> fix_ptr -> number_of_requisites = -1;  /* We don't want to see this
                                                    again. */
      save = fix -> next_fix;
      my_free (fix);
      fix = save;
  } /* while */

  fix_set_header -> next_fix = fix_set_trailer;
  fix_set_trailer -> previous_fix = fix_set_header;

} /* end create_sop_list */

/*--------------------------------------------------------------------------*
**
** NAME: create_sop_list_for_one_part
**
** FUNCTION:  This routine is not called during an APPLY operation.
**
**            This does most of the work for create_sop_list ().
**            This routine makes it convenient to bunch the options for the
**            same ROOT/USER/SHARE parts together.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
create_sop_list_for_one_part 
(
short            part_to_operate_on,
fix_info_type  * fix_info,
Option_t      ** last_option
)
{
   boolean         do_this_fix;
   Option_t      * new_option;
   short           work_required_on_this_part;


   /*
    * Don't consider non-base levels if this is a de-install operation.
    */
   if ((check_prereq.deinstall_submode) && 
       ! IF_INSTALL (fix_info->op_type))
      return;
   
   do_this_fix = TRUE;

   switch (check_prereq.mode)
    {
     case OP_COMMIT :
       if ( (fix_info -> commit_state == CANNOT_COMMIT)
                                      ||
            (fix_info -> commit_state == ALL_PARTS_COMMITTED) )
        {
          do_this_fix = FALSE;
          break;
        }

       /* Don't try to commit a part that has already been commited. */

       work_required_on_this_part = part_to_operate_on &
                                        (~ fix_info -> parts_committed);
       break;

     case OP_REJECT :
       if ( (fix_info -> reject_state == CANNOT_REJECT)
                                     ||
            (fix_info -> reject_state == DONT_NEED_TO_REJECT) )
        {
          do_this_fix = FALSE;
          break;
        }

       /* Don't try to reject a part that is not even applied. */

       work_required_on_this_part = part_to_operate_on &
                                                fix_info -> parts_applied;
       break;
    }

    /*
     * Need to see if our decision (thus far) is consistent with the 
     * parts applied and the parts that we're operating on.
     */
    if ( (do_this_fix) 
              && 
        (fix_info -> parts_applied & work_required_on_this_part &
                                     check_prereq.parts_to_operate_on))
    {

      /* The following copying of level information is probably not
         needed.  It was for some 3.1 updates.  */

      fix_info -> level.sys_ver = fix_info -> level.ver;
      fix_info -> level.sys_rel = fix_info -> level.rel;
      fix_info -> level.sys_mod = fix_info -> level.mod;
      fix_info -> level.sys_fix = fix_info -> level.fix;

      new_option = create_option (fix_info -> name, FALSE,
                                  QUIESCE_NO,
                                  convert_cp_flag_type_to_content_type (
                                               fix_info -> cp_flag),
                                  "",
                                  & fix_info -> level, "",
                                  NIL (BffRef_t));
      strcpy (new_option -> prodname, fix_info -> product_name);

      if (check_prereq.deinstall_submode)
         new_option->operation = OP_DEINSTALL;
      else
         new_option->operation = check_prereq.mode;
      
      switch (part_to_operate_on)
       {
          case LPP_ROOT :
             new_option -> vpd_tree = VPDTREE_ROOT;
             break;

          case LPP_USER :
             new_option -> vpd_tree = VPDTREE_USR;
             break;

          case LPP_SHARE :
             new_option -> vpd_tree = VPDTREE_SHARE;
             break;

       } /* end switch */

      new_option -> op_type = fix_info -> op_type;

      if (MIGRATING (fix_info -> cp_flag))
         new_option -> op_type |= OP_TYPE_MIGRATING;

      /*
       * Set bits and/or the Status field to aid in reporting successes.
       */
      set_report_attributes (fix_info, new_option);

      if (check_prereq.debug_mode)
      {
         dump_option (new_option);
      }

      /*  
       * Mark the flag bit of only one sop entry for a given usr-root pkg so
       * that installp doesn't count it twice in its STATISTICS table.
       */
      if ((strcmp ( (*last_option)->name, new_option->name) == 0) &&
          (inu_level_compare (& ( (*last_option)->level), 
                              & (new_option->level))    ==0))
      {
         new_option->flag = SET_OTHER_PART_ON_SOP_BIT (new_option->flag);
      }

      (* last_option) -> next = new_option;
      (* last_option) = new_option;
      (* last_option) -> next = NIL (Option_t);

    } /* end if */

} /* end create_sop_list_for_one_part */
/*--------------------------------------------------------------------------*
**
** NAME: bff_has_more_options_to_apply
**
** FUNCTION:  Called to see if there are other options on the fix_set list
**            from the same bff as source_fix which have yet to be applied.
**
** RETURNS:   This is a boolean function.  TRUE if there are other fixes on
**            the list with the same offset as source_fix, FALSE otherwise.
**
**--------------------------------------------------------------------------*/

static boolean 
bff_has_more_options_to_apply 
(
fix_info_type * source_fix,
fix_set_type  * fix_set_header,
fix_set_type  * fix_set_trailer
)
{
    fix_set_type * srch_ptr;

    srch_ptr = fix_set_header -> next_fix;
    while (srch_ptr != fix_set_trailer)
     {
       if ( (srch_ptr -> fix_ptr != source_fix) &&
           (srch_ptr -> fix_ptr -> TOC_Ptr -> bff -> off ==
                                           source_fix -> TOC_Ptr -> bff -> off))
        {
          return (TRUE);
        }
       srch_ptr = srch_ptr -> next_fix;
     } 
    return (FALSE);

} /* end bff_has_more_options_to_apply */

/*--------------------------------------------------------------------------*
**
** NAME: set_report_attributes
**
** FUNCTION:  Sets various flags and fields of the Option_t structure passed 
**            to enable subsequent reporting routines to know where the pkg 
**            being applied, comitted, etc. "came from"  ie. requested on 
**            installp command line, an auto supersedes, etc.
**
** RETURNS:   void function.
**
** SIDE EFFECTS:
**            Sets the flag and/or Status field in the Option_t structure 
**            passed.  flag field in fix_info structure passed is also 
**            modified.  description field of Option_t may also be set.
**
**--------------------------------------------------------------------------*/

static void set_report_attributes 
(
fix_info_type * fix_ptr,
Option_t      * new_option
)
{
   /*
    * Set primarily to make reporting of successes for V>V2 easier.  The
    * graph traversal routines search for "sop members" in the graph.
    * See the routine print_successful_requisite () for further info.
    */
   fix_ptr->flags |= MEMBER_OF_SOP;

   /*
    * Let's set the "REQUESTED" bit.  installp is expecting it.
    */
   if (fix_ptr -> flags & EXPL_REQUESTED_PKG)
   {
      new_option->flag = SET_SELECTED_BIT (new_option->flag);
   }
   else
   /*
    * If this is an unresolved ifreq which is being automatically applied,
    * set a temporary value in the status field for the report_success
    * routines.
    */
   if (fix_ptr->flags & UNRESOLVED_IFREQ)
   {
      new_option->Status = AUTO_IFREQ;
   }
   else
   /*
    * Set a temporary value in the status field for the report_success
    * routines if this is a REQUIRED update being automatically installed.
    * (We know this is the case because the MANDATORY_UPDATE bit was set
    * at the beginning of requisite checking.)
    */
   if ( (fix_ptr->flags & MANDATORY_UPDATE) &&
       (fix_ptr->apply_state == TO_BE_EXPLICITLY_APPLIED))
   {
      new_option->Status = MANDATORY;
   }
   else
   /*
    * See if this is an "auto supersedes" pkg (selected in evaluate_supersedes).
    * Set a global flag so that we can print this fact in report_successes ().
    */
   if (fix_ptr->flags & AUTO_SUPERSEDES_PKG)
   {
      new_option->Status = AUTO_SUPERSEDE;
      check_prereq.successful_auto_supersedes = TRUE;
   }

   /*
    * Get the description if we need one and there is one.
    */
   if (((*(new_option->desc)) =='\0' ) &&
       (fix_ptr->description != NIL (char)))
   {
      new_option->desc = strdup (fix_ptr->description);
   }

} /* set_report_attributes */
