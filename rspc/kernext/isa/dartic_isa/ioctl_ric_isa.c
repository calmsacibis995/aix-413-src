/* @(#)31        1.1  src/rspc/kernext/isa/dartic_isa/ioctl_ric_isa.c, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:15
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver for the ISA BUS.
 *
 * FUNCTIONS: darticioctl_isa, artic_icareadmem, artic_icawritemem, artic_icaissuecmd
 *            artic_icagetprimstat, artic_icagetbufaddrs, artic_icaintreg
 *            artic_icaintwait, artic_icaintdereg, artic_icareset
 *            artic_icasendconfig, artic_icagetparms, artic_icagetadaptype
 *            artic_icaiowrite, artic_icaioread
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
 *      darticioctl_isa
 *
 *      darticioctl_isa is the ARTIC Diag device driver ioctl function.  The ARTIC Diag
 *      ioctl is made up of sub-functions.  Following is a list of the sub-function
 *      symbols, and descriptions.
 *
 *      Symbol          Description
 *      ------          -----------
 *
 *      ICARESET        Reset
 *      ICAREADMEM      Read Memory
 *      ICAWRITEMEM     Write Memory
 *      ICAINTREG       Interrupt Register
 *      ICAINTWAIT      Interrupt Wait
 *      ICAINTDEREG     Interrupt Deregister
 *      ICAISSUECMD     Issue Command
 *      ICAGETPARMS     Get Parameters
 *      ICAGETBUFADDRS  Get Buffer Address
 *      ICAGETPRIMSTAT  Get Primary Status
 *      ICASENDCFG      Send Config Parameters
 *      ICAGETADAPTYPE  Return adapter's type (EIB+base card combination)
 */


darticioctl_isa(device, cmd, arg,devflag,chan,ext)
dev_t                device;        /* major/minor device number       */
int                  cmd;           /* ioctl function value            */
ARTIC_IOCTL_UNION     *arg;         /* pointer to argument structures  */
ulong                devflag;       /* device flags, ignored           */
int                  chan,ext;      /* mpx channel and extension, ignored   */
{
        int     retval;             /* return value from subroutines  */

#ifdef DARTIC_DEBUG
         printf("darticioctl_isa: Entered. cmd = %X arg = %X\n",cmd,arg);
#endif

        retval = 0;               /* preset return value to 0 (success)   */

        /*
         *      switch on the value of cmd, and call the specified ioctl sub-function.
         */

        switch (cmd)
        {

                /* ICAGETPARMS - This ioctl is used to retrieve the RCM operating
                                                 parameters (maxtask,maxpri,maxqueue,maxtime) from
                                                the artic_adapter table                                                            */

                case ICAGETPARMS:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAGETPARMS entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAGETPARMS
                         */

                        retval = artic_icagetparms(device, arg);

                        break;
                }

                /* ICARESET - This ioctl is used to issue a hardware reset
                   to the specified coprocessor                          */

                case ICARESET:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICARESET entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICARESET
                         */

                        retval = artic_icareset(device, arg);

                        break;
                }

                /* ICASENDCFG    - This ioctl is used to write RCM operating
                                                   parameters to the RCM Interface Block, in
                                                   coprocessor memory.                                                  */

                case ICASENDCFG:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICASENDCFG    entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICASENDCFG
                         */

                        retval = artic_icasendconfig(device, arg);
                        break;
                }

                /* ICAGETPRIMSTAT - This ioctl is used to retrieve a task's primary
                                                        status byte                                                                             */

                case ICAGETPRIMSTAT:
                {
#ifdef DARTIC_DEBUG
                        printf ("darticioctl_isa: case ICAGETPRIMSTAT entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAGETPRIMSTAT
                         */

                        retval = artic_icagetprimstat(device, arg);

                        break;
                }


                /* ICAREADMEM - This ioctl is used to read adapter memory */

                case ICAREADMEM:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAREADMEM entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAREADMEM
                         */

                        retval = artic_icareadmem(device, arg);

                        break;
                }

                /* ICAWRITEMEM - This ioctl is used to write adapter memory */

                case ICAWRITEMEM:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAWRITEMEM entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAWRITEMEM
                         */

                        retval = artic_icawritemem(device, arg);

                        break;
                }

                /* ICAGETBUFADDRS - This ioctl is used to retrieve  the
                        buffer control block contents   */

                case ICAGETBUFADDRS:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAGETBUFADDRS entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAGETBUFADDRS
                         */

                        retval = artic_icagetbufaddrs(device, arg);

                        break;
                }


                /* ICAISSUECMD - This is the "Issue Command" ioctl. */

                case ICAISSUECMD:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAISSUECMD entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAISSUECMD
                         */

                        retval = artic_icaissuecmd(device, arg);

                        break;
                }

                /* ICAINTREG - This is the "Interrupt Register" ioctl. */

                case ICAINTREG:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAINTREG entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAINTREG
                         */

                        retval = artic_icaintreg(device, arg);

                        break;
                }

                /* ICAINTWAIT - This is the "Interrupt Wait" ioctl. */

                case ICAINTWAIT:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAINTWAIT entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAINTWAIT
                         */

                        retval = artic_icaintwait(device, arg);

                        break;
                }

                /* ICAINTDEREG - This is the "Interrupt DeRegister" ioctl. */

                case ICAINTDEREG:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAINTDEREG entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAINTDEREG
                         */

                        retval = artic_icaintdereg(device, arg);

                        break;
                }

                /*      ICAIOREAD       - used to read from adapter I/O port            */

                case ICAIOREAD:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAIOREAD entered.\n");
#endif

                        retval = artic_icaioread(device, arg);
                        break;
                }

                /*  ICAIOWRITE      - used to write to adapter I/O port */

                case ICAIOWRITE:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAIOWRITE entered.\n");
#endif


                        retval = artic_icaiowrite(device, arg);
                        break;
                }

                case ICAGETADAPTYPE:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa: case ICAGETADAPTYPE entered.\n");
#endif


                        retval = artic_icagetadaptype(device, arg);
                        break;
                }

                default:                                                /* invalid command argument     */
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl_isa - invalid command");
#endif
                        retval = EINVAL;
                        break;
                }
        }

#ifdef DARTIC_DEBUG
        printf("darticioctl_isa: retval = %d, exiting.\n",retval);
#endif

        return(retval);         /* non-zero if error */
}


/*
 *      artic_icareadmem
 *
 *      This is the ioctl subfunction for "Read Memory".  This function
 *      reads adapter memory, and copies it to user-space.  The address
 *      format is specified by the user in the parameter block.
 */

artic_icareadmem(device, arg)
dev_t device;             /* major/minor number             */
ICAREADMEM_PARMS  *arg;   /* pointer to the parameter block */
{
        int     retval = 0;       /* return value              */
        int     coprocnum;        /* from major/minor  number  */
        ulong   b32address;       /* for address conversions   */
        ushort  rcmoffset;        /* for address conversions   */
        uchar   rcmpage;          /* for address conversions   */

#ifdef DARTIC_DEBUG
        printf("artic_icareadmem: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success                    */

        /*
         *      Validity and range checking:
         *
         *      2.      an adapter must be present.
         */

        coprocnum = minor(device);     /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icareadmem: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }


        /*
         *      Create page/offset format for artic_read_mem out of whatever
         *      the user gave us.  If format not recognized, return
         *      E_ICA_INVALID_FORMAT.
         */

        switch (arg->addr_format)
        {
                /*
                 *      ADDRFMT_PAGE: address is passed in page/offset format,
                 *  therefore no conversion is necessary to use artic_read_mem.
                 */

                case ADDRFMT_PAGE:                              /* page/offset format   */
                {
#ifdef DARTIC_DEBUG
                        printf("artic_icareadmem: case ADDRFMT_PAGE\n");
                        printf("artic_icareadmem:  No Conversion necessary.\n");
#endif

                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (arg->offset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (arg->segpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*       Read 'em       */

                                retval = artic_read_mem(coprocnum,         /* coprocessor number   */
                                                        arg->dest,                              /* user space buffer    */
                                                        (uchar)arg->segpage,    /* beginning page number*/
                                                        arg->offset,                    /* beginning offset     */
                                                        arg->length);                   /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }

                        break;
                }

                /*
                 *      ADDRFMT_SEGMENT: address is passed in segment/offset format,
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_SEGMENT:
                {
                        /* Intel segment:offset format.  Shift segment value left 4 bits
                         *      and add to offset to create a real 32bit address
                         */

                        b32address = (arg->segpage << 4) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

                        arg->segpage    = rcmpage;
                        arg->offset    =  rcmoffset;

#ifdef DARTIC_DEBUG
                        printf("artic_icareadmem: case ADDRFMT_SEGMENT\n");
                        printf("artic_icareadmem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icareadmem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icareadmem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
#endif
                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*       Read 'em       */

                                retval = artic_read_mem(coprocnum, /* coprocessor number   */
                                                        arg->dest,                      /* user space buffer    */
                                                        rcmpage,                        /* beginning page number*/
                                                        rcmoffset,                      /* beginning offset     */
                                                        arg->length);           /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 *      ADDRFMT_32BIT: address is passed in 32 bit format, as two shorts.
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_32BIT:
                {
                        /*      Create the 32bit address by shifting the segpage value left
                         *      16 bits and adding the offset.
                         */

                        b32address = (arg->segpage << 16) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

#ifdef DARTIC_DEBUG
                        printf("artic_icareadmem: case ADDRFMT_32BIT\n");
                        printf("artic_icareadmem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icareadmem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icareadmem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
#endif

                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*       Read 'em       */

                                retval = artic_read_mem(coprocnum, /* coprocessor number   */
                                                        arg->dest,                      /* user space buffer    */
                                                        rcmpage,                        /* beginning page number*/
                                                        rcmoffset,                      /* beginning offset     */
                                                        arg->length);           /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }

                        break;
                }

                /*
                 * Invalid address format identifier (arg->addr_format)
                 */

                default:
                {
                        arg->retcode = E_ICA_INVALID_FORMAT;
                        return(retval);
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icareadmem: Exiting.\n");
#endif

        return(retval);
}



/*
 *      artic_icawritemem
 *
 *      This is the ioctl subfunction for "Read Memory".  This function
 *      reads user memory space, and copies it to adapter memory.  The
 *      address format is specified by the user in the parameter block.
 */

artic_icawritemem(device, arg)
dev_t device;             /* major/minor number              */
ICAWRITEMEM_PARMS *arg;   /* pointer to the parameter block  */
{
        int     retval = 0;      /* return value             */
        int     coprocnum;       /* from major/minor(number) */
        ulong   b32address;      /* for address conversions  */
        ushort  rcmoffset;       /* for address conversions  */
        uchar   rcmpage;         /* for address conversions  */

#ifdef DARTIC_DEBUG
        printf("artic_icawritemem: Entered.\n");
#endif

        arg->retcode = NO_ERROR;                                /* preset to success                    */

        /*
         *      Validity and range checking:
         *
         *           an adapter must be present.
         */

        coprocnum = minor(device);             /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icawritemem: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Create page/offset format for artic_write_mem out of whatever
         *      the user gave us.  If format not recognized, return
         *      E_ICA_INVALID_FORMAT.
         */

        switch (arg->addr_format)
        {
                /*
                 *      ADDRFMT_PAGE: address is passed in page/offset format,
                 *  therefore no conversion is necessary to use artic_write_mem.
                 */

                case    ADDRFMT_PAGE:                           /* page/offset format   */
                {
#ifdef DARTIC_DEBUG
                        printf("artic_icawritemem: case ADDRFMT_PAGE\n");
                        printf("artic_icawritemem:  No Conversion necessary.\n");
#endif

                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (arg->offset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (arg->segpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*      Write 'em       */

                                retval = artic_write_mem(coprocnum,      /* coprocessor number   */
                                                       arg->source,    /* buffer in user-space */
                                                       (uchar)arg->segpage,/* beginning page number*/
                                                       arg->offset,    /* beginning offset             */
                                                       arg->length);   /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 *      ADDRFMT_SEGMENT: address is passed in segment/offset format,
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_SEGMENT:
                {
                        /* Intel segment:offset format.  Shift segment value left 4 bits
                         *      and add to offset to create a real 32bit address
                         */

                        b32address = (arg->segpage << 4) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

                        arg->segpage    = rcmpage;
                        arg->offset    =  rcmoffset;
#ifdef DARTIC_DEBUG
                        printf("artic_icawritemem: case ADDRFMT_SEGMENT\n");
                        printf("artic_icawritemem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icawritemem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icawritemem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
                        printf("artic_icawritemem:  maxpage    = %X\n",
                                   artic_adapter[coprocnum].maxpage);
                        printf("artic_icawritemem:  windowsize = %X\n",
                                   artic_adapter[coprocnum].windowsize);
#endif

                        /*
                         *      offset must be less than window size, page must be less
                         *      than the maximum page value
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*      Write 'em       */

                                retval = artic_write_mem(coprocnum,    /* coprocessor number   */
                                                       arg->source,  /* buffer in user-space */
                                                       rcmpage,      /* beginning page number*/
                                                       rcmoffset,    /* beginning offset             */
                                                       arg->length); /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 *      ADDRFMT_32BIT: address is passed in 32 bit format, as two shorts.
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_32BIT:
                {
                        /* Intel segment:offset format.  Shift segment value left 4 bits
                         *      and add to offset to create a real 32bit address
                         */

                        b32address = (arg->segpage << 16) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

#ifdef DARTIC_DEBUG
                        printf("artic_icawritemem: case ADDRFMT_32BIT\n");
                        printf("artic_icawritemem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icawritemem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icawritemem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
#endif

                        /*
                         *      offset must be less than window size, page must be less
                         *      than the maximum page value
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*      Write 'em       */

                                retval = artic_write_mem(coprocnum,        /* coprocessor number   */
                                                                arg->source,            /* buffer in user-space */
                                                                rcmpage,                        /* beginning page number*/
                                                                rcmoffset,                      /* beginning offset             */
                                                                arg->length);           /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 * Invalid address format identifier (arg->addr_format)
                 */

                default:
                {
                        arg->retcode = E_ICA_INVALID_FORMAT;
                        return(retval);
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icawritemem: Exiting.\n");
#endif

        return(retval);
}



/*
 *      artic_icaissuecmd
 *
 *      This is the ioctl subfunction for Issue Command.   This function is used
 *      to send an RCM style command to an adapter task, with optional parameters.
 */

artic_icaissuecmd(device, arg)
dev_t device;             /* major/minor number             */
ICAISSUECMD_PARMS *arg;   /* pointer to the parameter block */
{
        ICAGETBUFADDRS_PARMS thebcb; /* BCB for specified task       */
        int     retval = 0;          /* return value                 */
        int     coprocnum;           /* from minor number            */
        int     numticks;            /* number of ticks for timeout  */
        uchar   psb;                 /* Primary Status Byte          */
        uchar   pcselect;            /* PC Select Byte               */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Entered.\n");
#endif

        arg->retcode = NO_ERROR;     /* preset to success            */

        /*
         *      Do Validity Checks on parameters.
         *
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         *
         *              task number must be >=0 and <= artic_adapter[coprocnum].maxtask.
         */

        coprocnum = minor(device);   /* retrieve the coprocessor number  */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: coprocnum = %d\n",coprocnum);
#endif

        /*
         *      Adapter must be in READY state to accept commands.
         */

        if (artic_adapter[coprocnum].state != DARTIC_READY)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaissuecmd: Adapter not ready, state = %d\n",
                          artic_adapter[coprocnum].state);
#endif

                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /*      Retrieve the PSB for the specified task */

        retval = artic_get_psb(coprocnum, &psb);

        if (retval)
        {
                return(retval);   /* Error occured during artic_get_psb      */
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: artic_get_psb returned PSB = %X\n",psb);
#endif

        if (arg->length)      /* if there are parameters      */
        {

                /*       Retrieve the BCB       */

                retval = artic_get_bcb(coprocnum, &thebcb);

                if (retval)     /* failed with EINTR or EIO     */
                {
                        return(retval);
                }

                /*      if parameter length > bcb output buffer size,
                 *      return an error.
                 */

                if (arg->length > thebcb.ob.length)
                {
                        arg->retcode = E_ICA_OB_SIZE;
                        return(retval);
                }

                /*
                 *      Write parameters to output buffer
                 */

#ifdef DARTIC_DEBUG
                printf("artic_icaissuecmd: thebcb.ob.page    = %X\n",(ulong)thebcb.ob.page);
                printf("              : thebcb.ob.offset  = %X\n",(ulong)thebcb.ob.offset);
                printf("artic_icaissuecmd: thebcb.ob.length  = %X\n",(ulong)thebcb.ob.length);
#endif

                retval = artic_write_mem(coprocnum,
                                       arg->prms,
                                       thebcb.ob.page,
                                       thebcb.ob.offset,
                                       (int) arg->length);

                if (retval == E_ICA_INVALID_PAGE)
                {
                        arg->retcode=retval;
                        retval = 0 ;
                        return(retval);
                }

                if (retval)                     /* error EINTR or EIO   */
                {
                        return(retval);         /* return error         */
                }
        }

        /*      Put Command Code into the BCB   */

        retval = artic_put_cmd(coprocnum, arg->cmdcode);

        if (retval)                     /* error EINTR or EIO   */
        {
                return(retval);         /* return error         */
        }

        /*      Serialize access to the PC select byte  */

        if (retval = lockl(&pcselect_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaissuecmd: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*      Retrieve the PC select bytes current value      */

        retval = artic_get_pcselect(coprocnum, &pcselect);

        if (retval)
        {
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(retval);
        }

        /*      PC Select byte must be value 0xFF or 0xFE (OK_TO_INT1 & 2)
         *      in order to allow access
         */

        if (pcselect != OK_TO_INT1 && pcselect != OK_TO_INT2)
        {
                arg->retcode    = E_ICA_BAD_PCSELECT;
                arg->reserved   = pcselect;

                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(NO_ERROR);
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: first check: PC Select O.K.\n");
#endif

        /*
         *      Lock access to CPUPAGE register
         */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_pcselect: lockl failure - retval = %d.\n",
                                        retval);
#endif
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Mask interrupts as per RCM manual instructions (Chap 4, p 67).
         */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Masking interrupts...\n");
#endif

        i_mask((struct intr *) &artic_intr[coprocnum]);

        /*      Store the task number 0 in the PC Select byte.  This
         *      will cause an interrupt to the adapter, and RCM will
         *      vector to the task's command handler.
         */

        retval = artic_put_pcselect(coprocnum, (uchar) 0);

        if (retval)     /* Error! EINTR or EIO during put       */
        {
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(retval);
        }

        /*      Interrupt The Adapter */

        intboard(coprocnum);

        /*      Unlock resources */

        unlockl(&cpupage_lock[coprocnum]);

        /*      Unmask interrupts */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Unmasking interrupts.\n");
#endif

        i_unmask((struct intr *) &artic_intr[coprocnum]);

        /*
         *      Check PC Select byte after every timer tick up
         *      to arg->timeout milliseconds.
         */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: after intboard.\n");
#endif

        delay(1);

        retval = artic_get_pcselect(coprocnum, &pcselect);

        if (retval)
        {
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(retval);
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: after artic_get_pcselect.\n");
#endif
        delay(1);

        if (pcselect != OK_TO_INT1 && pcselect != OK_TO_INT2)
        {
                if (arg->timeout == 0)
                {
                        arg->retcode = E_ICA_TIMEOUT;
                }
                else
                {
                        /* Number of ticks to wait is the timeout
                         *      value divided by the number of ms per
                         *      tick (NMS_PER_TICK).
                         */

                        numticks = arg->timeout / NMS_PER_TICK;

                        /*
                         *      Loop waiting for PC Select byte to indicate command
                         *      was accepted.
                         */

                        while (numticks > 0)
                        {
                                /* retrieve the PC Select byte */

                                retval = artic_get_pcselect(coprocnum, &pcselect);

                                if (retval)
                                {
                                        unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                                        return(retval);
                                }

                                /* check to see if command was accepted         */

                                if (pcselect == OK_TO_INT1 || pcselect == OK_TO_INT2)
                                {
                                        break;
                                }
                                else
                                {
                                        /* delay for 1 tick (NMS_PER_TICK milliseconds) */

                                        delay(1);
                                        numticks--;
                                }
                        }

                        if (numticks <= 0)      /* timed out! */
                        {
                                arg->retcode = E_ICA_TIMEOUT;
                        }
                        else
                        {
                                if (pcselect == OK_TO_INT1) /* cmd accepted     */
                                {
                                        arg->retcode = NO_ERROR;                /* indicate success */
                                }
                                else                                    /* command was rejected */
                                {
                                        arg->retcode = E_ICA_CMD_REJECTED;
                                }
                        }
                }
        }

        /*
         *      Unlock access to the pcselect byte
         */

        unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Exiting.\n");
#endif

        return (retval);
}



/*
 *      artic_icagetprimstat
 *
 *              This ioctl is used to retrieve the Primary Status
 *              Byte (PSB) of a specified task.
 */

artic_icagetprimstat(device, arg)
dev_t device;                   /* major/minor number              */
ICAGETPRIMSTAT_PARMS    *arg;   /* pointer to the parameter block  */
{
        int     retval = 0;     /* return value                 */
        int     coprocnum;      /* from minor number            */
        uchar   psb;            /* returned Primary Status Byte */

#ifdef DARTIC_DEBUG
        printf("artic_icagetprimstat: Entered.\n");
#endif

        arg->retcode = NO_ERROR;    /* preset to success */

        /*
         *      Do Validity Checks on parameters.
         *
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         *              adapter state must not be DARTIC_EMPTY.
         *              task number must be >=0 and <= artic_adapter[coprocnum].maxtask.
         */

        coprocnum = minor(device);   /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icagetprimstat: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Call artic_get_psb to retrieve the PSB.  artic_get_psb will
         *      take care of locking access to the CPUPAGE register.
         */

        retval = artic_get_psb(coprocnum, &psb);

        arg->psb = psb;         /* put PSB into parameter block */

#ifdef DARTIC_DEBUG
        printf("artic_icagetprimstat: psb = %X Exiting.\n",psb);
#endif

        return (retval);
}




/*
 *      artic_icagetbufaddrs
 *
 *              This ioctl is used to retrieve the Buffer Control
 *              Block (BCB) of a specified task.
 */

artic_icagetbufaddrs(device, arg)
dev_t device;                   /* major/minor number              */
ICAGETBUFADDRS_PARMS    *arg;   /* pointer to the parameter block       */
{
        int     retval = 0;     /* return value         */
        int     coprocnum;      /* from minor number    */
        uchar   psb;            /* Primary Status Byte  */

#ifdef DARTIC_DEBUG
        printf("artic_icagetbufaddrs: Entered.\n");
#endif

        arg->retcode = NO_ERROR; /* preset to success */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter must not be in DARTIC_EMPTY state.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number */


        /*      Check adapter state.    */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /*
         *      Call artic_get_psb to retrieve the PSB.  artic_get_psb will
         *      take care of locking access to the CPUPAGE register.
         */

        retval = artic_get_psb(coprocnum, &psb);

        if (retval)
        {
                return(retval);         /* EINTR or EIO */
        }

#ifdef DARTIC_DEBUG
        printf("artic_icagetbufaddrs: psb = %X\n",psb);
#endif

        /*
         *      Get the BCB
         */

        retval = artic_get_bcb(coprocnum, arg);

#ifdef DARTIC_DEBUG
        printf("artic_icagetbufaddrs: exiting.\n");
#endif

        return (retval);
}


/*
 *      artic_icaintreg
 *
 *      This is the ioctl subfunction for Interrupt Register.
 *      This function allocates the ProcReg structure and puts it
 *      on the linked list pointed to by the DARTIC_Proc structure for
 *      the process.
 */

artic_icaintreg(device, arg)
dev_t device;           /* major/minor number                   */
ICAINTREG_PARMS *arg;   /* pointer to the parameter block       */
{
        struct DARTIC_Proc  *procptr;    /* pointer to per-process structure   */
        struct ProcReg  *prptr;        /* ptr to Process Registration struct */
        struct ProcReg  *lastptr;      /* ptr to Process Registration struct */
        int     retval = 0;            /* return value                       */
        int     coprocnum;             /* from arg->coprocnum                */

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: Entered.\n");
#endif

        arg->retcode = NO_ERROR;                                /* preset to success                    */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter must not be in DARTIC_EMPTY state.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Retrieve pointer to this processes DARTIC_Proc struct.
         */

        procptr = lookup_articproc();

        /*      Retrieve pointer to linked list of ProcReg structures  */

        prptr = procptr->prptr[coprocnum];

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: prptr = %X\n",prptr);
#endif

        /*
         *      Check to see if we are already registered.  If not,
         *      allocate a ProcReg structure.
         */

        if (prptr == PRNULL)
        {
                /*      Allocate a ProcReg structure */

                procptr->prptr[coprocnum] = prptr =
                                        xmalloc(sizeof(struct ProcReg), 1, kernel_heap);

                if (prptr == PRNULL)
                {
                        arg->retcode = E_ICA_XMALLOC_FAIL;
                        return(NO_ERROR);
                }
        }
        else
        {
                /*  Return error if already registered */

                arg->retcode = E_ICA_ALREADY_REG;
                return(NO_ERROR);
        }

        /*      prptr points to our new ProcReg structure.
         *      Initialize it with task number, current interrupt
         *      count, and set link pointer to PRNULL.
         */

        prptr->eventcount = artic_intr_count[coprocnum];

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: artic_intr_count[%d] = %d\n",
                                                coprocnum, artic_intr_count[coprocnum]);
#endif

        prptr->next = PRNULL;

        /*      Increase the number of the kernel's callout table entries
         */

        if (! timeoutcf(1))
        {
                procptr->tocfcount++;
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: exiting.\n");
#endif


        return(NO_ERROR);                               /* Success */

}


/*
 *      artic_icaintwait
 *
 *      This is the ioctl subfunction for "Interrupt Wait".  This function
 *      checks the current interrupt count versus the one stored in the
 *      processes ProcReg structure for the task.   If an interrupt has
 *      occured, then success is returned.
 */

artic_icaintwait(device, arg)
dev_t device;                   /* major/minor number              */
ICAINTWAIT_PARMS        *arg;   /* pointer to the parameter block  */
{
        struct DARTIC_Proc  *procptr;   /* pointer to per-process structure     */
        struct ProcReg  *prptr;       /* ptr to Process Registration struct   */
        int     retval = 0;           /* return value                         */
        int slpret;                   /* return value from e_sleep            */
        int     coprocnum;            /* from minor number                    */
        int     numticks;             /* number of timer ticks for timeout    */
        uchar   psb;                  /* for Primary Status Byte              */

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: Entered.\n");
#endif

        arg->retcode = NO_ERROR;      /* preset to success                    */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter state must not be DARTIC_EMPTY
         */

        coprocnum = minor(device);             /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Retrieve pointer to this processes DARTIC_Proc struct.
         */

        procptr = lookup_articproc();

        /*      Retrieve pointer to linked list of ProcReg structures  */

        prptr = procptr->prptr[coprocnum];

        /*
         *      Check to see if we are already registered.  If not,
         *      allocate a ProcReg structure.
         */

        if (prptr == PRNULL)
        {
                arg->retcode = E_ICA_NOT_REG;
                return(NO_ERROR);
        }

        /*      prptr points to our ProcReg structure.
         *      If the eventcount in the ProcReg structure does not
         *      match the current interrupt count then an interrupt
         *      has occured.
         */

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: eventcount = %d artic_intr_count[%d] = %d\n",
                prptr->eventcount,coprocnum, artic_intr_count[coprocnum]);
#endif

        if (prptr->eventcount != artic_intr_count[coprocnum])
        {
                prptr->eventcount = artic_intr_count[coprocnum];  /* rearm   */

                return(NO_ERROR);                               /* Success */
        }

        /*
         *      If we made it here then we must go to sleep for the
         *      recommended timeout, waiting for either an interrupt
         *      or the timeout function to wake us up.
         */

        if (arg->timeout)
        {
                /* calculate ticks delay */

                numticks = arg->timeout / NMS_PER_TICK;

                /* have function artic_time_func called after numticks
                        clock ticks                                                             */

#ifdef DARTIC_DEBUG
                printf("artic_icaintwait: TO: coproc = %d numticks = %d\n",
                                 coprocnum, numticks);
#endif

                timeout(artic_time_func,&artic_wu_cookie[coprocnum],numticks);

                /*      Go To Sleep, wakeup from artic_time_func or
                        darticintr interrupt handler  */

                slpret = e_sleep(&artic_wu_cookie[coprocnum],EVENT_SIGRET);

#ifdef DARTIC_DEBUG
                printf("artic_icaintwait: after sleep: slpret = %d ecount = %d icount = %d\n",
                                slpret,prptr->eventcount, artic_intr_count[coprocnum]);
#endif

                if (slpret == EVENT_SUCC)
                {

                        /*      Now We are Awake.  Determine if interrupt occured */

                        if (prptr->eventcount != artic_intr_count[coprocnum])
                        {
                                prptr->eventcount =
                                                artic_intr_count[coprocnum];      /* rearm        */

                                arg->retcode = NO_ERROR;                                        /* Success      */

                                untimeout(artic_time_func,&artic_wu_cookie[coprocnum]);
                        }
                        else
                        {
                                arg->retcode = E_ICA_TIMEOUT;           /* timed out */
                        }
                }
                else    /* assume slpret == EVENT_SIG   */
                {
                        arg->retcode = E_ICA_INTR;      /* Interrupted by signal         */
                        untimeout(artic_time_func,&artic_wu_cookie[coprocnum]);
                }
        }
        else
        {
                arg->retcode = E_ICA_TIMEOUT;           /* timed out */
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: Exiting, retcode = %X\n",
                  arg->retcode);
#endif
        return(NO_ERROR);
}

/*
 *      artic_icaintdereg
 *
 *      This is the ioctl subfunction for "Interrupt Deregister".  This function
 *      deregisters the process for task interrupt notification.  This is done
 *      by removing the ProcReg structure from the linked list pointed to by
 *      the processes DARTIC_Proc structure.
 */

artic_icaintdereg(device, arg)
dev_t  device;                  /* major/minor number              */
ICAINTDEREG_PARMS       *arg;   /* pointer to the parameter block  */
{
        struct DARTIC_Proc  *procptr;   /* pointer to per-process structure     */
        struct ProcReg  *prptr;       /* ptr to Process Registration struct   */
        struct ProcReg  *lastptr;     /* ptr to Process Registration struct   */
        int     retval = 0;           /* return value                         */
        int     coprocnum;            /* minor(device)                        */
        int     numticks;             /* number of timer ticks for timeout    */
        uchar   psb;                  /* for Primary Status Byte              */

#ifdef DARTIC_DEBUG
        printf("artic_icaintdereg: Entered.\n");
#endif

        arg->retcode = NO_ERROR;      /* preset to success                    */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter state must not be DARTIC_EMPTY.
         */

        coprocnum = minor(device);    /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icaintdereg: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Retrieve pointer to this processes DARTIC_Proc struct.
         */

        procptr = lookup_articproc();

        /*      Retrieve pointer to linked list of ProcReg structures  */

        prptr = procptr->prptr[coprocnum];

        /*
         *      Check to see if we are already registered.  If not,
         *      return an error.
         */

        if (prptr == PRNULL)
        {
                arg->retcode = E_ICA_NOT_REG;
                return(NO_ERROR);
        }
        else
        {
                        /* Remove ProcReg structure from the linked list */

                procptr->prptr[coprocnum] = PRNULL;

#ifdef DARTIC_DEBUG
                printf("artic_icaintdereg: freeing prptr = %X\n",
                                        prptr);
#endif

                xmfree(prptr,kernel_heap);     /* free memory  */

                if (procptr->tocfcount)
                {
                    if (!timeoutcf(-1))     /* decrement callout table entries      */
                     {
                         procptr->tocfcount--;   /* decrement count      */
                     }
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaintdereg: Exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icareset
 *
 *      This is the ioctl subfunction for "Reset".  This function issues
 *      a hardware reset to the specified coprocessor adapter.
 */

artic_icareset(device, arg)
dev_t device;
ICARESET_PARMS  *arg;   /* pointer to the parameter block       */
{
        int     retval;    /* return value         */
        int     coprocnum; /* from arg->coprocnum  */

#ifdef DARTIC_DEBUG
        printf("artic_icareset: Entered.\n");
#endif

        arg->retcode = NO_ERROR;     /* preset to success */

        coprocnum = minor(device);   /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icareset: coprocnum = %d\n",coprocnum);
#endif

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY )
        {
                arg->retcode = E_ICA_INVALID_COPROC;                    /* out of Range */
                return(retval);
        }

#ifdef DARTIC_DEBUG
        printf("artic_icareset: Calling boardreset(%d).\n",
                                        coprocnum);
#endif

        retval = boardreset(coprocnum);

#ifdef DARTIC_DEBUG
        printf("artic_icareset: Exiting.  retval = %d\n",retval);
#endif

        if (retval)
        {
                arg->retcode = E_ICA_TIMEOUT;                                   /* could not reset */
                artic_adapter[coprocnum].state = DARTIC_BROKEN;        /* say board is broken  */
        }

        return(NO_ERROR);
}




/*
 *      artic_icasendconfig
 *
 *      This is the ioctl subfunction for "Send Configuration Parameters".
 *      This function copies the RCM configuration parameters from the
 *      parameter block to the adapter table, and then to the Interface Block
 *      for RCM.  NOTE that this function uses the ICAGETPARMS_PARMS
 *      parameter block.
 */

artic_icasendconfig(device, arg)
dev_t device;             /* major/minor number              */
ICAGETPARMS_PARMS *arg;   /* pointer to the parameter block  */
{
        int   retval = 0;      /* for return values                */
        int   coprocnum;       /* from major/minor number          */
        uchar *dpram;         /*  pointer to adapter memory       */
        uchar *mem_io_start ;
        uchar lastpage;        /* used to restore CPUPAGE register */

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         */

        coprocnum = minor(device);    /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: coprocnum = %d\n",coprocnum);
#endif

        if (artic_adapter[coprocnum].state !=  DARTIC_READY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(NO_ERROR);
        }

        /*
         *      Copy parameters from the parameter block to the
         *      adapter table.
         */

        artic_adapter[coprocnum].maxpri    = (int) arg->cfgparms.maxpri;
        artic_adapter[coprocnum].maxtimer  = (int) arg->cfgparms.maxtime;
        artic_adapter[coprocnum].maxqueue  = (int) arg->cfgparms.maxqueue;
        artic_adapter[coprocnum].maxtask   = (int) arg->cfgparms.maxtask;

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icasendconfig: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*      Set the CPUPAGE so that the IB is revealed */

        lastpage = setCPUpage(coprocnum,(uchar)0);

        /*      Get a pointer to the Interface Block */

        mem_io_start = iomem_att(&(artic_isa[coprocnum].map_mem));

        /*      Copy to Interface Block */

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: maxpri   = 0x%X\n",
                                                     arg->cfgparms.maxpri);
        printf("artic_icasendconfig: maxtask  = 0x%X\n",
                                                     arg->cfgparms.maxtask);
        printf("artic_icasendconfig: maxtime  = 0x%X\n",
                                                     arg->cfgparms.maxtime);
        printf("artic_icasendconfig: maxqueue = 0x%X\n",
                                                     arg->cfgparms.maxqueue);
#endif

        dpram = (uchar *) ((artic_adapter[coprocnum].basemem + IBADDR + MAXTASK) + mem_io_start);
        BUS_PUTC( dpram, arg->cfgparms.maxtask);
        eieio();
        dpram = (uchar *) ((artic_adapter[coprocnum].basemem + IBADDR + MAXPRI) + mem_io_start);
        BUS_PUTC( dpram, arg->cfgparms.maxpri);
        eieio();
        dpram = (uchar *) ((artic_adapter[coprocnum].basemem + IBADDR + MAXTIME) + mem_io_start);
        BUS_PUTC( dpram, arg->cfgparms.maxtime);
        eieio();
        dpram = (uchar *) ((artic_adapter[coprocnum].basemem + IBADDR + MAXQUEUE) + mem_io_start);
        BUS_PUTC( dpram, arg->cfgparms.maxqueue);
        eieio();
        iomem_det(mem_io_start);

        /*      Restore the CPUPAGE register */

        setCPUpage(coprocnum,lastpage);

        /*      Unlock the CPUPAGE register lock        */

        unlockl(&cpupage_lock[coprocnum]);

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icagetparms
 *
 *      This is the ioctl subfunction for "Get Configuration Parameters".
 *      This function copies the RCM configuration parameters from the
 *      adapter table to the parameter block.
 */

artic_icagetparms(device, arg)
dev_t device;             /* major/minor number             */
ICAGETPARMS_PARMS *arg;   /* pointer to the parameter block */
{
        int retval = 0;   /* for return values               */
        int coprocnum;    /* from major/minor number         */
        char *dpram;      /*      pointer to adapter memory  */

#ifdef DARTIC_DEBUG
        printf("artic_icagetparms: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icagetparms: coprocnum = %d\n",coprocnum);
#endif

        /*      Check adapter state.    */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(NO_ERROR);
        }

        /*
         *      Copy parameters from the parameter block to the
         *      adapter table.
         */

        arg->cfgparms.maxpri    =  (uchar) artic_adapter[coprocnum].maxpri;
        arg->cfgparms.maxtime   =  (uchar) artic_adapter[coprocnum].maxtimer;
        arg->cfgparms.maxqueue  =  (uchar) artic_adapter[coprocnum].maxqueue;
        arg->cfgparms.maxtask   =  (uchar) artic_adapter[coprocnum].maxtask;
        arg->cfgparms.int_level =  (uchar) artic_adapter[coprocnum].intlevel;
        arg->cfgparms.io_addr   =  (ushort)artic_adapter[coprocnum].baseio;

        switch(artic_adapter[coprocnum].windowsize)
        {
                case 0x2000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 0;
                        break;
                }
                case 0x4000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 1;
                        break;
                }
                case 0x8000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 2;
                        break;
                }
                case 0x10000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 3;
                        break;
                }
                default:
                {
                        arg->cfgparms.ssw_size  =       -1;             /* Error! */
                        break;
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icagetparms: exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icagetadaptype
 *
 *      This is the ioctl subfunction for "Get Adapter type".
 *      This function copies the type received from the config method and
 *      stored in the adapter table to the parameter block.
 */

artic_icagetadaptype(device, arg)
dev_t device;                /* major/minor number             */
ICAGETADAPTYPE_PARMS *arg;   /* pointer to the parameter block */
{
        int retval = 0;   /* for return values               */
        int coprocnum;    /* from major/minor number         */

#ifdef DARTIC_DEBUG
        printf("artic_icagetadaptype: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icagetadaptype: coprocnum = %d\n",coprocnum);
#endif

        /*      Check adapter state.    */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(NO_ERROR);
        }

        /*
         *      Copy type to the parameter block from the
         *      adapter table.
         */

        arg->type =  artic_adapter[coprocnum].adaptertype;


#ifdef DARTIC_DEBUG
        printf("artic_icagetadaptype: exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icaiowrite:
 *
 *      This function is used to write to I/O ports on the coprocessor adapter
 */

artic_icaiowrite(device, arg)
dev_t device;                 /* major/minor number              */
ICAIOWRITE_PARMS      *arg;   /* pointer to the parameter block  */
{
        int      retval = 0;        /* return value            */
        int      coprocnum;         /* from major/minor number */

#ifdef DARTIC_DEBUG
        printf("oa_icaiowrite: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *      Validity and range checking:
         *
         *      1.      an adapter must be present.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number  */

#ifdef DARTIC_DEBUG
        printf("artic_icaiowrite: coprocnum = %d\n",coprocnum);
        printf("              : portnum  = %d value = %d\n",
                 (int)arg->portnum,(int)arg->value);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaiowrite: lockl failure - retval = %d.\n",
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
                printf("artic_icaiowrite: setjmpx returned %d\n",retval);
#endif
                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /*      Write value to port                     */

        outbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + arg->portnum, arg->value);

        clrjmpx(&artic_jumpbuffers[coprocnum]);    /* clear exception stack */

        unlockl(&cpupage_lock[coprocnum]);       /* unlock cpupage lock  */

        return(retval);                          /* return no error      */
}

/*
 *      artic_icaioread:
 *
 *      This function is used to read from I/O ports on the coprocessor adapter
 */

artic_icaioread(device, arg)
dev_t device;                 /* major/minor number              */
ICAIOREAD_PARMS *arg;         /* pointer to the parameter block  */
{
        int      retval = 0;    /* return value        */
        int      coprocnum;     /* from arg->coprocnum */

#ifdef DARTIC_DEBUG
        printf("artic_icaioread: Entered.\n");
#endif

        arg->retcode = NO_ERROR;        /* preset to success */

        /*
         *      Validity and range checking:
         *
         *      1.      an adapter must be present.
         */

        coprocnum = minor(device); /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icaioread: coprocnum = %d\n",coprocnum);
        printf("             : portnum  = %d\n", arg->portnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaioread: lockl failure - retval = %d.\n",
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
                printf("artic_icaioread: setjmpx returned %d\n",retval);
#endif
                unlockl(&cpupage_lock[coprocnum]); /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /*      Read value from port    */

        arg->value = inbyte_isa(coprocnum, artic_adapter[coprocnum].baseio + arg->portnum);

        clrjmpx(&artic_jumpbuffers[coprocnum]);  /* clear exception stack */

        unlockl(&cpupage_lock[coprocnum]);    /* unlock cpupage lock   */

        return(retval);                       /* return no error       */
}

