/* @(#)33        1.1  src/rspc/kernext/isa/dartic_isa/open_ric_isa.c, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:25
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver for the ISA bus.
 *
 * FUNCTIONS: darticopen_isa
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
 *      darticopen_isa
 *
 *      darticopen_isa is the device driver open function.  It is called when
 *      a user issues an open system call on the /dev/artic device.  darticopen_isa
 *      allocates an DARTIC_Proc structure for the process, and places it on a
 *      linked list pointed to by artic_procptr[index], where index is
 *
 *                      index = PID % ARTICPA_SIZE.
 */

darticopen_isa(devno,oflag,ext)
dev_t    devno;             /* major/minor device number    */
int      oflag;             /* open flags (ignored)         */
char    *ext;               /* extended parameters (ignored)*/
{
        int             i;          /* used for loop index                  */
        int             retval;     /* return value from kernel services    */
        int             index;      /* used to index artic_procptr            */
        struct DARTIC_Proc  *artic_ptr; /* pointer to per-process structure     */
        struct DARTIC_Proc  *lst_ptr; /* "current" pointer for linked list    */

#ifdef DARTIC_DEBUG
        printf("darticopen_isa: Entered.\n");
#endif

        /*
         *      Determine if this process has already opened the device driver.
         *      If it has, then there will be an DARTIC_Proc structure already allocated
         *      for this process, and lookup_articproc will return a pointer.  If
         *      lookup_articproc returns PANULL (null pointer), then there is no DARTIC_Proc
         *      structure allocated for this process, and therefore it has not yet
         *      opened the driver.
         */

        artic_ptr = lookup_articproc();    /* uses kernel service getpid() */

        if (artic_ptr != PANULL)
        {
#ifdef DARTIC_DEBUG
                printf("darticopen_isa: process %d already open error.\n",
                          getpid() );
#endif

                return(E_ICA_ALREADY_OPEN);       /* return error code    */
        }


        /*
         *      Allocate a new process structure from kernel memory
         */

        artic_ptr = xmalloc(sizeof(struct DARTIC_Proc), 1, kernel_heap);

        if (artic_ptr == PANULL)  /* unable to allocate memory */
        {
                return (E_ICA_XMALLOC_FAIL);
        }

        /*
         *      Initialize structure
         */


        artic_ptr->myPID    = getpid();   /* processes PID        */
        artic_ptr->next     = PANULL;             /* NULL link ptr        */
        artic_ptr->tocfcount = 0;                 /* no additional callout entries added
                                                                                                to kernel               */

#ifdef DARTIC_DEBUG
        printf("darticopen_isa: artic_ptr = %X pid = %d\n",
                                        artic_ptr,artic_ptr->myPID);
#endif

        for (i=0 ; i<MAXADAPTERS ; i++)      /* NULL all ProcReg pointers    */
        {
                artic_ptr->prptr[i]    = PRNULL;      /* NULL ProcReg ptr     */
        }

        /*      The index to the pointer array is calculated by dividing the
         *      process ID by the array size (ARTICPA_SIZE) and taking the remainder:
         */

        index = getpid() & ARTICPA_MASK;   /* same as PID % ARTICPA_SIZE */

        /*
         *      Lock access to artic_procptr array while we add new structure
         *      to list
         */

        retval = lockl(&proc_lock, LOCK_SIGRET);

        if (retval)                     /* interrupted by a signal! */
        {
                xmfree(artic_ptr,kernel_heap);    /* free allocated memory        */

                return(EINTR);        /* return signal error code     */
        }

        lst_ptr = artic_procptr[index]; /* first element of linked list */

        if (lst_ptr == PANULL)
        {
#ifdef DARTIC_DEBUG
                printf("darticopen_isa: lst_ptr = PANULL, list was empty\n");
#endif

                artic_procptr[index] = artic_ptr;    /* First on the list            */
        }
        else                                                            /* Find end of list                     */
        {
                while (lst_ptr->next != PANULL)      /* while link ptr is not null   */
                {
                        lst_ptr = lst_ptr->next;     /* next element in linked list  */

#ifdef DARTIC_DEBUG
                        printf("darticopen_isa: lst_ptr = %X\n",lst_ptr);
#endif
                }
                lst_ptr->next = artic_ptr;             /* add new structure to list    */
        }

#ifdef DARTIC_DEBUG
        printf("darticopen_isa: artic_ptr = %X\n",artic_ptr);
#endif

        /*
         *      Done with artic_procptr array.  Unlock it.
         */

        unlockl(&proc_lock);

#ifdef DARTIC_DEBUG
        printf("darticopen_isa: Normal Exit.\n");
#endif

        return(NO_ERROR);                             /*      Success */
}

