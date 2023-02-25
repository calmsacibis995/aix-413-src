static char sccsid[] = "@(#)71  1.17.1.24  src/bos/usr/sbin/install/installp/inu_ls_toc.c, cmdinstl, bos411, 9428A410j 6/27/94 16:17:19";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_ls_toc
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
#include <instl_define.h>
#include <instl_options.h>
#include <inu_string.h>
#include <ckp_common_defs.h>

/**
 ** Added static prototypes (these functions are local only to the
 ** routines in this module).
 **/

static fix_info_type * find_common_entry (Option_t *);

static fix_info_type * locate_fix (char * name, char * ptf);

static char * get_latest_superseding_ptf (fix_info_type *);

static void print_gui_output (Option_t *, char *, char *, char *, char *, 
                              char *, char *);

static char  get_state (fix_info_type * fix);
static void  print_ls_toc_hdr ();


/*----------------------------------------------------------------------------
 * NAME: inu_ls_toc
 *
 * FUNCTION:    This function will print out the information required by the -l
 *              option on installp.
 *
 * EXECUTION
 * ENVIRONMENT: Part of user command.
 *
 * RETURNS:     SUCCESS If no error were found in outputing the information
 *              FAILURE If any error was found.
 *
 *---------------------------------------------------------------------------*/

int inu_ls_toc (Option_t *op)
{
   boolean  error = FALSE;
   char  last_lpp[MAX_LPP_FULLNAME_LEN];     /* base name of the last lpp    */
   char  lpp_basename[MAX_LPP_FULLNAME_LEN]; /* base name of the current lpp */
   int            print;              /* flag for printing an entry          */
   int            rc;                 /* return code from first fprintf call */
   char           S;                  /* 1st char on line depending on smit  */
   Option_t      *toc_op;             /* Copy of the option head pointer     */
   char           type[3];            /* Translated type of the option       */
   char          *part;               /* Contents of the option              */
   int            option = 0;         /* Non-0 after first option is printed */
   fix_info_type *fix_in_loop;        /* Ptr to a record in fix_info_table   */
   fix_info_type *sup_in_loop;        /* Ptr to a record in fix_info_table   */
   fix_info_type *loop;               /* Ptr to a record in fix_info_table   */
   char           state[2];           /* State of the product                */
   char           sup_state[2];       /* State of the superseding ptf        */
   char           supersede[2];       /* to indicate if prod has been seded  */
   char          *latest_sup;         /* Latest version of a product         */
   boolean        printed_hdr=FALSE;  /* Have we printed the header yet?     */
   char           level_str [MAX_LEVEL_LEN];

   lpp_basename[0] = '\0';
   last_lpp[0] = '\0';

   inu_init_strings ();

   /* if not invoked from the GUI */

   if ( ! L_FLAG)
   {
       /* if smit add # at beginning of the non-selectable lines */
       if (J_FLAG)
          S = '#';
       else
          S = ' ';
   }   

  /*-----------------------------------------------------------------*
   *   Sort the toc linked list by product, option, v.r.m.f and ptf id
   *-----------------------------------------------------------------*/

   inu_sort_sop (op, &error);
   if (error)
      return (FAILURE);


   /* if invoked from the GUI */

   if (L_FLAG)
   {

     /*--------------------------------------------------------------------*
      *  Read the entire contents of the product vpd, merging USR/ROOT/SHARE
      *  information. Use the check_prereq function "load_fix_info_table" to
      *  read the vpd and toc info into a fix_info_type structure.
      *--------------------------------------------------------------------*/

      check_prereq.number_of_errors = 0;
      check_prereq.function_return_code = 0;
      check_prereq.dump_on_error = TRUE;
      check_prereq.mode = OP_APPLY;
      check_prereq.debug_mode = FALSE;
      check_prereq.parts_to_operate_on = LPP_ROOT |LPP_USER |LPP_SHARE;
      check_prereq.SOP = NIL (Option_t);
      check_prereq.First_Option = op;
      check_prereq.Auto_Include = FALSE;
      check_prereq.verbose = FALSE;
      check_prereq.keep_apar_info = FALSE;
      check_prereq.keep_description_info = FALSE;
      check_prereq.called_from_ls_programs = TRUE;

      /*
       * TRUE tells load_fix_info that it's being called without a sop.
       */
      load_fix_info_table (TRUE, &error);

      /*-------------------------------------------------------------------*
       * We don't want to print the header if the L_FLAG was given so 
       * we'll fake out the for loop below and say we've already printed it
       *-------------------------------------------------------------------*/

       printed_hdr = TRUE;

   }  /**** if invoked from the GUI  ****/

   /* Loop through the Option structure printing out the information for
      each. */

   for (toc_op = op->next;
        toc_op != NIL (Option_t);
        toc_op = toc_op->next)
   {
      strcpy (type, "?");
      switch (toc_op->bff->action)
      {
         case ACT_INSTALL:
            strcpy (type, "I");
            break;
         case ACT_SING_UPDT:
            strcpy (type, "S");
            break;
         case ACT_MULT_UPDT:
            strcpy (type, "M");
            break;
         case ACT_GOLD_UPDT:
            strcpy (type, "G");
            break;
         case ACT_EN_MEM_UPDT:
            strcpy (type, "Se");
            break;
         case ACT_INSTALLP_UPDT:
            strcpy (type, "SF");
            break;
         case ACT_REQUIRED_UPDT:
            strcpy (type, "SR");
            break;
         case ACT_CUM_UPDT:
            strcpy (type, "MC");
            break;
         case ACT_EN_PKG_UPDT:
            strcpy (type, "ME");
            break;
         case ACT_MAINT_LEV_UPDT:
            strcpy (type, "ML");
            break;
        /** ------------------------------------------------------------- *
         **  We don't wanna print a line for the bos.rte image, since 
         **  it can't be installed with installp.
         ** ------------------------------------------------------------- */
         case ACT_OTHER:
            continue;
      }

     /** ------------------------------------------------------------- *
      ** Don't print the header until we know we're gonna print 
      ** at least one valid line.
      ** ------------------------------------------------------------- */

      if (printed_hdr == FALSE)
      {
         print_ls_toc_hdr ();
         printed_hdr = TRUE;
      }

      inu_get_prodname (toc_op, lpp_basename);

      if ( ! L_FLAG)
      {
          if (toc_op->content == CONTENT_SHARE)
             part = string[SHARE2_STR];
          else
             if (toc_op->content == CONTENT_BOTH)
                part = string[BOTH_STR];
             else
                part = string[USR2_STR];

      }   /**** if ( ! L_FLAG) *****/

      /* decide if this line should be printed */

      print = TRUE;

      /* if its a SHARE only */
      if (SHARE_PART  &&  ! USR_PART  &&  ! ROOT_PART  && 
          ! (toc_op->content == CONTENT_SHARE))
         print = FALSE;

      /* if its a ROOT only */
      if (ROOT_PART  &&  ! USR_PART  &&  ! SHARE_PART  && 
          ! (toc_op->content == CONTENT_BOTH))
         print = FALSE;

      if ((strcmp (type, "I") == 0)  &&  B_FLAG  &&  ! I_FLAG)
         print = FALSE;

      /* if we want installs and this is a golden update consider it an
       * install */

      if ((IF_ACT_UPDATE (toc_op->bff->action)
                              && 
           ! IF_GOLD_UPDATE (toc_op->bff->action))
                              && 
                           I_FLAG
                              && 
                          ! B_FLAG)
         print = FALSE;

      /* print out the information in the TOC if appropriate */

      if (print  &&  ! L_FLAG)
      {
         if (J_FLAG  &&  (strcmp (lpp_basename, last_lpp) != 0))
         {
            /* if SMIT, print out the lpp with .all for user selection */

            if (option != 0)
            {
               instl_msg (LOG_MSG, "# %s\n", 
                        MSGSTR (MS_INSTALLP, INP_LIST_UL_I, INP_LIST_UL_D));
               option = 1;
            } /* end if */

            strcpy (last_lpp, lpp_basename);

            /* if SMIT, print out the lpp with .all if name includes a . */

            instl_msg (LOG_MSG, "  %s.all\n", lpp_basename);
            instl_msg (LOG_MSG, MSGSTR (MS_INSTALLP, INP_LPPALL_I, 
                      INP_LPPALL_D), S, lpp_basename);
            instl_msg (LOG_MSG, "#\n");


         } /* end if smit */

         if (toc_op->level.ptf [0] == '\0')
            sprintf (level_str, "%d.%d.%d.%d", toc_op->level.ver, 
                     toc_op->level.rel, toc_op->level.mod, toc_op->level.fix);
         else
            sprintf (level_str, "%d.%d.%d.%d.%s", toc_op->level.ver, 
                     toc_op->level.rel, toc_op->level.mod, 
                     toc_op->level.fix, toc_op->level.ptf);

         instl_msg (LOG_MSG, "  %-*s %-*s", OPTION_LEN, toc_op->name, 
                    FULL_LEVEL_LEN, level_str);

         if (J_FLAG)
         {
            instl_msg (LOG_MSG, "\n#   %-52.52s %-2s %c %s\n", 
                     toc_op->desc, 
                     type, 
                     toc_op->quiesce, 
                     part);
            instl_msg (LOG_MSG, "#\n");
         }
         else
         {
            instl_msg (LOG_MSG, " %-2s %c %s\n", 
                     type, 
                     toc_op->quiesce, 
                     part);
            instl_msg (LOG_MSG, "#   %-52.52s\n", toc_op->desc);
            instl_msg (LOG_MSG, "\n");
         }

      } /* end if (print && ! L_FLAG) */

     /*---------------------------------------------*
      *  If installp is called from the GUI.
      *---------------------------------------------*/

      if (print && L_FLAG)
      {
          state [0] = '\0';
          sup_state[0] = '\0';
          supersede[0] = '\0';

         /*------------------------------------------------------------------*
          *  Search the fix_info table for the toc entry. If the toc entry is
          *  found in the fix_info table, then check if the option/ptf has
          *  been applied/committed. If the option/ptf has been superseded 
          *  find the latest ptf which supersedes the toc entry.
          *------------------------------------------------------------------*/

          fix_in_loop = find_common_entry (toc_op);
          if (fix_in_loop != NULL)
          {
             /*--------------------------------------------------*
              *  We've found an entry in the fix_info table.
              *  Now find out if it has been applied/committed or
              *  available for installation.
              *--------------------------------------------------*/

              state[0] = get_state (fix_in_loop);
              state[1] = '\0';

             /*--------------------------------------------------*
              *  Check to see if the product has been superseded
              *  either "actively" or "passively" ! If true, then
              *  get the latest version of the product.
              *--------------------------------------------------*/

              if ((fix_in_loop->superseding_ptf[0] != '\0')
                                   || 
                   (fix_in_loop->superseded_by != NULL))
              {
                 supersede[0] = 's';
                 supersede[1] = '\0';
                 latest_sup = get_latest_superseding_ptf (fix_in_loop);

                /*------------------------------------------------------*
                 *  If the product has been actively superseded then
                 *  get the state of the latest version of the product.
                 *------------------------------------------------------*/

                 if (fix_in_loop->superseding_ptf[0] != '\0')
                 {
                     sup_in_loop = locate_fix (fix_in_loop->name, 
                                              fix_in_loop->superseding_ptf);
                     if (sup_in_loop != NULL)
                     {
                         sup_state[0] = get_state (sup_in_loop);
                         sup_state[1] = '\0';

                     } /**** if sup_in_loop != NULL ****/
                 } /**** if fix_in_loop->superseding_ptf[0].. ****/
              }
              else
                 /*--------------------------------------------------*
                  *  The option/ptf was not superseded, so we set 
                  *  latest_sup to '\0'.
                  *--------------------------------------------------*/

                 latest_sup = '\0';

          }  /**** end of if fix_in_loop ****/


         /*------------------------------------------------------------*
          * print the information for the current product. The output
          * is colon separated. This output will only be used by the
          * GUI.
          *------------------------------------------------------------*/

          print_gui_output (toc_op, type, state, supersede, 
                         fix_in_loop->superseding_ptf, sup_state, latest_sup);

      }  /**** if (print  &&  L_FLAG) ****/


   } /* end for (loop through the options */

   return (SUCCESS);
} /* end inu_ls_toc */


/*----------------------------------------------------------------------*
  *
  * Function:    find_common_entry
  *
  * Purpose:     To find the toc entry in the fix info table.
  *
  * Returns:     Ptr to the common entry in the fix info table.
  *
  *----------------------------------------------------------------------*/


static fix_info_type * find_common_entry (Option_t  *op)
{

   fix_info_type  * loop;

   for (loop = check_prereq.fix_info_anchor;
         loop != NULL;
         loop = loop->next_fix)
   {
       if ((strcmp (op->level.ptf, loop->level.ptf) == 0)
                             && 
            (strcmp (op->name, loop->name) == 0)
                         && 
            (op->level.ver == loop->level.ver)
                         && 
            (op->level.rel == loop->level.rel)
                         && 
            (op->level.mod == loop->level.mod)
                         && 
            (op->level.fix == loop->level.fix))
        {
           return (loop);
        }
   }  /***** end of for loop ****/


}     /**** end of find_common_entry ****/


  /*--------------------------------------------------------------------*
   *
   *  Function:   locate_fix
   *
   *  Purpose:    To locate a particular fix in the fix_info_table list
   *              given the product name and ptf id.
   *
   *  Returns:    Returns a pointer to the particular fix entr in the
   *              fix_info_table list. Returns NIL if no entry is
   *              found.
   *
   *--------------------------------------------------------------------*/


static fix_info_type * locate_fix (char * name, 
                                   char * ptf)
{
   fix_info_type * fix;

   for (fix = check_prereq.fix_info_anchor->next_fix;
        fix != NIL (fix_info_type);
        fix = fix->next_fix)
    {
       if ((strcmp (fix->level.ptf, ptf) == 0)
                             && 
            (strcmp (fix->name, name) == 0))
        {
           return (fix);
        }
    }

   return (NIL (fix_info_type));

} /* end locate_fix */


 /*----------------------------------------------------------------------*
  *
  * Function:    get_latest_superseding_ptf
  *
  * Purpose:     To find the latest superseding ptf.
  *
  * Returns:     Ptr to the latest superseding ptf entry.
  *
  *----------------------------------------------------------------------*/



static char * get_latest_superseding_ptf (fix_info_type * tmp)
{
   supersede_list_type  * tail;
   supersede_list_type  * supersede_info;

   tail = NULL;
   supersede_info = tmp->superseded_by;
   while ((supersede_info) != NULL)
   {
      tail = supersede_info;
      supersede_info = supersede_info->superseding_fix->superseded_by;
   }
   if (tail == NULL)
      return ('\0');
   else
      return (tail->superseding_fix->level.ptf);

}    /**** end of get_latest_superseding_ptf ****/


  /*--------------------------------------------------------------------*
   *
   *  Function: print_gui_output
   *
   *  Purpose:  To print the requested information to be used by the GUI.
   *
   *  Returns:  This is a void function.
   *
   *--------------------------------------------------------------------*/
static
void print_gui_output (Option_t * toc_op, char * type, char * state, 
                       char * supersede, char * superseding_ptf, 
                       char * sup_state, char * latest_sup)
{
     instl_msg (LOG_MSG, 
                  "%s:%s:%d.%d.%d.%d:%s:", 
                  toc_op->prodname, 
                  toc_op->name, 
                  toc_op->level.ver, 
                  toc_op->level.rel, 
                  toc_op->level.mod, 
                  toc_op->level.fix, 
                  toc_op->level.ptf);
 
        instl_msg (LOG_MSG, "%s:%s:%s:", type, state, supersede);

        instl_msg (LOG_MSG, "%s:%s:%s:%c:", 
              superseding_ptf, 
              sup_state, 
              latest_sup, 
              toc_op->quiesce);

     instl_msg (LOG_MSG, "%s:", toc_op->desc);
    
    /*----------------------------------------------------------------*
     *  If the toc contains the vendor_id, then we assume that the 
     *  rest of the netls information is available to be printed. If the
     *  vendor_id field is empty, then just print blanks for the netls
     *  information fields.
     *----------------------------------------------------------------*/

     if (toc_op->netls->vendor_id != NIL (char))
     {
        instl_msg (LOG_MSG, "%s:%s:%s\n", toc_op->netls->vendor_id, 
                                      toc_op->netls->prod_id, 
                                      toc_op->netls->prod_ver);  
     }
     else
        instl_msg (LOG_MSG, ":::\n");
     latest_sup = '\0';


}

  /*--------------------------------------------------------------------*
   *
   *  Function:   get_state
   *
   *  Purpose:    To get the state of a particular product from the
   *              corresponding entry in the fix_info_table.
   *
   *  Returns:    A char which signifies the state.
   *
   *--------------------------------------------------------------------*/

static
 char get_state (fix_info_type * fix)
{

  char  state;

  if ((fix->apply_state == BROKEN)
                            || 
       (fix->commit_state == BROKEN))
  {
       state = 'B';                  /* broken state */
  }

  else if ((fix->apply_state == PARTIALLY_APPLIED)
                                || 
            (fix->commit_state == PARTIALLY_COMMITTED))
       {
            state = '?';                  /* inconsistent state */
       }

  else if ((fix->apply_state == ALL_PARTS_APPLIED)
                            && 
            (fix->commit_state == ALL_PARTS_COMMITTED))
       {
            state = 'C';            /* committed state */
       }

  else if ((fix->apply_state == ALL_PARTS_APPLIED))
       {
           state = 'A';             /* applied state */
       }

  else if ((fix->apply_state == AVAILABLE))

       {
           state = 'N';             /* not applied, is on the media but has
                                       been actively superseded.
                                     */
       }

  else if (fix->apply_state == IN_TOC)
       {
           state = 'T';            /* on installation media */
       }

  else
        state = '?';               /* inconsistent state */

  return (state);

}  /*** end of get_state ***/

/*----------------------------------------------------------------------*
 *
 * Function:    print_ls_toc_hdr  
 *
 * Purpose:     prints out the header for -l output
 *
 * Returns:     None
 *
 *----------------------------------------------------------------------*/


static void print_ls_toc_hdr ()
{
   if (J_FLAG)
   {
      instl_msg (LOG_MSG, "# %-*s %-*s %-*s %-*s %-*s\n", OPTION_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_LABEL_I, INP_LPP_LABEL_D), 
                              FULL_LEVEL_LEN - 1, 
                      MSGSTR (MS_INSTALLP, INP_LPP_LEVEL_I, INP_LPP_LEVEL_D), 
                              TYPE_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_ACTION_I, INP_LPP_ACTION_D), 
                              QUI2_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_QUISCENT2_I, 
                              INP_LPP_QUISCENT2_D), CON_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_CONTENT2_I, 
                              INP_LPP_CONTENT2_D));
   }
   else
   {
      instl_msg (LOG_MSG, "  %-*s %-*s %-*s %-*s %-*s\n", OPTION_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_LABEL_I, INP_LPP_LABEL_D), 
                              FULL_LEVEL_LEN - 1, 
                      MSGSTR (MS_INSTALLP, INP_LPP_LEVEL_I, INP_LPP_LEVEL_D), 
                              TYPE_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_ACTION_I, INP_LPP_ACTION_D), 
                              QUI2_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_QUISCENT2_I, 
                              INP_LPP_QUISCENT2_D), CON_LEN, 
                      MSGSTR (MS_INSTALLP, INP_LPP_CONTENT2_I, 
                              INP_LPP_CONTENT2_D));
   }

   if (J_FLAG)
      instl_msg (LOG_MSG, "# %s\n", 
                  MSGSTR (MS_INSTALLP, INP_LISTHEAD_UL_I, INP_LISTHEAD_UL_D));
   else
      instl_msg (LOG_MSG, "  %s\n", 
                  MSGSTR (MS_INSTALLP, INP_LISTHEAD_UL_I, INP_LISTHEAD_UL_D));
}
