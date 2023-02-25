static char sccsid[] = "@(#)05  1.9  src/bos/usr/sbin/install/installp/inu_pseudo_ckreq.c, cmdinstl, bos412, 9446B 11/11/94 09:44:16";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_pseudo_ckreq
 *            check_requisites
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/
#include <ckp_common_defs.h>
#include <ckprereqmsg.h>

extern fix_info_type * fix_info_lookup (char * name, Level_t * level, 
                                        boolean  use_fix_id);

extern void unmark_SUBGRAPH (fix_info_type * fix_ptr, int bit_pattern);

static int check_requisites (fix_info_type * parent, 
                             fix_info_type * fix, 
                             boolean       * print_hdr_msg1, 
                             boolean       * print_hdr_msg2, 
                             boolean         doing_root_parts);

/* ------------------------------------------------------------------------ *
**
** Function    inu_pseudo_ckreq
**
** Purpose  Marks every member of "sop range" passed from caller as 
**          APPLYING.  Traverses the subgraph of the sop member's fix_info
**          node in the fix_info_table checking that all requisites are
**          in the APPLYING or ALL_PARTS_APPLIED state.
**
** Returns Void routine.
**
** ------------------------------------------------------------------------ */

int inu_pseudo_ckreq
 (Option_t  * SopPtr, 
  Option_t ** start_op, 
  Option_t  * next_op)

{
   boolean         doing_root_parts;
   boolean         print_hdr_msg1;
   boolean         print_hdr_msg2;
   Option_t      * op;
   Option_t      * op_prev;
   Option_t      * start_prev;
   fix_info_type * fix;

   /* Find the previous sop entry to the "start_op". */

   op = SopPtr->next;
   start_prev = SopPtr;
   while (op != *start_op)
   {
      start_prev = op;
      op = op->next;
   }

   /* Mark all options as "APPLYING" in the requisite graph. */

   for (op = *start_op;
        op != next_op;
        op = op->next) 
   {
      /*
       * Look up the fix entry in the graph.  (TRUE says don't ignore fix id).
       */
      fix = fix_info_lookup (op->name, &(op->level), TRUE);
      if (fix == NIL (fix_info_type))
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E, 
                  CKP_INT_CODED_MSG_D), inu_cmdname, SOP_ENTRY_NOT_IN_GRAPH);
         inu_quit (INUUNKWN);
      }
      fix->apply_state = APPLYING;
   }

   /* For every sop entry, check the requisites in the requisite graphs.
      Move sop entries with failing requisites outside of the 
      "start_op - next_op" range. */

   op = *start_op;            /* List traversal pointer. */
   op_prev = start_prev;      /* Pointer to previous sop member to "op" */
   print_hdr_msg1 = TRUE;     /* Used to defer printing msg until failure.*/
   while (op != next_op)
   {
      fix = fix_info_lookup (op->name, &(op->level), TRUE);
      if (fix == NIL (fix_info_type))
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E, 
                  CKP_INT_CODED_MSG_D), inu_cmdname, SOP_ENTRY_NOT_IN_GRAPH);
         inu_quit (INUUNKWN);
      }

      print_hdr_msg2 = TRUE; /* Another variation of hdr_msg1. */

      doing_root_parts = (op->vpd_tree == VPDTREE_ROOT);

      if (check_requisites (fix, fix, &print_hdr_msg1, &print_hdr_msg2, 
                            doing_root_parts) != STAT_SUCCESS)
      {
         fix->apply_state = CANCELLED;
         op->Status = STAT_CANCEL;

         /* Now move the failed option to the left of the start option in
            our range of options. */

         if (op == *start_op)
         {
            start_prev = *start_op;         /* We have a new start_prev. */
            op_prev = *start_op;           /* We have a new op prev. */
            (*start_op) = (*start_op)->next; /* We have a new start_op. */
            op = *start_op;                /* Increment op. */
         }
         else
         {
            op_prev->next = op->next;    /* Point around this op. */

            /* The next 2 steps links this op to the left of start_op. */
            op->next = *start_op;
            start_prev->next = op;

            start_prev = op;             /* This is our new start_prev. */
            op = op_prev->next;          /* Increment op. */
         }
      }
      else
      {
         op_prev = op;
         op = op->next;
      }
      /* Unmark the "traversal-bits" */

      unmark_SUBGRAPH (fix, (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED));
   }

   /* Print a trailing message about reasons for failure. */

   if ( ! print_hdr_msg1)
   {
      instl_msg (INFO_MSG, "\n");
      instl_msg (INFO_MSG, MSGSTR (MS_CKPREREQ, CKP_WHERE_I, CKP_WHERE_D));
      instl_msg (INFO_MSG, MSGSTR (MS_CKPREREQ, CKP_INSTP_FAILED_REQ_I, 
                                              CKP_INSTP_FAILED_REQ_D));
   }

} /* inu_pseudo_ckreq */

/* ------------------------------------------------------------------------ *
**
** Function    check_requisites
**
** Purpose     Traverses the subgraph of the parent fix_info_node passed by 
**             the caller  checking for nodes in the APPLYING or 
**             ALL_PARTS_APPLIED state.  
**
** Returns     STAT_FAILURE if any nodes in the subgraph are not applied(ing)
**             STAT_SUCCESS otherwise.
**
** ------------------------------------------------------------------------ */

static int check_requisites
 (fix_info_type * parent, 
  fix_info_type * fix, 
  boolean       * print_hdr_msg1, 
  boolean       * print_hdr_msg2, 
  boolean         doing_root_parts)
{
  int                   return_val;
  char                  fix_name [MAX_LPP_FULLNAME_LEN];
  requisite_list_type * requisite;

  return_val = STAT_SUCCESS;

  /* Check for cycles in our graph. */

  if (fix->flags &VISITING_THIS_SUBGRAPH  || 
      fix->flags &SUBGRAPH_VISITED)
     return (return_val);

  fix->flags |= VISITING_THIS_SUBGRAPH;

  if (fix->apply_state != APPLYING  && fix->apply_state != ALL_PARTS_APPLIED)
  {
     return_val = STAT_FAILURE;

     /* Do some error reporting. */

     if (*print_hdr_msg1)
     {
        instl_msg (INFO_MSG, "\n");
        if (doing_root_parts)
        {
           instl_msg (INFO_MSG, MSGSTR (MS_CKPREREQ, 
                                CKP_INSTP_REQ_CANCL_ROOT_E, 
                                CKP_INSTP_REQ_CANCL_ROOT_D), inu_cmdname);
        }
        else
        {
           instl_msg (INFO_MSG, MSGSTR (MS_CKPREREQ, CKP_INSTP_REQ_CANCL_E, 
                                        CKP_INSTP_REQ_CANCL_D), inu_cmdname);
        }
        *print_hdr_msg1 = FALSE;
     }

     if (*print_hdr_msg2)
     {
        instl_msg (INFO_MSG, MSGSTR (MS_CKPREREQ, CKP_PROD_AT_LEV_REQRS_I, 
                 CKP_PROD_AT_LEV_REQRS_D), parent->name, 
                 get_level_from_fix_info (parent, fix_name));
        *print_hdr_msg2 = FALSE;
     }
     instl_msg (INFO_MSG, "\
 * %s\n", get_fix_name_from_fix_info (fix, fix_name)); 

  } /* end if */

  /* Recurse. */

  for (requisite = fix->requisites;
       requisite != NIL (requisite_list_type);
       requisite = requisite->next_requisite)
  {
     if (requisite->type == A_PREREQ  ||  
         requisite->type == AN_INSTREQ)
        return_val += check_requisites (parent, requisite->fix_ptr, 
                                        print_hdr_msg1, print_hdr_msg2, 
                                        doing_root_parts); 
  }

  fix->flags &= ~VISITING_THIS_SUBGRAPH;
  fix->flags |= SUBGRAPH_VISITED;

  return (return_val);

} /* check_requisites */
