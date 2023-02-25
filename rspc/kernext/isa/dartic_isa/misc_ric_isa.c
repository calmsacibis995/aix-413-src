/* @(#)32        1.1  src/rspc/kernext/isa/dartic_isa/misc_ric_isa.c, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:17
 * COMPONENT_NAME: dd_artic_isa -- ARTIC diagnostic driver for the ISA bus.
 *
 * FUNCTIONS: boardreset, isbrdready
 *            artic_put_pcselect, artic_get_pcselect
 *            artic_put_cmd, artic_get_bcb, to16local
 *            artic_get_psb, readdreg, writedreg
 *            inteltolocal, localtointel, lookup_articproc
 *            remove_articproc, intboard, artic_read_mem
 *            artic_write_mem
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
 *      boardreset
 *
 *      This routine does a hardware reset of the coprocessor
 *  board.  It is called by ioctl function artic_icareset.
 */


boardreset(coprocnum)
int     coprocnum;
{
        int      retval, i;
        uchar   comregvalue;
        uchar   initreg1;
        uchar   cc_enable;

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("boardreset: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("boardreset: setjmpx returned %d\n",retval);
#endif
                unlockl(&cpupage_lock[coprocnum]);              /* release lock word    */

                return(EIO);                                                    /* return error code    */
        }

        artic_adapter[coprocnum].state = DARTIC_NOTREADY;      /* mark adapter as not ready */

        /*
         *      Do the hardware reset by setting COMREG to CR_RESETVAL (0x11),
         *      then 00, then to INTENABLE (0x10) to enable interrupts.
         */

        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + COMREG,
                    CR_RESETVAL);
        eieio();
        delay(1);
        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + COMREG,
                    (uchar) 0);
        eieio();
        delay(1);
        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + COMREG,
                    (uchar) INTENABLE);
        delay(HZ);  /* Wait for one second */


        /*
         *      Now loop with sleep waiting for the board to reset.
         */

        retval = -1;

        for  (i = 0 ; i < 30 ; i++)
        {
                if ((artic_adapter[coprocnum].state = isbrdready(coprocnum)) == DARTIC_READY)
                {
                        retval = 0;
                        break;
                }
                else
                {
                        /*
                         *      sleep for 1 second       (delay in units of 1/HZ seconds)
                         */
                        delay(HZ);
#ifdef DARTIC_DEBUG
                        printf("boardreset: iteration %d....\n",i);
#endif
                }
        }

        clrjmpx(&artic_jumpbuffers[coprocnum]); /* clear exception stack   */

        unlockl(&cpupage_lock[coprocnum]);      /* unlock cpupage lock     */

#ifdef DARTIC_DEBUG
        printf("boardreset: iteration %d: brdready(%d) returned %d\n",
                  i,coprocnum,artic_adapter[coprocnum].state);
#endif

        return(retval);
}

/*
 *      isbrdready
 *
 *              This routine determines if the board is ready by checking the
 *      PROM READY bit in the INITREG1 register (on the board).  If this
 *      bit is set, then the board PROM completed its initialization and POST.
 *
 *              We check this bit by first programming the PTRREG, and then reading
 *      INITREG1 from the DREG register.
 */

isbrdready(coprocnum)
int coprocnum;
{
        uchar    value = 0x00;
        uchar    psb;
        uchar    sig;
        uchar    *mem_io_start;
        uchar    *dpram;

        /*
         *      Call readdreg to read INITREG1 from DREG after programming PTRREG.
         */

        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + PTRREG, 0xAA);
        eieio();
        value =  inbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + PTRREG);
        eieio();

#ifdef DARTIC_DEBUG
        printf("isbrdready: inbyte_isa(%X,%X) returns %X\n",
                  coprocnum,artic_adapter[coprocnum].baseio+PTRREG,value);
#endif

        if (value == 0xAA)
        {
           mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
           dpram = mem_io_start + artic_adapter[coprocnum].basemem + 0x40C;
           sig = BUSIO_GETC(dpram);
           eieio();
           if (sig == 0xBA)
           {
              dpram = mem_io_start + artic_adapter[coprocnum].basemem + 0x47C;
              psb = BUSIO_GETC(dpram);
              eieio();
              iomem_det(mem_io_start);
              if (psb != 0xC0)
                 return(DARTIC_READY);     /* PROM completed initialization and POST */
              else
                 return(DARTIC_NOTREADY);
           }
           else
           {
              iomem_det(mem_io_start);
              return(DARTIC_NOTREADY);
           }
        }
        else
        {
           return(DARTIC_NOTREADY);    /* PROMREADY bit not set!  */
        }
}

/**
 *      Adapter Memory determination masks (used with initreg1)
 */

#define C1X_INIT1_VAL           0x0A


/*
 *      artic_initialize
 *
 *      Initializes adapter so that memory can be accessed
 *
 */
void artic_initialize(coprocnum)
int coprocnum;
{
int         intr_lvl, crd_base, val;
uint        mem_addr, ioaddr;

        intr_lvl = intr_mask[artic_adapter[coprocnum].intlevel];
        crd_base = (((artic_adapter[coprocnum].baseio - 0x2A0)/0x400) << 4);
        val = (intr_lvl | crd_base );

        /* write adapter number and interrupt level */
        outbyte_isa( coprocnum, artic_adapter[coprocnum].baseio + PTRREG, INITREG0);
        eieio();
        outbyte_isa( coprocnum, artic_adapter[coprocnum].baseio + DREG, val);
        eieio();

        /* write adapter bus configuration */
        outbyte_isa( coprocnum, artic_adapter[coprocnum].baseio + PTRREG, INITREG1);
        eieio();
        outbyte_isa( coprocnum, artic_adapter[coprocnum].baseio + DREG, C1X_INIT1_VAL);
        eieio();

        /* write adapter memory base address */
        mem_addr = (ushort)((artic_adapter[coprocnum].basemem & 0xF00000)>>20);
        mem_addr |= (ushort)((artic_adapter[coprocnum].basemem & 0x000FE000) >> 5);

        ioaddr=iomem_att ( &(artic_isa[coprocnum].map_io));
        BUSIO_PUTS(ioaddr + artic_adapter[coprocnum].baseio, mem_addr);
        iomem_det(ioaddr);
        eieio();

        /* Enable memory */
        outbyte_isa( coprocnum, artic_adapter[coprocnum].baseio + CPUPAGE, 0);
        eieio();

        return;

}

/*
 *      artic_put_pcselect
 *
 *      artic_put_pcselect is used to put a value in the PC Select
 *      byte of RCM's Interface Block.
 */

artic_put_pcselect(coprocnum, pcselect)
int     coprocnum;        /* coprocessor number          */
uchar   pcselect;         /* Value to put in PC select   */
{
        int     retval;
        caddr_t dpram;
        caddr_t mem_io_start;
        uchar   lastpage;

#ifdef DARTIC_DEBUG
        printf("artic_put_pcselect: Entered.\n");
#endif

        /*
         *      NOTE: CPUPAGE register should be locked prior to this call
         */

        /* position window over IB page 0 */

        lastpage = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to ISA memory bus */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        /*      Put the value in the PC Select byte */
        BUS_PUTC(dpram + IBADDR + PC_SELECT, pcselect);
        eieio();


#ifdef DARTIC_DEBUG
        printf("artic_put_pcselect: pcselect = %d\n",(int)pcselect);
#endif

        /* detach from ISA bus  */
        iomem_det(mem_io_start);


        /*      replace CPUPAGE value   */

        setCPUpage(coprocnum,lastpage);

#ifdef DARTIC_DEBUG
        printf("artic_put_pcselect: exiting.\n");
#endif

        return(NO_ERROR);                                                       /* indicate success             */
}

/*
 *      artic_get_pcselect
 *
 *      artic_get_pcselect is used to retrieve the PC Select
 *      Byte from RCM's Interface Block.  The PCS is returned
 *      in *pcsptr.  This is because the PCS may have any
 *      value, and we need a way to indicate failure.
 */

artic_get_pcselect(coprocnum, pcsptr)
int coprocnum;                  /* coprocessor number           */
uchar   *pcsptr;                /* char for return value        */
{
        int      retval;
        caddr_t  dpram;
        caddr_t  mem_io_start;
        uchar    lastpage;

        /*      Lock the cpupage_lock to serialize access       */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_pcselect: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_pcselect: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);                      /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

#ifdef DARTIC_DEBUG
        printf("artic_get_pcselect: Entered.\n");
#endif

        /* position window over IB page 0 */

        lastpage = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to ISA memory bus */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        /*      Retrieve the PC Select Byte     */

        *pcsptr = BUS_GETC(dpram + IBADDR);
        eieio();

        /* detach from ISA bus  */

        iomem_det(mem_io_start);

        /*      replace CPUPAGE value   */

        setCPUpage(coprocnum,lastpage);

        /*      Clear exception stack           */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock access to adapter        */

        unlockl(&cpupage_lock[coprocnum]);

#ifdef DARTIC_DEBUG
        printf("artic_get_pcselect: exiting.\n");
#endif

        return(NO_ERROR);                                                       /* indicate success             */
}


/*
 *      artic_put_cmd
 *
 *      artic_put_cmd is used to retrieve the Buffer Control
 *      Block for a specified task.
 *
 *      The return value is either 0 for success, EINTR
 *      if lockl fails, and EIO if an exception occurs
 *      (setjmpx returns non-zero).
 */

artic_put_cmd(coprocnum, cmdcode)
int      coprocnum;             /* coprocessor number       */
uchar   cmdcode;                /* command code for BCB     */
{
        int      retval;      /* return value from lockl      */
        char    *dpram;       /* pointer to adapter memory    */
        char    *mem_io_start;
        ushort   bcboffset;   /* offset of BCB                */
        uchar    lastvalue;   /* previous value in CPUPAGE    */
        uchar    temp[2];

#ifdef DARTIC_DEBUG
        printf("artic_put_cmd: Entered.\n");
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_put_cmd: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_put_cmd: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /* position window over IB page 0 */

        lastvalue = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to ISA memory bus */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        /*      Get offset of specified BCB     */

        temp[0] = BUS_GETC(dpram + IBADDR + BCB_OFF);
        eieio();
        temp[1] = BUS_GETC(dpram + IBADDR + BCB_OFF + 1);
        eieio();
        bcboffset = (temp[1] << 8) | temp[0];

        /*
         *      Put command code in the BCB
         */

#ifdef DARTIC_DEBUG
        printf("*** artic_put_cmd: Putting %d in BCB ***\n", cmdcode);
#endif
        BUS_PUTC((dpram + bcboffset + CMD), cmdcode);
        eieio();
        iomem_det(mem_io_start);                /* detach from ISA bus */

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                            */

#ifdef DARTIC_DEBUG
        printf("artic_put_cmd: Exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_get_bcb
 *
 *      artic_get_bcb is used to retrieve the Buffer Control
 *      Block for task 0.
 *
 *      The return value is either 0 for success, EINTR
 *      if lockl fails, and EIO if an exception occurs
 *      (setjmpx returns non-zero).
 */

artic_get_bcb(coprocnum, bcbptr)
int                  coprocnum;  /* coprocessor number    */
ICAGETBUFADDRS_PARMS *bcbptr;    /* char for return value */
{
        int      retval;     /* return value from lockl      */
        char    *dpram;      /* pointer to adapter memory    */
        char     *mem_io_start;
        uchar    lastvalue;  /* previous value in CPUPAGE    */
        ushort   bcboffset;  /* offset of specified BCB      */
        uchar    temp[2];

#ifdef DARTIC_DEBUG
        printf("artic_get_bcb: Entered.\n");
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_bcb: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_bcb: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /* position window over IB page 0 */

        lastvalue = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to ISA memory bus */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        /*      Get offset of specified BCB     */

        temp[0] = BUS_GETC(dpram + IBADDR + BCB_OFF);
        eieio();
        temp[1] = BUS_GETC(dpram + IBADDR + BCB_OFF + 1);
        eieio();
        bcboffset = (temp[1] << 8) | temp[0];


#ifdef DARTIC_DEBUG
        printf("          : bcboffset = %X\n",bcboffset);
#endif

        /*
         *      Retrieve the BCB, swapping bytes for offset and length fields.
         */

        /*      Output buffer */

        temp[0] = BUS_GETC(dpram + bcboffset + OUTLNG);
        eieio();
        temp[1] = BUS_GETC(dpram + bcboffset + OUTLNG + 1);
        eieio();
        bcbptr->ob.length = (temp[1] << 8) | temp[0];
        temp[0] = BUS_GETC(dpram + bcboffset + OUTOFF);
        eieio();
        temp[1] = BUS_GETC(dpram + bcboffset + OUTOFF + 1);
        eieio();
        bcbptr->ob.offset = (temp[1] << 8) | temp[0];
        bcbptr->ob.page = BUS_GETC(dpram + bcboffset + OUTPAG);
        eieio();

#ifdef DARTIC_DEBUG2
        printf("          : ob.page   = %X\n",(ulong)bcbptr->ob.page);
        printf("          : ob.offset = %X\n",(ulong)bcbptr->ob.offset);
        printf("          : ob.length = %X\n",(ulong)bcbptr->ob.length);
#endif

        /*      Input Buffer    */

        temp[0] = BUS_GETC(dpram + bcboffset + INLNG);
        eieio();
        temp[1] = BUS_GETC(dpram + bcboffset + INLNG + 1);
        eieio();
        bcbptr->ib.length = (temp[1] << 8) | temp[0];
        temp[0] = BUS_GETC(dpram + bcboffset + INOFF);
        eieio();
        temp[1] = BUS_GETC(dpram + bcboffset + INOFF + 1);
        eieio();
        bcbptr->ib.offset = (temp[1] << 8) | temp[0];
        bcbptr->ib.page = BUS_GETC(dpram + bcboffset + INPAG);
        eieio();

#ifdef DARTIC_DEBUG2
        printf("          : ib.page   = %X\n",(ulong)bcbptr->ib.page);
        printf("          : ib.offset = %X\n",(ulong)bcbptr->ib.offset);
        printf("          : ib.length = %X\n",(ulong)bcbptr->ib.length);
#endif

        /*      Secondary Status Buffer */

        temp[0] = BUS_GETC(dpram + bcboffset + STATLNG);
        eieio();
        temp[1] = BUS_GETC(dpram + bcboffset + STATLNG + 1);
        eieio();
        bcbptr->ssb.length = (temp[1] << 8) | temp[0];
        temp[0] = BUS_GETC(dpram + bcboffset + STATOFF);
        eieio();
        temp[1] = BUS_GETC(dpram + bcboffset + STATOFF + 1);
        eieio();
        bcbptr->ssb.offset = (temp[1] << 8) | temp[0];
        bcbptr->ssb.page = BUS_GETC(dpram + bcboffset + STATPAG);
        eieio();

#ifdef DARTIC_DEBUG2
        printf("          : ssb.page   = %X\n",(ulong)bcbptr->ssb.page);
        printf("          : ssb.offset = %X\n",(ulong)bcbptr->ssb.offset);
        printf("          : ssb.length = %X\n",(ulong)bcbptr->ssb.length);
#endif

        iomem_det(mem_io_start);                /* detach from ISA bus */

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

#ifdef DARTIC_DEBUG
        printf("artic_get_bcb: exiting.\n");
#endif
        return(NO_ERROR);
}


/*
 *      to16local:
 *
 *      This subroutine accepts an intel byte-order ushort as a parameter,
 *      and returns the same value in local byte order format.
 */

ushort
to16local(thebit16)
ushort thebit16;
{
        SPEC16  tb16;

        tb16.b16 = thebit16;

        return  ((ushort)((tb16.sep.i_low         & 0x00FF) |
                                         ((tb16.sep.i_high << 8)  & 0xFF00)));
}


/*
 *      artic_get_psb
 *
 *      artic_get_psb is used to retrieve the task 0 Primary Status
 *      Byte from the Interface Block.  The PSB is returned
 *      in *psbptr.  The return value is either 0 for success,
 *      EINTR if lockl fails, and EIO if an exception occurs
 *      (setjmpx returns non-zero).
 */

artic_get_psb(coprocnum, psbptr)
int coprocnum;        /* coprocessor number           */
uchar   *psbptr;      /* char for return value        */
{
        int      retval;    /* return value from lockl      */
        char    *dpram;     /* pointer to adapter memory    */
        char     *mem_io_start;
        uchar    lastvalue; /* previous value in CPUPAGE    */

#ifdef DARTIC_DEBUG
        printf("artic_get_psb: Entered.\n");
#endif

        /*      Lock the cpupage_lock to serialize access       */


        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_psb: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_psb: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);   /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /* position window over IB page 0 */

        lastvalue = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to ISA memory bus */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        /*      Retrieve the Primary Status Byte */

        *psbptr = BUS_GETC(dpram + IBADDR + STATARRAY);
        eieio();

#ifdef DARTIC_DEBUG
        printf("artic_get_psb: *psbptr = %X\n",*psbptr);
#endif

        iomem_det(mem_io_start);                /* detach from ISA bus */

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

        return(NO_ERROR);
}


/*
 *      readdreg - writedreg
 *
 *      On the IBM Artic board, various registers are written to and read from
 *      indirectly by first programming the PTRREG register and then reading
 *      the desired register from DREG.  DREG is a "window" to other registers,
 *      based on the "pointer" value first programmed into PTRREG.  Values for the
 *      register "pointers" and such are in articdd.h and taken from the Realtime
 *      Interface Co-Processor Portmaster Adapter hardware tech ref, in the System
 *      Addressable Registers chapter.
 *
 */

uchar
readdreg(coprocnum,regp)
int coprocnum;       /* coprocessor number     */
int regp;            /* PTRREG "pointer"       */
{
        uchar    tbyte;         /* holds byte read from register        */

        /*      Program "pointer" register */
        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + PTRREG,
                    (uchar) regp);

        /*      Get the byte from the data register */

        tbyte = inbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + DREG);

        return(tbyte);                          /* return DREG value    */
}

writedreg(coprocnum,regp,rvalue)
int     coprocnum;                      /* coprocessor number */
int     regp;                           /* PTRREG "pointer"   */
int             rvalue;
{

        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + PTRREG,
                    (uchar) regp);
        eieio();
        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + DREG,
                    (uchar) rvalue);
        eieio();


}



/*
 *      inteltolocal
 *
 *      This subroutine accepts as its parameter an address assumed to be
 *      in Intel byte order format.  It returns the same address in local byte
 *      order format, whatever that may be.  Returns a char pointer.
 */

char *
inteltolocal(intelptr)
SPEC32 intelptr;
{
        char    *retvalue;              /* for return value     */

        /* put address into local byte format */

        retvalue =      (char *)
    ((((ulong) intelptr.bytes.ll      )   & 0x000000FF)  +
         (((ulong) intelptr.bytes.lh << 8 )   & 0x0000FF00)  +
         (((ulong) intelptr.bytes.hl << 16)   & 0x00FF0000)  +
         (((ulong) intelptr.bytes.hh << 24)   & 0xFF000000));

        return(retvalue);
}

/*
 *      localtointel
 *
 *              This routine accepts a ulong in local format and returns
 *      the "same" value in Intel byte order format.
 */


localtointel(local)
ulong local;
{
        SPEC32  inteladd;

        /* put address into Intel format */

        inteladd.seper.offset.sep.i_low =
                        (uchar) ( local                 & 0xFF);
        inteladd.seper.offset.sep.i_high =
                        (uchar) ((local >> 8)   & 0xFF);
        inteladd.seper.segment.sep.i_low =
                        (uchar) ((local >> 16)  & 0xFF);
        inteladd.seper.segment.sep.i_high =
                        (uchar) ((local >> 24)  & 0xFF);

        /* send it back whole */

        return(inteladd.b32);
}

/*
 *      lookup_articproc
 *
 *      lookup_articproc uses the calling processes PID (obtained from the kernel
 *      service getpid() ) to see if an DARTIC_Proc structure is on
 *      any linked list pointed to by the artic_procptr array.
 *
 *      lookup_articproc returns a pointer to the DARTIC_Proc structure on the linked
 *      list, or returns (struct DARTIC_Proc *) NULL if one is not found.
 */

struct DARTIC_Proc *
lookup_articproc()
{
        struct DARTIC_Proc *artic_ptr;   /* pointer to an DARTIC_Proc structure */
        int              index;      /* used to index artic_procptr array  */
        int              retval;

        /*      The index to the pointer array is calculated by dividing the
         *      process ID by the array size (ARTICPA_SIZE) and taking the remainder:
         */

        index = getpid() & ARTICPA_MASK;   /* same as PID % ARTICPA_SIZE */

#ifdef DARTIC_DEBUG
        printf("lookup_articproc: index = %d\n",index);
#endif

        /*
         *      Lock access to artic_procptr array
         */

        retval = lockl(&proc_lock, LOCK_SIGRET);

        if (retval)
        {
                return(PANULL);
        }

        /*      The linked list pointed to by artic_procptr[index] is traversed, looking
         *      for a matching pid in the myPID field of the current DARTIC_Proc struct.
         *      IF we find a match, we break, and return the pointer to that structure,
         *      else we reach the end of the list, and return NULL.
         */

        artic_ptr = artic_procptr[index];    /* get pointer to first in list */

        while (artic_ptr != PANULL )              /* while not  NULL                              */
        {
                if (artic_ptr->myPID == getpid() ) /* Matching PIDs?       */
                {
                        break;
                }
                artic_ptr = artic_ptr->next;                /* next link    */
        }

#ifdef DARTIC_DEBUG
        printf("lookup_articproc: artic_ptr = %X\n",artic_ptr);
#endif

        unlockl(&proc_lock);            /* unlock access to proc_lock   */

        return(artic_ptr);                        /* return either NULL or pointer to struct */
}

/*
 *      remove_articproc
 *
 *      remove_articproc uses the calling processes PID (obtained from the kernel
 *      service getpid() ) to see if an DARTIC_Proc structure is on
 *      any linked list pointed to by the artic_procptr array.
 *
 *      remove_articproc returns a pointer to the DARTIC_Proc structure that was on
 *      on the linked list after removeing it from that list, or returns
 *      PANULL if one is not found.
 */

struct DARTIC_Proc *
remove_articproc()
{
        struct DARTIC_Proc *artic_ptr;    /* pointer to an DARTIC_Proc structure */
        struct DARTIC_Proc *last_ptr;   /* pointer to an DARTIC_Proc structure */
        int              index;       /* used to index artic_procptr array  */

        /*      The index to the pointer array is calculated by dividing the
         *      process ID by the array size (ARTICPA_SIZE) and taking the remainder:
         */

        index = getpid() & ARTICPA_MASK;   /* same as PID % ARTICPA_SIZE */

#ifdef DARTIC_DEBUG
        printf("remove_articproc: index = %d\n",index);
#endif

        /*      The linked list pointed to by artic_procptr[index] is traversed, looking
         *      for a matching pid in the myPID field of the current DARTIC_Proc struct.
         *      IF we find a match, we break, and return the pointer to that structure,
         *      else we reach the end of the list, and return NULL.
         */

        artic_ptr = artic_procptr[index];    /* get pointer to first in list */

        if (artic_ptr != PANULL)
        {
                if (artic_ptr->myPID == getpid() )         /* First on list        */
                {
                        artic_procptr[index] = artic_ptr->next;              /* next on list         */
                }
                else
                {
                        while (artic_ptr != PANULL )                      /* while not  NULL              */
                        {
                                if (artic_ptr->myPID == getpid() )
                                {
                                        last_ptr->next = artic_ptr->next;         /* close list   */
                                        break;
                                }
                                last_ptr = artic_ptr;                             /* save current pointer */
                                artic_ptr  = artic_ptr->next;               /* next link                    */
                        }
                }
        }

#ifdef DARTIC_DEBUG
        printf("remove_articproc: artic_ptr = %X\n",artic_ptr);
#endif

        return(artic_ptr);                        /* return either NULL or pointer to struct */
}


/*
 *      intboard
 *
 *      Sends an interrupt to the specified
 *      coprocessor adapter.
 */

void
intboard(coprocnum)
int coprocnum;
{
        int    retval;        /* return value         */
        char   *busio_addr;   /* adapter I/O address  */

#ifdef DARTIC_DEBUG
        printf("intboard: Entered. adapter %d\n",coprocnum);
#endif

        /*      Put INTBOARD value in the PTRREG to cause interrupt */
        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + PTRREG,
                    (uchar) INTBOARD);


#ifdef DARTIC_DEBUG
        printf("intboard: Exiting.\n");
#endif
}

/*
 *      artic_read_mem
 *
 *      This function is used to read data to user-space from
 *      coprocessor adapter memory.  Addresses are always in
 *      page/offset format.
 */

artic_read_mem(coprocnum, destbuff,page,offset,length)
int    coprocnum;             /* coprocessor number         */
char   *destbuff;             /* destination in user-space  */
uchar  page;                  /* destination page           */
ushort offset;                /* destination offset         */
ulong  length;                /* # bytes to transfer        */
{
        int     i;              /* loop index                   */
        int     retval;         /* return value from lockl      */
        char    *dpram;         /* pointer to adapter memory    */
        char    *mem_io_start;
        uchar    lastvalue;     /* previous value in CPUPAGE    */
        uchar   temp;

#ifdef DARTIC_DEBUG
        printf("artic_read_mem: Entered.\n");
        printf("           : coprocnum = %d\n",coprocnum);
        printf("           : destbuff   = %X\n",destbuff  );
        printf("           : page      = %X\n",(ulong)page);
        printf("           : offset    = %X\n",(ulong)offset);
        printf("           : length    = %d\n",length);
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_read_mem: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_read_mem: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        retval = NO_ERROR;              /* preset return value to success */

        /* position window over destination page        */

        lastvalue = setCPUpage(coprocnum,page);

        /*      Attach to ISA bus */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        i = 0;
        while (i < length)
        {
                /* fetch coproc byte and put in user memory                     */

/*                subyte(destbuff + i,*(dpram + offset)); */
                temp = BUS_GETC(dpram + offset);
                eieio();
                subyte(destbuff + i, temp);

                offset++;                       /* increment source      offset         */
                i++;                            /* increment destination offset */

                /* check window wrap    */

                if (offset >= artic_adapter[coprocnum].windowsize && i < length)
                {
                        page++;                 /* increment page number        */
                        offset = 0;             /* reset offset                         */

                        /*      Check page Range        */

                        if (page > artic_adapter[coprocnum].maxpage)
                        {
                                retval = E_ICA_INVALID_PAGE;
#ifdef DARTIC_DEBUG
                                printf("artic_read_mem: Invalid Page %d\n",page);
#endif
                                break;
                        }

                        /* reprogram cpupage register   */

#ifdef DARTIC_DEBUG
                        printf("artic_read_mem: setting page to %d\n",page);
#endif

                        outbyte_isa(coprocnum,
                                    artic_adapter[coprocnum].baseio + CPUPAGE,
                                    page);
                }
        }

        iomem_det(mem_io_start);                /* detach from ISA bus */

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

#ifdef DARTIC_DEBUG
        printf("artic_read_mem: exiting.\n");
#endif

        return(retval);
}

/*
 *      artic_write_mem
 *
 *      This function is used to write data from user-space to
 *      coprocessor adapter memory.  Addresses are always in
 *      page/offset format.
 */

artic_write_mem(coprocnum, srcbuff,page,offset,length)
int     coprocnum;             /* coprocessor number           */
char   *srcbuff;               /* source buffer in user-space  */
uchar   page;                  /* destination page             */
ushort  offset;                /* destination offset           */
int     length;                /* # bytes to transfer          */
{
        int    i;              /* loop index                   */
        int    retval;         /* return value from lockl      */
        char   *dpram;         /* pointer to adapter memory    */
        char    *mem_io_start;
        uchar  lastvalue;      /* previous value in CPUPAGE    */
        uchar  temp;

#ifdef DARTIC_DEBUG
        printf("artic_write_mem: Entered.\n");
        printf("            : coprocnum = %d\n",coprocnum);
        printf("            : srcbuff   = %X\n",srcbuff  );
        printf("            : page      = %X\n",(ulong)page);
        printf("            : offset    = %X\n",(ulong)offset);
        printf("            : length    = %d\n",length);
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_write_mem: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_write_mem: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        retval = NO_ERROR;              /* preset to success    */

        /* position window over destination page        */

        lastvalue = setCPUpage(coprocnum,page);

        /*      Attach to ISA bus */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));
        dpram = mem_io_start + artic_adapter[coprocnum].basemem;

        i = 0;
        while (i < length)
        {
                /* fetch user byte and put in coproc memory                     */

/*                *(dpram + offset) = fubyte(srcbuff + i); */
                temp = fubyte(srcbuff + i);
                BUS_PUTC(dpram + offset, temp);
                eieio();

                offset++;       /* increment destination offset */
                i++;            /* increment source offset      */

                /* check window wrap    */

                if (offset >= artic_adapter[coprocnum].windowsize && i < length)
                {
                        page++;        /* increment page number */
                        offset = 0;    /* reset offset          */

                        /*      Check page Range        */

                        if (page > artic_adapter[coprocnum].maxpage)
                        {
                                retval = E_ICA_INVALID_PAGE;
#ifdef DARTIC_DEBUG
                                printf("artic_write_mem: Invalid Page %d\n",page);
#endif
                                break;
                        }

                        /* reprogram cpupage register   */

                        outbyte_isa(coprocnum,
                                    artic_adapter[coprocnum].baseio + CPUPAGE,
                                    page);
                }
        }

        /*      detach from ISA bus     */
        iomem_det(mem_io_start);

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

#ifdef DARTIC_DEBUG
        printf("artic_write_mem: exiting.\n");
#endif

        return(retval);
}

/*
 *      outbyte_isa
 *
 *      This function is used to write data to Co-Processor adapter
 *      I/O space.
 */
void outbyte_isa (int coprocnum, int ioport, uchar data)
{
   caddr_t ioaddr ;

   /*
   ** Map the IO segment into the virtual address space accessable from
   ** this driver.
   **
   ** ioaddr will be set to address the subject port.
   */
   ioaddr = iomem_att(&(artic_isa[coprocnum].map_io));
   BUSIO_PUTC ((ioaddr + ioport), data) ;
   eieio();

   /*
   ** Remove the bus memory & IO segment mapping from the virtual address space
   */
   iomem_det(ioaddr);
}


/*
 *      inbyte_isa
 *
 *      This function is used to read data from Co-Processor adapter
 *      I/O space.
 */
uchar inbyte_isa (int coprocnum, int ioport)
{
   uchar data ;
   caddr_t ioaddr ;

   /*
   ** Map the IO segment into the virtual address space accessable from
   ** this driver.
   **
   ** ioaddr will be set to address the subject port.
   */
   ioaddr = iomem_att(&(artic_isa[coprocnum].map_io));

   data = BUSIO_GETC (ioaddr + ioport) ;

   /*
   ** Remove the bus memory & IO segment mapping from the virtual address space
   */
   iomem_det(ioaddr);
   return (data) ;
}

