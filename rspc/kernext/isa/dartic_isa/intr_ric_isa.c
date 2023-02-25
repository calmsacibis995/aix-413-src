/* @(#)30        1.1  src/rspc/kernext/isa/dartic_isa/intr_ric_isa.c, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:13
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver.
 *
 * FUNCTIONS: darticintr_isa, ackRCMintr, artic_time_func, setCPUpage
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
 *      WARNING !!!!!  This entire object module is to be pinned. None
 *      of these routines should be moved to another source file unless
 *      that files object module is also pinned.
 */

/**
 *      ARTIC Diag Include files
 */

#include        <darticext_isa.h>

/*
 *      darticintr_isa
 *
 *      This is the interrupt handler for the ARTIC Diag device driver.
 *      It determines if the interrupt source was an ARTIC Diag adapter,
 *      and if so, determines the interrupting task's number.  The
 *      interrupt count for the task is incremented, and an e_wakeup
 *      is done on the counts address.
 */


darticintr_isa(handler)
struct DARTIC_IntrPlus *handler;
{
        int     coprocnum;    /* coprocessor number goes here   */
        int     retval;       /* retval from setjmpx            */
        uchar   taskregvalue; /* value read from task register  */

        /*
         *      Set up an exception handler.  setjmpx will return non-zero if
         *      is entered from an exception condition.   This will prevent
         *      the default action for unforseen exceptions, which is to "assert"
         *      (a nice name for system crash).
         */

        if (retval = setjmpx(&oaintr_jumpbuffer))
        {
#ifdef DARTIC_DEBUG
                printf("darticintr: setjmpx returned %d\n",retval);
#endif

                i_reset(handler);

                return(INTR_SUCC);
        }

#ifdef DARTIC_DEBUG
        printf("darticintr: Entered.\n");
#endif

        coprocnum = handler->cardnumber;           /* coprocessor number   */

        retval = INTR_FAIL;                        /* interrupt not for us */

        /*
         *      Read task register
         */

        taskregvalue = inbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + TASKREG);

        /*
         *      If value is not 0xFF, then we have an interrupting task .
         *
         *      Note: We acknowledge the interrupt by writing a 0xFF to
         *                the INTID byte of the IB (interface block).  This
         *                is done in the rcm_intr() and task_intr() subroutines.
         */

        switch (taskregvalue)
        {
                case    0xFF:
                {
                        break;          /* nothing from this coprocnum */
                }
                default:                /* interrupt from a task       */
                {

#ifdef DARTIC_DEBUG
                        printf("darticintr: Adapter %d Task %d Interrupt.\n",
                                                        coprocnum,(int)taskregvalue);
#endif

                        /* increment interrupt count    */

                        artic_intr_count[coprocnum]++;

                        /* Wakeup sleeping processes    */

                        e_wakeup(&artic_wu_cookie[coprocnum]);

                        ackRCMintr(coprocnum);    /* ack the intr         */
                        retval = INTR_SUCC;       /* interrupt was for us */

                        break;
                }
        }

        i_reset(handler);

        clrjmpx(&oaintr_jumpbuffer);

#ifdef DARTIC_DEBUG
        printf("darticintr: exiting.\n");
#endif

        return(retval);

}


/*
 *      ackRCMintr()
 *
 *      This routine acknowledges an interrupt from the coprocessor
 *      board by writing the value 0xFF to address 0x441 on the board.
 *      This address is the INTID byte of the IB (Interface Block).
 */

ackRCMintr(coprocnum)
int coprocnum;
{
        uchar   lastpage;
        uchar   page;
        char   *dpram;
        char   *mem_io_start;
        int isec_array[] = {
                                0x0,            /* 0 not valid */
                                0x0,            /* 1 not valid */
                                0x2F2,          /* Interrupt Level 02 */
                                0x2F3,          /* Interrupt Level 03 */
                                0x2F4,          /* Interrupt Level 04 */
                                0x0,            /* 5 not valid */
                                0x0,            /* 6 not valid */
                                0x2F7,          /* Interrupt Level 07 */
                                0x0,            /* 8 not valid */
                                0x2F2,          /* Interrupt Level 09 */
                                0x6F2,          /* Interrupt Level 10 */
                                0x6F3,          /* Interrupt Level 11 */
                                0x6F4,          /* Interrupt Level 12 */
                                0x0,            /* 13 not valid */
                                0x0,            /* 14 not valid */
                                0x6F7           /* Interrupt Level 15 */
                            };


        /*
         *      Set page register to show the Interface Block in the window.
         *      Save the last page value so we can restore it.
         */

#ifdef DARTIC_DEBUG
        printf("ackRCMintr: Entered.\n");
#endif

        page     = 0;
        lastpage = setCPUpage(coprocnum,page);

        /*      Ack the inter */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        BUS_PUTC(dpram + IBADDR + INTID, 0xFF);
        eieio();

        if (isec_array[artic_adapter[coprocnum].intlevel] != 0x0)
           outbyte_isa( coprocnum, isec_array[artic_adapter[coprocnum].intlevel], 0);

        iomem_det(mem_io_start);

        /*      restore page value */

        setCPUpage(coprocnum,lastpage);

#ifdef DARTIC_DEBUG
        printf("ackRCMintr: exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      setCPUpage
 *
 *      This programs the CPUPAGE register for the specified adapter
 *      with the value passed in the page parameter.
 */

uchar
setCPUpage(coprocnum,page)
int     coprocnum;
uchar   page;
{
        uchar    lastpage;              /* last value for page                  */
        char    *busio_addr;    /* address of CPUPAGE register  */

#ifdef DARTIC_DEBUG
        printf("setCPUpage: Entered. page = %d\n",(int)page);
#endif

        lastpage = inbyte_isa(coprocnum, artic_adapter[coprocnum].baseio+CPUPAGE);
        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio+CPUPAGE, page);


#ifdef DARTIC_DEBUG
        printf("setCPUpage: exiting. lastpage = %d\n",(int)lastpage);
#endif

        return(lastpage);
}

/*
 *      artic_time_func
 *
 *      Called after a timeout in icaintwait
 *
 */

void
artic_time_func(wakupaddr)
int     *wakupaddr;
{
#ifdef DARTIC_DEBUG
        printf("artic_time_func: Called.\n");
#endif

        e_wakeup(wakupaddr);    /* Wake Up Sleepers */
}

