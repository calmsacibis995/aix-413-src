static char sccsid[] = "@(#)03  1.13  src/bos/usr/sbin/install/installp/inu_set_cur_oper.c, cmdinstl, bos412, 9447C 11/21/94 17:28:31";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_set_cur_oper (installp)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <installp.h>

/*--------------------------------------------------------------------------*
**
** NAME: inu_set_cur_oper()
**
** FUNCTION: Based on the lppname (option name up to the first period), and
**           the vpd tree set the current operation.
**
** NOTES:    The current operation is the group of options that must be done 
**           together. We will scan the sop and when the lppname changes, or
**           the vpd tree that we're dealing with changes, or if the type of
**           operation changes, then that is the end of the current operation.
**
** RETURNS:  void
**
**--------------------------------------------------------------------------*/
void inu_set_cur_oper(
Option_t    *sop,        /* Ptr to first option of this operation - input */
Option_t    **next_op)   /* Ptr to first option of next operation - output */

{
    char sv_lppname[MAX_LPP_NAME]; /* top level lppname */
    char lppname[MAX_LPP_NAME]; /* top level lppname */
    char     *p;         /* temporary char pointer */
    char     sv_tree;    /* char indicating the vpd for current operation */
    int      sv_op_type; /* int indicating the op_type for current operation */
    int      sv_operation; /* int indicating the operation for current oper.*/
    Option_t *op;

   /*----------------------------------------------------------------------*
    * Find the option for the next operation in the Selected Option List
    * based on the when the lppname, the vpd tree, or the operation type 
    * values change...
    *----------------------------------------------------------------------*/
    sv_tree    = sop->vpd_tree;
    sv_op_type = sop->op_type;
    sv_operation = sop->operation;
    (void) inu_get_prodname(sop, sv_lppname);

    for (op = sop->next; op != NIL(Option_t); op = op->next) {
       /*------------------------------------------------------------------*
        * If the tree (user,root or share) changes then set end of operation.
        *------------------------------------------------------------------*/
        if (op->vpd_tree != sv_tree) {
            *next_op = op;
            break;
        }

       /*------------------------------------------------------------------*
        * If the operation type (install or update) changes then set end of 
	* operation.
        *------------------------------------------------------------------*/
        if ( (IF_INSTALL(sv_op_type) && IF_UPDATE(op->op_type))
                 ||
             (IF_UPDATE(sv_op_type) && IF_INSTALL(op->op_type)) )
        {
            *next_op = op;
            break;
        }

       /*------------------------------------------------------------------*
        * If the operation field changes (cleanup_apply, cleanup-commit)
	* then set end of operation.
        *------------------------------------------------------------------*/
        if (op->operation != sv_operation) {
            *next_op = op;
            break;
        }

        p = op->name;

       /*------------------------------------------------------------------*
        * If the lppname changes then set end of operation.
        *------------------------------------------------------------------*/
	inu_get_prodname (op, lppname);
        if (strcmp (lppname, sv_lppname) != 0)
	{
            *next_op = op;
            break;
	}

       /*------------------------------------------------------------------*
        * If the bff changes then set end of operation.
        *------------------------------------------------------------------*/
	if (sop->bff != op->bff)
	{
            *next_op = op;
            break;
	}

       /*------------------------------------------------------------------*
        * If the option type is version 4 or later...
        *------------------------------------------------------------------*/
        if ( ! IF_3_X (op->op_type))
        {
          /*---------------------------------------------------------------*
           * If its an update and the option name changes then set end of
           * operation.
           *---------------------------------------------------------------*/
           if (IF_UPDATE (op->op_type)  &&
               strcmp (sop->name, op->name) != 0)
           {
               *next_op = op;
               break;
           }
        }

       /*------------------------------------------------------------------*
        * If its a version 3 option type and the levels don't match then 
        * set end of operation.
        *------------------------------------------------------------------*/
        else if (inu_level_compare (&op->level, &sop->level) != 0 )
        {
            *next_op = op;
            break;
        }
        
    } /* end for (op = sop; op != NIL(Option_t); op = op->next) */

   /*----------------------------------------------------------------------*
    * If we went all the way thru the sop (op = NIL(Option_t)) then this is
    * the last operation in the sop so we need to set the next_op to null.
    *----------------------------------------------------------------------*/
    if (op == NIL(Option_t))
       *next_op = NIL(Option_t); 

    return;

} /* end inu_set_cur_oper() */

