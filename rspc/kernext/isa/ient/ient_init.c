static char sccsid[] = "@(#)74  1.7.1.5  src/rspc/kernext/isa/ient/ient_init.c, isaent, rspc41J, 9522A_all 5/30/95 15:07:39";
/*
 * COMPONENT_NAME: isaent -- IBM ISA Bus Ethernet Device Driver
 *
 * FUNCTIONS:  ient_init
 *             ient_start
 *             ient_start_NIC
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

#define MC_STRING_LENGTH    4

extern dd_ctrl_t dd_ctrl;


/*****************************************************************************/
/*
 * NAME:  ient_getvpd
 *
 * FUNCTION: Get the firmware specifics from the adapter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  0 - OK
 */
/*****************************************************************************/

int
ient_getvpd(dds_t *p_dds) /* dev_ctl area ptr */
{
    int i;                         /* Simple loop counter. */
    volatile uchar *ioaddr;        /* IO-handle.           */

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "VPDb", (ulong) p_dds, 0, 0);

    /* Attach to the IO-bus. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /* Stop the NIC. */
    ient_stop_NIC(p_dds, ioaddr);

    /* Set Page #1. */
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                                              IEN_ISA_STOP_NIC |
                                              IEN_ISA_PAGE1;
    eieio();

    /* Setup the MicroChannel-like identifier string for NIM and lsdev. */
    VPD.vpd[0] = 0x2A; 
    VPD.vpd[1] = 0x4E; 
    VPD.vpd[2] = 0x41; 
    VPD.vpd[3] = 0x05;

    /* Read the burned-in Ethernet address. */
    for (i = 0; i < ENT_NADR_LENGTH; i++)
    {
        VPD.vpd[i + MC_STRING_LENGTH] = WRK.burn_addr[i];
    }

    /* Set Page #0. */

    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                                              IEN_ISA_STOP_NIC;
    eieio();

    /* Restart the NIC. */
    ient_start_NIC(p_dds, ioaddr);

    /* Detach from the IO-bus. */
    iomem_det((void *) ioaddr);
    VPD.status = VPD_VALID;
    VPD.length = ENT_NADR_LENGTH + MC_STRING_LENGTH;
    
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "VPDe", VPD.status, WRK.adapter_type, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME: ient_probe
 *
 * FUNCTION: Probe for IBM ISA adapter cards
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  0 - OK
 *           EIO - I/O error
 */
/*****************************************************************************/

int
ient_probe(dds_t  *p_dds)
{
    int            rc=0;            /* Return code. */
    int            i;               /* Simple counter. */
    uint           shared_mem_base; /* The addr. of the shared mem. */
    uchar          io_base_bits;    /* Used when configuring the IO-base. */
    uchar          irq_bits;        /* Used when configuring the intr-line. */
    uchar          laar_value;      /* High-addr. bits for ctrl. reg. #2. */
    uchar          read_back;       /* Used to see if the adapter is there. */
    uchar          sum;             /* Used to sum up the ROM checksum. */
    uchar          not_used;        /* Dummy variable. */
    volatile uchar *ioaddr;         /* IO-handle.           */
    volatile uchar *memaddr;        /* Shared-mem.-handle.           */

    rc = ENXIO;                     /* No adapters found so far. */

    /* Setup a segment register. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /*
    ** Calculate the bit-pattern in order to configure the io-base bits. 
    ** This is for cards without jumpers ..., done in a rather stupid (hw)
    ** fashion, why not arrange it so we can just right-shift?
    */

    io_base_bits = (DDI.io_addr == IEN_ISA_IO_ADDR1) ? IEN_ISA_IO_BITS1 :
                   (DDI.io_addr == IEN_ISA_IO_ADDR2) ? IEN_ISA_IO_BITS2 :
                   (DDI.io_addr == IEN_ISA_IO_ADDR3) ? IEN_ISA_IO_BITS3 :
                   (DDI.io_addr == IEN_ISA_IO_ADDR4) ? IEN_ISA_IO_BITS4 :
                   (DDI.io_addr == IEN_ISA_IO_ADDR5) ? IEN_ISA_IO_BITS5 :
                   (DDI.io_addr == IEN_ISA_IO_ADDR6) ? IEN_ISA_IO_BITS6 :
                   (DDI.io_addr == IEN_ISA_IO_ADDR7) ? IEN_ISA_IO_BITS7 :
                   IEN_ISA_IO_BITS4; /* Use this in case on an unknown val. */

    /*
    ** Setup the IO-base, since it's software programmable (done once / reboot)
    ** Write four times to 0x278H, write #4 will load the data
    ** into the I/O bits of Config. register A.
    */

    for (i = 0; i < 4; i++)
        *(ioaddr + IEN_ISA_INIT_BASE) = (uchar) io_base_bits;

    eieio();

    /* Calculate the irq bit-pattern for config-register A. */
    irq_bits = (DDI.intr_level == IEN_ISA_INTR_LEVEL_1) ? IEN_ISA_INTR_BITS1 :
               (DDI.intr_level == IEN_ISA_INTR_LEVEL_2) ? IEN_ISA_INTR_BITS2 :
               (DDI.intr_level == IEN_ISA_INTR_LEVEL_3) ? IEN_ISA_INTR_BITS3 :
               (DDI.intr_level == IEN_ISA_INTR_LEVEL_4) ? IEN_ISA_INTR_BITS4 :
               (DDI.intr_level == IEN_ISA_INTR_LEVEL_5) ? IEN_ISA_INTR_BITS5 :
               (DDI.intr_level == IEN_ISA_INTR_LEVEL_6) ? IEN_ISA_INTR_BITS6 :
               (DDI.intr_level == IEN_ISA_INTR_LEVEL_7) ? IEN_ISA_INTR_BITS7 :
               IEN_ISA_INTR_BITS1; /* Use this for any unknown value. */

    /* Now setup configuration register B (Must read first). */
    not_used = *(ioaddr + DDI.io_addr + IEN_ISA_INITIAL_REG_B_OFFSET);
    *(ioaddr + DDI.io_addr + IEN_ISA_INITIAL_REG_B_OFFSET) = DDI.media_type & 0x03;
    eieio();

    /*
    ** Setup the irq and indicate 'Shared Memory Mode' preserving the IO bits.
    ** We must read first, this is done to satisfy the hw (no accidental writes)
    */

    not_used = *(ioaddr + DDI.io_addr + IEN_ISA_INITIAL_REG_A_OFFSET);
    *(ioaddr + DDI.io_addr + IEN_ISA_INITIAL_REG_A_OFFSET) = (irq_bits |
        IEN_ISA_MEM_MODE | io_base_bits);
    eieio();

    /* Now setup configuration register B (Must read first). */
    not_used = *(ioaddr + DDI.io_addr + IEN_ISA_CFG_REG_B_OFFSET);
    *(ioaddr + DDI.io_addr + IEN_ISA_CFG_REG_B_OFFSET) = DDI.media_type & 0x03;
    eieio();

    /*
    ** Setup the irq and indicate 'Shared Memory Mode' preserving the IO bits.
    ** We must read first, this is done to satisfy the hw (no accidental writes)
    */

    not_used = *(ioaddr + DDI.io_addr + IEN_ISA_CFG_REG_A_OFFSET);
    *(ioaddr + DDI.io_addr + IEN_ISA_CFG_REG_A_OFFSET) = (irq_bits |
        IEN_ISA_MEM_MODE | io_base_bits);
    eieio();

    /* Now reset the NIC core registers. */
    *(ioaddr + DDI.io_addr + IEN_ISA_CTRL_REG_1_OFFSET) = IEN_ISA_RESET_NIC;
    eieio();

    DELAYMS(1000);

    *(ioaddr + DDI.io_addr + IEN_ISA_CTRL_REG_1_OFFSET) = 0;
    eieio();

    /* Give the NIC time to read the EEPROM into the io-map. */
    DELAYMS(1000);

    /* Setup the shared mem. base addr. in the adapters mem ctrl. reg.s 1&2. */
    /* First set a18..a13 ---> bits 0..5. */

    *(ioaddr + DDI.io_addr + IEN_ISA_CTRL_REG_1_OFFSET) = (IEN_ISA_MEM_ENABLE |
        ((DDI.shared_mem_addr >> 13) & IEN_ISA_LOW_MEM_MASK));
    eieio();

    /* Now setup the rest of the address bit's a23..a19. */
    *(ioaddr + DDI.io_addr + IEN_ISA_CTRL_REG_2_OFFSET) =
                                              IEN_ISA_16BIT_WIDE | 0x01;
    eieio();

    memaddr = iomem_att(dd_ctrl.mem_map_ptr);
    
    /* Shared memory test. */
    for (i = 0; i < 16384; i++)
        *(memaddr + DDI.shared_mem_addr + i) = 0;
    iomem_det((void *) memaddr);


    /* Read the bus width. */

    if ((WRK.slot_width = (*(ioaddr + DDI.io_addr +  
                             IEN_ISA_AT_DETECT_REG_OFFSET)) & 0x01))
    {
        /* The adapter is in a 16 bit slot. */

        read_back = *(ioaddr + DDI.io_addr + IEN_ISA_CFG_REG_A_OFFSET);

        if ((read_back & 0x07) == io_base_bits)
        {
            rc = 0; /* We found an adapter. */
        }
    }

    /* Get the adapter type. */
    WRK.adapter_type = *(ioaddr + DDI.io_addr + IEN_ISA_TYPE_OFFSET);

    /* Get the ethernet address. */
    for (i = 0; i < ENT_NADR_LENGTH; i++)
        WRK.burn_addr[i] =
            *(ioaddr + DDI.io_addr + IEN_ISA_ETHERNET_ADDR_OFFSET + i);

    /* Now also check the ROM, the checksum should equal 0xFF. */

    for (sum = 0, i = 0; i < 8; i++)
    {
        sum += (uchar) *(ioaddr + DDI.io_addr + IEN_ISA_ROM_OFFSET + i);
    }

    if ((sum & 0xff) != 0xff)
    {
        rc = ENXIO;
    }

    iomem_det((void *) ioaddr);

    return(rc);
}


/*****************************************************************************/
/*
 * NAME: ient_init
 *
 * FUNCTION:  Perform the device-specific initialization during configuration. 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  0 - OK
 *           EIO - I/O error
 */
/*****************************************************************************/

int
ient_init(dds_t  *p_dds)
{
    int    rc=0;
    ushort temp_short;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "INIb", (ulong)p_dds, 0, 0);

    /* Initialize global work variables. */
    VPD.status = VPD_NOT_READ;

    /* Check for an adapter. */
    if ((rc = ient_probe(p_dds)))
    {
        TRACE_SYS(HKWD_IEN_ISA_ERR, "INI1", rc, 0xDEAD, 0xDEAD);
        return(rc);
    }

    /* Get any "Vital Product Data", Ethernet-addr. and such... no MC VPD. */

    if (rc = ient_getvpd(p_dds))
    {
        TRACE_SYS(HKWD_IEN_ISA_ERR, "INI2", rc, 0, 0);
        return(rc);
    }

    /* Test if we got the board information */

    if (VPD.status != VPD_VALID)
    {
        ient_logerr(p_dds, ERRID_IEN_ISA_CARD_INFO, __LINE__,
                    __FILE__, VPD.status, 0 , 0);
        TRACE_SYS(HKWD_IEN_ISA_ERR, "INI3", VPD.status, 0, 0);
        return(EIO);
    }

    /*
    ** Determine which Network Address to use, either DDI or burned-in
    ** version
    */

    if (DDI.use_alt_addr == 0)
    {
        /* Use the burned-in Ethernet address. */
        COPY_NADR(WRK.burn_addr, WRK.net_addr);
    }
    else
    {
        /* Use the Ethernet address that was passed in via the DDI. */
        COPY_NADR(DDI.alt_addr, WRK.net_addr);
    }

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "iniE", rc, DDI.use_alt_addr, 0);
    return(rc);
}


/*****************************************************************************/
/*
 * NAME: ient_start
 *
 * FUNCTION:  Initialize and activate the adapter.    
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * RETURNS:   0 - OK
 */
/*****************************************************************************/

int
ient_start(dds_t *p_dds)    /* dev_ctl area ptr */
{
    int    rc=0;
    int    i, j;             /* Loop Counter */
    int    ipri;
    uint   shared_mem_base;
    uchar  laar_value;       /* Setup bits for second mem ctrl. register. */
    uchar  outstanding_cmd;
    uchar  io_base_bits;
    uchar  irq_bits;
    uchar  not_used;
    cmd_reg cmd;             /* NIC command structure. */
    uchar  out_value;        /* Holding place for stuff going to a register. */
    rcv_conf_reg rcv_cfg_reg;/* Receive configuration register structure. */
    volatile uchar *ioaddr;  /* IO-handle. */
    intr_stat_reg intr_stat;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "AstB", (ulong)p_dds, 0, 0);

    /* Disable interrupt */
    ipri = disable_lock(PL_IMP, &SLIH_LOCK);

    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /* Stop any remote DMA and the NIC. */
    ient_stop_NIC(p_dds, ioaddr);

    /* Check to see if the NIC got stopped. */
    outstanding_cmd = *(ioaddr + DDI.io_addr + PAGE0_COMMAND);
    eieio();
    
    if (!((outstanding_cmd | IEN_ISA_NO_DMA) && (outstanding_cmd | 
                                                 IEN_ISA_STOP_NIC)))
    {
        iomem_det((void *) ioaddr);
        unlock_enable(ipri, &SLIH_LOCK);
        return(-1);
    }

    /* Set the data configuration register. */

    if (WRK.slot_width)            /* 16 bit. */
    {
        out_value = 0x40 | 0x08 | IEN_ISA_DCR_WTS;
    }
    else                           /* 8 bit. */
    {
        out_value = IEN_ISA_DCR_FT1;
    }
    
    *(ioaddr + DDI.io_addr + PAGE0_DCR) = out_value;
    eieio();

    /* reset the remote byte count registers. */
    *(ioaddr + DDI.io_addr + PAGE0_RBCR0) = 0;
    *(ioaddr + DDI.io_addr + PAGE0_RBCR1) = 0;
    eieio();

    /*
    ** Set the receive configuration register for error packets rejected,
    ** runt packets rejected, multicast packets rejected, physical address
    ** match, not monitor mode (packets buffered to memory).
    */

    rcv_cfg_reg.value = IEN_ISA_ACCEPT_BCASTS;
    *(ioaddr + DDI.io_addr + PAGE0_RCR) = rcv_cfg_reg.value;
    eieio();

    /*
    ** Set the transmit configuration register for normal crc, loopback mode,
    ** no auto disable, normal backoff.
    */

    *(ioaddr + DDI.io_addr + PAGE0_TCR) = IEN_ISA_INTERNAL_LOOPBACK;
    eieio();

    /* Xmit register setup BEGIN. */

    /* Set the xmit byte count to zero. */
    *(ioaddr + DDI.io_addr + PAGE0_TBCR0) = 0;
    *(ioaddr + DDI.io_addr + PAGE0_TBCR1) = 0;
    eieio();

    /*
    ** Setup the Transmit Page Start Register (Points to packet to be send).
    */

    *(ioaddr + DDI.io_addr + PAGE0_TPSR) = 0;
    eieio();

    /* Xmit register setup END. */

    /*
    ** Receive register setup BEGIN.
    ** Setup the Page Start/Stop Registers.
    ** When setting up the beginning of the receive pages, skip over the
    ** transmit pages.
    */

    *(ioaddr + DDI.io_addr + PAGE0_PSTART) = IEN_ISA_RCV_START;
    *(ioaddr + DDI.io_addr + PAGE0_PSTOP) = (DDI.shared_mem_size / 
                                             IEN_ISA_PAGE_SIZE);

    /* Setup the Boundary Register to the beginning of the receive RAM buffer.*/

    *(ioaddr + DDI.io_addr + PAGE0_BNRY) = IEN_ISA_RCV_START;
    eieio();

    /*
    ** The current receive page is setup later, since the register for
    ** that action is in I/O page #1.
    */

    /* Setup the Interrupt Status Register (clear any pending interrupts). */
    *(ioaddr + DDI.io_addr + PAGE0_ISR) = 0xff;
    eieio();

    /* Disable interrupts. */
    *(ioaddr + DDI.io_addr + PAGE0_IMR) = 0;
    eieio();

    /* Begin configuring I/O Page #1 (the second page). */
    /* Setting Page 1. */

    cmd.value  = *(ioaddr + DDI.io_addr + PAGE0_COMMAND);
    eieio();
    cmd.cmd_bits.do_dma   = IEN_ISA_STOP_REMOTE_DMA;
    cmd.cmd_bits.stop     = TRUE;
    cmd.cmd_bits.set_page = IEN_ISA_PAGE_1;
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = cmd.value;
    eieio();

    /* Configure the Ethernet addr. (which sits in the second page = #1) */

    for (i = 0; i < ENT_NADR_LENGTH; i++)
        *(ioaddr + DDI.io_addr + PAGE1_PAR0 + i) = WRK.burn_addr[i];
    eieio();

    /* Configure the broadcast addr.  (which sits in the second page = #1) */
    for (i = 0; i < ENT_NADR_LENGTH; i++)
        *(ioaddr + DDI.io_addr + PAGE1_MAR0) = 0xff;
    eieio();

    /* Configure the current receive page . */
    *(ioaddr + DDI.io_addr + PAGE1_CURR) = IEN_ISA_RCV_START + 1;
    eieio();

    /* End of configuring I/O Page #1 (the second page). */

    /* Set page 0 and start the adapter. */
    cmd.value = IEN_ISA_SET_PAGE_0;
    *(ioaddr + DDI.io_addr + PAGE1_COMMAND) = cmd.value;
    eieio();

    ient_start_NIC(p_dds, ioaddr);

    /* Go to sleep at INTMAX. */
    unlock_enable(ipri, &SLIH_LOCK);

    DELAYMS(100);

    /* Disable interrupt again. */
    ipri = disable_lock(PL_IMP, &SLIH_LOCK);

    intr_stat.value = *(ioaddr + DDI.io_addr + PAGE0_ISR);
    eieio();

    if ((intr_stat.stat_bits.reset_stat))
    {

        TRACE_SYS(HKWD_IEN_ISA_ERR, "cci1", rc, 0, 0);
        iomem_det((void *) ioaddr);
        unlock_enable(ipri, &SLIH_LOCK);
        return(EIO);
    }

    /* Check the NIC command byte. */
    cmd.value = *(ioaddr + DDI.io_addr + PAGE0_COMMAND);
    eieio();

    if (cmd.cmd_bits.stop)
    {
        TRACE_SYS(HKWD_IEN_ISA_ERR, "cci2", rc, 0, 0);
        iomem_det((void *) ioaddr);
        unlock_enable(ipri, &SLIH_LOCK);
        return(EIO);
    }

    /*
    ** Setup the Interrupt Mask Register (enable rx, tx, rx err's and tx err's).
    */

    *(ioaddr + DDI.io_addr + PAGE0_IMR) = (IEN_ISA_ENB_RX_INTR |
                                           IEN_ISA_ENB_TX_INTR |
                                           IEN_ISA_ENB_RX_ERR_INTR |
                                           IEN_ISA_ENB_TX_ERR_INTR);
    eieio();

    unlock_enable(ipri, &SLIH_LOCK);

    /* save the device start time for statistics */
    WRK.dev_stime = lbolt;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "AstE", 0, 0, 0);
    iomem_det((void *) ioaddr);

    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_start_NIC
 *
 * FUNCTION:
 *        
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * RETURNS:  0 - OK
 */
/*****************************************************************************/

int
ient_start_NIC(dds_t *p_dds, volatile uchar *ioaddr)
{
    tx_conf_reg    tx_cfg;
    cmd_reg        cmd;
    intr_stat_reg  istat;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "nicB", (ulong)p_dds, 0, 0);

    /* This sequence of events is prescribed to start the NIC. */
    /* First put the NIC in internal loopback mode. */

    tx_cfg.value = IEN_ISA_INTERNAL_LOOPBACK;
    *(ioaddr + DDI.io_addr + PAGE0_TCR) = tx_cfg.value;
    eieio();

    /* Start the NIC. */
    cmd.value = IEN_ISA_NO_DMA | IEN_ISA_START_NIC;
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = cmd.value;
    eieio();

    /* Reset the interrupt status. */
    istat.value = IEN_ISA_RECEIVE_MASK;
    *(ioaddr + DDI.io_addr + PAGE0_ISR) = istat.value;
    eieio();

    /* Enable the NIC on the network. */
    *(ioaddr + DDI.io_addr + PAGE0_TCR) = 0;
    eieio();
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "nicE", (ulong)p_dds, 0, 0);

    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_stop_NIC
 *
 * FUNCTION:
 *        
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 - OK
 */
/*****************************************************************************/

int
ient_stop_NIC(dds_t *p_dds, volatile uchar *ioaddr)
{

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "nixB", (ulong)p_dds, 0, 0);

    /* Stop the NIC. */

    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = 
	( IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC );
    eieio();

    /* Reset the remote byte count registers. */
    /* These can't be read, only written. */

    *(ioaddr + DDI.io_addr + PAGE0_RBCR0) = 0;
    *(ioaddr + DDI.io_addr + PAGE0_RBCR1) = 0;
    eieio();
    io_delay(10000);

    /* check to see if the adapter was reset */

    if ( ( IEN_ISA_RESET_PENDING ) == *(ioaddr + DDI.io_addr + PAGE0_ISR) )
    {
    	TRACE_SYS(HKWD_IEN_ISA_OTHER, "nixE", (ulong)p_dds, 1, 0 );
    	return(0);
    }

    /* if the bit was zero, it may have been cleared when a receive buffer  */
    /* ring overflow condition was set, then a packet pulled off the ring.  */
    /* not very likely to occur, but not impossible, so we'll try this two  */
    /* more times to try and get past any overflow packet clearing window's */

    /* second try */

    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = 
	( IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC );
    eieio();

    *(ioaddr + DDI.io_addr + PAGE0_RBCR0) = 0;
    *(ioaddr + DDI.io_addr + PAGE0_RBCR1) = 0;
    eieio();
    io_delay(10000);

    if ( ( IEN_ISA_RESET_PENDING ) == *(ioaddr + DDI.io_addr + PAGE0_ISR) )
    {
    	TRACE_SYS(HKWD_IEN_ISA_OTHER, "nixE", (ulong)p_dds, 2, 0);
    	return(0);
    }

    /* third try */

    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = 
	( IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC );
    eieio();

    *(ioaddr + DDI.io_addr + PAGE0_RBCR0) = 0;
    *(ioaddr + DDI.io_addr + PAGE0_RBCR1) = 0;
    eieio();
    io_delay(10000);

    if ( ( IEN_ISA_RESET_PENDING ) == *(ioaddr + DDI.io_addr + PAGE0_ISR) )
    {
    	TRACE_SYS(HKWD_IEN_ISA_OTHER, "nixE", (ulong)p_dds, 3, 0);
    	return(0);
    }

    /* three tries and still cannot see the bit being set, will assume the */
    /* NIC is stopped and continue                                         */

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "nixE", (ulong)p_dds, 4, 
	*(ioaddr + DDI.io_addr + PAGE0_ISR));
    return(1);
}
