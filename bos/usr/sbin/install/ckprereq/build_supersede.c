static char sccsid[] = "@(#)57  1.17  src/bos/usr/sbin/install/ckprereq/build_supersede.c, cmdinstl, bos411, 9440E411d 10/11/94 13:16:01";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:  
 *             add_supersede_info 
 *             add_to_superseded_list 
 *             build_base_sups_link
 *             build_expl_supersede_info 
 *             build_expl_sups_from_fix_info_table
 *             get_superseded_ptf
 *             process_expl_base_sups
 *             process_supersede_info
 *             unlink_supersede_info
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

#include <check_prereq.h>

static void add_supersede_info (
fix_info_type * superseded_fix,
fix_info_type * superseding_fix,
boolean       * error);

static void 
add_to_superseded_list (
fix_info_type       * superseding_fix,
supersede_list_type * superseded_by);

static void
build_base_sups_link (
fix_info_type * seding_base,
fix_info_type * seded_base,
boolean       * error );

static void 
build_expl_sups_from_fix_info_table (
boolean * error);

static void 
convert_cume_3_1_updates_to_supersedes (
boolean * error);

static char *
convert_supersedes_string (
char * supersede_info,
boolean * error);

static void 
get_superseded_ptf (
char * ptf,
char * supersede_info);

static void
process_expl_base_sups (
fix_info_type * seding_base,
boolean       * error );

static void 
process_supersede_info (
char    * product_name,
Level_t * superseding_level,
char    * supersede_info,
boolean * error);

static void 
unlink_supersede_info (
fix_info_type * superseded_fix,
fix_info_type * superseding_fix);

/*--------------------------------------------------------------------------*
**
** NAME:      build_expl_supersede_info
**
** FUNCTION:  This routine is called at startup to seed our fix_info table
**            with all it needs to know about superseding fixes from the VPD
**            or on the media.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
build_expl_supersede_info (
boolean * error
)
{
   fix_info_type * fix;
   Option_t      * option;
   char          * str_ptr;
   char          * supersede_info;
   char          * superseded_ptfs;
   int             ptf_length;

   /*
    * OP_APPLY:  Traverse the toc list passed from installp.  Use
    * data in the supersedes field of a toc entry ("who I supersede" for
    * 3.2 updates) to construct supersedes links between nodes in the 
    * fix_info table.  Then traverse the fix_info table looking for available
    * MC type entries from the vpd which may be treated like a ptf
    * from the toc w.r.t apply operation (This is a 3.2 feature which
    * should not be a applicable on most 4.1 installations -- maybe on 
    * a migrated system.)
    *
    * ALL OPS:  Traverse the fix_info table processing supersedes
    * info from the vpd (sceded_by info for 3.2 updates) AND 
    * 4.1 compatibility entries (supersedes info for 4.1 base levels).
    * This is done in the call to build_expl_sups_from_fix_info_table().
    * A compatibility entry is added to the fix_info table corresponding to
    * the info from the supersedes data in the 4.1 base level.  A supersedes
    * link is created between the compat entry and the base level 
    * containing the compatibility entry.  The compat entry will be used to
    * resolve requisites on filesets whose name changed between 3.2 and 4.1.
    */
   if (check_prereq.mode == OP_APPLY)
    {
      if (check_prereq.First_Option != NIL (Option_t) )
       {
         for (option = check_prereq.First_Option -> next;
              option != NIL (Option_t);
              option = option -> next)
          {
            if (IF_4_1 (option->op_type))
               /* 
                * Handle any explicit supersedes for 4.1 filesets 
                * (base levels) when traversing the fix_info table in
                * build_expl_sups_from_fix_info_table.
                */
               continue;

            if (check_prereq.called_from_ls_programs) {
               superseded_ptfs = convert_supersedes_string (option->supersedes,
                                                            error);
               RETURN_ON_ERROR;
            } else {
               superseded_ptfs = option->supersedes;
            }

            process_supersede_info (option -> name, & (option -> level),
                                    superseded_ptfs, error);
            RETURN_ON_ERROR;
          } /* end for */
       } /* end if */

       if (!check_prereq.called_from_command_line)
       {
          convert_cume_3_1_updates_to_supersedes (error);
          RETURN_ON_ERROR;
       }
    } /* end if */


   /* Now, take into account supersede info in the fix_info table (typically
      sceded_by info from the vpd for 3.2 updates but also compatibility 
      supersedes info for 4.1 base levels).  Build superseded_by
      and supersedes lists for every node involved.  If a superseding fix
      is applied, then remove any older supersedes info that came in from the
      media. */

   build_expl_sups_from_fix_info_table (error);
   RETURN_ON_ERROR;

} /* end build_expl_supersede_info */

/*--------------------------------------------------------------------------*
**
** NAME:      process_supersede_info
**
** FUNCTION:  This routine constructs dictionary entries from the given
**            product_name, level and supersede_info.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
process_supersede_info 
 (
char    * product_name,
Level_t * superseding_level,
char    * supersede_info,
boolean * error
)
{
  char             fix_name[MAX_LPP_FULLNAME_LEN];
   fix_info_type * superseded_fix;
   Level_t         superseded_level;
   fix_info_type * superseding_fix;

  if ( (supersede_info != NIL (char) )
                  &&
       (supersede_info[0] != '\0') )
   {
     superseded_level.ver = superseding_level -> ver;
     superseded_level.rel = superseding_level -> rel;
     superseded_level.mod = superseding_level -> mod;
     superseded_level.fix = superseding_level -> fix;

     superseding_fix = locate_fix (product_name, superseding_level, USE_FIXID);

     get_superseded_ptf (superseded_level.ptf, supersede_info);
     while (superseded_level.ptf[0] != '\0')
      {
         /* Because of potential differences in fix id for different
            superseding ptfs, we need to make sure we use any existing level
            for the superseded ptf (rather than default to the superseding
            ptf).  This allows ptfs with different fix ids to supersede one
            another, when both are available on the media. */

         superseded_fix = locate_fix (product_name, & superseded_level, 
                                      IGNORE_FIXID);

         if (superseding_fix == NIL (fix_info_type) )
          {
            /*"Error: Cannot find superseding fix for : %s in VPD.\n
               \tsuperseding fix is: %s.\n"*/

            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SUP_FIX_NOT_IN_VPD_E,
                     CKP_SUP_FIX_NOT_IN_VPD_D), inu_cmdname,
                     construct_fix_name (product_name,
                                         superseding_level, fix_name),
                     superseding_level -> ptf);
            * error = TRUE;
            check_prereq.function_return_code = INTERNAL_ERROR_RC;
            return;
          }

         if (superseded_fix == NIL (fix_info_type) )
          {
            /* Create a dummy superseded fix so that if anything prereqs it,
               it will find it and eventually this superseding fix. */

            superseded_fix = create_fix_info_node (error);
            RETURN_ON_ERROR;

            superseded_fix -> name = dupe_string (product_name, error);
            RETURN_ON_ERROR;

            superseded_fix -> level = superseded_level;
            superseded_fix -> apply_state = AVAILABLE;
            superseded_fix -> commit_state = CANNOT_COMMIT;
            superseded_fix -> reject_state = DONT_NEED_TO_REJECT;
            superseded_fix -> origin = DUMMY_SUPERSEDED_FIX;
            add_fix_to_fix_info_table (superseded_fix, error);
            RETURN_ON_ERROR;

          } /* end if */

         add_supersede_info (superseded_fix, superseding_fix, error);
         RETURN_ON_ERROR;

         get_superseded_ptf (superseded_level.ptf, supersede_info);
       } /* end while */
    } /* end if */
} /* end process_supersede_info */

/*--------------------------------------------------------------------------*
**
** NAME:      add_supersede_info
**
** FUNCTION:  Adds the supersede info to the supersede tree in the fix_info
**            table.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void add_supersede_info 
(
fix_info_type * superseded_fix,
fix_info_type * superseding_fix,
boolean       * error
)
{
   supersede_list_type * superseded_by;

   /* Is the superseded_fix already superseded by somebody? */

   if (superseded_fix -> superseded_by != NIL (supersede_list_type) )
    {
      /* Is the superseded_fix already superseded by the superseding_fix? */

      if (superseded_fix -> superseded_by -> superseding_fix ==
                                                               superseding_fix)
       {
         return;   /* Don't do it again. */
       }
      /* If we are superseded by something that in turn supersedes our
         superseding_fix, then switch to the superseding_fix. */

      if (superseded_fix -> superseded_by -> superseding_fix ==
                           superseding_fix -> superseded_by -> superseding_fix)
       {
         unlink_supersede_info (superseded_fix, superseding_fix);
         add_to_superseded_list (superseding_fix, superseded_fix->superseded_by);
       }

      /* Otherwise, we must be superseded by something that is superseded by
         our superseding fix.  Do nothing. */

      return;
    }
   /* In this situation, our superseded_fix has never been superseded.
      Generate a supersede node and link in. */

   superseded_by = my_malloc (sizeof (supersede_list_type), error);
   RETURN_ON_ERROR;

   superseded_fix -> superseded_by = superseded_by;
   superseded_by -> superseded_fix = superseded_fix;
   add_to_superseded_list (superseding_fix, superseded_by);
   return;

} /* end add_supersede_info */

/*--------------------------------------------------------------------------*
**
** NAME:      unlink_supersede_info
**
** FUNCTION:  Unlinks the superseded_fix from the superseding_fix's supersedes
**            list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static 
void unlink_supersede_info 
 (
fix_info_type * superseded_fix,
fix_info_type * superseding_fix
)
{
   supersede_list_type * superseded_by;

  /***
   *** Structure diagram:  Empty boxes are fix_info_type nodes (our data)
   ***                     Filled-in boxes (with +'s) are supersedes nodes. 
   ***                     Link's are numbered for discussion.
   ***
   ***                         seding_fix                               
                 +-----------------9---------------+
                 v                                 |
              +----+  supersedes                   |
              | U2 | -------1-------+              |
              +----+ <------2-----+ |              |
                      seding_fix  | v    next      |
                                 +++++ -----5----> +++++
                                 ++X++   prev      ++Y++    seded_fix
                +--------3-----> +++++ <----6----- +++++ ------7------+
                | superseded_by   |                ^                  |
                |                 |                |                  v
              +----+   seded_fix  |                | superseded_by  +----+
              | U1 | <---4--------+                +------8-------- | U0 |
              +----+                                                +----+

         Given U1 and U2, this routine:
                          Makes 1 point to Y 
                          Clears 5 and 6.
                          (Does not clear 2, probably should).
         Given U0 and U2, this routine:
                          Clears 5 and 6.
                          (Does not clear 9, probably should).
    *** 
    ***
    ***
    ***/

   /*
    * Get the supersede node (NOT A FIX_INFO STRUCTURE) of the superseded_fix.
    */
   superseded_by = superseded_fix -> superseded_by;

   /* 
    * Delete the supersede node from it's doubly linked supersedes_list. 
    * (Recall: as well as who we are sceded_by and who we supersede, we 
    *  keep a next and previous list in case we supersede multiple things.)
    */
   if (superseded_by -> previous_supersede != NIL (supersede_list_type) )
   {
      /*
       * There's a previous node in the list of supersedes node.  "My previous 
       * node (another node pointing to the folks that are supersededed by
       * my superseding_fix) inherits my next node."
       */
      superseded_by -> previous_supersede -> next_supersede =
                                               superseded_by -> next_supersede;
   }
   else
   {
      /*
       * No previous node.  "My superseding fix acquires the next node in 
       * my list of supersedes nodes (if I have a next node)."
       */
      superseded_by -> superseding_fix -> supersedes =
                                               superseded_by -> next_supersede;
   }

   if (superseded_by -> next_supersede != NIL (supersede_list_type) )
   {
      /* 
       * There's a next node in the list of supersedes node.  "My next node 
       * inherits my previous node.
       */
      superseded_by -> next_supersede -> previous_supersede =
                                           superseded_by -> previous_supersede;
   }
   /*
    * Zap the next and previous pointers.
    */
   superseded_by -> next_supersede = NIL (supersede_list_type);
   superseded_by -> previous_supersede = NIL (supersede_list_type);

 } /* end unlink_supersede_info */

/*--------------------------------------------------------------------------*
**
** NAME:      add_to_superseded_list
**
** FUNCTION:  Adds the given superseded_by node to the doubly linked
**            supersedes list pointed to by superseding_fix.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
add_to_superseded_list (
fix_info_type       * superseding_fix,
supersede_list_type * superseded
)
{
  
  /***
   *** Structure diagram:  Empty boxes are fix_info_type nodes (our data)
   ***                     Filled-in boxes (with +'s) are supersedes nodes. 
   ***                     Link's are numbered for discussion.
   ***
   ***                         seding_fix                               
                 +-----------------9---------------+
                 v                                 |
              +----+  supersedes                   |
              | U2 | -------1-------+              |
              +----+ <------2-----+ |              |
                      seding_fix  | v    next      |
                                 +++++ -----5----> +++++
                                 ++X++   prev      ++Y++    seded_fix
                +--------3-----> +++++ <----6----- +++++ ------7------+
                | superseded_by   |                ^                  |
                |                 |                |                  v
              +----+   seded_fix  |                | superseded_by  +----+
              | U1 | <---4--------+                +------8-------- | U0 |
              +----+                                                +----+

         Given U2 and X (3, 4, 7, 8, 9 already exist), 
                          this routine:
                          Creates 1, 2, 5 and 6)
    *** 
    ***
    ***
    ***/

   /*
    * Create link to superseding fix. (link 2)
    */
   superseded -> superseding_fix = superseding_fix;
   /* 
    * Create link to any other superseded nodes of superseding fix (link 5)
    */  
   superseded -> next_supersede = superseding_fix -> supersedes;
   /* 
    * If there were other superseded nodes of superseding fix.... (node Y)
    */
   if (superseding_fix -> supersedes != NIL (supersede_list_type) )
      /* 
       * Point back to newly added supersedes node (link 6).
       */
      superseding_fix -> supersedes -> previous_supersede = superseded;

   /*
    * Point from the superseding fix to the newly added supersedes node.
    */
   superseding_fix -> supersedes = superseded;
   /*
    * Nullify the previous pointer of the newly added node.
    */
   superseded -> previous_supersede = NIL (supersede_list_type);
} /* end add_to_superseded_list */

/*--------------------------------------------------------------------------*
**
** NAME:      build_expl_sups_from_fix_info_table
**
** FUNCTION:  This routine looks for supersedes information in one of two 
**            places: in the superseding_ptf field of installed 3.2 updates
**            (ie. sceded_by field from the vpd) and the supersedes_string
**            field of 4.1 base levels (ie. compatibility entries for name
**            changes between 3.2 and 4.1 lpp options).  This information is
**            converted to a superseded_by lists for more convenient handling.
**
**            In addition to recording what fix supersedes another fix,
**            this routine also notes what fixes are superseded by a fix by
**            building the supersedes list in the fix_info table.
**
**            When a fix is superseded by some fix(es) on the media, the
**            superseded_by list will already have some entries on it.  If a
**            fix is already superseded, any older superseding fixes are
**            removed from the list so that we don't try to apply something
**            that has already been superseded by something on the system.
**            In the case of explicit base level supersedes, the removal of
**            the superseded entries is handled in evaluate_base_levels().
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
build_expl_sups_from_fix_info_table 
(
boolean * error
)
{
   supersede_list_type * a_superseding_fix;
   fix_info_type       * fix;
   boolean               fix_found;
   char                  fix_name[MAX_LPP_FULLNAME_LEN];
   Level_t               level;
   fix_info_type       * superseding_fix;

   /* Build superseded_by and supersedes lists for every node involved.  Also,
      if a superseding fix is applied, then remove any older supersedes info
      that came in from the media. */

   for (fix = check_prereq.fix_info_anchor->next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {

       /*
        * No need to process 4.1 filesets, unless its a base level with a
        * supersedes/compat entry.
        */
       if (IF_4_1 (fix->op_type))
       {
          if ((IF_INSTALL (fix->op_type))           &&
              (fix->supersedes_string[0] != '\0')   &&
              (!check_prereq.called_from_lslpp))
          {
             process_expl_base_sups(fix, error); 
             RETURN_ON_ERROR;
          }
          continue;
       }

      /* We prefer for our fixes to be ACTIVELY superseded in the VPD.
         However, if we are about to actively supersede a fix,  in particular,
         when we want to apply a superseded root part for usr-root consistency,
         we want to build the superseded_by and supersedes lists also. */

      if (    (fix -> superseding_ptf[0] == '\0')
                                &&
          (fix -> superseded_by != NIL (supersede_list_type))
                                &&
          (fix -> apply_state == PARTIALLY_APPLIED) )
       {
         superseding_fix = fix -> superseded_by -> superseding_fix;
         if (superseding_fix -> apply_state == TO_BE_EXPLICITLY_APPLIED)
          {
             strcpy (fix -> superseding_ptf, superseding_fix -> level.ptf);
          }
       }

      if (fix -> superseding_ptf[0] != '\0')
       {
         level = fix -> level;
         strcpy (level.ptf, fix -> superseding_ptf);
         superseding_fix = locate_fix (fix -> name, & level, IGNORE_FIXID);
         if (superseding_fix == NIL (fix_info_type) )
          {
            /*"Error: Can not find superseding fix for : %s in VPD.\n
               \tsuperseding fix is: %s.\n"*/

            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SUP_FIX_NOT_IN_VPD_E,
                     CKP_SUP_FIX_NOT_IN_VPD_D), inu_cmdname,
                     get_fix_name_from_fix_info (fix, fix_name),
                     fix -> superseding_ptf);
            * error = TRUE;
            check_prereq.function_return_code = INTERNAL_ERROR_RC;
            return;
          }

         /* See if this superseding fix is already known about.  Do this by
            running up the superseding fix tree. */

         fix_found = FALSE;
         a_superseding_fix = fix -> superseded_by;
         while (a_superseding_fix != NIL (supersede_list_type) )
          {
            if (a_superseding_fix -> superseding_fix == superseding_fix)
             {
               fix_found = TRUE;
               break;
             }
            a_superseding_fix =
                         a_superseding_fix -> superseding_fix -> superseded_by;
          } /* end while */

         if (fix_found)
          {
            if (fix->superseded_by == NIL (supersede_list_type))
            {
               /* Since this superseding fix ACTIVELY supersedes me, make me 
                  point to it. */
               fix -> superseded_by = a_superseding_fix;
            }
            else
            {
               unlink_supersede_info (fix ,
                                      fix -> superseded_by -> superseding_fix);
               add_to_superseded_list (superseding_fix, fix -> superseded_by);
            }
               
          }
         else
          {
            /* Means that we have been superseded by something that was not
               on our media.  We could argue that this superseding fix must
               supersede anything that supersedes this fix on the media, but
               let's not get into that. */

            if (fix -> superseded_by != NIL (supersede_list_type) )
             {
               unlink_supersede_info (fix ,
                                      fix -> superseded_by -> superseding_fix);
               add_to_superseded_list (superseding_fix, fix -> superseded_by);
             }
            else
             {
               add_supersede_info (fix, superseding_fix, error);
               RETURN_ON_ERROR;
             }
          } /* end if */

       }
    } /* end for */

} /* end build_expl_sups_from_fix_info_table */

/*--------------------------------------------------------------------------*
**
** NAME:      get_superseded_ptf
**
** FUNCTION:  Scans for the next
**            and returns a pointer to the entry if found.  Otherwise, null
**            is returned.
**
** RETURNS:   Returns a pointer to the entry found (or NULL).
**
**--------------------------------------------------------------------------*/

static void 
get_superseded_ptf 
(
char * ptf,
char * supersede_info
)
{
          int        ptf_length;
   static char     * old_supersede_info;
   static char     * superseded_ptfs;

   if (supersede_info != old_supersede_info)
    {
      superseded_ptfs = supersede_info;
      old_supersede_info = supersede_info;
    }

   if (* superseded_ptfs == '\0')
    {
      ptf[0] = '\0';
    }
   else
    {
      /* Skip over leading blanks. */

      superseded_ptfs += strspn (superseded_ptfs, " \n\t");

      /* Copy the ptf to the level field. */

      ptf_length = strcspn (superseded_ptfs, " \n\t");
      strncpy (ptf, superseded_ptfs, ptf_length);
      ptf[ptf_length] = '\0';

      /* Point to next token. */

      superseded_ptfs += ptf_length;
    }

} /* end get_superseded_ptf */

/*--------------------------------------------------------------------------*
**
** NAME: convert_cume_3_1_updates_to_supersedes
**
** FUNCTION:  We look to see if we have multiple updates for the same
**            3.1 product.  If we do, we convert these updates to superseding
**            fixes so that convert_supersedes_to_prereqs can eliminate
**            any stupid duplication (remember that a 3.1 update is
**            a cumulative update).
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void
convert_cume_3_1_updates_to_supersedes 
 (
boolean * error
)
{
   fix_info_type       * fix;
   fix_info_type       * fix_2;

   for (fix = check_prereq.fix_info_anchor;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
      if (IF_3_1_UPDATE (fix -> op_type) )
       {
         for (fix_2 = fix -> collision_list;
              fix_2 != NIL (fix_info_type);
              fix_2 = fix_2 -> collision_list)
          {
             if (      IF_3_1_UPDATE (fix_2 -> op_type)
                                     &&
                  (fix -> level.ver == fix_2 -> level.ver)
                                     &&
                  (fix -> level.rel == fix_2 -> level.rel) )
              {
                if ( (fix -> level.mod > fix_2 -> level.mod)
                                    ||
                     ( (fix -> level.mod == fix_2 -> level.mod) &&
                                    (fix -> level.fix > fix_2 -> level.fix) ) )
                 {
                   /* fix supersedes fix_2 */

                   add_supersede_info (fix_2, fix, error);
                   RETURN_ON_ERROR;
                 }
                else
                 {
                   /* fix_2 supersedes fix */

                   add_supersede_info (fix, fix_2, error);
                   RETURN_ON_ERROR;
                 }
              } /* end if */
          }
       }

    } /* end for */

} /* end convert_cume_3_1_updates_to_supersedes */

/*--------------------------------------------------------------------------*
**
** NAME: convert_supersedes_string
**
** FUNCTION:  Converts a string of the form 
**            <option> <ptfid> <option> <ptfid>....
**            to <ptfid> <ptfid>....
**            Required because cume ptfs store supersedes info in "raw" form
**            and normal supersedes processing expects this "filtered" form.
**            Also required for ls_program processing -- a toc entry also
**            contains supersede info in "raw" format.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static char *
convert_supersedes_string 
 (
char * supersede_info,
boolean * error 
)
{
   char * str_ptr;
   char * superseded_ptfs;
   short  ptf_length;
   short  tmp_len;

   superseded_ptfs = NIL (char);
   str_ptr = supersede_info;

   /* Skip over leading blanks. */
   str_ptr += strspn (supersede_info, " \n\t");
   while ((*str_ptr) != '\0')
    {
      /* Skip over the option name. */
      str_ptr += strcspn (str_ptr, " \n\t");

      /* Skip over the whitespace between option name and ptf. */
      str_ptr += strspn (str_ptr, " \n\t");

      /* Add the ptf to our superseded string. */
      ptf_length = strcspn (str_ptr, " \n\t");
      if (superseded_ptfs == NIL (char))
       {
         superseded_ptfs = my_malloc (ptf_length + 1, error);
         * superseded_ptfs = '\0';
       }
      else
       {
         tmp_len = strlen (superseded_ptfs);
         superseded_ptfs = my_realloc (superseded_ptfs, 
                                       tmp_len + ptf_length + 1, error);
         superseded_ptfs [tmp_len] = '\0';
         strcat (superseded_ptfs, " "); /* Add blank separator. */
       }
      RETURN_ON_ERROR;

      strncat (superseded_ptfs, str_ptr, ptf_length);

      /* Skip over the ptf. */
      str_ptr += ptf_length;

     /* Skip over white space (to beginning of next option name). */
      str_ptr += strspn (str_ptr, " \n\t");

    } /* end while */

    return (superseded_ptfs);
 
} /* convert_supersedes_string */

/*--------------------------------------------------------------------------*
**
** NAME: process_expl_base_sups
**
** FUNCTION:  Given a base level fileset with a non-blank supersedes string, 
**            this routine parses that string looking for entries with 
**            different names from the provided argument.  A dummy fix_info
**            structure is created for the compat/supersedes entry and a
**            supersedes link is created between the structure and the
**            superseded entry.  Any barrier information (entries with the
**            same name as the argument provided) in the supersedes data
**            is parsed and only checked against a few error conditions.
**            Compatibility data is stripped from the supersedes_string,
**            and the barrier info, if any is left in tact.
**       
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void
process_expl_base_sups 
(
fix_info_type * seding_base,
boolean       * error 
)
{

   FILE     fp;
   char     level [MAX_LEVEL_LEN];
   char     barrier_lev [MAX_LEVEL_LEN];
   char     name_buf [MAX_LPP_FULLNAME_LEN];
   fix_info_type * rm_compat_entry;
   fix_info_type * seded_base;
   Level_t  supd_level;
   short    barrier_count=0;
   short    sups_count=0;

   /*
    * RULES:
    * 1. illegal to have multiple entries with same name as superseding
    *    base level. (multiple barrier entries are illegal).
    * 2. illegal to have multiple entries with different name as superseding
    *    base level (ie. "many-to-one" migration or multiple 
    *    supersedes/compat entries not currently supported here or 
    *    elsewhere in the code).
    * 3. A compat entry must NOT match any "real" entries in the fix_info table.
    */
   /*
    * Make the supersedes string look like a file for easy parsing.
    */
   fp._flag = (_IOREAD | _IONOFD);
   fp._ptr = fp._base = (unsigned char *) seding_base->supersedes_string;
   fp._cnt = strlen (seding_base->supersedes_string);
   fp._file = _NFILE;

   /*
    * Loop until end of file/sups string:
    *    if entry has same name, must be barrier entry.
    *       save, increment count, perform error check.
    *    else, must be compat/supersedes entry.
    *       increment count, perform error checks, process if ok
    */ 
   while (fscanf (&fp, "%199s %99s", name_buf, level) != EOF)
   {
      if (strcmp (seding_base->name, name_buf) == 0)
      {
         barrier_count++;
         if (barrier_count > 1)
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_MULT_BARR_E,
                     CKP_NO_MULT_BARR_D), inu_cmdname,
                     get_fix_name_from_fix_info (seding_base, name_buf));
            *error = TRUE;
            return;
         }
         strcpy (barrier_lev, level);
      }
      else
      {
         sups_count++;
         /*
          * Look up the supersedes/compat entry.  If not already in 
          * fix_info table, create a dummy fix.
          */
         inu_level_convert (level, &supd_level);
         if ((seded_base = locate_fix (name_buf, &supd_level, TRUE))
                                                       == NIL (fix_info_type))
         {
            create_DUMMY_FIX (&seded_base, name_buf, &supd_level, error);
            RETURN_ON_ERROR;
            seded_base->op_type = OP_TYPE_INSTALL;
            seded_base->op_type |= OP_TYPE_4_1;
            seded_base->cp_flag =  LPP_COMPAT;
            add_fix_to_fix_info_table (seded_base, error);
            RETURN_ON_ERROR;

            /*
             * Now that we've added the dummy compat entry to the 
             * fix_info table, lets do a little error checking and
             * perhaps a little error correction.
             *
             * The goal is to make sure that the compat entry does not
             * have a level less than any that may be known about (on the
             * media or installed).  If it does, then the lpp has committed
             * a violation of the rules (since the compat level must be
             * greater than all levels released for the old fileset).
             * 
             * Check this simply by looking at the next element in the
             * sorted table.
             *
             * We can correct the problem if the existing element is itself
             * a compat entry.  We'll assume that the newer compat entry
             * is that which is desired.
             */
            rm_compat_entry = NIL (fix_info_type);
            if ((seded_base->next_fix != NIL (fix_info_type)) &&
                (strcmp (seded_base->name, seded_base->next_fix->name) == 0))
            {
               if (IS_SUPD_COMPAT_ENTRY (seded_base->next_fix))
               {
                  /* We'll be removing the element we just added. */
                  rm_compat_entry = seded_base;      
 
                  /* The newer compat entry will be used. */ 
                  seded_base = seded_base->next_fix;
               }
               else
               {
                  /*
                   * Compat entry can't have lower level than anything on the 
                   * media with the same name.  Use the NO_MATCH msg which is 
                   * generic enough to handle this case.
                   */
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_COMPAT_MATCH_E,
                           CKP_NO_COMPAT_MATCH_D), inu_cmdname,
                           get_fix_name_from_fix_info (seding_base, name_buf));
                  /*
                   * Recover from the error.  Remove the compat entry from the 
                   * fix_info table.
                   */
                  if (seded_base->next_fix != NIL (fix_info_type))
                      seded_base->next_fix->prev_fix = 
                                           seded_base->prev_fix;
                  seded_base->prev_fix->next_fix = seded_base->next_fix;
                  free_fix_info_node (seded_base);

                  /*
                   * hash table needs to be rebuilt to adjust for element
                   * which we removed.
                   */
                  destroy_hash_table ();
                  create_hash_table(error);
                  RETURN_ON_ERROR;
                  load_fix_hash_table (check_prereq.fix_info_anchor, error);
                  RETURN_ON_ERROR;
                  return;
               }
            }
            else
            {
               /*
                * Again, we want to make sure there's only one compat entry
                * for a given fileset.  If the previous element to the one
                * we just added is a compat entry with the same name, we
                * want to remove it.
                */
               if ((seded_base->prev_fix != NIL (fix_info_type)) 
                                          &&
                   (strcmp (seded_base->name, seded_base->prev_fix->name) == 0)
                                          &&
                   (IS_SUPD_COMPAT_ENTRY (seded_base->prev_fix)))
               {
                  /* We'll be removing the older compat entry. */
                  rm_compat_entry = seded_base->prev_fix;

                  /* preserve the supersedes links. */
                  seded_base->superseded_by = rm_compat_entry->superseded_by;
                  seded_base->superseded_by->superseded_fix=seded_base;
                  rm_compat_entry->superseded_by = NIL(supersede_list_type);
               }
            }

            /*
             * Remove extraneous compat entry if we detected one above.
             */
            if (rm_compat_entry != NIL (fix_info_type))
            {
               /* adjust pointers around the compat entry */
               if (rm_compat_entry->next_fix != NIL (fix_info_type))
                   rm_compat_entry->next_fix->prev_fix = 
                                           rm_compat_entry->prev_fix;
               rm_compat_entry->prev_fix->next_fix = rm_compat_entry->next_fix;
               free_fix_info_node (rm_compat_entry);

               /*
                * hash table needs to be rebuilt to adjust for element
                * which we removed.
                */
               destroy_hash_table ();
               create_hash_table(error);
               RETURN_ON_ERROR;
               load_fix_hash_table (check_prereq.fix_info_anchor, error);
               RETURN_ON_ERROR;
            } 
         } 
         else 
         { 
            /*
             * A match already exists for the compat entry meaning another
             * fileset may already have this as a compat entry -- make sure 
             * it's not a real fileset.
             */
            if (!IS_SUPD_COMPAT_ENTRY (seded_base))
            {
               /*
                * Compat entry can't match anything on media or installed.
                */
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_COMPAT_MATCH_E,
                        CKP_NO_COMPAT_MATCH_D), inu_cmdname,
                        get_fix_name_from_fix_info (seding_base, name_buf));
               /*
                * No hard error necessary.  Explicit supersedes matching
                * will not take place for this base level.
                */
               return;
            }
         }
         build_base_sups_link (seding_base, seded_base, error);
         RETURN_ON_ERROR;
      } /* if (strcmp... */
   } /* while */

   if (sups_count == 0)  /* must only be a barrier in sups string. */
      return;

   if (sups_count > 1)
   {
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_SUPPORT_MANY_COMPAT_E,
                  CKP_NO_SUPPORT_MANY_COMPAT_D), inu_cmdname,
                  get_fix_name_from_fix_info (seding_base, name_buf));
      *error = TRUE;
      return;
   }
   /*
    * Free the supersedes_string and replace with the barrier string (if one 
    * existed).
    */
   free (seding_base->supersedes_string);
   if (barrier_count > 0)
   { 
      sprintf (name_buf, "%s %s", seding_base->name, barrier_lev);
      seding_base->supersedes_string = strdup (name_buf);
   }
   else
      seding_base->supersedes_string = NIL (char);  

}  /* process_expl_base_sups */

/*--------------------------------------------------------------------------*
**
** NAME: build_base_sups_link
**
** FUNCTION:  Creates the supersedes link between seded_base (compat entry)
**            and seding_base if one doesn't already exist.
**
** RETURNS:   void function.
**
**--------------------------------------------------------------------------*/

static void
build_base_sups_link 
(
fix_info_type * seding_base,
fix_info_type * seded_base,
boolean       * error 
)
{
   fix_info_type * existing_sup;

   if (seded_base->superseded_by != NIL (supersede_list_type))
   {
      existing_sup = seded_base->superseded_by->superseding_fix;
      /*
       * The compat entry is already superseded by another base level.  
       * If it's an older version of the same option, that's ok -- return
       *    (processing in evaluate_base_levels will take both 
       *    base levels into account)
       * If the state of the existing superseding entry is the same as or
       *    "better" (lower) than the current superseding entry, leave 
       *    the existing supersedes relationship in tact. 
       *    exs.  A.new.opt1 --sups--> A.old (compat)
       *          A.new.opt2 --sups--> A.old (compat)
       *          If opt1 is APPLIED  && opt2 is IN_TOC || TO_BE_EXPL_APPLD
       *          -- A.new.opt1 will sup A.old
       *          if opt2 is APPLIED && opt2 is IN_TOC || TO_BE_EXPL_APPLD 
       *          -- A.new.opt2 will sup A.old
       */
      if (strcmp (seding_base->name, existing_sup->name) == 0)
      {
         return;
      }
      else
      {
         if ((check_prereq.mode != OP_APPLY) ||
             (existing_sup->apply_state <= seding_base->apply_state))
            /*
             * Use existing entry.
             */
            return;
         else
         {
            /*
             * Let new option sup existing compat entry.
             */
            seding_base->supersedes = existing_sup->supersedes;
            existing_sup->supersedes = NIL (supersede_list_type);
            seded_base->superseded_by->superseding_fix = seding_base;    
         }
      }
   }
   else
   {
      /*
       * No supersedes exists yet for this compat entry.  Create link.
       */
      seded_base->superseded_by = 
                              my_malloc (sizeof (supersede_list_type), error);
      RETURN_ON_ERROR;
      seded_base->superseded_by->next_supersede = NIL (supersede_list_type);
      seded_base->superseded_by->previous_supersede = NIL (supersede_list_type);
      seded_base->superseded_by->superseding_fix = seding_base;
      seded_base->superseded_by->superseded_fix = seded_base;
      seding_base->supersedes = seded_base->superseded_by;
   }

   /*
    * Set the state of the compat entry.  Let it be the same as the superseding
    * entry if doing apply (it will be adjusted appropriately in
    * evaluate_base_levels().  For all other ops, if the superseding entry
    * is installed, call disable_older_pkgs() to set the state of
    * any MIGRATING entries that are still in the vpd to superseded.
    * Otherwise, set the state of the superseded base to SUPERSEDED (means
    * there's something wrong with the state of the superseding base level).
    */
   seded_base->apply_state = seding_base->apply_state;
   if (check_prereq.mode != OP_APPLY)
   {
      if (seded_base->apply_state != ALL_PARTS_APPLIED &&
          seded_base->apply_state != PARTIALLY_APPLIED)  /* for diskless */
      {
         seded_base->apply_state = SUPD_BY_NEWER_BASE;
      }
      else
      {
         disable_older_pkgs (seded_base->superseded_by->superseding_fix,
                             seded_base,
                             seded_base->name,
                             TRUE,
                             error);
         RETURN_ON_ERROR;
      }
   }

} /* build_base_sups_link */
