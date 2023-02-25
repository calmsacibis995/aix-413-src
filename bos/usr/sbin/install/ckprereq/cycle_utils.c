static char sccsid[] = "@(#)93  1.3.1.11  src/bos/usr/sbin/install/ckprereq/cycle_utils.c, cmdinstl, bos411, 9428A410j 3/6/94 19:28:50";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            check_for_prereq_cycles 
 *            check_for_prereq_cycle
 *            push_cycle_stack
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

#ifndef STRESS_MALLOC
   #define CYCLE_STACK_CHUNK 250
#else
   #define CYCLE_STACK_CHUNK 1
#endif

typedef struct
         {
           fix_info_type  * node;
         } cycle_stack_type;

static cycle_stack_type * cycle_stack;
static int                cycle_stack_depth;
static int                cycle_stack_size;

static void check_for_prereq_cycle (
fix_info_type * node,
boolean       * error);

static void pop_cycle_stack (
boolean * error);

static void push_cycle_stack (
fix_info_type  * node,
boolean        * error);

static void print_cycle (
fix_info_type * start_of_cycle);

/*--------------------------------------------------------------------------*
**
** NAME: check_for_prereq_cycles
**
** FUNCTION:  We want to scan the given graph to see if there are any illegal
**            prereq cycles.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
check_for_prereq_cycles 
 (
fix_info_type * node,
boolean       * error
)
{

   fix_info_type * fix;

   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
      check_for_prereq_cycle (fix, error);
      RETURN_ON_ERROR;
   }

   unmark_graph (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED);

} /* end check_for_prereq_cycles */

/*--------------------------------------------------------------------------*
**
** NAME: check_for_prereq_cycle
**
** FUNCTION:  We want to scan the given graph to see if there are any illegal
**            prereq cycles.  A cycle must have at least one coreq.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
check_for_prereq_cycle 
 (
fix_info_type * node,
boolean       * error
)
{
   requisite_list_type * requisite;

   if (node->flags & VISITING_THIS_SUBGRAPH)
   {
      /* 
       * Since we're only looking at prereqs, this must be an illegal cycle.
       * Print the cycle and return.
       */
      print_cycle (node);
      check_prereq.function_return_code = PREREQ_CYCLE_RC;

      *error=TRUE;
      return;
   } /* end if */


   if (node->flags & SUBGRAPH_VISITED)
      return;

   node -> flags |= VISITING_THIS_SUBGRAPH;

   /*
    * Traverse the requisite chains of this node if the requisites are prereqs.
    * (or instreqs -- same thing for the most part.)   
    * Never look at any other kind of requisite.
    */
   requisite = node -> requisites;
   while (requisite != NIL (requisite_list_type))
   {
      if (requisite->type == A_PREREQ || requisite->type == AN_INSTREQ)
      {
         /*
          * Keep track of the members of the cycle for the error message.
          */
         push_cycle_stack (node, error);
         RETURN_ON_ERROR;

         check_for_prereq_cycle (requisite -> fix_ptr, error);
         RETURN_ON_ERROR;

         pop_cycle_stack (error);
         RETURN_ON_ERROR;
      }
      requisite = requisite -> next_requisite;
   }  /* end for */

   node -> flags &= ~VISITING_THIS_SUBGRAPH; /* Cycle detection. */

   node -> flags |= SUBGRAPH_VISITED;  /* To reduce time visiting nodes again*/

} /* end check_for_prereq_cycle */

/*--------------------------------------------------------------------------*
**
** NAME: pop_cycle_stack
**
** FUNCTION:  This routine is only called by check_for_prereq_cycles ().
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
pop_cycle_stack 
 (
boolean * error
)
{
   if (cycle_stack_depth < 0)
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
               CKP_INT_CODED_MSG_D), inu_cmdname, STACK_UFLOW_CODE);

      * error = TRUE;
      return;
   }
   cycle_stack_depth--;

   if (cycle_stack_depth == -1)
   {
      my_free (cycle_stack);
      cycle_stack = NIL (cycle_stack_type);
      cycle_stack_size = 0;
   }

} /* end pop_cycle_stack */

/*--------------------------------------------------------------------------*
**
** NAME: push_cycle_stack
**
** FUNCTION:  This routine is called only by check_for_prereq_cycles ().
**            It is used to keep track of the path to get to this node.
**            When we detect a cycle, we look up the stack to verify that
**            we got here only via coreq links.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
push_cycle_stack 
 (
fix_info_type  * node,
boolean        * error
)
{

   if ( (cycle_stack_depth + 1) >= cycle_stack_size)
   {
      if (cycle_stack_size == 0)
      {
         cycle_stack_depth = -1;
         cycle_stack_size = CYCLE_STACK_CHUNK;
         cycle_stack = (cycle_stack_type *) my_malloc (cycle_stack_size *
                                                     sizeof (cycle_stack_type),
                                                     error);
         RETURN_ON_ERROR;
      }
      else
      {
         cycle_stack_size += CYCLE_STACK_CHUNK;
         cycle_stack = (cycle_stack_type *)
                          my_realloc (cycle_stack,
                                      cycle_stack_size *
                                                      sizeof (cycle_stack_type),
                                      error);
         RETURN_ON_ERROR;
      }
   }
   cycle_stack_depth++;
   cycle_stack[cycle_stack_depth].node = node;

} /* end push_cycle_stack */

/*--------------------------------------------------------------------------*
**
** NAME: print_cycle
**
** FUNCTION:  Print whatever's on the cycle stack.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void print_cycle
 (
fix_info_type * start_of_cycle
)
{
   char    fix_name[MAX_LPP_FULLNAME_LEN];
   int     stack_index;

   inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SELF_PREREQ_E, 
            CKP_SELF_PREREQ_D), inu_cmdname,
            get_fix_name_from_fix_info (start_of_cycle, fix_name));

   inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_CYCLE_HDR_I, CKP_CYCLE_HDR_D));
   for (stack_index = cycle_stack_depth; stack_index != 0; stack_index--)
   {
      inu_msg (ckp_errs, "-> %s\n",
               get_fix_name_from_fix_info (cycle_stack[stack_index].node,
                                           fix_name));

      if (start_of_cycle == cycle_stack[stack_index].node)
         break;
   }

} /* print_cycle */
