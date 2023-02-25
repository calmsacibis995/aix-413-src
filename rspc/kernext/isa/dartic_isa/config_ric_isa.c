/* @(#)28        1.1  src/rspc/kernext/isa/dartic_isa/config_ric_isa.c, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:10
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver for ISA cards.
 *
 * FUNCTIONS: darticconfig_isa, cleanup_for_return
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
 */

/**
 *      ARTIC Diag Include files
 */

#include        <darticext_isa.h>

/**
 *      Adapter Memory determination masks (used with initreg1)
 */

#define MEMMASK                 0x28   /* Mask for memory size bits */
#define HALFMEG                 0x20   /* 512 K                     */
#define ONEMEG                  0x00   /* 1 MEG                     */
#define TWOMEG                  0x28   /* 2 MEG                     */

/*
 *
 *      darticconfig_isa
 *
 *
 *      darticconfig_isa is the ARTIC Diag device driver configuration routine.  It is
 *      called by the device configuration methods "cfgdgric_isa" and "uconfigure"  via
 *      the sysconfig system call.  cfgdgric_isa calls it to configure the device, and
 *      unconfigure calls it to unconfigure  the device.  It does the following
 *      during configuration:
 *
 *              1.      Installs driver entry points into the system device switch tables.
 *
 *              2.      Programs the adapter to operating parameters passed by cfgdgric_isa.
 *
 *              3.      Binds the interrupt handler to the interrupt level.
 *
 *              4.      Pins the program module into kernel memory.
 *
 *      It does the following during "unconfiguration":
 *
 *              1.      Deinstalls the driver entry points.
 *
 *              2.      Unbinds the interrupt handler.
 *
 *              3.      Unpins the program module.
 *
 */



#define         DARTIC_INVALID_IO_ADDR                      112
#define         DARTIC_INVALID_INTR_LEVEL                   113
#define         DARTIC_INVALID_CARDNUMBER                   114
#define         DARTIC_SETJMPX_FAILURE                      115
#define         DARTIC_LOCKL_FAILURE                        116
#define         DARTIC_I_CLEAR_FAILURE                      117
#define         DARTIC_I_INIT_FAILURE                       118
#define         DARTIC_UNKNOWN_ADAPTER                      119
#define         DARTIC_UNKNOWN_EXCEPTION                    120
#define         DARTIC_PINCODE_FAILURE                      121
#define         DARTIC_UIOMOVE_FAILURE                      122
#define         DARTIC_INVALID_ADAPTER                      123
#define         DARTIC_DEVSWADD_FAILURE                     124
#define         DARTIC_INVALID_WINDOW_SIZE                  125
#define         DARTIC_DEVSWDEL_FAILURE                     126
#define         DARTIC_UNPINCODE_FAILURE                    127

char    sccsver[64];            /* to hold SCCS identifier string     */

dev_t   articdevicenumber;      /* to hold major/minor device number    */


darticconfig_isa(devno,cmd,uiop)
dev_t       devno;          /* device number (major and minor)      */
int         cmd;            /*      command value                   */
char       *uiop;           /* pointer to uio structure (DDS)       */
{
        DARTIC_Adapter  devinfo;              /*  The device dependent struct */
        ulong       majornum = major(devno);  /*  Major Device Number         */
        int         cardnumber = minor(devno);/*  from minor number           */
        char       *busio_addr;               /*  used to hold MCA I/O address*/
        int         retval;                   /* for return values            */
        int         i,j;                      /* traditional indices          */
        int         amemsize;                 /* read from initreg1           */
        int         adaptmemsize;             /* actual adapter memory size   */
        int         maxpage;                  /* Max page value for mem size  */
        /*
         *      Save major/minor device number
         */

        articdevicenumber = devno;

#ifdef DARTIC_DEBUG
        printf("darticconfig_isa: Entered. ");
        printf(" cmd = %d maj = %d min = %d\n", cmd,majornum,cardnumber);
        printf("           :\n");
#endif

        /*
         *      Obtain lock word for darticconfig_isa - this serializes access to this
         *      function.
         */

        retval = lockl(&config_lock, LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("darticconfig_isa: lockl return %d\n",retval);
#endif

                return(DARTIC_LOCKL_FAILURE);       /* return error code    */
        }

        if (retval = setjmpx(&config_jumpbuffer))
        {
#ifdef DARTIC_DEBUG
                printf("ricdconfig: setjmpx return %d\n",retval);
#endif
                unlockl(&config_lock);            /* release lock word    */

                return(DARTIC_SETJMPX_FAILURE);     /* return error code    */
        }

        /*
         *      Switch on value of cmd
         *
         *      CFG_INIT is the value passed when this function is called via device
         *      method cfgt1pmd.
         *
         *      CFG_TERM is the value passed when this function is called via device
         *      method ucfgdevice
         */

        switch (cmd)
        {
                case    CFG_INIT:                       /* initialize the driver */
                {
                        /*
                         *      If artic_loadcount is 0, then this is the first time ricdconfig
                         *      has been called since IPL.  We do the following operations
                         *      during the first execution ONLY:
                         *
                         *      1.      Pin code and data associated with this module.
                         *      2.      Initialize the adapter table by setting the field "state"
                         *              to DARTIC_EMPTY.
                         *
                         */

#ifdef DARTIC_DEBUG
                                printf("darticconfig_isa: CFG_INIT entered\n");
#endif
                        /*
                         *      COPY THE DEVICE DEPENDENT STRUCTURE (DDS) FROM USER SPACE.
                         *      The DDS is of type DARTIC_Adapter, and will be copied to
                         *      the global table artic_adapter if valid.
                         */

                        retval = uiomove((caddr_t) &devinfo, sizeof(DARTIC_Adapter), UIO_WRITE, uiop);

                        if (retval)             /* uiomove Failed       */
                        {
#ifdef DARTIC_DEBUG
                                printf("darticconfig_isa: uiomove failed, retval = %d\n",
                                          retval);
#endif

                                /*
                                 * cleanup_for_return does unpin (if needed), clrjmpx, and
                                 *      unlockl(&config_lock).
                                 */

                                cleanup_for_return();

                                return(DARTIC_UIOMOVE_FAILURE);             /* return failure code          */
                        }

                        if (artic_loadcount == 0)
                        {
                                /*      Pin interrupt handler in memory */

                                retval = pincode((void *)darticintr_isa);

                                if (retval)             /* pincode Failed       */
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig_isa: pincode() failed, retval = %d\n",
                                                  retval);
#endif
                                        clrjmpx(&config_jumpbuffer);    /* clear exception stack */
                                        unlockl(&config_lock);                  /* release lock word     */
                                        return(DARTIC_PINCODE_FAILURE);             /* return failure code   */
                                }

                                /*      Pin global data in memory */

                                retval = pincode((void *)darticmpx_isa);

                                if (retval)             /* pincode Failed       */
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig_isa: pincode() failed, retval = %d\n",
                                                  retval);
#endif
                                        clrjmpx(&config_jumpbuffer);    /* clear exception stack */
                                        unlockl(&config_lock);                  /* release lock word     */
                                        unpincode((void *)darticintr_isa);
                                        return(DARTIC_PINCODE_FAILURE);             /* return failure code   */
                                }

                                /*      Initialize adapter table */

                                for (i=0 ; i < MAXADAPTERS ; i++)
                                {
                                        /* No adapter present */

                                        artic_adapter[i].state       = DARTIC_EMPTY;

                                        /* -1 indicates no valid value present */

                                        artic_adapter[i].slot        = -1;
                                        artic_adapter[i].cardnumber  = -1;

                                        artic_adapter[i].maxpri      = DEF_MAXPRI;   /* defaults     */
                                        artic_adapter[i].maxtimer    = DEF_MAXTIME;
                                        artic_adapter[i].maxqueue    = DEF_MAXQUEUE;
                                        artic_adapter[i].maxtask     = DEF_MAXTASK;

                                        /* Setup the global structure for doing I/O accesses. */
                                        artic_isa[i].map_io.key     = IO_MEM_MAP;
                                        artic_isa[i].map_io.flags   = 0;
                                        artic_isa[i].map_io.size    = 0x1000;
                                        artic_isa[i].map_io.bid     = devinfo.bus_id;
                                        artic_isa[i].map_io.busaddr = 0;

                                        /* Now setup the global structure for doing MEM accesses. */
                                        artic_isa[i].map_mem.key      = IO_MEM_MAP;
                                        artic_isa[i].map_mem.flags    = 0;
                                        artic_isa[i].map_mem.size     = 0x2000;
                                        artic_isa[i].map_mem.bid
                                              = BID_ALTREG(devinfo.bus_id, ISA_BUSMEM);
                                        artic_isa[i].map_mem.busaddr  = 0;

                                        /*      Zero the interrupt counters  */

                                         artic_intr_count[i] = 0;             /* no intrs yet */
                                         artic_wu_cookie[i]  = EVENT_NULL;    /* for e_sleep  */

                                        /*      Initialize the lock words       */

                                        cpupage_lock[i]  = LOCK_AVAIL;      /* lock available */
                                        pcselect_lock[i] = LOCK_AVAIL;
                                }

                                /*      Initialize artic_procptr queue heads */

                                for (i = 0 ; i < ARTICPA_SIZE ; i++)
                                {
                                        artic_procptr[i] = PANULL;         /* NULL pointer */
                                }
                        }


                        /*
                         *      PARAMETER VALIDITY CHECKS:
                         *
                         *      1)      Adapter number (cardnumber) must be in the range:
                         *
                         *                      0 <= cardnumber < MAXADAPTERS
                         *
                         */

                        /* adapter type check */


                        /*      cardnumber range check */

                        if ( devinfo.cardnumber < 0 || devinfo.cardnumber >= MAXADAPTERS)
                        {
#ifdef DARTIC_DEBUG
                                printf("darticconfig_isa: Invalid cardnumber: %X\n",
                                         devinfo.cardnumber);
#endif

                                /*
                                 * cleanup_for_return does unpin (if needed),clrjmpx and
                                 *  unlockl.
                                 */

                                cleanup_for_return();

                                return(DARTIC_INVALID_CARDNUMBER);  /*  return failure code     */
                        }

                        /* save this value for easy use */

                        cardnumber = devinfo.cardnumber;


                        /*
                         *      If the debug flag is set, print out some stats
                         */

#ifdef DARTIC_DEBUG
                         printf("           : module_id   = %8X  bus_id    = %8X\n",
                                                 devinfo.module_id, devinfo.bus_id  );
                         printf("           : basemem     = %8X  baseio    = %8X\n",
                                                 devinfo.basemem ,devinfo.baseio  );
                         printf("           : dmalevel    = %8X  intlevel  = %8X\n",
                                                 devinfo.dmalevel ,devinfo.intlevel);
                         printf("           : windowsize  = %8X  slot      = %8X\n",
                                                 devinfo.windowsize, devinfo.slot);
                         printf("           : dma memory  = %8X  cardnum   = %8X\n",
                                                 devinfo.dmamem, devinfo.cardnumber);

#endif

                        /*
                         *      If artic_loadcount is 0, then this is the first time darticconfig_isa
                         *      has been called since IPL.  Do the following in this case:
                         *
                         *        1.    Populate artic_switchtable with driver entry points.
                         *        2.    Call devswadd with artic_switchtable
                         */

                        if (artic_loadcount == 0)
                        {

                                artic_switchtable.d_open           = darticopen_isa;    /* Entry Points */
                                artic_switchtable.d_close          = darticclose_isa;
                                artic_switchtable.d_ioctl          = darticioctl_isa;
                                artic_switchtable.d_config         = darticconfig_isa;
                                artic_switchtable.d_select         = nodev;
                                artic_switchtable.d_mpx            = darticmpx_isa;

                                artic_switchtable.d_read           = nodev;        /* No Entry Point       */
                                artic_switchtable.d_write          = nodev;
                                artic_switchtable.d_strategy       = nodev;
                                artic_switchtable.d_revoke         = nodev;
                                artic_switchtable.d_print          = nodev;
                                artic_switchtable.d_dump           = nodev;

                                retval = devswadd(devno,&artic_switchtable);       /* add to table */

                                if (retval)
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig_isa: devswadd failed with %d return\n",
                                                 retval);
#endif
                                      /*
                                       * cleanup_for_return does unpin (if needed),clrjmpx and
                                       *  unlockl.
                                       */

                                        cleanup_for_return();

                                        return(DARTIC_DEVSWADD_FAILURE);        /* return error code */
                                }
                                else
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig_isa: devswadd succeeded.\n");
                                        printf("darticconfig_isa: devno = %X\n",articdevicenumber);
#endif
                                }
                        }

                        /*
                         *      Verify interrupt level is valid
                         */

                         if (intr_mask[devinfo.intlevel] == 0xff)

                         {
                            retval = DARTIC_INVALID_INTR_LEVEL; /* invalid interrupt lev*/

                            if (artic_loadcount == 0) /* sole owner of device */
                            {
                               /* remove device from kernel switchtable */

                               retval = devswdel(devno);

                               if (retval)    /* removal Failed! */
                               {
                                  /* change return code */
                                  retval = DARTIC_DEVSWDEL_FAILURE;
                               }
                            }

                            /*
                             * cleanup_for_return does unpin (if needed),clrjmpx and
                             *      unlockl.
                             */

                            cleanup_for_return();

                            return(retval);
                         }

                        /*
                         *      Copy DDS values into the adapter table.
                         */

                        artic_adapter[cardnumber].state            = DARTIC_NOTREADY;
                        artic_adapter[cardnumber].cardnumber       = cardnumber;
                        artic_adapter[cardnumber].adaptertype      = devinfo.adaptertype;
                        artic_adapter[cardnumber].basetype         = devinfo.basetype;
                        artic_adapter[cardnumber].slot             = devinfo.slot;
                        artic_adapter[cardnumber].intlevel         = devinfo.intlevel;
                        artic_adapter[cardnumber].windowsize       = devinfo.windowsize;
                        artic_adapter[cardnumber].debugflag        = devinfo.debugflag;
                        artic_adapter[cardnumber].module_id        = devinfo.module_id;
                        artic_adapter[cardnumber].basemem          = devinfo.basemem;
                        artic_adapter[cardnumber].baseio           = devinfo.baseio;
                        artic_adapter[cardnumber].bus_id           = devinfo.bus_id;

                        /*
                         *      Initialize adapter so memory can be accessed
                         */

                        artic_initialize(cardnumber);

                        delay(HZ);      /* delay for 1 second   */


                        /*
                         *      Poll adapter to see if adapter passed self-test.
                         *      function "isbrdready" returns DARTIC_READY or DARTIC_NOTREADY.
                         */

                        for  (i = 0 ; i < 30 ; i++)
                        {
                                if (isbrdready(cardnumber) == DARTIC_READY)
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig_isa: isbrdready returned DARTIC_READY iteration  %d\n",i);
#endif
                                        artic_adapter[cardnumber].state = DARTIC_READY;
                                        break;
                                }
                                else                            /* not ready yet                */
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig_isa: isbrdready iteration %d\n",i);
#endif

                                        delay(HZ);              /* sleep for 1 second   */
                                }
                        }

#ifdef DARTIC_DEBUG
                        printf("darticconfig_isa: artic_adapter[%d].state = %d\n",
                                        cardnumber,artic_adapter[cardnumber].state);
#endif
                        /*
                         *      If adapter is still not ready, then issue a hardware
                         *      reset to the adapter.  IF still not ready, then mark
                         *      the adapter as DARTIC_BROKEN.
                         */

                        if (artic_adapter[cardnumber].state != DARTIC_READY)
                        {
                                /*      Still Not Ready!  Issue hardware reset! */

                                if (boardreset(cardnumber))
                                {
                                        artic_adapter[cardnumber].state = DARTIC_BROKEN;
#ifdef DARTIC_DEBUG
                                        printf("darticconfig_isa: boardreset FAILED.  Marking adapter as BROKEN.\n");
#endif

                                        retval = DARTIC_SELFTEST_FAILURE;

                                        if (artic_loadcount == 0)          /* sole owner of device */
                                        {
                                                /* remove device from kernel switchtable */

                                                devswdel(devno);
                                        }

                                        /*
                                         *      cleanup_for_return does unpin (if needed),clrjmpx
                                         *      and unlockl.
                                         */

                                        cleanup_for_return();

                                        return(retval);
                                }
                        }

                        /*
                         *      Determine the adapter memory size
                         *      1.      read initreg1 and mask with 0x28
                         *      2.      switch on value and adapter type.
                         *      3.      Upper 64K of Multiport/2 and x25 adapters unreadable for
                         *              1 Meg versions.
                         */

                        amemsize = MEMMASK & readdreg(cardnumber,INITREG1);

                        switch (amemsize)
                        {
                                case ONEMEG:
                                {
                                        adaptmemsize = 0x100000;                /* access 1Meg  */
                                        break;
                                }

                                case TWOMEG:
                                {
                                        adaptmemsize = 0x200000;                /* access 2 Meg */
                                        break;
                                }

                                case HALFMEG:
                                {
                                        adaptmemsize = 0x80000;                 /* Only access 512K     */
                                        break;
                                }

                                /* Use 512 K as the default */
                                default:
                                {
                                        adaptmemsize = 0x80000;                 /* Only access 512K     */
                                        break;
                                }
                        }

                        /*
                         *      Now set maxpage based on chosen window size
                         */

                        switch (devinfo.windowsize)
                        {
                                case 0x2000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0xff;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x7f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x77;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x3f;
                                                        break;
                                        }
                                        break;
                                }
                                case 0x4000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0x7f;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x3f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x3b;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x1f;
                                                        break;
                                        }
                                        break;
                                }
                                case 0x8000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0x3f;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x1f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x1d;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x0f;
                                                        break;
                                        }
                                        break;
                                }
                                case 0x10000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0x1f;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x0f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x0e;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x07;
                                                        break;
                                        }
                                        break;
                                }
                        }

                        artic_adapter[cardnumber].maxpage = maxpage;

#ifdef DARTIC_DEBUG
                        printf("darticconfig_isa:  maxpage    = %X\n", maxpage);
#endif

                        /*
                         *      Bind interrupt handler to kernel
                         */

                        artic_intr[cardnumber].cardnumber = cardnumber;

                        artic_intr[cardnumber].artic_interrupt.next     = (struct intr *) 0;

                        artic_intr[cardnumber].artic_interrupt.handler  =  darticintr_isa;

                        artic_intr[cardnumber].artic_interrupt.level    = devinfo.intlevel;

                        artic_intr[cardnumber].artic_interrupt.bid      = devinfo.bus_id;

                        artic_intr[cardnumber].artic_interrupt.bus_type = BUS_BID;

                        artic_intr[cardnumber].artic_interrupt.flags    = 0;

                        artic_intr[cardnumber].artic_interrupt.priority = INTCLASS3;

                        retval = i_init(&artic_intr[cardnumber]);

                        /*      If i_init failed, then clean up and return */

                        if (retval)
                        {
                                retval = DARTIC_I_INIT_FAILURE;     /* i_init failed                */

                                if (artic_loadcount == 0)          /* sole owner of device */
                                {
                                        /* remove device from kernel switchtable */

                                        retval = devswdel(devno);

                                        if (retval)             /* removal Failed!      */
                                        {
                                                /* change return code */
                                                retval = DARTIC_DEVSWDEL_FAILURE;
                                        }
                                }

                                /*
                                 *      cleanup_for_return does unpin (if needed),clrjmpx
                                 *      and unlockl.
                                 */

                                cleanup_for_return();

                                return(retval);
                        }


                        /*
                         * Turn on RCM interrupts if we successfully attached
                         *      interrupt handler
                         */


                        outbyte_isa( cardnumber, devinfo.baseio + COMREG, INTENABLE);

#ifdef DARTIC_DEBUG
                        printf("darticconfig_isa: Interrupts enabled.\n");
#endif

                        artic_loadcount++;                 /* increment the load counter */

                        break;
                }

                /*
                 *      Terminate device
                 */

                case    CFG_TERM:
                {

#ifdef DARTIC_DEBUG
                        printf("darticconfig_isa: Load count = %d\n", artic_loadcount);
                        printf("darticconfig_isa: Card number = %d\n", cardnumber);
#endif

                        /*
                         *      Range check the cardnumber
                         */

                        if (cardnumber < 0 || cardnumber >= MAXADAPTERS)
                        {
                                cleanup_for_return();
                                return(DARTIC_INVALID_ADAPTER);
                        }

                        retval = 0;        /* preset to 0 in case not used */

                        artic_loadcount--;   /* decrement the load counter   */

                        artic_adapter[cardnumber].state = DARTIC_NOTREADY;     /* mark as Gone */

                        /*
                         *      Unbind interrupt handler from kernel
                         */

                        i_clear(&artic_intr[cardnumber]);

                        if (artic_loadcount == 0)
                        {

                                /*
                                 *      Remove driver from kernel device switch table
                                 */

                                retval = devswdel(devno);

                                /*
                                 *      Unpin driver module
                                 */

                                unpincode((void *) darticintr_isa);
                                unpincode((void *) darticmpx_isa);
                        }

                        break;
                }

                default:
                {
                        break;
                }
        }


#ifdef DARTIC_DEBUG
        printf("darticconfig_isa: exiting.\n");
#endif
        clrjmpx(&config_jumpbuffer);    /* clear exception stack        */
        unlockl(&config_lock);          /* relinquish lock word         */
        return (0);                     /* Return 0 for Success         */
}


/*
 *      cleanup_for_return
 *
 *      This function is used Exclusively in darticconfig_isa of the device driver.
 *      It does the following:
 *
 *              1.      IF artic_loadcount == 0, this indicates that this invokation has
 *                      sole possition of the device.  That means we can finish cleanup
 *                      by unpinning the kernel memory that we are occupying.
 *
 *              2.      Clear our jump buffer from the exception stack by calling clrjmpx.
 */

void
cleanup_for_return()
{
        if (artic_loadcount == 0)                  /* If only user of device  */
        {
                unpincode((void *) darticintr_isa);/*   unpin the code        */
                unpincode((void *) darticmpx_isa); /*   unpin the data        */
        }
        clrjmpx(&config_jumpbuffer);    /* clear exception stack */
        unlockl(&config_lock);          /* release lock word     */

        return;
}

