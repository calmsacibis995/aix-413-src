/* @(#)27        1.1  src/rspc/kernext/isa/dartic_isa/close_ric_isa.c, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:09
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver for the ISA bus.
 *
 * FUNCTIONS: darticclose_isa
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 */

/**
 *      ARTIC Diag Include files
 */

#include        <darticext_isa.h>

/*
 *      darticclose_isa
 *
 *      darticclose_isa is the device driver close function.  The close function
 *      is responsible for freeing any resources that the device driver has
 *      allocated on behalf of the process.  For ARTIC Diag, the process will have
 *      allocated memory to store the following:
 *
 *              DARTIC_Proc structure
 *              ProcReg structures
 */


darticclose_isa(devno,flag,ext)
dev_t    devno;   /* major/minor device number  */
int      flag;    /* multiplexed channer number (ignored) */
caddr_t  ext;     /* extension parameter (ignored) */
{
        int              i,retval;   /* index, returned value from lockl   */
        struct DARTIC_Proc *artic_ptr;   /* pointer to per-process structure   */
        struct DARTIC_Proc *lst_ptr;   /* "current" pointer for linked list  */
        struct ProcReg   *prptr;     /* pointer to ProcReg for removal     */
        struct ProcReg   *lastptr;   /* pointer to ProcReg for removal     */

#ifdef DARTIC_DEBUG
        printf("darticclose_isa: Entered.\n");
#endif

        /*
         *      Lock access to artic_procptr array so we can remove our DARTIC_Proc struct
         */

        retval = lockl(&proc_lock, LOCK_SIGRET);

        if (retval)                       /* interrupted by a signal! */
        {
                return(EINTR);            /* return signal error code */
        }

        /*
         *      Function remove_articproc is called to remove the structure from
         *      the linked list pointed to by artic_procptr.  It returns the pointer
         *      to the structure, so we can free allocated ProcReg memory before
         *      freeing the DARTIC_Proc memory.
         */

        artic_ptr = remove_articproc();       /* delete from artic_procptr list  */

        retval = 0;                      /* Preset to successful return..*/

        if (artic_ptr == PANULL)           /* could not find our struct    */
        {
#ifdef DARTIC_DEBUG
                printf("darticclose_isa: could not find DARTIC_Proc.\n");
#endif

                retval = EBADF;          /* Could Not find our Structure */
        }
        else
        {
#ifdef DARTIC_DEBUG
                printf("darticclose_isa: artic_ptr = %X pid = %d\n",
                          artic_ptr, artic_ptr->myPID);
#endif

                /*
                 *      Free any ProcReg structures that have been allocated for
                 *      this process
                 */

                for (i = 0 ; i<MAXADAPTERS ; i++)
                {
                        /* Interrupt Register ProcReg structs   */

                        prptr = artic_ptr->prptr[i];

                        while (prptr != PRNULL)
                        {
                                lastptr = prptr;   /* save current pointer */
                                prptr = prptr->next;/* get next link       */
                                xmfree(lastptr,kernel_heap); /* free current pointer    */
#ifdef DARTIC_DEBUG
                                printf("darticclose_isa: IR xmfree (%X) \n",lastptr);
#endif
                        }

                }

                /*      check tocfcount to  see how many times timeoutcf(1) was called */

                for (i=0 ; i < artic_ptr->tocfcount ; i++)
                {
                        timeoutcf(-1);          /* decrement callout table entries      */
                }

                xmfree(artic_ptr,kernel_heap);    /* free allocated memory    */
        }

        unlockl(&proc_lock);           /* unlock proc_lock access lock */

#ifdef DARTIC_DEBUG
        printf("darticclose_isa: Exit.  retval = %d\n",retval);
#endif

        return(retval);           /* return either 0 or EBADF */
}
