static char sccsid[] = "@(#)33  1.16.1.21  src/bos/usr/sbin/install/ckprereq/graph_dump.c, cmdinstl, bos411, 9428A410j 5/12/94 16:48:34";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: dump_fix_info
 *              dump_option
 *              dump_sop
 *              dump_sop_entry
 *              graph_dump
 *              print_lpp_name
 *              code4_check
 *              print_op_type
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
** Includes
*/
#include <stdio.h>
#include <check_prereq.h>

#pragma covcc !instr

static void dump_sop_entry (Option_t * sop);

static void print_op_type (int op_type);

static void print_requisite (requisite_list_type * requisite);

char *apply_state_strings[] = { "ALL_PARTS_APPLIED",
                                "TO_BE_EXPLICITLY_APPLIED",
                                "TO_BE_IMPLICITLY_APPLIED",
                                "APPLY_TO_BE_COMPLETED",
                                "PARTIALLY_APPLIED",
                                "IN_TOC",
                                "SUPD_BY_NEWER_BASE",
                                "SUPD_BY_NEWER_UPDT",
                                "TO_BE_OVERWRITTEN_BY_SUP_BASE",
                                "CAN_NOT_COMPLETE_APPLY",
                                "CAN_NOT_APPLY",
                                "CANNOT_APPLY_CONFL_LEVEL",
                                "TO_BE_OVERWRITTEN_BY_CONFL_LEVEL",
                                "AVAILABLE",
                                "BROKEN",
                                "APPLYING",
                                "CANCELLED",
                                "FAILED",
                                "UNKNOWN" };

char *commit_state_strings[] = { "ALL_PARTS_COMMITTED",
                                 "TO_BE_EXPLICITLY_COMMITTED",
                                 "TO_BE_IMPLICITLY_COMMITTED",
                                 "COMMIT_TO_BE_COMPLETED",
                                 "PARTIALLY_COMMITTED",
                                 "UNCOMMITTED",
                                 "CAN_NOT_COMPLETE_COMMIT",
                                 "CAN_NOT_COMMIT",
                                 "COMMIT_UNKNOWN" };

char *reject_state_strings[] = { "CAN_NOT_REJECT",
                                 "NOT_REJECTABLE",
                                 "NOT_REJECTED",
                                 "TO_BE_EXPLICITLY_REJECTED",
                                 "TO_BE_IMPLICITLY_REJECTED",
                                 "REJECTED",
                                 "DONT_NEED_TO_REJECT",
                                 "REJECT_UNKNOWN" };

char *origin_type_strings[] = { "COMMAND_LINE",
                                "TOC",
                                "RQD_UPDATE_FROM_TOC",
                                "AUTO_MC_FROM_VPD",
                                "VPD",
                                "DUMMY_SUPERSEDED_FIX",
                                "DUMMY_FIX",
                                "DUMMY_GROUP",
                                "UNKNOWN_ORIGIN" };

#define print_lpp_name(ptr) \
    if (ptr -> level.ptf[0] == '\0') \
        fprintf (fd, "%s.%d.%d.%d.%d", \
            (ptr -> name == NIL (char)) ? "NIL" : ptr -> name, \
            ptr -> level.ver, \
            ptr -> level.rel, \
            ptr -> level.mod, \
            ptr -> level.fix); \
    else \
        fprintf (fd, "%s.%d.%d.%d.%d.%s", \
            (ptr -> name == NIL (char)) ? "NIL" : ptr -> name, \
            ptr -> level.ver, \
            ptr -> level.rel, \
            ptr -> level.mod, \
            ptr -> level.fix, \
            ptr -> level.ptf) \

static FILE *fd=stdout;

/*--------------------------------------------------------------------------*
**
** NAME: dump_fix_info
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints a specific node.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void dump_fix_info (fix_info_type * fix_info )
{
    requisite_list_type * requisite;
    requisite_list_type * requisite_info;
    dependants_type     * dependant;
    supersede_list_type * supersede_info;

    if (fix_info -> flags & DEBUG_CYCLE_DETECTION)
       return;

    fix_info -> flags |= DEBUG_CYCLE_DETECTION;

    if (((check_prereq.graph_dump_code != 1) &&
         (check_prereq.graph_dump_code != 2))
                       ||
        ((check_prereq.graph_dump_code == 1) &&
         (fix_info -> flags & SUCCESSFUL_NODE))
                       ||
        ((check_prereq.graph_dump_code == 2) &&
         (fix_info -> flags & SUCCESSFUL_NODE) &&
         (fix_info -> apply_state != ALL_PARTS_APPLIED)))
     {
       print_lpp_name(fix_info);
       fprintf (fd, " (0x%lx)\n", fix_info);

       fprintf (fd, "\tapply_state = %s\n",
               apply_state_strings[fix_info -> apply_state]);

       fprintf (fd, "\tcommit_state = %s\n",
             commit_state_strings[fix_info -> commit_state]);

      fprintf (fd, "\treject_state = %s\n",
             reject_state_strings[fix_info -> reject_state]);

       fprintf (fd, "\tflags = 0x%x (", fix_info -> flags);
       if (fix_info -> flags == 0)
           fprintf (fd, " NONE )\n");
       else {
        if (fix_info -> flags & VISITING_THIS_NODE)
            fprintf (fd, " VISITING_THIS_NODE");

        if (fix_info -> flags & MEMBER_OF_SOP)
            fprintf (fd, " MEMBER_OF_SOP");

        if (fix_info -> flags & CONFL_TOC_BASE_LEVEL)
            fprintf (fd, " CONFL_TOC_BASE_LEVEL");

        if (fix_info -> flags & VISITING_THIS_SUBGRAPH)
            fprintf (fd, " VISITING_THIS_SUBGRAPH");

        if (fix_info -> flags & SUBGRAPH_VISITED)
            fprintf (fd, " SUBGRAPH_VISITED");

        if (fix_info -> flags & SUCCESSFUL_NODE)
            fprintf (fd, " SUCCESSFUL_NODE");

        if (fix_info -> flags & UPDATES_ONLY)
            fprintf (fd, " UPDATES_ONLY");

        if (fix_info -> flags & CANDIDATE_NODE)
            fprintf (fd, " CANDIDATE_NODE"); 
        
        if (fix_info -> flags & FIX_HAS_IFREQS) 
           fprintf (fd, " FIX_HAS_IFREQS"); 

        if (fix_info -> flags & UNRESOLVED_IFREQ) 
           fprintf (fd, " UNRESOLVED_IFREQ");

        if (fix_info -> flags & FAILED_NODE)
            fprintf (fd, " FAILED_NODE");

        if (fix_info -> flags & AUTO_C_UPDATE)
            fprintf (fd, " AUTO_C_UPDATE");

        if (fix_info -> flags & REPT_CMD_LINE_FAILURE)
            fprintf (fd, " REPT_CMD_LINE_FAILURE");

        if (fix_info -> flags & REPT_REQUISITE_FAILURE)
            fprintf (fd, " REPT_REQUISITE_FAILURE");

        if (fix_info -> flags & REPT_IFREQ_WARNING)
            fprintf (fd, " REPT_IFREQ_WARNING");

        if (fix_info -> flags & REPT_FAILED_REQUISITE)
            fprintf (fd, " REPT_FAILED_REQUISITE");

        if (fix_info -> flags & AUTO_SUPERSEDES_PKG)
            fprintf (fd, " AUTO_SUPERSEDES_PKG");
       
        if (fix_info -> flags & EXPL_REQUESTED_PKG)
            fprintf (fd, " EXPL_REQUESTED_PKG");
       
        if (fix_info -> flags & MANDATORY_UPDATE)
            fprintf (fd, " MANDATORY_UPDATE");
       
        if (fix_info -> flags & WARN_NO_USR_SUPS_INFO)
            fprintf (fd, " WARN_NO_USR_SUPS_INFO");
       
        if (fix_info -> flags & WARN_NO_USR_PRQ_INFO)
            fprintf (fd, " WARN_NO_USR_PRQ_INFO");
       
        if (fix_info -> flags & WARN_NO_PRQ_BASE)
            fprintf (fd, " WARN_NO_PRQ_BASE");
       
        if (fix_info -> flags & WARN_MISS_USR_APPLD_ROOT)
            fprintf (fd, " WARN_MISS_USR_APPLD_ROOT");
       
        if (fix_info -> flags & WARN_MISS_USR_COMTD_ROOT)
            fprintf (fd, " WARN_MISS_USR_COMTD_ROOT");
       
        if (fix_info -> flags & FAILED_DEINST_CHECK)
            fprintf (fd, " FAILED_DEINST_CHECK");
      
        if (fix_info -> flags & CKP_ST_APPLYING)
            fprintf (fd, " CKP_ST_APPLYING");
      
        if (fix_info -> flags & CKP_ST_COMMITTING)
            fprintf (fd, " CKP_ST_COMMITTING");
      
        if (fix_info -> flags & CKP_ST_REJECTING)
            fprintf (fd, " CKP_ST_REJECTING");
      
        if (fix_info -> flags & CKP_ST_DEINSTALLING)
            fprintf (fd, " CKP_ST_DEINSTALLING");
      
        if (fix_info -> flags & DISABLING_INT_ERR_FLAG)
            fprintf (fd, " DISABLING_INT_ERR_FLAG");
      
        if (fix_info -> flags & ~(VISITING_THIS_NODE         |
                                  MEMBER_OF_SOP              |
                                  CONFL_TOC_BASE_LEVEL       |
                                  VISITING_THIS_SUBGRAPH     |
                                  SUBGRAPH_VISITED           |
                                  SUCCESSFUL_NODE            |
                                  UPDATES_ONLY               |
                                  CANDIDATE_NODE             |
                                  FIX_HAS_IFREQS             |
                                  UNRESOLVED_IFREQ           |
                                  FAILED_NODE                |
                                  DEBUG_CYCLE_DETECTION      |    
                                  AUTO_C_UPDATE              |
                                  REPT_CMD_LINE_FAILURE      |
                                  REPT_REQUISITE_FAILURE     |
                                  REPT_IFREQ_WARNING         |
                                  REPT_FAILED_REQUISITE      |
                                  AUTO_SUPERSEDES_PKG        |      
                                  EXPL_REQUESTED_PKG         |
                                  FAILED_DEINST_CHECK        |
                                  WARN_MISS_USR_COMTD_ROOT   |
                                  WARN_MISS_USR_APPLD_ROOT   |
                                  WARN_NO_PRQ_BASE           |
                                  WARN_NO_USR_SUPS_INFO      |
                                  WARN_NO_USR_PRQ_INFO       |
                                  CKP_ST_APPLYING            |
                                  CKP_ST_COMMITTING          |
                                  CKP_ST_REJECTING           |
                                  CKP_ST_DEINSTALLING        |
                                  MANDATORY_UPDATE           |
                                  DISABLING_INT_ERR_FLAG) )
            fprintf (fd, " **** ERROR:- unknown flag ****");

        fprintf (fd, " )\n");
    }

    fprintf (fd, "\torigin = %s\n", origin_type_strings[fix_info -> origin]);

    fprintf (fd, "\tparts_applied = %d (", fix_info -> parts_applied);
    if (fix_info -> parts_applied == 0)
        fprintf (fd, " NONE )\n");
    else {
        if ((fix_info -> parts_applied & LPP_ROOT) != 0)
            fprintf (fd, " ROOT");
        if ((fix_info -> parts_applied & LPP_USER) != 0)
            fprintf (fd, " USR");
        if ((fix_info -> parts_applied & LPP_SHARE) != 0)
            fprintf (fd, " SHARE");
        fprintf (fd, " )\n");
    }

    fprintf (fd, "\tparts_committed = %d (", fix_info -> parts_committed);
    if (fix_info -> parts_committed == 0)
        fprintf (fd, " NONE )\n");
    else {
        if ((fix_info -> parts_committed & LPP_ROOT) != 0)
            fprintf (fd, " ROOT");
        if ((fix_info -> parts_committed & LPP_USER) != 0)
            fprintf (fd, " USR");
        if ((fix_info -> parts_committed & LPP_SHARE) != 0)
            fprintf (fd, " SHARE");
        fprintf (fd, " )\n");
    }

    fprintf (fd, "\tparts_in_fix = %d (", fix_info -> parts_in_fix);
    if (fix_info -> parts_in_fix == 0)
        fprintf (fd, " NONE )\n");
    else {
        if ((fix_info -> parts_in_fix & LPP_ROOT) != 0)
            fprintf (fd, " ROOT");
        if ((fix_info -> parts_in_fix & LPP_USER) != 0)
            fprintf (fd, " USR");
        if ((fix_info -> parts_in_fix & LPP_SHARE) != 0)
            fprintf (fd, " SHARE");
        fprintf (fd, " )\n");
    }

    fprintf (fd, "\tcp_flag = %d (", fix_info -> cp_flag);
    if (fix_info -> cp_flag == 0)
        fprintf (fd, " NONE )\n");
    else {
        if ((fix_info -> cp_flag & LPP_ROOT) != 0)
            fprintf (fd, " ROOT");
        if ((fix_info -> cp_flag & LPP_USER) != 0)
            fprintf (fd, " USR");
        if ((fix_info -> cp_flag & LPP_SHARE) != 0)
            fprintf (fd, " SHARE");
        if ((fix_info -> cp_flag & LPP_MICRO) != 0)
            fprintf (fd, " MICRO");
        fprintf (fd, " )\n");
    }

    print_op_type (fix_info -> op_type);

    fprintf (fd, "\tpasses_required = %d\n",
            fix_info -> passes_required);

    fprintf (fd, "\tnum_rejectable_requisites = %d\n",
            fix_info -> num_rejectable_requisites);

    fprintf (fd, "\tnum_rejected_requisites = %d\n",
            fix_info -> num_rejected_requisites);

    fprintf (fd, "\trequisites = ");
    if ((requisite_info = fix_info -> requisites) == NULL)
        fprintf (fd, "NONE\n");
    else
     {
        print_requisite (requisite_info);

        requisite_info = requisite_info -> next_requisite;
        while (requisite_info != NULL) {
            fprintf (fd, "\t             ");
            print_requisite (requisite_info);
            requisite_info = requisite_info -> next_requisite;
        }
        requisite_info = fix_info -> requisites;
    }

    fprintf (fd, "\tsuperseding_ptf = \"%s\"\n", fix_info -> superseding_ptf);

   fprintf (fd, "\tsupersedes_string = \"%s\"\n", fix_info -> supersedes_string);

    fprintf (fd, "\tsupersedes = ");
    if ((supersede_info = fix_info -> supersedes) == NULL)
        fprintf (fd, "NONE\n");
    else
     {
        print_lpp_name (supersede_info -> superseded_fix);
        fprintf (fd, " (0x%lx)\n", supersede_info -> superseded_fix);
        supersede_info = supersede_info -> next_supersede;
        while (supersede_info != NULL) {
            fprintf (fd, "\t             ");
            print_lpp_name (supersede_info -> superseded_fix);
            fprintf (fd, " (0x%lx)\n", supersede_info -> superseded_fix);
            supersede_info = supersede_info -> next_supersede;
        }
     }

    fprintf (fd, "\tsuperseded_by = ");
    if ((supersede_info = fix_info -> superseded_by) == NULL)
        fprintf (fd, "NONE\n");
    else
     {
        print_lpp_name (supersede_info -> superseding_fix);
        fprintf (fd, " (0x%lx)\n", supersede_info -> superseding_fix);
     }

   fprintf (fd, "\tprereq_file = \"%s\"\n", fix_info -> prereq_file);

   fprintf (fd, "\tTOC_Ptr = (0x%lx)\n", fix_info -> TOC_Ptr);

   if (fix_info -> number_of_requisites != 0)
      fprintf (fd, "\tnumber_of_requisites = %d\n",
               fix_info -> number_of_requisites);

   if ((dependant = fix_info -> dependants) != NIL (dependants_type))
    {
      fprintf (fd, "\tdependants =");
      print_lpp_name (dependant -> fix_ptr);
      fprintf (fd, " (0x%lx)\n", dependant -> fix_ptr);
      dependant = dependant -> next_dependant;
      while (dependant != NIL (dependants_type))
       {
         fprintf (fd, "\t            ");
         print_lpp_name (dependant -> fix_ptr);
         fprintf (fd, " (0x%lx)\n", dependant -> fix_ptr);
         dependant = dependant -> next_dependant;
       }
    }

   fprintf (fd, "\n");

   if (fix_info -> apar_info != NIL (char) )
    {
      fprintf (fd, "\tapar_info = %s\n\n", fix_info -> apar_info);
    }

   if (fix_info -> description != NIL (char) )
    {
      fprintf (fd, "\tdescription = %s\n\n", fix_info -> description);
    }
   }


   fix_info -> flags &= ~ DEBUG_CYCLE_DETECTION;
}

/*--------------------------------------------------------------------------*
**
** NAME: print_op_type
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints out an
**            op_type field.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void print_op_type (int op_type)
 {
    fprintf (fd, "\top_type = %d (",  op_type);
    if ( op_type == 0)
        fprintf (fd, " NONE )\n");
    else {
        if ((op_type & OP_TYPE_INSTALL) != 0)
            fprintf (fd, " OP_TYPE_INSTALL");
        if ((op_type & OP_TYPE_UPDATE) != 0)
            fprintf (fd, " OP_TYPE_UPDATE");
        if ((op_type & OP_TYPE_3_1) != 0)
            fprintf (fd, " OP_TYPE_3_1");
        if ((op_type & OP_TYPE_3_2) != 0)
            fprintf (fd, " OP_TYPE_3_2");
        if ((op_type & OP_TYPE_4_1) != 0)
            fprintf (fd, " OP_TYPE_4_1");
        if ((op_type & OP_TYPE_C_UPDT) != 0)
            fprintf (fd, " OP_TYPE_C_UPDT");
        if ((op_type & OP_TYPE_E_UPDT) != 0)
            fprintf (fd, " OP_TYPE_E_UPDT");
        if ((op_type & OP_TYPE_M_UPDT) != 0)
            fprintf (fd, " OP_TYPE_M_UPDT");
        if ((op_type & OP_TYPE_BOSBOOT) != 0)
            fprintf (fd, " OP_TYPE_BOSBOOT");
        if (op_type &
            ~(OP_TYPE_INSTALL | OP_TYPE_UPDATE | OP_TYPE_3_1 |
              OP_TYPE_3_2 | OP_TYPE_4_1 | OP_TYPE_C_UPDT |
              OP_TYPE_E_UPDT | OP_TYPE_M_UPDT | OP_TYPE_BOSBOOT) )
           fprintf (fd, "ILLEGAL VALUE");
        fprintf (fd, " )\n");

    if ( (op_type & (OP_TYPE_INSTALL | OP_TYPE_UPDATE) )
                            ==
           (OP_TYPE_INSTALL | OP_TYPE_UPDATE) )
       fprintf (fd, "ILLEGAL install & update");
    fprintf (fd, "\n");

    }
 } /* end print_op_type */

/*--------------------------------------------------------------------------*
**
** NAME: print_requisite
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints out a
**            requisite_list_type.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void print_requisite (requisite_list_type * requisite)
 {
        if (requisite -> type == UNKNOWN_REQUISITE)
           fprintf (fd, "UNKNOWN_REQUISITE");
        if (requisite -> type == A_PREREQ)
           fprintf (fd, "PREREQ ");
        if (requisite -> type == AN_INSTREQ)
           fprintf (fd, "INSTREQ ");
        if (requisite -> type == A_COREQ)
           fprintf (fd, "COREQ  ");
        if (requisite -> type == AN_IFREQ)
           fprintf (fd, "IFREQ  ");
        if (requisite -> type == AN_IFFREQ)
           fprintf (fd, "IFFREQ ");

        print_lpp_name (requisite -> fix_ptr);
        fprintf (fd, " (0x%lx)", requisite -> fix_ptr);
        fprintf (fd, " flags = 0x%lx", requisite -> flags);
        if (requisite -> flags.selected_member_of_group)
           fprintf (fd, " selected_member_of_group");
        if (requisite -> flags.superseded_consistency_apply)
           fprintf (fd, " superseded_consistency_apply");
        if (requisite -> flags.superseded_usr_commit)
           fprintf (fd, " superseded_usr_commit");
        if (requisite -> flags.supersedes_fix)
           fprintf (fd, " supersedes_fix");
        if (requisite -> flags.old_selected_member_of_group)
           fprintf (fd, " old_selected_member_of_group");
        fprintf (fd, "\n");
 } /* end print_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: graph_dump
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints either the
**            entire fix_info graph to a /tmp file, or a specific node to
**            stdout.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void graph_dump (char * description)
{
    fix_info_type *fix_info;

    fd = fopen ("/tmp/ckprereq.graph.out", "a");

    fprintf (fd, "%s: %s\n====================\n", inu_cmdname, description);

    fprintf (fd, "Auto_Include = %s\n",
        (check_prereq.Auto_Include ? "True" : "False"));

    fprintf (fd, "Force_Install = %s\n",
        (check_prereq.Force_Install ? "True" : "False"));

    fprintf (fd, "called_from_command_line = %s\n",
        (check_prereq.called_from_command_line ? "True" : "False"));

    fprintf (fd, "mode = %d (", check_prereq.mode);
    switch (check_prereq.mode) {
        case OP_APPLY:
            fprintf (fd, " OP_APPLY )\n");
            break;
        case OP_APPLYCOMMIT:
            fprintf (fd, " OP_APPLYCOMMIT )\n");
            break;
        case OP_COMMIT:
            fprintf (fd, " OP_COMMIT )\n");
            break;
        case OP_CLEANUP_APPLY:
            fprintf (fd, " OP_CLEANUP_APPLY )\n");
            break;
        case OP_CLEANUP_COMMIT:
            fprintf (fd, " OP_CLEANUP_COMMIT )\n");
            break;
        case OP_REJECT:
            fprintf (fd, " OP_REJECT )\n");
            break;
        case OP_STATUS:
            fprintf (fd, " OP_STATUS )\n");
            break;
        case OP_INFO:
            fprintf (fd, " OP_INFO )\n");
            break;
        default:
            fprintf (fd, " OP_UNKNOWN )\n");
            break;
    }

    fprintf (fd, "number_of_errors = %d\n", check_prereq.number_of_errors);

    fprintf (fd, "function_return_code = %d\n",
        check_prereq.function_return_code);

    fprintf (fd, "parts_to_operate_on = %d (", check_prereq.parts_to_operate_on);
    if (check_prereq.parts_to_operate_on == 0)
        fprintf (fd, " NONE )\n");
    else {
        if ((check_prereq.parts_to_operate_on & LPP_ROOT) != 0)
            fprintf (fd, " ROOT");
        if ((check_prereq.parts_to_operate_on & LPP_USER) != 0)
            fprintf (fd, " USR");
        if ((check_prereq.parts_to_operate_on & LPP_SHARE) != 0)
            fprintf (fd, " SHARE");
        fprintf (fd, " )\n");
    }

    fprintf (fd, "verbose = %s\n", (check_prereq.verbose ? "True" : "False"));

    fprintf (fd, "fix_info_anchor\n---------------\n");

    fix_info = check_prereq.fix_info_anchor;
    while(fix_info != NULL) {
        dump_fix_info(fix_info);
        fix_info = fix_info -> next_fix;
    }

    fclose (fd);
    fd=stdout;
} /* end graph_dump */

/*--------------------------------------------------------------------------*
**
** NAME: dump_option
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints a specific
**            option.  It opens the debug file.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void dump_option (Option_t * sop)
 {

   fd = fopen ("/tmp/ckprereq.graph.out", "a");

   dump_sop_entry (sop);

   fclose (fd);
   fd=stdout;

 } /* end dump_option */

/*--------------------------------------------------------------------------*
**
** NAME: dump_sop
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints a given option
**            to the currently opened debug file.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void dump_sop_entry (Option_t * sop)
 {
    fprintf (fd, "-----------------------------------------\n");
    fprintf (fd, "name = \"%s\"\n", sop -> name);
    fprintf (fd, "prodname = \"%s\"\n", sop -> prodname);
    fprintf (fd, "quiesce = %c\n", sop -> quiesce);
    fprintf (fd, "content = %c\t", sop -> content);
    switch (sop -> content)
     {
     case 'B' :
       fprintf (fd, "CONTENT_BOTH\n");
       break;

     case 'D' :
       fprintf (fd, "CONTENT_MCODE\n");
       break;

     case 'H' :
       fprintf (fd, "CONTENT_SHARE\n");
       break;

     case 'M' :
       fprintf (fd, "CONTENT_MRI\n");
       break;

     case 'O' :
       fprintf (fd, "CONTENT_OBJECT\n");
       break;

     case 'P' :
       fprintf (fd, "CONTENT_PUBS\n");
       break;

     case 'U' :
       fprintf (fd, "CONTENT_USR\n");
       break;

     case ' ' :
       fprintf (fd, "CONTENT_UNKNOWN\n");
       break;

     default :
       fprintf (fd, "** ILLEGAL VALUE **\n");
       break;
     } /* end switch */
    fprintf (fd, "vpd_tree = %c\t", sop -> vpd_tree);
    switch (sop -> vpd_tree)
     {
       case 'U' :
         fprintf (fd, "VPDTREE_USR\n");
         break;

       case 'M' :
         fprintf (fd, "VPDTREE_ROOT\n");
         break;

       case 'S' :
         fprintf (fd, "VPDTREE_SHARE\n");
         break;

       default :
         fprintf (fd, "ILLEGAL VALUE\n");
         break;
     } /* end switch */

    fprintf (fd, "lang = \"%s\"\n", sop -> lang);
    fprintf (fd, "desc = \"%s\"\n", sop -> desc);
    fprintf (fd, "op_checked = %d\n", sop -> op_checked);
    fprintf (fd, "operation = %d\t", sop -> operation);
    switch (sop -> operation)
     {
       case (0) :
          fprintf (fd, "OP_APPLY\n");
          break;

       case (1) :
          fprintf (fd, "OP_APPLYCOMMIT\n");
          break;

       case (2) :
          fprintf (fd, "OP_COMMIT\n");
          break;

       case (3) :
          fprintf (fd, "OP_CLEANUP_APPLY\n");
          break;

       case (4) :
          fprintf (fd, "OP_CLEANUP_COMMIT\n");
          break;

       case (5) :
          fprintf (fd, "OP_REJECT\n");
          break;

       case (6) :
          fprintf (fd, "OP_STATUS\n");
          break;

       case (7) :
          fprintf (fd, "OP_INFO\n");
          break;

       default :
          fprintf (fd, "ILLEGAL VALUE\n");
          break;
     } /* end switch */

    print_op_type (sop -> op_type);

    fprintf (fd, "Status = %d\t", sop -> Status);
    switch (sop -> Status)
     {
        case (STAT_SUCCESS) :
           fprintf (fd, "STAT_SUCCESS\n");
           break;

        case (STAT_IFREQ_FAIL) :
           fprintf (fd, "STAT_IFREQ_FAIL\n");
           break;

        case (STAT_BYPASS) :
           fprintf (fd, "STAT_BYPASS\n");
           break;

        case (STAT_FAILURE) :
           fprintf (fd, "STAT_FAILURE\n");
           break;

        case (STAT_CLEANUP) :
           fprintf (fd, "STAT_CLEANUP\n");
           break;

        case (STAT_CLEANUP_SUCCESS) :
           fprintf (fd, "STAT_CLEANUP_SUCCESS\n");
           break;

        case (STAT_CLEANUP_FAILED) :
           fprintf (fd, "STAT_CLEANUP_FAILED\n");
           break;

        case (STAT_EXPAND_FAIL) :
           fprintf (fd, "STAT_EXPAND_FAIL\n");
           break;

        case (STAT_FAILURE_INUCONVERT) :
           fprintf (fd, "STAT_FAILURE_INUCONVERT\n");
           break;

        default :
           fprintf (fd, "ILLEGAL VALUE\n");
     } /* end switch */
    fprintf (fd, "size = \"%s\"\n", sop -> size);
    fprintf (fd, "prereq = \"%s\"\n", sop -> prereq);
    fprintf (fd, "supersedes = \"%s\"\n", sop -> supersedes);
    fprintf (fd, "bff = 0x%x\n", sop -> bff);
    if (sop -> bff != NIL (BffRef_t))
     {
       fprintf (fd, "  vol = %d\n", sop -> bff -> vol);
       fprintf (fd, "  off = %d\n", sop -> bff -> off);
       fprintf (fd, "  size = %d\n", sop -> bff -> size);
       fprintf (fd, "  fmt = %c\t", sop -> bff -> fmt);
       switch (sop -> bff -> fmt)
        {
          case (FMT_3_1) :
            fprintf (fd, "FMT_3_1\n");
            break;

          case (FMT_3_1_1) :
            fprintf (fd, "FMT_3_1_1\n");
            break;

          case (FMT_3_2) :
            fprintf (fd, "FMT_3_2\n");
            break;

          default :
            fprintf (fd, "ILLEGAL VALUE\n");
            break;

        } /* end switch */

       fprintf (fd, "  platform = %c\t", sop -> bff -> platform);
       switch (sop -> bff -> platform)
        {
          case (PLAT_PS2) :
            fprintf (fd, "PS/2\n");
            break;

          case (PLAT_6000) :
            fprintf (fd, "6000\n");
            break;

          case (PLAT_S370) :
            fprintf (fd, "S370\n");
            break;

          case (PLAT_UNKNOWN) :
            fprintf (fd, "UNKNOWN\n");
            break;

          default :
            fprintf (fd, "ILLEGAL VALUE\n");
            break;
        } /* end switch */

       fprintf (fd, "  action = %d\t", sop -> bff -> action);
       switch (sop -> bff -> action)
        {
          case (ACT_INSTALL) :
            fprintf (fd, "ACT_INSTALL\n");
            break;

          case (ACT_UNKNOWN) :
            fprintf (fd, "ACT_UNKNOWN\n");
            break;

          case (ACT_SING_UPDT) :
            fprintf (fd, "ACT_SING_UPDT\n");
            break;

          case (ACT_MULT_UPDT) :
            fprintf (fd, "ACT_MULT_UPDT\n");
            break;

          case (ACT_GOLD_UPDT) :
            fprintf (fd, "ACT_GOLD_UPDT\n");
            break;

          case (ACT_EN_PKG_UPDT) :
            fprintf (fd, "ACT_EN_PGK_UPDT\n");
            break;

          case (ACT_EN_MEM_UPDT) :
            fprintf (fd, "ACT_EN_MEM_UPDT\n");
            break;

          case (ACT_INSTALLP_UPDT) :
            fprintf (fd, "ACT_INSTALLP_UPDT\n");
            break;

          case (ACT_REQUIRED_UPDT) :
            fprintf (fd, "ACT_REQUIRED_UPDT\n");
            break;

          case (ACT_CUM_UPDT) :
            fprintf (fd, "ACT_CUM_UPDT\n");
            break;

          default :
            fprintf (fd, "ILLEGAL VALUE\n");
            break;
        } /* end switch */

       fprintf (fd, "  path = \"%s\"\n", sop -> bff -> path);
       fprintf (fd, "  crc = %d\n", sop -> bff -> crc);
       fprintf (fd, "  options = 0x%x\n", sop -> bff -> options);
       fprintf (fd, "  next = 0x%x\n", sop -> bff -> next);
     }

    fprintf (fd, "next = 0x%x\n", sop -> next);
    fprintf (fd, "SelectedList = 0x%x\n", sop -> SelectedList);
    fprintf (fd, "  level.ver = %d\n", sop -> level.ver);
    fprintf (fd, "  level.rel = %d\n", sop -> level.rel);
    fprintf (fd, "  level.mod = %d\n", sop -> level.mod);
    fprintf (fd, "  level.fix = %d\n", sop -> level.fix);
    fprintf (fd, "  level.ptf = \"%s\"\n", sop -> level.ptf);
    fprintf (fd, "\n");

    fprintf (fd, "flag = 0x%x ", sop -> flag);
    if (IF_DUPE (sop -> flag) )
       fprintf (fd, "DUPLICATE ");
    if (sop -> flag & ~ 0x1)
       fprintf (fd, "ILLEGAL VALUE ");

    fprintf (fd, "\n");


 } /* end dump_sop_entry */

/*--------------------------------------------------------------------------*
**
** NAME: dump_sop
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints the TOC & SOP.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void dump_sop (char * description)
 {
   Option_t * sop;


   fd = fopen ("/tmp/ckprereq.graph.out", "a");

   fprintf (fd, "%s: %s\n====================\n", inu_cmdname, description);

   fprintf (fd, "\n\t\tTOC\n");

   for (sop = check_prereq.First_Option;
        sop != NIL (Option_t);
        sop = sop -> next)
    {
      dump_sop_entry (sop);
    }

   fprintf (fd, "\n\t\tSOP\n");

   for (sop = check_prereq.SOP;
        sop != NIL (Option_t);
        sop = sop -> next)
    {
      dump_sop_entry (sop);
    }

   fclose (fd);
   fd = stdout;
 } /* end dump_sop */

/*--------------------------------------------------------------------------*
**
** NAME: dump_sop
**
** FUNCTION:  This routine is for DEBUG use ONLY.  It prints the TOC & SOP.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void code4_check (do_reachable, do_flags, do_unknowns, do_rest)
{
  requisite_list_type  * requisite;
  fix_info_type        * fix1;
  fix_info_type        * fix2;

  fd = fopen ("/tmp/ckprereq.graph.out", "a");

  for (fix1 = check_prereq.fix_info_anchor;
       fix1 != NIL (fix_info_type);
       fix1 = fix1 -> next_fix)
   {
       if (do_flags == 1)
        {
          if ((fix1 -> flags & VISITING_THIS_NODE)
                           ||
              (fix1 -> flags & VISITING_THIS_SUBGRAPH)
                           ||
              (fix1 -> flags & SUBGRAPH_VISITED))
            {
            fprintf (fd, "\n\
<<<< Inconsistency Type2.  Tag bit is set when it's not supposed to be. >>>>\n");
              dump_fix_info(fix1);
            }
        }
       if (do_unknowns == 1 && (fix1 != check_prereq.fix_info_anchor))
          if (fix1 -> apply_state == UNKNOWN)
           {
            fprintf (fd, "\n\
<<<< Inconsistency Type3.  Nodes in graph have an unknown apply state. >>>>\n");
              dump_fix_info(fix1);
           }

       if (! do_rest)
          continue;

       if (fix1 -> flags & SUCCESSFUL_NODE)
        {
          /* Check to make sure that anything I point to is also in graph*/

              for (requisite = fix1 -> requisites;
                   requisite != NIL (requisite_list_type);
                   requisite = requisite -> next_requisite)
               {
                  fix2 = requisite -> fix_ptr;
                  if ( !(requisite -> fix_ptr -> flags & SUCCESSFUL_NODE)
                                             &&
                        (fix2 -> origin != DUMMY_FIX))
                     {
            fprintf (fd, "\n\
<<<< Inconsistency Type4.  Node in graph has requisite not in graph. >>>>\n");
                       fprintf (fd, "IN GRAPH>>>>");
                       dump_fix_info (fix1);
                       fprintf (fd, "\n\nrequisite: (NOT IN GRAPH)>>>>\n");
                       dump_fix_info (fix2);
                     }
               }

        }
      else
        {
          /* Check to make sure that anyone who points to me is also not
             in graph. */
          if (fix1 -> origin != DUMMY_FIX)
           {
             for (fix2 = check_prereq.fix_info_anchor;
                  fix2 != NIL (fix_info_type);
                  fix2 = fix2 -> next_fix)
               {

                 for (requisite = fix2 -> requisites;
                      requisite != NIL (requisite_list_type);
                      requisite = requisite -> next_requisite)
                  {
                     if ( (requisite -> fix_ptr == fix1)
                                  &&
                          (fix2 -> flags & SUCCESSFUL_NODE)
                                        &&
                          ! ((fix2 -> origin == DUMMY_GROUP)
                                        &&
                             ! (requisite -> flags.selected_member_of_group)))
                        {
            fprintf (fd, "\n\
<<<< Inconsistency Type5.  Node NOT in graph pointed to by node in graph. >>>>\n");
                          fprintf (fd, "NOT IN GRAPH>>>>");
                          dump_fix_info (fix1);
                          fprintf (fd, "\n\nrequisite of: (IN GRAPH)>>>>\n");
                          dump_fix_info (fix2);
                        }
                  }

               }
           }
      }
   }
  fclose (fd);
  fd=stdout;
}
