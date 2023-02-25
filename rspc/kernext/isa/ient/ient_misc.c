static char sccsid[] = "@(#)77  1.2.1.1  src/rspc/kernext/isa/ient/ient_misc.c, isaent, rspc41B, 412_41B_sync 1/6/95 15:47:08";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: 
 *        ient_multi_add
 *        ient_multi_del
 *        ient_hash_addr
 *        ient_mcast_ctrl
 *        ient_cdt_func
 *        ient_add_cdt
 *        ient_del_cdt
 *        ient_trace
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
 * NAME:     ient_multi_add
 *
 * FUNCTION: Add a multicast address to the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  
 *           0      - successful
 *           EINVAL - not a legal mutlticast address
 *           ENOMEM - unable to allocate required memory
 */
/*****************************************************************************/

ient_multi_add(dds_t *p_dds,    /* point to the dev_ctl area */
               char  *addr)     /* point to the multicast address */
{
    ient_multi_t *p_multi;
    ient_multi_t *p_prev = (ient_multi_t *) NULL;
    int i, rc;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MADb", (ulong)p_dds, (ulong)addr, 0);

    /*
    ** First check to see if this is a valid multi-cast address.
    */
    
    if (!((*((char *) addr) & MULTI_BIT_MASK)))
    {
        return(EINVAL);
    }

    p_multi = WRK.multi_table;

    /*
    ** Verify that the multicast address is a duplicate or not.
    ** For a duplicate address, simply increment the ref_count and return
    */

    while (p_multi)
    {
        if (SAME_NADR(addr, p_multi->m_addr))
        {
            p_multi->ref_count++;
            TRACE_SYS(HKWD_IEN_ISA_OTHER, "MAD1", 0, 0, 0);
            return(0);
        }
        else
        {
            p_prev = p_multi;      /* Keep track of the last one!! */
        }
        p_multi = p_multi->next;
    }

    /*
    ** We are here because either this is the first multicast address
    ** or we are adding a different one.  Using net_malloc because
    ** interrupts are disabled with disable_lock().
    */

    p_multi = net_malloc(sizeof(ient_multi_t), M_DEVBUF, M_NOWAIT);
    
    if (!p_multi)
    {
        TRACE_SYS(HKWD_IEN_ISA_ERR, "MAD2", ENOMEM, 0, 0);
        return(ENOMEM);
    }

    bzero(p_multi, sizeof(ient_multi_t));

    if (WRK.multi_table)
    {
        /*
        ** This is not the first one, so add to the end of the chain.
        */
        p_prev->next = p_multi;
    }
    else
    {
        WRK.multi_table = p_multi;
    }
    
    /* add the address into the table and increment counters */

    COPY_NADR(addr, p_multi->m_addr);
    p_multi->ref_count = 1;
    p_multi->next = (ient_multi_t *) NULL;

    /*
    ** Call the hashing function to see which Multicast Address Register
    ** and bit that needs to be set.
    */
    
    ient_hash_addr(p_dds, addr, &p_multi->mar_num, &p_multi->mar_val);

    /*
    ** Now determine if the MARs in th AT/LANTIC chip need updating and
    ** if so, configure the chip to accept MCast addresses of the hashed
    ** value.
    */

    ient_mcast_ctrl(p_dds, p_multi, TRUE);
    
    WRK.multi_count++;
    
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MADE", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_multi_del
 *
 * FUNCTION: Delete a multicast address from the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  
 *           0 - successful
 *           EINVAL - invalid parameter
 */
/*****************************************************************************/

ient_multi_del(dds_t *p_dds,    /* dev_ctl area ptr. */
               char  *addr)     /* multicast address */

{
    ient_multi_t *p_multi = WRK.multi_table;
    ient_multi_t *p_prev = NULL;
    int i, DDrc = 0;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MDLB", (ulong)p_dds, (ulong)addr, 0);

    /* search for the address */

    while (p_multi)
    {
        if (SAME_NADR(addr, p_multi->m_addr))
        {
            break;
        }
        else   /* Try next address */
        {
            p_prev = p_multi;
            p_multi = p_multi->next;
        }
    }

    /* if found the address */

    if (p_multi)
    {
        /*
        ** If the ref_count is greater than 1, simply decrement it.
        ** Otherwise, remove the address and collaspe the chain. 
        */

        if (p_multi->ref_count) p_multi->ref_count--;
        
        if (p_multi->ref_count == 0)
        {
            /*
            ** Call ient_mcast_ctrl() to turn off multicasting for this
            ** address.
            */

            ient_mcast_ctrl(p_dds, p_multi, FALSE);
            
            if (p_prev)
            {
                p_prev->next = p_multi->next;
            }
            else
            {
                WRK.multi_table = p_multi->next;
            }
            
            if (WRK.multi_count) WRK.multi_count--;

            net_free(p_multi, M_DEVBUF);
        }

        TRACE_SYS(HKWD_IEN_ISA_OTHER, "MDLE", p_multi->ref_count, 0, 0);
    }
    else
    {
        TRACE_SYS(HKWD_IEN_ISA_ERR, "MDL1", EINVAL, 0, 0);
        DDrc = EINVAL;
    }

    return(DDrc);
}


/*****************************************************************************/
/*
 * NAME:     ient_hash_addr
 *
 * FUNCTION: Hashes the multicast address.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:    In order for a this adapter to receive packets that have
 *           destination addresses other than the adapters physical node
 *           address, it is a requirement to maintain a list of these
 *           destination addresses.  It is impossible for the AT/LANTIC
 *           Ethernet Controller to store all of the possible addresses, so
 *           the AT/LANTIC contains 8 multicast address registers namely,
 *           MAR0-7.  These are used to decode the addresses as they are
 *           received.
 *
 * RETURNS:  0 - Ok.
 *
 */
/*****************************************************************************/

int
ient_hash_addr(dds_t *p_dds, caddr_t addr, uchar *mar_no, uchar *mar_val)
{
    uchar multi_addr[6];
    uint  crc_value, carry_val, latchCRC;
    int   addr_byte, bit;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MhaB", (ulong)addr, (ulong)mar_no,
              (ulong)mar_val);
    
    /*
    ** All destination addresses are routed through CRC logic inside the
    ** AT/LANTIC Ethernet Coprocessor.  As the last bit of the destination
    ** address enters the CRC, the 6 MSBs of the CRC generator are latched.
    ** These six bits are used to index into a unique filter bit (FB0-63)
    ** in the MARs.
    */

    crc_value = 0xFFFFFFFF;            /* Initialize CRC value */

    COPY_NADR(addr, multi_addr);

    /*
    ** The following code creates the 32-bit CRC value.
    */

    /* Loop through each byte of the destination address */
    for (addr_byte = 0; addr_byte < ENT_NADR_LENGTH; addr_byte++) 
    {
        for (bit = 0; bit < 8; bit++)    /* Loop through each bit */
        {
            carry_val = ((uchar)(((crc_value & 0x80000000) >> 31)) ^
                        ((multi_addr[addr_byte] & (1 << bit)) >> bit));

            crc_value = crc_value << 1;

            if (carry_val)
            {
                crc_value = ((crc_value ^ CRC_POLYNOMIAL) | carry_val);
            }
        }
    }

    /*
    **  There are 8 8-bit registers that provide the storage for the 64 bits
    **  used for filtering multicast addresses (FB0-FB63).  After we determine
    **  the CRC value, we latch the 6 most significant bits and then of those
    **  six, we use the 3 most significant bits to determine which one of the
    **  8 registers is used.  We then shift the value 1 left by the value
    **  of the 3 least significant bits of this 6 bit value.  This value is
    **  what will be OR'd with the current value already set there.
    */

    latchCRC = (crc_value >> 26) & 0x3F;

    *mar_no = latchCRC >> 3;
    *mar_val = 1 << (latchCRC & 0x07);
    
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MhaE", *mar_no, *mar_val, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_mcast_ctrl
 *
 * FUNCTION: Turns on/Turns off Multicast on the National DP83905 (AT/LANTIC).
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:    There is always a possiblity that a Multicast address will hash
 *           to the same value as that of another address.  Therefore, before
 *           anything is changed in the hardware, a check is made of the table
 *           to see if a hardware change is actually required.
 *
 * RETURNS:  0  -  Always successful
 */
/*****************************************************************************/

int
ient_mcast_ctrl(dds_t *p_dds, ient_multi_t *p_multi, ulong add_mcast)
{
    ient_multi_t *p_mtable;     /* Pointer to chaing of MCast addresses   */
    volatile uchar *ioaddr;     /* IO- handle.                            */
    volatile uchar cur_rcr;     /* Current value of the receive cfg reg.  */
    volatile uchar cur_val;     /* Current value of the MultiCast reg.    */
    int val_match = 0;          /* Number of multicast matches            */

    TRACE_SYS(HKWD_IEN_ISA_OTHER,"MctB",(ulong)p_dds,(ulong)p_multi,add_mcast);

    /*
    ** First determine if there is a need to change anything.  We go through
    ** the table and if there is more than one entry that matches the same
    ** register and value for that register.  If this happens then we need
    ** to return because clearing the bit would turn off multicast addressing
    ** for someone else.  As the documentation says, it not perfect.
    */

    p_mtable = WRK.multi_table;

    while (p_mtable)
    {
        if (p_multi->mar_num == p_mtable->mar_num)
        {
            if (p_multi->mar_val == p_mtable->mar_val)
            {
                val_match++;
            }
        }
        p_mtable = p_mtable->next;
    }

    if (val_match > 1)
    {
        /*
        ** There is more than one multicast address that hashes to the
        ** same value.  Therefore, we cannot do anything or we will
        ** stop receiving for another valid multicast address.  If
        ** someone complains about this, then the fix is for them to
        ** come up with a different multicast addressing scheme.
        */

        TRACE_SYS(HKWD_IEN_ISA_OTHER, "Mct1", p_multi->mar_num,
                                              p_multi->mar_val, 0);
        return(0);
    }

    /*
    ** If we make it to this point then we are either going to enable or
    ** disable multicast addresses.
    */

    /* Attach to the IO-bus. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /* First stop the NIC. */
    ient_stop_NIC(p_dds, ioaddr);

    /* Set page. #2. */
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                                              IEN_ISA_STOP_NIC |
                                              IEN_ISA_PAGE2;
    eieio();

    /* Read the current value of 'RCR' from page 2. */
    cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
    eieio();

    /* Switch back to the first page. */
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC;
    eieio();

    /*
    ** I believe that if you read one of these registers then the bits get
    ** cleared.  I am not sure but that I think I heard that somewhere.  If
    ** it doesn't happen that way then some of the following code can be
    ** cleaned up.  At least this way it stays consistent.
    **
    ** Receive Configuration Register (RCR) to accept/not accept
    ** multicast addresses. This is bit #3 (AM).
    **
    ** If we are disabling a multicast address and there are no more multicast
    ** addresses left then we need to inform the AT/LANTIC chip.
    */

    if (!add_mcast && ((WRK.multi_count - 1) == 0))
    {
        cur_rcr &= ~(IEN_ISA_ACCEPT_MCASTS);
    }
    else
    {
        cur_rcr |= IEN_ISA_ACCEPT_MCASTS;
    }

    *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
    eieio();

    /* Switch to page 1. */
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                                              IEN_ISA_STOP_NIC |
                                              IEN_ISA_PAGE1;
    eieio();

    /*
    ** Set/Unset the appropriate multicast bits. Some of the otherbits may be
    ** set because of other hashings so don't destroy them. Read current
    ** value and then put in the correct value.
    */

    cur_val = *(ioaddr + DDI.io_addr + (PAGE1_MAR0 + p_multi->mar_num));
    eieio();

    if (add_mcast)                   /* Enable multicast for this hash value */
        cur_val |= p_multi->mar_val;
    else                             /* Disable multicat for this hash value */
        cur_val &= ~(p_multi->mar_val);

    
    *(ioaddr + DDI.io_addr + (PAGE1_MAR0 + p_multi->mar_num)) = cur_val;
    eieio();

    /* Switch to page 0. */
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC;
    eieio();

    /* Re-start the NIC. */
    ient_start_NIC(p_dds, ioaddr);

    iomem_det((void *) ioaddr);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MctE", 0, 0, 0);
    return(0);
}
    
/*****************************************************************************/
/*
 * NAME:     ient_cdt_func
 *
 * FUNCTION: return the address of the component dump table 
 *
 * EXECUTION ENVIRONMENT: 
 *
 * NOTES:
 *
 * RETURNS:      the address of the component dump table
 */
/*****************************************************************************/

struct ient_cdt_t *ient_cdt_func()
{
    return((struct ient_cdt_t *) &dd_ctrl.cdt.head);
}

/*****************************************************************************/
/*
 * NAME:     ient_add_cdt
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM: ient_open
 *
 * INPUT:
 *           name   - character string of the name of the data structure
 *           addr   - address of the data structure
 *           len    - length of the data structure
 *
 * RETURNS:  
 *    none.
 */
/*****************************************************************************/

void
ient_add_cdt(char *name,      /* label string for area dumped */
             char *addr,      /* address of the area to be dumped */
             int   len)       /* amount of data to be dumped */

{
    struct cdt_entry *p_entry;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "OdaB", (ulong)name, (ulong)addr, len);

    p_entry = &dd_ctrl.cdt.entry[dd_ctrl.cdt.count];
    strcpy(p_entry->d_name, name);
    p_entry->d_len = len;
    p_entry->d_ptr = addr;
    p_entry->d_xmemdp = NULL;

    dd_ctrl.cdt.count++;
    dd_ctrl.cdt.head._cdt_len += sizeof(struct cdt_entry);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "OdaE", 0, 0, 0);
    return;
}

/*****************************************************************************/
/*
 * NAME:     ient_cdt_del
 *
 * FUNCTION: delete an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM: ient_open
 *              ient_close
 *
 * INPUT:
 *            name        - character string of the name of the data structure
 *            addr        - address of the data structure
 *            len        - length of the data structure
 *
 * RETURNS:   none.
 */
/*****************************************************************************/

void
ient_del_cdt(char *name,     /* label string for area dumped */
             char *addr,     /* address of the area to be dumped */
             int  len)       /* amount of data to be dumped */

{
    struct cdt_entry *p_entry;
    int i;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "OddB", (ulong)name, (ulong)addr, len);

    /* find the entry in the table (match only the memory pointer) */

    for (p_entry = &dd_ctrl.cdt.entry[0], i = 0; i < dd_ctrl.cdt.count;
         p_entry++, i++)
    {
        if (p_entry->d_ptr == addr) break;
    }

    /* if found the entry, remove the entry by re-arrange the table */

    if (i < dd_ctrl.cdt.count)
    {
        for (; i < dd_ctrl.cdt.count; p_entry++, i++)
        {
            strcpy(p_entry->d_name, dd_ctrl.cdt.entry[i+1].d_name);
            p_entry->d_len = dd_ctrl.cdt.entry[i+1].d_len;
            p_entry->d_ptr = dd_ctrl.cdt.entry[i+1].d_ptr;
        }

        dd_ctrl.cdt.count--;
        dd_ctrl.cdt.head._cdt_len -= sizeof(struct cdt_entry);

    }

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "OddE", 0, 0, 0);
    return;
}

/*****************************************************************************/
/*
 * NAME:     ient_trace
 *
 * FUNCTION: Put a trace into the internal trace table and the external
 *           system trace.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *    This routine is only called through macros when DEBUG is defined.
 *    
 * CALLED FROM:
 *    every routine in the driver 
 *
 * INPUT:
 *           hook            - trace hook 
 *           tag             - four letter trace ID
 *           arg1 to arg 3   - arguments to trace
 *
 * RETURNS:  none.
 */
/*****************************************************************************/

void
ient_trace(ulong hook,    /* trace hook */
           char  *tag,    /* trace ID */
           ulong arg1,    /* 1st argument */
           ulong arg2,    /* 2nd argument */
           ulong arg3)    /* 3rd argument */
{
    int i;
    int ipri;

    ipri = disable_lock(PL_IMP, &TRACE_LOCK);

    /* Copy the tracetable point into the internal tracetable table */

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

    /* Call the external trace routine */
    TRCHKGT(hook | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);
    return;
}

/*****************************************************************************/
/*
 * NAME:     ient_logerr
 *
 * FUNCTION: Collect information for making system error log entry.
 *
 * EXECUTION ENVIRONMENT: process or interrupt 
 *
 * NOTES:
 *    
 * RETURNS:  nothing.
 */
/*****************************************************************************/

void
ient_logerr(dds_t    *p_dds,    /* pointer to dev_ctl area */
            ulong  errid,       /* Error id for logging */
            int    line,        /* line number */
            char   *p_fname,    /* file name */
            ulong  parm1,       /* log 4 bytes data 1 */
            ulong  parm2,       /* log 4 bytes data 2 */
            ulong  parm3)       /* log 4 bytes data 3 */
{
    struct  error_log_def   log;
    uchar   lbuf[64];
    int i;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "NlgB", (ulong)p_dds, (ulong)errid, line);
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "NlgC", parm1, parm2, parm3);

    /* Load the error id and device driver name into the log entry */

    log.errhead.error_id = errid;
    strncpy(log.errhead.resource_name, DDI.lname, ERR_NAMESIZE);

    /* put the line number and filename in the table */
    sprintf(lbuf, "line: %d file: %s", line, p_fname);
    strncpy(log.fname, lbuf, sizeof(log.fname));

    /* Load Network address in use value into the table  */
    for (i = 0; i < ENT_NADR_LENGTH; i++)
        log.ent_addr[i] = WRK.net_addr[i];

    /* Start filling in the table with data  */
    log.parm1 = parm1;
    log.parm2 = parm2;
    log.parm3 = parm3;

    /* log the error message */
    errsave(&log, sizeof(struct error_log_def));

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "NlgE", 0, 0, 0);
    return;
}
