static char sccsid[] = "@(#)58  1.41.1.33  src/bos/usr/sbin/install/ckprereq/utils.c, cmdinstl, bos41B, 412_41B_sync 1/4/95 20:24:04";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *            check_failsop_for_special_cases
 *            check_for_illegal_superseding_parts
 *            construct_fix_name 
 *            convert_action_type_to_cp_flag_type
 *            convert_content_type_to_cp_flag_type
 *            convert_cp_flag_type_to_content_type 
 *            format_lpp_level
 *            get_barrier_from_sups_str
 *            get_fix_name_from_fix_info 
 *            get_superseding_fix
 *            get_superseding_name_or_lev
 *            is_compat_levels
 *            is_dummy_supd_vpd_rec
 *            same_fix 
 *            same_level
 *            strcmp_non_wsp
 *            Quit 
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

static int comparison_function (
const void * first_arg,
const void * second_arg);

/*--------------------------------------------------------------------------*
**
** NAME:      check_failsop_for_special_cases
**
** FUNCTION:  Scans the failsop looking for "missing" updates (updates which
**            were requested by the user but for which there is no entry
**            in the toc and no entry in the vpd).  Also looks for entries
**            for which a general error code has been used which needs
**            refining for better error messages.
**
** RETURNS:   void function
**
** SIDE EFFECTS:
**            Sets unresolved_updates and refine_failsop flags to TRUE if
**            appropriate.
**
**--------------------------------------------------------------------------*/

void
check_failsop_for_special_cases
(
Option_t * failsop,
boolean  * unresolved_updates_on_failsop,
boolean  * refine_failsop_errors,
short      parts_to_operate_on
)
{
   Option_t * op;

   for (op = failsop->next;
        op != NIL (Option_t);
        op = op->next)
   {
       if (check_prereq.mode == OP_APPLY) 
       {
          if (IS_UNRESOLVED_UPDATE (op, parts_to_operate_on))
             *unresolved_updates_on_failsop = TRUE;
       }
       else
          if (op->Status == STAT_NUTTIN_TO_COMMIT ||
              op->Status == STAT_NUTTIN_TO_REJECT)
             *refine_failsop_errors = TRUE;
   } /* end for */

} /* check_failsop_for_special_cases */

/*--------------------------------------------------------------------------*
**
** NAME:      check_for_illegal_superseding_parts
**
** FUNCTION:  checks the parts_seen flags against the pkg passed as 
**            arguments to see if an illegal supersedes case is present.
**
** RETURNS:   TRUE if there is an illegal supersedes.
**            FALSE otherwise
** SIDE EFFECTS:
**            Sets one of the flag arguments according to the parts in 
**            the pkg argument.
**
**--------------------------------------------------------------------------*/

boolean
check_for_illegal_superseding_parts
(
fix_info_type * pkg,
boolean       * seen_usr_only,
boolean       * seen_usr_root,
boolean       * seen_share_only
)
{
   if (pkg->origin == DUMMY_FIX ||
       pkg->origin == DUMMY_SUPERSEDED_FIX)
      return (FALSE);

   switch (pkg->parts_in_fix)
   {
      case (LPP_USER | LPP_ROOT):
         if (*seen_share_only)
            return (TRUE);
         (*seen_usr_root) = TRUE;
         break;

      case LPP_USER:
         if ((*seen_share_only) || (*seen_usr_root))
            return (TRUE);
         (*seen_usr_only) = TRUE;
         break;

      case LPP_SHARE:
         if ((*seen_usr_only) || (*seen_usr_root))
            return (TRUE);
         (*seen_share_only) = TRUE;
         break;

   } /* switch */
   return (FALSE);

} /* check_for_illegal_superseding_parts */

/*--------------------------------------------------------------------------*
**
** NAME: convert_content_type_to_cp_flag_type
**
** FUNCTION:  Converts a given argument from the Option_t -> content type
**            to the swvpd product -> cp_flag type.
**
** RETURNS:   Returns the converted version
**
**
**--------------------------------------------------------------------------*/

short 
convert_content_type_to_cp_flag_type 
(
char content
)
{
   short cp_flag;

   switch (content)
    {
      case CONTENT_BOTH :
        cp_flag = LPP_USER | LPP_ROOT;
        break;

      case CONTENT_MCODE :
        cp_flag = LPP_USER;      /* Treat as a USER part install. */
        break;

      case CONTENT_SHARE :
        cp_flag = LPP_SHARE;
        break;

      case CONTENT_MRI :
        cp_flag = 0;
        break;

      case CONTENT_OBJECT :
        cp_flag = 0;
        break;

      case CONTENT_PUBS :
        cp_flag = 0;
        break;

      case CONTENT_USR :
        cp_flag = LPP_USER;
        break;

      case CONTENT_UNKNOWN :
        cp_flag = 0;
        break;
    }

  return (cp_flag);

} /* end convert_content_type_to_cp_flag_type */

/*--------------------------------------------------------------------------*
**
** NAME: convert_action_type_to_cp_flag_type
**
** FUNCTION:  Converts a given argument from the BffRef_t -> action type
**            to the swvpd product -> cp_flag type.
**
** RETURNS:   Returns the converted version
**
**
**--------------------------------------------------------------------------*/

short 
convert_action_type_to_cp_flag_type 
 (
int action
)
{
   long cp_flag;

   switch (action)
    {
      case ACT_INSTALL :
         cp_flag = LPP_INSTAL;
         break;

      case ACT_SING_UPDT :
        cp_flag = LPP_UPDATE;
        break;

      case ACT_MULT_UPDT :
        cp_flag = LPP_UPDATE | LPP_PKG_PTF_TYPE_M;
        break;

      case ACT_GOLD_UPDT :
        cp_flag = LPP_UPDATE;
        break;

      case ACT_EN_PKG_UPDT :
        cp_flag = LPP_UPDATE | LPP_PKG_PTF_TYPE_E;
        break;

      case ACT_EN_MEM_UPDT :
        cp_flag = LPP_UPDATE;
        break;

      case ACT_INSTALLP_UPDT :
        cp_flag = LPP_UPDATE;
        break;

      case ACT_REQUIRED_UPDT :
        cp_flag = LPP_UPDATE;
        break;

      case ACT_CUM_UPDT :
        cp_flag = LPP_UPDATE | LPP_PKG_PTF_TYPE_C;
        break;

      default:
        cp_flag = 0;

    } /* end switch */

   return (cp_flag);

} /* end convert_action_type_to_cp_flag_type */

/*--------------------------------------------------------------------------*
**
** NAME: convert_cp_flag_type_to_content_type
**
** FUNCTION:  Converts a given argument from the swvpd_product -> cp_flag type
**            to the Option_t -> content type.
**
** RETURNS:   Returns the converted version
**
**--------------------------------------------------------------------------*/

int 
convert_cp_flag_type_to_content_type 
 (
short cp_flag
)
{
   int content;

   switch (cp_flag & (LPP_USER | LPP_ROOT | LPP_SHARE | LPP_MICRO) )
    {
      case LPP_USER :
         content = CONTENT_USR;
         break;

      case (LPP_USER | LPP_ROOT) :
         content = CONTENT_BOTH;
         break;

      case LPP_MICRO :              /* Should never get, should be LPP_USER */
         content = CONTENT_MCODE;
         break;

      case LPP_SHARE :
         content = CONTENT_SHARE;
         break;

      default :
         content = CONTENT_UNKNOWN;
         break;
    }

   return (content);

} /* end convert_cp_flag_type_to_content_type */

/*--------------------------------------------------------------------------*
**
** NAME: same_fix
**
** FUNCTION:  Determines if the two fixes are for the same package
**            or not.
**
** RETURNS:   Returns a boolean indicating the result of the test.
**
**--------------------------------------------------------------------------*/

boolean 
same_fix 
 (
fix_info_type * fix_1,
fix_info_type * fix_2
)
{
   if ( (strcmp (fix_1 -> level.ptf, fix_2 -> level.ptf) == 0)
                           &&
        (strcmp (fix_1 -> name, fix_2 -> name) == 0)
                           &&
        same_level (& (fix_1 -> level), & (fix_2 -> level) ) )
    {
      return (TRUE);
    }
   else
    {
      return (FALSE);
    }
} /* end same_fix */

/*--------------------------------------------------------------------------*
**
** NAME: same_level
**
** FUNCTION:  Determines if the two levels are for the same.
**
** RETURNS:   Returns a boolean indicating the result of the test.
**
**--------------------------------------------------------------------------*/

boolean 
same_level 
 (
Level_t * level_1,
Level_t * level_2
)
{
   if (
        (level_1 -> ver == level_2 -> ver)
                        &&
        (level_1 -> rel == level_2 -> rel)
                        &&
        (level_1 -> mod == level_2 -> mod)
                        &&
        (level_1 -> fix == level_2 -> fix) )
    {
      return (TRUE);
    }
   else
    {
      return (FALSE);
    }
} /* end same_level */

/*--------------------------------------------------------------------------*
**
** NAME: get_level_from_fix_info
**
** FUNCTION:  Generates a string with the level of the fix in the given node.
**
** RETURNS:   Returns a pointer to the string generated (this is to make
**            its use in printf easier).
**
**
**--------------------------------------------------------------------------*/
char * 
get_level_from_fix_info 
 (
fix_info_type * node,
char          * level
)
{
  if (node -> level.ptf[0] == '\0')
     sprintf (level, "%d.%d.%d.%d", node -> level.ver,
             node -> level.rel, node -> level.mod, node -> level.fix);
  else
     sprintf (level, "%d.%d.%d.%d.%s", node -> level.ver,
             node -> level.rel, node -> level.mod, node -> level.fix,
             node -> level.ptf);
  return (level);
} /* end get_level_from_fix_info */

/*--------------------------------------------------------------------------*
**
** NAME: get_barrier_from_sups_str
**
** FUNCTION:  Checks sups_str for the presence of opt_name.
**            If present, captures the level (v.r.m.f) which is assumed to
**            follow opt_name, and returns a pointer to the string.
**
** RETURNS:   A pointer to the string representing the level of opt_name
**            present in sups_str.  NIL  if opt_name is not present in sups_str.
**
**--------------------------------------------------------------------------*/

char * 
get_barrier_from_sups_str 
(
char  * opt_name,
char  * sups_str,
char  * barrier_lev
)
{
   FILE fp;
   char level [MAX_LEVEL_LEN];
   char name  [MAX_LPP_FULLNAME_LEN];

   if (sups_str [0] == '\0')  /* anything to process ? */
      return (NIL (char));

   if (strstr (sups_str, opt_name) == NIL (char))  /* is opt_name present? */
      return (NIL (char));

   /* make supersedes pointer look like a file */

   fp._flag = (_IOREAD | _IONOFD);
   fp._ptr = fp._base = (unsigned char *) sups_str;
   fp._cnt = strlen (sups_str);
   fp._file = _NFILE;

   *level = '\0';
   *name = '\0';
   *barrier_lev = '\0';

   while (fscanf (&fp, "%199s %99s", name, level) != EOF); 
   {
      if (strcmp (name, opt_name) == 0)
      {
         strcpy (barrier_lev, level);
         return (barrier_lev);
      }

   } 
   return (NIL (char));  /* must have been a substring, name doesn't match */
 
} /* get_barrier_str */

/*--------------------------------------------------------------------------*
**
** NAME: get_fix_name_from_fix_info
**
** FUNCTION:  formats a fix name so that it is suitable for printing.
**
** RETURNS:   A pointer to the given buffer (simplifies usage in printfs).
**
**--------------------------------------------------------------------------*/

char * 
get_fix_name_from_fix_info 
(
fix_info_type * const node,
char          * const buffer
)
{
   if (               (node -> origin != DUMMY_FIX)
                                      &&
        ! (check_prereq.called_from_command_line &&
                                           (node -> origin == COMMAND_LINE) ) )
    {
      return (construct_fix_name (node -> name, & node -> level, buffer));
    }
   else
    {
      return (construct_fix_name (node -> name, NIL (Level_t), buffer ) );
    }

} /* end get_fix_name_from_fix_info */

/*--------------------------------------------------------------------------*
**
** NAME: construct_fix_name
**
** FUNCTION:  formats a fix name so that it is suitable for printing.
**
** RETURNS:   A pointer to the given buffer (simplifies usage in printfs).
**
**--------------------------------------------------------------------------*/

char * 
construct_fix_name 
 (
char    * const name,
Level_t * const level,
char    * const buffer
)
{
   if (name != NIL (char) )
    {
      strcpy (buffer, name);
    }
   else
    {
      strcpy (buffer, "unknown name");
    }

   if (level != NIL (Level_t) )
    {
      strcat (buffer, " ");
      format_lpp_level (level, & buffer[strlen (buffer)]);
    }
   return (buffer);

} /* end get_fix_name_from_fix_info */

/*--------------------------------------------------------------------------*
**
** NAME: format_lpp_level
**
** FUNCTION:  formats an lpp_level so that it is suitable for printing.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
format_lpp_level 
 (
Level_t * const level,
char    * const buffer
)
{
   if (level -> ptf[0] == '\0')
    {
      sprintf (buffer, "%d.%d.%d.%d", level -> ver,
               level ->rel, level ->mod, level ->fix);
    }
   else
    {
      sprintf (buffer, "%d.%d.%d.%d.%s", level ->ver,
               level ->rel, level ->mod, level ->fix,
               level ->ptf);
    }

} /* end format_lpp_level */

/*--------------------------------------------------------------------------*
**
** NAME: Quit
**
** FUNCTION:  Causes execution to terminate, printing any messages or doing
**            cleanup as necessary.  Calls exit ().
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void Quit 
 (
int const return_code
)
{
   if (return_code != 0)

      /*  The ckprereq routine is exiting with an error. */

      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_EXIT_E, CKP_EXIT_D), inu_cmdname);
   exit (return_code);

} /* end Quit */

/*--------------------------------------------------------------------------*
**
** NAME: strcmp_non_wsp
**
** FUNCTION: Does a string compare ignoring white space.
**
** RETURNS:   True if the strings match.  False otherwise.
**
**--------------------------------------------------------------------------*/

boolean strcmp_non_wsp 
 (
char * source, 
char * dest
)
{
  boolean match;
  char * tmp_ptr;
  char * str1 = source;
  char * str2 = dest;

  match = TRUE;

  if (strcmp (str1, str2) == 0)
   {
     return (match);
   }

  while ( (* str1 != '\0') && (* str2 != '\0'))
   {

     if ( (* str1) != (* str2))
      {
        if (isspace (* str1))
         {
           str1++;
         }
        else
         if (isspace (* str2))
          {
             str2++;
          }
         else
          {
             match = FALSE;
             break;
          }
      }
    else
      {
        str1++;
        str2++;
      }
   } /* while */

  if (match)
   {
     for (tmp_ptr = str1; * tmp_ptr != '\0'; tmp_ptr++)
      {
          if (! isspace (* tmp_ptr))
           {
             match = FALSE;
             break;
           }
      }

     for (tmp_ptr = str2; * tmp_ptr != '\0'; tmp_ptr++)
      {
          if (! isspace (* tmp_ptr))
           {
             match = FALSE;
             break;
           }
      }
   }

  return (match);

} /* strcmp_non_wsp */

/*--------------------------------------------------------------------------*
**
** NAME: is_compat_levels
**
** FUNCTION:  Checks if the barrier/supersedes level specified by 
**            supersedes_string is <= to the level of target.
**
** RETURNS:   TRUE if it is (or if supersedes string is NULL), FALSE otherwise.
**
**--------------------------------------------------------------------------*/

boolean 
is_compat_levels 
(
char    * opt_name,
char    * supersedes_string,
Level_t * target
)
{
   Level_t    barrier_lev;
   char       barrier_str [MAX_LEVEL_LEN];

   if (get_barrier_from_sups_str (opt_name, 
                                  supersedes_string, 
                                  barrier_str) == NIL (char))
      return (TRUE);

   inu_level_convert (barrier_str, &barrier_lev);

   return (inu_level_compare (&barrier_lev, target) <= 0);

} /* is_compat_levels */

/*--------------------------------------------------------------------------*
**
** NAME: is_dummy_supd_vpd_rec
**
** FUNCTION: Checks if fix is an available, superseded vpd record.
**
** RETURNS:   TRUE if it is, FALSE otherwise.
**
**--------------------------------------------------------------------------*/

boolean 
is_dummy_supd_vpd_rec 
(
fix_info_type * fix
)
{

   return (fix -> apply_state == AVAILABLE &&
           fix -> superseded_by != NIL (supersede_list_type));

} /* end of is_dummy_supd_vpd_rec */

/*--------------------------------------------------------------------------*
**
** NAME:      get_superseding_fix
**
** FUNCTION:  Traverses the superseded_by chain of "fix" looking for the 
**            first usable fix in the graph.  
**            Returns:
**              The first applied in the chain OR
**              The first which needs its root part applied OR
**              The first to be explicitly applied OR
**              The first which can be applied (IN_TOC and ! doing auto sup) OR
**              The latest in the chain.
**
** RETURNS:   pointer to fix_info_structure
**
**--------------------------------------------------------------------------*/

fix_info_type * 
get_superseding_fix 
 (
fix_info_type * fix
)
{
   fix_info_type       * current_fix;
   supersede_list_type * supersede;

   for (supersede = fix->superseded_by;
        supersede != NIL (supersede_list_type);
        supersede = supersede->superseding_fix->superseded_by)
   {
      current_fix = supersede -> superseding_fix;
      if ((current_fix -> apply_state == ALL_PARTS_APPLIED)        ||
          (USR_ROOT_PKG_W_NO_ROOT_INSTLD(current_fix))             ||
          (current_fix -> apply_state == TO_BE_EXPLICITLY_APPLIED) ||
          (current_fix -> apply_state == IN_TOC)                   ||
          (current_fix -> superseded_by == NIL (supersede_list_type)))
      {
          return (current_fix);
      }
   } /* end for */

} /* get_superseding_fix */

/*--------------------------------------------------------------------------*
**
** NAME:      get_superseding_name_or_lev
**
** FUNCTION:  Traverses the fix_info_table from left to right starting from
**            fix looking for the first installed/being-installed entry
**            for this fileset.  If an explicit supersedes link for a base
**            level is detected, that path is used to continue the search.
**      
** RETURNS:   A string containing either the name of the superseding fileset
**            in the case of explicit supersedes of a base level, or the
**            level of the superseding fileset.
**
**--------------------------------------------------------------------------*/

char * 
get_superseding_name_or_lev
(
fix_info_type * superseded_fix,
char          * buf
)
{
   fix_info_type * reference_node;  
   fix_info_type * tmp;
  
   for (tmp = superseded_fix, reference_node = superseded_fix;
        (tmp != NIL (fix_info_type)) &&
             (strcmp (tmp->name, reference_node->name) == 0); 
         ) /* incremented in the loop */
   {
      if (tmp->apply_state == ALL_PARTS_APPLIED  ||
          tmp->apply_state == TO_BE_EXPLICITLY_APPLIED)
      {
         /*
          * This must be the superseding entry -- let's get its name if
          * its different from the superseded fix (mean it was a supersede
          * via a compat entry), else get its level.
          */
         if (strcmp (tmp->name, superseded_fix->name) != 0)
         {
            strcpy (buf, tmp->name);
            return (buf);
         }
         else
            return (get_level_from_fix_info (tmp, buf));
      }
      else
      {
         /*
          * Look up the supersedes chain of a compat entry if necessary.
          */
         if (IS_SUPD_COMPAT_ENTRY (tmp))
         {
            tmp = tmp->superseded_by->superseding_fix;
            reference_node = tmp;
         }
         else
         {
            tmp = tmp->next_fix;
         }
      }
   } /* end for */
  
   return (NIL (char)); /* should never really get here but it's not worth
                           erroring off if we do. */

} /* get_superseding_name_or_lev */

 /*----------------------------------------------------------------------*
  *
  * Function:    sort_fix_info_table
  *
  * Purpose:     To sort the fix_info_table by product name, installable
  *              option, ver, rel, mod, fix and ptf id.
  *
  * Returns:     This is a void function.
  *
  *----------------------------------------------------------------------*/

void 
sort_fix_info_table 
 (
boolean * error
)
{
   fix_info_type      * fix;         /* Current Op being Processed. */
   int                  fix_number;
   int                  number_of_fixes;
   fix_info_type     ** sort_array;

   /* Count the number of elements that need to be sorted. */

   number_of_fixes = 0;
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
       number_of_fixes++;
    }

   sort_array = (fix_info_type **) my_malloc (
                      number_of_fixes * (sizeof (fix_info_type *)), error);
   if (* error) {
      return;
   }

   /* Now copy pointers to corresponding elements in array. */

   fix_number = 0;
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
      sort_array[fix_number++] = fix;
    }


   /* Now call qsort to get the pointers in the correct sort order. */

   qsort (sort_array, number_of_fixes, sizeof (fix_info_type *),
          comparison_function);

   /* Now rebuild the linked list so that they are in sorted order. */

   fix = check_prereq.fix_info_anchor;
   for (fix_number = 0;
        fix_number < number_of_fixes;
        fix_number++)
    {
      fix -> next_fix = sort_array[fix_number];
      fix = fix -> next_fix;
    } /* end for */

   fix -> next_fix = NIL (fix_info_type);

   my_free (sort_array);

} /* end sort_fix_info_table */

/*----------------------------------------------------------------------*
 *
 * Function:    comparison_function
 *
 * Purpose:  Compares two fix records and determines their relative order.
 *
 * Returns:  Returns an int indicating which record is greater.
 *           -1 if first_arg is less, otherwise 1.
 *
 *----------------------------------------------------------------------*/

static int 
comparison_function 
(
const void * first_arg,
const void * second_arg
)
{
  fix_info_type * first_fix;
  fix_info_type * second_fix;
  short           rc;

  first_fix = * ( (fix_info_type **) first_arg);
  second_fix = * ( (fix_info_type **) second_arg);

 if ((rc = strcmp (first_fix -> name, second_fix -> name)) < 0)
     return (-1);
  else if(rc > 0)
     return (1);

  if (first_fix -> level.ver < second_fix -> level.ver)
     return (-1);

  if (first_fix -> level.ver > second_fix -> level.ver)
     return (1);

  if (first_fix -> level.rel < second_fix -> level.rel)
     return (-1);

  if (first_fix -> level.rel > second_fix -> level.rel)
     return (1);

  if (first_fix -> level.mod < second_fix -> level.mod)
     return (-1);

  if (first_fix -> level.mod > second_fix -> level.mod)
     return (1);

  if (first_fix -> level.fix < second_fix -> level.fix)
     return (-1);

  if (first_fix -> level.fix > second_fix -> level.fix)
     return (1);

  if (first_fix -> level.ptf[0] != '\0')
  {
     if ((second_fix->level.ptf[0] != '\0') &&
         (strcmp (first_fix -> level.ptf, second_fix -> level.ptf) < 0))
           return (-1);
        else
           return (1);
  }
  else
     if (second_fix->level.ptf[0] != '\0')
        return (-1);

  return (1);

} /* end comparison_function */

