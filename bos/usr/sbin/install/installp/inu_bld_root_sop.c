static char sccsid[] = "@(#)88  1.9  src/bos/usr/sbin/install/installp/inu_bld_root_sop.c, cmdinstl, bos411, 9428A410j 6/18/93 16:50:49";
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS:
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

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/
#include <installp.h>

/*--------------------------------------------------------------------------*
**
** NAME: inu_bld_root_sop()
**
** FUNCTION:Scans the Selected Options List looking for options that also
**          have root parts.  If any are found, another option structure is
**          created and added on to a separate selected option list (rsop).
**          currently being processed.
**
** NOTE:
**
** RETURNS:  The number of root applys the needs to be done.
**
**--------------------------------------------------------------------------*/
int inu_bld_root_sop(
Option_t    **sop,       /* Ptr to the 1st option of this operation */
Option_t    *next_op)   /* Ptr to the 1st option of the next operation */
{
    Option_t *op;           /* temp ptr to the Option_t struct type.        */
    Option_t *rop;          /* temp ptr to the copied Option_t struct       */
    Option_t *last_option;  /* Ptr to the last option in the current bff.   */
    Option_t *last_insert;  /* Ptr to the last option that was inserted.    */
    int num_found = 0;      /* count of the number of root applys to do     */
    int rc;                 /* return code                                  */
    char content;           /* Indicator that we found a root apply to do   */

   /*----------------------------------------------------------------------*
    * Scan through the sop to find the last option of this operation, because
    * we're going to put the root apply options in the sop after the user
    * apply options.
    *----------------------------------------------------------------------*/
    for (op = *sop; op->next != next_op; op = op->next);
    last_option = op;
    last_insert = op;

   /*----------------------------------------------------------------------*
    * Scan through the Selected Options List to see if any of the options
    * for this operation has root parts that needs to be done.
    *----------------------------------------------------------------------*/
    for (op = *sop; op != next_op; op = op->next ) {

       /*------------------------------------------------------------------*
        * This routine is used for both APPLY and CLEANUP operations...
        *------------------------------------------------------------------*/
        if (op->content == CONTENT_BOTH && 
            (op->Status == STAT_SUCCESS && 
            (op->vpd_tree == VPDTREE_USR && 
	    (op->operation == OP_APPLY || op->operation == OP_APPLYCOMMIT)))) {
           /*--------------------------------------------------------------*
            * Make a copy of this option so we have an option entry for
            * the root part and add it to the sop after the /usr apply options
            * in the linked list. Copy only the user options. We don't
	    * want to duplicate operations on root parts.
            *--------------------------------------------------------------*/
            num_found++;
            rop = copy_option(op);          /* copy the /usr option struct  */
            rop->vpd_tree = VPDTREE_ROOT;   /* set it to be a root option   */
            last_insert->next = rop;        /* link to the end of last opt  */
            rop->next = next_op;            /* link new opt to next bff opt */
            last_insert = rop;              /* make the new option the last */
        }

        if (op == last_option)
            break;

    } /* end main for loop */

   /*----------------------------------------------------------------------*
    * If no options were found with root parts then return 0.
    *----------------------------------------------------------------------*/
    if (num_found == 0)
        return(num_found);
    
   /*----------------------------------------------------------------------*
    * Now change the sop pointer (the pointer to the first option of this
    * operation) to be the first option of the root parts to be done.
    *----------------------------------------------------------------------*/
    *sop = last_option->next;

   /*----------------------------------------------------------------------*
    * If for the apply operation, make the "lpp.options" file.
    *----------------------------------------------------------------------*/
    content = CONTENT_BOTH;
    if ((*sop)->operation == OP_APPLY || (*sop)->operation == OP_APPLYCOMMIT) {
    	if ((rc = inu_mk_opt_fil(*sop, next_op, STAT_SUCCESS, &content))
                                                            != SUCCESS) {
           for (op = *sop; op != next_op; op = op->next)
               if (op->Status == STAT_SUCCESS && op->content == CONTENT_BOTH)
                       op->Status  = STAT_CANCEL;
           return(0);
        }
    }
    
    return(num_found);
}
