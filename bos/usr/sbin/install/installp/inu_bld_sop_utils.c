static char     sccsid[] = "@(#)13      1.16  src/bos/usr/sbin/install/installp/inu_bld_sop_utils.c, cmdinstl, bos411, 9428A410j 4/23/94 15:08:25";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: already_in_sop_list
 *		already_installed
 *		inu_mark_dups_in_toc
 *		inu_rm_2ndary_dups
 *		inu_rm_dup_updates_from_sop
 *		inu_rm_dups_from_sop
 *		inu_rm_opt_from_sop
 *		mark_toc_selected_list_as_nil
 *		mv_F_ptfs_from_sop_to_savesop
 *		pull_in_ptfs_from_toc
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <installp.h>
#include <instl_define.h>
#include <instl_options.h>



static void inu_rm_opt_from_sop           (Option_t *, Option_t *, TOC_t *);
static int  inu_rm_2ndary_dups            (Option_t *, Option_t **, int,
                                           TOC_t *,    Option_t *);
static void mark_toc_selected_list_as_nil (Option_t *, TOC_t *);
static void inu_rm_dup_updates_from_sop   (Option_t *, TOC_t *, Option_t *);
static int  already_in_sop_list           (Option_t *, Option_t *, 
                                           int set_selected_bit);
static int  already_installed             (Option_t *);

static int  base_on_or_going_on           (Option_t * op, Option_t * sop);

/*--------------------------------------------------------------------------*
**
**  Function:        mv_F_ptfs_from_sop_to_savesop
**
**  Description:     Moves all 'SF' ptfs from the sop list specified by the
**                   first paramter to the sop list specified by the second
**                   parameter.  'SF' ptfs are installp updates.
**
**  Parameters:      sop     - sop list to move 'SF' ptfs FROM.
**                   savesop - sop list to move 'SF' ptfs TO.
**
**  Returns:         The number of 'SF' ptfs found and moved.
**
**--------------------------------------------------------------------------*/

int  mv_F_ptfs_from_sop_to_savesop     (Option_t *sop, Option_t *savesop)
{
   Option_t *op;
   Option_t *prev;
   Option_t *end;
   char lpplevname [150];
   int  num_found=0;

  /** ------------------------------------------------------- *
   **  Make the end ptr point to the end of the savesop list
   **  so we can hang additional options off it.
   ** ------------------------------------------------------- */

   if (savesop->next == NIL (Option_t))
      end = savesop;
   else
      for (end = savesop->next;
           end->next != NIL (Option_t);
           end = end->next)
         ;

  /** --------------------------------------------------------- *
   **  Traverse the sop and search for 'SF' (installp) ptfs.
   **  When one is found, move it from the sop to the savesop.
   ** --------------------------------------------------------- */

   for (prev = sop, op = sop->next;
        op != NIL(Option_t);
        prev = op, op = op->next)
   {
     /** ----------------------------------------------------------------*
      **  Make sure that we only pull in 4.1 installp updates.
      ** ----------------------------------------------------------------*/

      if (op->bff->action == ACT_INSTALLP_UPDT  &&  op->op_type == OP_TYPE_4_1)
      {
         ++num_found;
         prev->next = op->next;
         end->next = op;
         end = op;
         op->next = NIL (Option_t);
         op = prev;
      }
   }

   return num_found;
}

/*--------------------------------------------------------------------------*
**
**  Function:        already_in_sop_list
**
**  Description:     Determines if the option specified by the op paramter
**                   currently resides in the sop list specified by the
**                   sop parameter.
**
**  Parameters:      sop  -  the sop list in which to search for op
**                    op  -  the option to search sop for
**       set_selected_bit -  boolean should we set selected bit if match?
**
**  Returns:         TRUE - if op does reside in the sop list
**                  FALSE - if op does NOT reside in the sop list
**
**--------------------------------------------------------------------------*/

static int already_in_sop_list 
  (Option_t *sop, 
   Option_t *op, 
   int set_selected_bit)   /* should we set the selected bit if op is found ? */
{
   int already_in_sop=FALSE;
   Option_t *top;              /* temporary op ptr used to traverse the sop */

   for (top=sop->next; top!=NIL(Option_t); top=top->next)
   {
      if ((strcmp (op->name, top->name) == 0)
                        &&
          (inu_level_compare (&(op->level), &(top->level)) == 0)
                        &&
           op->content == top->content)
      {
         already_in_sop = TRUE;

         if (set_selected_bit == TRUE)
            top->flag = SET_SELECTED_BIT(top->flag); 

         break;
      }
   }
   return already_in_sop;
}

/*--------------------------------------------------------------------------*
**
**  Function:       already_installed
**
**  Description:    Determines if the option that corresponds to the
**                  op parameter is already installed or not.  It does
**                  this by querying the product table of the VPD based
**                  on name and full level (version, release, mod, fix,
**                  and ptf id).
**
**  Parameters:     op - the option that we're checking
**
**  Returns:        TRUE - op IS already installed
**                 FALSE - op is NOT already installed
**
**--------------------------------------------------------------------------*/
static int already_installed (Option_t *op)
{
   int rc;
   prod_t prod;

  /** ------------------------------------------------------------ *
   **  make sure we're dealing with the correct VPD tree.
   ** ------------------------------------------------------------ */

   inu_set_vpdtree (op->vpd_tree);

  /** ------------------------------------------------------------ *
   **  Stuff the search criteria in the prod_t structure.
   ** ------------------------------------------------------------ */

   strcpy (prod.lpp_name, op->name);
   prod.ver = op->level.ver;
   prod.rel = op->level.rel;
   prod.mod = op->level.mod;
   prod.fix = op->level.fix;
   strcpy (prod.ptf, op->level.ptf);

  /** ------------------------------------------------------------ *
   **  Do the VPD query of the product table based on the prod_t
   **  structure and return the appropriate TRUE or FALSE answer.
   ** ------------------------------------------------------------ */

   rc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_VER | PROD_REL |
                               PROD_MOD      | PROD_FIX | PROD_PTF, &prod);

   inu_ck_vpd_sys_rc (rc);

   if (rc == VPD_OK)
      vpd_free_vchars (PRODUCT_TABLE, &prod);

  /** ------------------------------------------------------------ *
   **   If no VPD record exists OR
   **     if one does exist and it's not in the APPLIED or COMMITTED state,
   **     and this option is not superseded,
   **   then this option is not installed -- so return FALSE.
   **   Else it must already be installed -- so return TRUE.
   ** ------------------------------------------------------------ */

   if (rc != VPD_OK || (prod.state != ST_APPLIED && prod.state != ST_COMMITTED
                        && prod.sceded_by[0] == '\0'))
      rc = FALSE;
   else
   {
      rc = TRUE;
   }

   return rc;
}

/*--------------------------------------------------------------------------*
**
**  Function:        pull_in_ptfs_from_toc
**
**  Description:     Finds all instances of ptfs in the toc list that
**                   have the same action field value as the desired_action
**                   parameter.  Then two conditions are tested:
**                     (i)   the ptf is NOT already in the savesop list
**                     (ii)  the ptf is NOT already installed.
**                   If both conditions test true, then the toc instance
**                   is copied and inserted into the sop specified by the
**                   savesop parameter.
**
**  Parameters:          toc - ptr to the toc linked list
**                   savesop - ptr to a sop linked list
**
**  Returns:         The number of matching ptfs found in the toc that meet
**                   both conditions mentioned above that were successfully
**                   copied.
**
**--------------------------------------------------------------------------*/

int pull_in_ptfs_from_toc 
  (TOC_t *toc,             /* Table Of Contents list of media contents */ 
   Option_t *savesop,      /* Save sop containing saved options        */ 
   int desired_action)     /* action field value to match              */
{
   Option_t *op;
   Option_t *end;
   Option_t *newop;
   char lpplevname [150];
   int num_found=0;

  /** ------------------------------------------------------- *
   * Make sure that the toc anchor, or the options pointer
   *  are not NULL pointers  -- Defect 99020
   ** ------------------------------------------------------- */

   if ( (toc == NIL(TOC_t)) || (toc->options == NIL(Option_t)) )
      return num_found;

  /** ------------------------------------------------------- *
   **  Make the end ptr point to the end of the savesop list,
   **  so we can hang additional option ptrs off of it.
   ** ------------------------------------------------------- */

   if (savesop->next == NIL (Option_t))
      end = savesop;
   else
      for (end = savesop->next;
           end->next != NIL (Option_t);
           end = end->next)
         ;

  /** ------------------------------------------------------------- *
   **  Loop thru the toc linked list and find all matches.
   ** ------------------------------------------------------------- */

   for (op  =  toc->options->next;
        op != NIL(Option_t);
        op  =  op->next)
   {
      if (op->bff->action == desired_action)
      {
        /** ----------------------------------------------------------------*
         **  Make sure that we only pull in 4.1 installp updates.
         ** ----------------------------------------------------------------*/

         if (op->bff->action == ACT_INSTALLP_UPDT  &&  
             ! (IF_4_1(op->op_type)) )
            continue;

        /** ------------------------------------------------------------------*
         **  Ensure that we only pull in SR updates such that their base level 
         **  is either already installed or is on the sop to be installed.
         **  Note:  savesop will be the real sop when desired_action =
         **  ACT_REQUIRED_UPDT.
         ** ----------------------------------------------------------------*/

         if (op->bff->action == ACT_REQUIRED_UPDT  &&  
             !(base_on_or_going_on (op, savesop)) )
            continue;

         /* --------------------------------------------------------------- *
         **  Make sure that op is not already in the savesop and that it's
         **  not already installed.  If neither case is true, then copy op
         **  from the toc to the savesop.
         ** --------------------------------------------------------------- */

         if (already_in_sop_list (savesop, op, FALSE) || already_installed (op))
            continue;

         ++num_found;

         /* ------------------------------------------------------------- *
         **  Copy the ptf's option structure from the toc to the savesop.
         ** ------------------------------------------------------------- */

         if ((newop = copy_option (op))  ==  NIL(Option_t))
            inu_quit (INUMALLOC);

         op->SelectedList = newop;
         end->next =  newop;
         end       =  newop;
      }
   }

   return num_found;
}



/*--------------------------------------------------------------------------*
**
**  FUNCTION:         mark_toc_selected_list_as_nil
**
**  DESCRIPTION:      Searches the toc's option linked list for an option
**                    that matches the option passed in.  A match is found if
**                    both option names and both levels match.  When a match is
**                    found, the toc option's SelectedList field is set to NIL.
**
**  PARAMETERS:       sopop  -  the original opt to compare other opts against
**
**  NOTES:            sopop can be an option structure for update images as
**                    well as for install images.  At most ONLY one option in
**                    the toc linked list is modified.
**
**  RETURNS:          none
**
**--------------------------------------------------------------------------*/

static void mark_toc_selected_list_as_nil 
 (
  Option_t * sopop, /* op to search toc for & mark its * SelectedList as Nil */
  TOC_t *toc)

{
   Option_t       *tocop;
   int             found = 0;   /* flag indicating whether op was found in toc
                                 * ll */

   for (tocop = toc -> options -> next;
        tocop != NIL (Option_t) && !found;
        tocop = tocop -> next)
   {
      if ((strcmp (sopop -> name, tocop->name) == 0) &&
          (sopop->level.ver == tocop->level.ver)     &&
          (sopop->level.rel == tocop->level.rel)     &&
          (sopop->level.mod == tocop->level.mod)     &&
          (sopop->level.fix == tocop->level.fix)     &&
          (strcmp (sopop -> level.ptf, tocop->level.ptf) == 0))
      {
         tocop -> SelectedList = NIL (Option_t);
         ++found;
      }
   } /* end for */

} /* end mark_toc_selected_list_as_nil */

/*--------------------------------------------------------------------------*
**
**  FUNCTION:              inu_rm_dup_updates_from_sop
**
**  DESCRIPTION:  Searches for updates to all install packages in
**                the sop. All such updates that do not update the install
**                level specified by the sop parm, are removed from the sop.
**                We know that this is indeed the case by comparing the level
**                of the install package (the sop parm) with any updates found.
**                All install duplicates are removed before this
**                function is called.
**
**  PARAMETERS:            sop        -
**                         toc        -
**
**  NOTES:        Removes all updates which update an install option
**                not in the sop.  THIS FUNCTION SHOULD ONLY BE CALLED IN
**                THE CASE OF AN APPLY (a flag).
**
**  RETURNS:
**
**--------------------------------------------------------------------------*/

static void
inu_rm_dup_updates_from_sop 
 (Option_t *sop, /* ptr to sop linked list head */
  TOC_t    *toc,    /* ptr to toc linked list */
  Option_t *failsop)
                                               
{
   Option_t       *tp;       /* traverse ptr -- to traverse the sop ll    */
   Option_t       *iptr;     /* install ptr -- to traverse the sop ll     */
   Option_t       *prevptr;  /* traverse ptr -- to traverse the sop ll    */


   /**
    **  Rip through the sop and search for all install options.
    **  If an option is an install image, then make sure that all
    **  updates for that install image have the same version and
    **  release level.  If not, then remove the update option.
    **/

   for (iptr = sop -> next;
        iptr != NIL (Option_t);
        iptr = iptr -> next)
   {
      if (IF_INSTALL (iptr -> op_type))
      {
         for (prevptr = sop, tp = sop -> next;
              tp != NIL (Option_t);
              prevptr = tp, tp = tp -> next)
         {
           /** ------------------------------------------------------------- *
            **  If this option in the sop is
            **    1.) an update to the install package specified by iptr and
            **    2.) an update to a DIFFERENT level of the install package
            **        based on version, release, and modification fields,
            **  then remove this option.
            ** ------------------------------------------------------------- */

            if ((IF_UPDATE (tp->op_type)) &&
                (strcmp (iptr->name, tp->name) == 0))
            {

              /** ----------------------------------------------------------- *
               **  3.1 and 3.2 updates ==> check version, release, *and* mod
               ** ----------------------------------------------------------- */

               if  (IF_3_1(tp->op_type) || IF_3_2(tp->op_type))
               {
                  if ((iptr -> level.ver != tp -> level.ver) ||
                      (iptr -> level.rel != tp -> level.rel) ||
                      (iptr -> level.mod != tp -> level.mod))
                  {
                     inu_add_op_to_fail_sop (failsop, tp->name, &(tp->level),
                                   STAT_DUPE_VERSION, NIL(char), tp->desc);
                     inu_rm_opt_from_sop (prevptr, tp, toc);
                     tp = prevptr;
                  }
               }

              /** ----------------------------------------------------------- *
               **  4.1 and greater updates ==> check only version and release
               ** ----------------------------------------------------------- */

               else  /* 4.1 or greater */
               {
                  if ((iptr -> level.ver != tp -> level.ver) ||
                      (iptr -> level.rel != tp -> level.rel))
                  {
                     inu_add_op_to_fail_sop (failsop, tp->name, &(tp->level),
                                   STAT_DUPE_VERSION, NIL(char), tp->desc);
                     inu_rm_opt_from_sop (prevptr, tp, toc);
                     tp = prevptr;
                  }
               }
            } /* if */
         } /* for */
      } 
   } /* for */
}

/*--------------------------------------------------------------------------*
**
** NAME:            inu_mark_dups_in_toc
**
** FUNCTION:        Marks all duplicates in the toc linked list as dups.
**
** NOTE:            The 1st bit of the optios flag field is turned on --
**                  thereby indicating a dup.
**
** RETURNS:         none
**
**--------------------------------------------------------------------------*/

void inu_mark_dups_in_toc (TOC_t *tp)
{

   if (tp == NIL (TOC_t) ||tp -> options == NIL (Option_t))
      return;

   /**
    **  TOC_LL  ==> we're dealing with the toc's option list
    **/

   inu_rm_dups_from_sop (tp->options, TOC_LL, tp, NIL(Option_t));

}

/*--------------------------------------------------------------------------*
**
**  NAME:            inu_rm_opt_from_sop
**
**  FUNCTION:        Breaks old link from op to op->next.  Establishes
**                   a link from prevptr to op->next, ie...
**
**                   prevptr ==> op ==> op->next
**                            becomes
**                   prevptr ==>        op->next.
**
**                   The memory held by op then gets freed.
**
**  NOTE:
**
**  RETURNS:   none
**
**--------------------------------------------------------------------------*/

static void inu_rm_opt_from_sop (Option_t *prevptr, /* ptr that points to op */
                                 Option_t *op,      /* ptr to be removed --
                                                       links * broken & space
                                                       freed */
                                 TOC_t *toc)

{
   Option_t       *tmpop;   /* used to free op when links to op are broken */

   if (op == NIL (Option_t))
      return;

   tmpop = op;

   mark_toc_selected_list_as_nil (op, toc);

   /**
    **  remove op from linked list by removing the link to it.
    **/

   if (prevptr != NIL (Option_t))
      prevptr -> next = op -> next;

   /**
    **  free up the space previously held by op
    **/

   if (tmpop != NIL (Option_t))
      free (tmpop);
}

/*--------------------------------------------------------------------------*
**
** NAME:        inu_rm_2ndary_dups
**
** FUNCTION:    One of two functions:
**       1.) if working with a sop linked list (tocsop = 0), then this funct:
**              Removes 2ndary occurances -- if any -- of duplicates of the
**              dup_opt structure in the sop linked list.  2ndary occurances
**              of duplicates are defined to be all occurances other than
**              the primary duplicate (the duplicate with the latest level).
**                                     or
**       2.) if working with a toc's option linked list (tocsop = 1), then
**           this function:
**              Only marks the duplicates as dups.  No warnings are given
**              to the user.
**
** NOTE:        Two option structures are duplicates if their name fields
**              match.
**
** RETURNS:     The number of duplicates found -- which can be 0 to n.
**
**--------------------------------------------------------------------------*/

static int inu_rm_2ndary_dups 
    (Option_t *dup_opt,     /* the opt to check for dups against      */
     Option_t **latest_op,  /* the current duplicate  w/ latest level */
     int tocsop,            /* 1 => just mark dups, 0 =>remove them   */
     TOC_t *toc,
     Option_t *failsop)     /* failsop anchor */

{
   Option_t       *prevptr;                /* Ptr to node directly preceding
                                              tmptr. */
   Option_t       *tmptr;                  /* Ptr to node currently being
                                              checked. */
   char            optname[PATH_MAX + 1];  /* Option name we're dealing
                                            * with. */
   char            lppname[PATH_MAX + 1];  /* lppname the option belongs to. */
   int             dupfound = 0;           /* counter for # duplicates found. */
   int             cmp;                    /* compare flag for 2 options. */

   /**
    **  Search the sop -- starting with the 1st option after dup_opt --
    **  for duplicates.  Remove all duplicates that have a lesser (or
    **  older) level than the latest_op (to be dups they must both be
    **  the same part, ie.. both be usr parts or both be share parts).
    **/

   for (prevptr = dup_opt, tmptr = dup_opt -> next;
        tmptr != NIL (Option_t);
        prevptr = tmptr, tmptr = tmptr -> next)
   {
      /** --------------------------------------------------- *
       **  Only install images can be duplicates.
       ** --------------------------------------------------- */

      if (IF_INSTALL (tmptr -> op_type) &&
          (strcmp (dup_opt -> name, tmptr -> name) == 0) &&
          (tmptr -> vpd_tree == dup_opt -> vpd_tree))
      {
         ++dupfound;

         cmp = inu_level_compare (&(tmptr->level), &((*latest_op)->level));

         switch (cmp)
         {
            case -1:
               /** --------------------------------------------------- *
                **  level is lesser than the opt we're comparing to.
                ** --------------------------------------------------- */

               if (tocsop == TOC_LL)
               {
                  tmptr->flag = SET_DUPE_BIT(tmptr->flag);
                  break;
               }

               inu_add_op_to_fail_sop (failsop, tmptr->name, &(tmptr->level), 
                                  STAT_DUPE_VERSION, NIL(char), tmptr->desc);
               inu_rm_opt_from_sop (prevptr, tmptr, toc);
               tmptr = prevptr;
               break;

            case 1:
               /** --------------------------------------------------- *
                **  level is greater than the opt we're comparing to.
                ** --------------------------------------------------- */

               if (tocsop == TOC_LL)
                  tmptr -> flag |= 1;

               *latest_op = tmptr;
               break;

            default:
               break;
         } /* switch */
      } /* if */
   } /* for */

   return dupfound;
}

/*--------------------------------------------------------------------------*
**
** NAME: - inu_rm_dups_from_sop
**
** FUNCTION:   The function is 1.) or 2.):
**
**     1.) If dealing with a sop, this function:
**          checks for and removes any duplicates from the sop linked
**          list.  Lists all duplicates to the user and retains the one
**          with the latest level.
**          So, the problem solved by this function is:
**          Assume that more than 1 version of an image is on the media and 
**          consequently gets on the sop.  We can't act on both of them.  So,
**          this routine chooses one (latest level if no level was specified) 
**          and removes all others.
**                                or
**     2.) If dealing with the toc's option linked list, this function:
**          checks for and marks any duplicates in the toc's option linked
**          list.  It DOES NOT remove any duplicates.  The duplicates only
**          get marked as a dupes (the 1st bit of the flag field of the option
**          structure).
**
** NOTE:   It is assumed that all command line duplicates
**         have been removed by the time we get here.  For example, if
**         the user says: "installp -a lpp1 lpp1 lpp1 lpp2", the 2 re-
**         dundant lpp1's will be weeded out by the time this function is
**         called.
**
** RETURNS:        none
**
**--------------------------------------------------------------------------*/

void inu_rm_dups_from_sop 
   (Option_t * sop,         /* the options linked list */
    int        tocsop,      /* indicates if toc or sop linked list  */
    TOC_t    * toc,         /* ptr to toc linked list */
    Option_t * failsop)     /* failure sop linked list */
{
   Option_t *prevptr;                /* ptr that points to node before tmptr */
   Option_t *tmptr;                  /* option being checked if dup or not.  */
   Option_t *latest_op = NULL;       /* the duplicate with the latest level  */
   char     latest_opname[PATH_MAX]; /* name of latest_op.                   */
   char     optname[PATH_MAX];       /* name of tmptr option.                */
   int      any_dups = 0;

   if (sop == NIL (Option_t))
      return;

   for (prevptr=sop, tmptr=sop->next;
        tmptr != NIL(Option_t);
        prevptr=tmptr, tmptr=tmptr->next, any_dups=0)
   {
      if (IF_INSTALL (tmptr -> op_type))
      {
        /** ----------------------------------------------------------- *
         **  Find and remove all secondary duplicates (all dups with
         **  levels lesser than dup with latest level).  The
         **  inu_rm_all_2ndary_dups function will return the number
         **  of 2ndary dups found (0, 1, 2, ...).
         ** ----------------------------------------------------------- */

         latest_op = tmptr;
         if (tmptr->vpd_tree != VPDTREE_ROOT)
            any_dups = inu_rm_2ndary_dups (tmptr, &latest_op, tocsop, toc, 
                                           failsop);

         if (any_dups)
         {
            if (tocsop == TOC_LL)
            {
               tmptr->flag = SET_DUPE_BIT(tmptr->flag);
               continue;
            }

            /** ----------------------------------------------------------- *
             **  If the op being checked is not the duplicate with the
             **  latest level, then blast it (the op pointed to by tmptr).
             ** ----------------------------------------------------------- */

            if (latest_op != tmptr)     /* && tocsop == SOP_LL */
            {
               inu_add_op_to_fail_sop (failsop, tmptr->name, &(tmptr->level), 
                                  STAT_DUPE_VERSION, NIL(char), tmptr->desc);
               inu_rm_opt_from_sop (prevptr, tmptr, toc);
               tmptr = prevptr;
            }
         }      
      } /* if */
   }  /* for */

   if (tocsop == SOP_LL)
      inu_rm_dup_updates_from_sop (sop, toc, failsop);
}

/*--------------------------------------------------------------------------*
**
**  Function:       inu_add_op_to_fail_sop
**
**  Description:    Given some field values, this routine creates an
**                  Option_t structure, assigns the field values to
**                  the passed in data, and adds the structure onto
**                  the failsop list passed in.
**
**  Parameters:     See comments below.
**
**  Returns:        SUCCESS - all successful
**                  FAILURE - on malloc error
**
**--------------------------------------------------------------------------*/

int inu_add_op_to_fail_sop
 (
  Option_t *failsop,   /* Head ptr to the top of the failure sop   */
  char *opname,        /* installable option name                  */
  Level_t *lev,        /* opton's level, ver = -1 if none          */
  int failcode,        /* code used in printing pif error msg      */
  char *supersedes,    /* ptf that supersedes opname above, if any */
  char *description    /* option description                       */
 )

{
   Option_t *top;      /* traverse ptr */

  /** -----------------------------------------------------------------  *
   **  malloc space for this option and insert it into the fail sop.
   **  The only error that should result is a malloc error.
   ** -----------------------------------------------------------------  */

   top = create_option (opname,
                NO,               /* default op_checked field to NO      */
                NO,               /* default quiesce field to NO         */
                CONTENT_UNKNOWN,  /* default content field to unknown    */
                "",               /* default lang field to NIL(char)     */
                lev,              /* default level field to NIL(Level_t) */
                description,      /* default desc field to NIL(char)     */
                NIL (BffRef_t));  /* default bff field to NIL(BffRef_t)  */

   if (top == NIL (Option_t))
      return (FAILURE);

   top->Status = failcode;

   if (supersedes != NIL(char))
      top->supersedes = strdup (supersedes);

   if (USR_PART  && (!ROOT_PART && !SHARE_PART) )
      top->vpd_tree = VPDTREE_USR;
   else if (ROOT_PART  && (!USR_PART && !SHARE_PART) )
      top->vpd_tree = VPDTREE_ROOT;
   else if (SHARE_PART  && (!USR_PART && !ROOT_PART) )
      top->vpd_tree = VPDTREE_SHARE;
   else
      top->vpd_tree = ' ';  /* an UNKNOWN value */

   (void) set_operation (top);

/*    top->flag = SET_SELECTED_BIT(top->flag); */

   insert_option (failsop, top);

   return (SUCCESS);
}

/*--------------------------------------------------------------------------*
**
**  Function:       insert_option
**
**  Description:    Is passed a list and an option to put on the list.  The
**                  option passed in is added to the end of the list.
**
**  Parameters:     See below.
**
**  Returns:        None
**
**--------------------------------------------------------------------------*/

static void insert_option
 (
   Option_t * L,    /* Sop list to add op to */
   Option_t * op    /* option to add to list */
 )
{
   Option_t    *p;  /* tmp traversal ptr */

  /** --------------------------------- *
   **  Traverse to the end of the list.
   ** --------------------------------- */

   for (p = L;
        p->next != NIL (Option_t);
        p = p->next)
   {
     ;
   }

  /** --------------------------------- *
   **  Add op to the end of the list.
   ** --------------------------------- */

   p->next  = op;
   op->next = NIL (Option_t);

} /* end insert_option */


/*--------------------------------------------------------------------------*
**
**  Function:       prune_dups_between_sops
**
**  Description:    Throws out all sop members residing on the 2nd sop
**                  passed in that also reside on the 1st sop passed in.
**
**  Parameters:     See below.
**
**  Returns:        None
**
**--------------------------------------------------------------------------*/

void prune_dups_between_sops 
 (Option_t *sop,            /* this sop list doesn't get pruned */
  Option_t *savesop)        /* rip dupes outta this sop list    */
{
   Option_t *op,
            *prev;

   for (prev=savesop, op=savesop->next;
        op != NIL(Option_t);
        prev=op, op=op->next)
   {
      if (already_in_sop_list(sop, op, TRUE) == TRUE)
      {
         prev->next = op->next;
         free (op);
         op = prev;
      }

   }
}

/*--------------------------------------------------------------------------*
**
**  Function:       base_on_or_going_on  
**
**  Description:    Deteremines if the base level for the update specified
**                  by the op parameter is either
**                     1. Already installed
**                     2. On the sop to be installed
**
**  Returns:        TRUE   - Either condition above is TRUE
**                  FALSE  - BOTH condition above are FALSE
**
**--------------------------------------------------------------------------*/

int base_on_or_going_on 
 (Option_t * op,      /* The SR update option to check base level */
  Option_t * sop)     /* SOP list to search for base level */
{
   lpp_t     lpp;
   int       rc;
   Option_t *top;
   
  /** ---------------------------------------------------------------------- *
   **  Determine if SR's base level IS installed.  If it is return TRUE.
   **  Just query the lpp table based on the option name and state.
   ** ---------------------------------------------------------------------- */

   memset (&lpp, NULL, sizeof (lpp_t));

   inu_set_vpdtree (op->vpd_tree);

   strcpy (lpp.name, op->name);

   rc = vpdget (LPP_TABLE, LPP_NAME, &lpp);

   inu_ck_vpd_sys_rc (rc);

   vpd_free_vchars (LPP_TABLE, &lpp); /* Don't need these fields. */

   if (rc == VPD_OK  &&  
      (lpp.state == ST_APPLIED || lpp.state == ST_COMMITTED))
   {
      return (TRUE);
   }

  /** ---------------------------------------------------------------------- *
   **  Determine if SR's base level IS ABOUT TO BE installed.  If it is, 
   **  then return TRUE.  Do this by searching the sop based on option name,
   **  if it's an INSTALL, VERSION and RELEASE.
   ** ---------------------------------------------------------------------- */

   for (top = sop;
        top != NIL(Option_t);
        top = top->next)
   {
      if ( IF_INSTALL(top->op_type) 
                     && 
           strcmp(op->name,top->name) == 0
                     &&
           op->level.ver == top->level.ver
                     &&
           op->level.rel == top->level.rel)
       {
          return (TRUE);
       }
   }

   return (FALSE);
}
