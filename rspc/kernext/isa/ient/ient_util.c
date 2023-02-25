static char sccsid[] = "@(#)80  1.2.1.1  src/rspc/kernext/isa/ient/ient_util.c, isaent, rspc41B, 412_41B_sync 1/6/95 15:47:15";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: 
 *        ient_multi_add
 *        ient_multi_del
 *        ient_cdt_func
 *        ient_add_cdt
 *        ient_del_cdt
 *        ient_tracetable
 *        ient_logerr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
 * NAME:     ient_tracetable
 *
 * FUNCTION: Put a tracetable into the internal tracetable table and the
 *           external system tracetable.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *           This routine is only called through macros when DEBUG is defined.
 *    
 * CALLED FROM:
 *           Every routine in the driver 
 *
 * INPUT:
 *         hook            - tracetable hook 
 *         tag             - four letter tracetable ID
 *         arg1 to arg 3   - arguments to tracetable
 *
 * RETURNS:  
 *         none.
 */
/*****************************************************************************/

void
ient_tracetable(ulong    hook,    /* tracetable hook */
                char     *tag,    /* tracetable ID */
                ulong    arg1,    /* 1st argument */
                ulong    arg2,    /* 2nd argument */
                ulong    arg3)    /* 3rd argument */

{     
    int i;
    int ipri;

    ipri = disable_lock(PL_IMP, &TRACE_LOCK);

    /*
    ** Copy the tracetable point into the internal tracetable table
    */

    i = dd_ctrl.tracetable.next_entry;

    dd_ctrl.tracetable.table[i] = *(ulong *)tag;
    dd_ctrl.tracetable.table[i+1] = arg1;
    dd_ctrl.tracetable.table[i+2] = arg2;
    dd_ctrl.tracetable.table[i+3] = arg3;
    
    if ((i += 4) < IEN_ISA_TRACE_SIZE)
    {
        dd_ctrl.tracetable.table[i] = IEN_ISA_TRACE_END;
        dd_ctrl.tracetable.next_entry = i;
    }
    else
    {
        dd_ctrl.tracetable.table[0] = IEN_ISA_TRACE_END;
        dd_ctrl.tracetable.next_entry = 0;
    }
 
    unlock_enable(ipri, &TRACE_LOCK);

    /*
    ** Call the external tracetable routine
    */

    TRCHKGT(hook | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);
    return;
}
