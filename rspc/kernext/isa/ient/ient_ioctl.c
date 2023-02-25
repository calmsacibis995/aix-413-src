static char sccsid[] = "@(#)76  1.5.1.4  src/rspc/kernext/isa/ient/ient_ioctl.c, isaent, rspc41J, 9517A_all 4/21/95 13:06:03";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: ient_ioctl(), ient_getmib(), ient_getstat(),
 *            ient_clrstat()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

extern dd_ctrl_t dd_ctrl;
extern ethernet_all_mib_t ient_mib_status;
extern uchar ent_broad_adr[];

/*****************************************************************************/
/*
 * NAME:                  ient_ioctl
 *
 * FUNCTION:              ethernet driver ioctl routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:             0 - OK
 *                      ENETUNREACH - device is currently unreachable
 *                      ENETDOWN - device is down
 *                      EINVAL - invalid paramter
 *                      ENOMEM - unable to allocate required memory
 *                      EOPNOTSUPP - operation not supported
 */
/*****************************************************************************/

ient_ioctl(ndd_t   *p_ndd,               /* ndd dev_ctl area ptr.            */
           int     cmd,                  /* control command.                 */
           caddr_t arg,                  /* argument of the control command. */
           int     length)               /* length of the argument.          */

{
    dds_t *p_dds;                        /* Device structure ptr.            */
    int   rc;                            /* Function return code.            */
    int   i;                             /* Simple loop counter.             */
    int   ipri;                          /* Current priority.                */
    volatile uchar *ioaddr;              /* IO- handle.                      */
    volatile uchar cur_rcr;              /* Current value of 'recv cfg reg.  */
    volatile uchar cur_val;              /* Current value of MCast Addr. Reg.*/

    /* Get the adapter structure */
    p_dds = (dds_t *)(((ulong) p_ndd) - offsetof(dds_t, ndd));

    TRACE_SYS(HKWD_IEN_ISA_OTHER,"IOCb",(ulong)p_dds,(ulong)cmd,(ulong)arg);
    TRACE_SYS(HKWD_IEN_ISA_OTHER,"IOC+",(ulong)length,(ulong)0,(ulong)0);

    rc = 0;
    ipri = disable_lock(PL_IMP, &CMD_LOCK);

    /*
     * if the driver is in a power management mode of suspend/hibernate
     * then put the caller to sleep until the driver resumes normal operation
     * except for open this needs to be done under a lock so that the state
     * check is valid
     */
     if (WRK.adap_state == SUSPEND_STATE)
          e_sleep_thread (&WRK.pm_event,&CMD_LOCK, LOCK_HANDLER);

    /* Branch on the ioctl. */
    switch (cmd)
    {
        /*
        ** Get the generic statistics. This is for ndd_genstats + ent_genstats.
        */

        case NDD_GET_STATS:
        {

            /* Make sure there is enough space. */
            if (length != sizeof(ent_ndd_stats_t))
            {
                rc = EINVAL;
                break;
            }

            /* Get the statistics from the adapter. */
            if (rc = ient_getstat(p_dds))
            {
                break;
            }

            /* Copy the statistics to the user's buffer. */
            bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
            bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t), 
                  sizeof(ent_genstats_t));

            break;
        }

        case NDD_GET_ALL_STATS: 
        {

            /* Get device statistics: ndd_genstats,ent_genstats,ien_isa_stats */
            if (length != sizeof(ien_isa_all_stats_t))
            {
                rc = EINVAL;
                break;
            }

            /* Get the statistics from the adapter. */
            if (rc = ient_getstat(p_dds))
            {
                break;
            }

            /* Copy statistics to user's buffer. */
            bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
            bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t), 
                   sizeof(ent_genstats_t) + sizeof(ien_isa_stats_t));

            break;
        }

        case NDD_CLEAR_STATS: 
        {

            rc = ient_clrstat(p_dds);
            break;
        }

        /* Enable the adapter to receive all multicast packets. */

        case NDD_ENABLE_MULTICAST: 
        {

            /* Check the current state of the adapter. */
            if (WRK.adap_state != OPEN_STATE)
            {
                if (WRK.adap_state == DEAD_STATE)
                {
                    rc = ENETDOWN;
                }
                else
                {
                    rc = ENETUNREACH;
                }
                break;
            }

            /*
            ** If the adapter is not already in receive multicast packets, then
            ** enable that mode.
            */

            if (!WRK.enable_multi)
            {
                /* This is the first 'enable', enable all multicasts. */
                /* Reflect the new state. */

                p_ndd->ndd_flags |= NDD_MULTICAST;

                /*
                ** Update the adapter.
                ** Set the 'accept multicast' bit in the receive config
                ** register, with the following sequence of commands.
                */

                /* Attach to the IO-bus. */
                ioaddr = iomem_att(dd_ctrl.io_map_ptr);

                /* First stop the NIC. */
                ient_stop_NIC(p_dds, ioaddr);

                /* Set page. #2. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC | IEN_ISA_PAGE2;
                eieio();

                /* Read the current value of 'RCR' from page 2. */
                cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
                eieio();

                /* Switch back to the first page. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC;
                eieio();

                /* Set the 'accept multicast' packets bit. */
                cur_rcr |= IEN_ISA_ACCEPT_MCASTS;
                *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
                eieio();

                /* Switch to page 1. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC | IEN_ISA_PAGE1;
                eieio();

                /* Set all the multicast bits. */
                for (i = PAGE1_MAR0; i <= PAGE1_MAR7; i++) {
                    *(ioaddr + DDI.io_addr + i) = 0xff;
                }
                eieio();

                /* Switch to page 0. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC;
                eieio();

                /* Re-start the NIC. */
                ient_start_NIC(p_dds, ioaddr);

                iomem_det((void *) ioaddr);
            }

            WRK.enable_multi++;    /* Increment the reference counter. */
            break;
        }

        /* Disable the all multicast function.  */
        case NDD_DISABLE_MULTICAST: 
        {
            ient_multi_t        *p_multi = WRK.multi_table;
            
            /* Make sure that we are in the 'receive multicast' mode. */
            if (!WRK.enable_multi)
            {
                rc = EINVAL;
                break;
            }

            if (WRK.enable_multi) WRK.enable_multi--;

            if (!WRK.enable_multi)
            {
                /* Reflect the new state. */
                p_ndd->ndd_flags &= ~NDD_MULTICAST;

                /*
                ** Update the adapter.
                ** Set the 'disallow multicast' bit in the receive config
                ** register, with the following sequence of commands.
                */

                /* Attach to the IO-bus. */
                ioaddr = iomem_att(dd_ctrl.io_map_ptr);

                /* First stop the NIC. */
                ient_stop_NIC(p_dds, ioaddr);

                /* Set page. #2. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                    IEN_ISA_STOP_NIC | IEN_ISA_PAGE2;
                eieio();

                /* Read the current value of 'RCR' from page 2. */
                cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
                eieio();

                /* Switch back to the first page. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC;
                eieio();

                /*
                ** Make sure that if there were multicasting going on when ALL
                ** MultiCast was enabled, that we restore things back the way
                ** they were.
                */
                
                if (WRK.multi_count)
                {
                    cur_rcr |= IEN_ISA_ACCEPT_MCASTS;
                }
                else
                {
                    cur_rcr &= ~(IEN_ISA_ACCEPT_MCASTS);
                }
                
                *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
                eieio();

                /* Switch to page 1. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                    IEN_ISA_STOP_NIC | IEN_ISA_PAGE1;
                eieio();

                /*
                ** First zero out all of the MAR's and then reprogram them
                ** with the correct bits for Multicasts that were previously
                ** going on.
                */
                
                for (i = PAGE1_MAR0; i <= PAGE1_MAR7; i++)
                {
                    *(ioaddr + DDI.io_addr + i) = 0x00;
                }
                eieio();

                while (p_multi)
                {
                    cur_val = *(ioaddr + DDI.io_addr +
                                (PAGE1_MAR0 + p_multi->mar_num));
                    eieio();
                    cur_val |= p_multi->mar_val;

                    *(ioaddr + DDI.io_addr +
                      (PAGE1_MAR0 + p_multi->mar_num)) = cur_val;
                    eieio();

                    p_multi = p_multi->next;
                }
                
                /* Switch to page 0. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                     IEN_ISA_STOP_NIC;
                eieio();

                /* Re-start the NIC. */
                ient_start_NIC(p_dds, ioaddr);

                iomem_det((void *) ioaddr);

            }
            break;
        }

        /* Enable the promiscuous mode function.  */
        case NDD_PROMISCUOUS_ON: 
        {

            /* Check the adapter state first. */
            if (WRK.adap_state != OPEN_STATE)
            {
                if (WRK.adap_state == DEAD_STATE)
                {
                    rc = ENETDOWN;
                }
                else
                {
                    rc = ENETUNREACH;
                }
                break;
            }

            /* If we are not already in promiscuous mode, then enable it. */

            if (!WRK.promiscuous_count)
            {
                /* Reflect the new state. */
                p_ndd->ndd_flags |= NDD_PROMISC;
                MIB.Generic_mib.ifExtnsEntry.promiscuous = TRUE;

                /*
                ** Update the adapter.
                ** Set the 'promiscuous mode' bit in the receive config
                ** register, with the following sequence of commands.
                */
                /* Attach to the IO-bus. */
                ioaddr = iomem_att(dd_ctrl.io_map_ptr);

                /* First stop the NIC. */
                ient_stop_NIC(p_dds, ioaddr);

                /* Set page. #2. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                  IEN_ISA_STOP_NIC | IEN_ISA_PAGE2;
                eieio();

                /* Read the current value of 'RCR' from page 2. */
                cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
                eieio();

                /* Switch back to the first page. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC;
                eieio();

                /* Set the 'promiscuous mode' bit. */
                cur_rcr |= IEN_ISA_PROMIS_MODE;
                *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
                eieio();

                /* Switch to page 0. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC;
                eieio();

                /* Re-start the NIC. */
                ient_start_NIC(p_dds, ioaddr);

                iomem_det((void *) ioaddr);
            }

            WRK.promiscuous_count++;    /* Increment the reference counter. */
            break;
        }

        /* Disable the promiscuous mode function. */

        case NDD_PROMISCUOUS_OFF: 
        {

            /* Make sure that we are in the 'promiscuous' mode. */
            if (!WRK.promiscuous_count)
            {
                rc = EINVAL;
                break;
            }

            WRK.promiscuous_count--;    /* dev the reference counter */

            if (!WRK.promiscuous_count)
            {
                /* Reflect the new state. */
                p_ndd->ndd_flags &= ~NDD_PROMISC;
                MIB.Generic_mib.ifExtnsEntry.promiscuous = FALSE;

                /*
                ** Update the adapter.
                ** Reset the 'promiscuous mode' bit in the receive config
                ** register, with the following sequence of commands.
                */
                /* Attach to the IO-bus. */
                ioaddr = iomem_att(dd_ctrl.io_map_ptr);

                /* First stop the NIC. */
                ient_stop_NIC(p_dds, ioaddr);

                /* Set page. #2. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
                  IEN_ISA_STOP_NIC | IEN_ISA_PAGE2;
                eieio();

                /* Read the current value of 'RCR' from page 2. */
                cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
                eieio();

                /* Switch back to the first page. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |   
                    IEN_ISA_STOP_NIC;
                eieio();

                /* Reset the 'promiscuous mode' bit. */
                cur_rcr &= ~(IEN_ISA_PROMIS_MODE);
                *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
                eieio();

                /* Switch to page 0. */
                *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
                    IEN_ISA_STOP_NIC;
                eieio();

                /* Re-start the NIC. */
                ient_start_NIC(p_dds, ioaddr);

                iomem_det((void *) ioaddr);

            }
            break;
        }

        case NDD_ADD_FILTER: 
        {
            rc = EOPNOTSUPP;    /* Not supported. */
            break;
        }

        case NDD_DEL_FILTER: 
        {
            rc = EOPNOTSUPP;    /* Not supported. */
            break;
        }

        /* Query the MIB support status on the driver.  */

        case NDD_MIB_QUERY: 
        {

            /* Check the adapter state. */
            if (WRK.adap_state != OPEN_STATE)
            {
                if (WRK.adap_state == DEAD_STATE)
                {
                    rc = ENETDOWN;
                }
                else
                {
                    rc = ENETUNREACH;
                }
                break;
            }

            /* Make sure that we have enough room for all the information. */

            if (length != sizeof(ethernet_all_mib_t))
            {
                rc = EINVAL;
                break;
            }

            /* Copy the info to the user's buffer. */
            bcopy(&ient_mib_status, arg, sizeof(ethernet_all_mib_t));
            break;

        }

        /* Get all MIB values.  */

        case NDD_MIB_GET: 
        {

            /* Make sure that we have enough room for all the information. */
            if (length != sizeof(ethernet_all_mib_t))
            {
                rc = EINVAL;
                break;
            }

            /* Get the mib statistics. */
            if (rc = ient_getmib(p_dds))
            {
                break;
            }

            /* Copy the mibs to the user's buffer. */
            bcopy(&MIB, arg, sizeof(ethernet_all_mib_t));

            break;
        }

        /*
        ** Get the receive address table (mainly for MIB variables). 
        ** The receive address table consists of all the addresses that the
        ** adapter is armed to receive packets with. It includes the host
        ** network address, the broadcast address and the currently registered
        ** multicast addresses.
        */

        case NDD_MIB_ADDR: 
        {
            ndd_mib_addr_t      *p_table = (ndd_mib_addr_t *)arg;
            ndd_mib_addr_elem_t *p_elem;
            ient_multi_t        *p_multi = WRK.multi_table;
            int                 elem_len;
            int                 count = 0;
            int                 i;

            /* Check the adapter state. */
            if (WRK.adap_state != OPEN_STATE)
            {
                if (WRK.adap_state == DEAD_STATE)
                {
                    rc = ENETDOWN;
                }
                else
                {
                    rc = ENETUNREACH;
                }
                break;
            }

            /* Make sure that we have enough room for all the information. */

            if (length < sizeof(ndd_mib_addr_t))
            {
                rc = EINVAL;
                break;
            }

            elem_len = sizeof(ndd_mib_addr_elem_t) + ENT_NADR_LENGTH - 2;
            length -= sizeof(u_int);   /* room for count field */
            arg += 4;

            /* copy the specific network address in use first */
            if (length >= elem_len)
            {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_VOLATILE;
                p_elem->addresslen = ENT_NADR_LENGTH;
                COPY_NADR(WRK.net_addr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
            }
            else
            {
                p_table->count = 0;
                rc = E2BIG;
                break;
            }

            /* copy the broadcast address */
            if (length >= elem_len)
            {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_NONVOLATILE;
                p_elem->addresslen = ENT_NADR_LENGTH;
                COPY_NADR(ent_broad_adr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
            }
            else
            {
                p_table->count = 1;
                rc = E2BIG;
                break;
            }

            /* copy multicast addresses */

            while (p_multi)
            {
                if (length >= elem_len)
                {
                    p_elem = (ndd_mib_addr_elem_t *) arg;
                    p_elem->status = NDD_MIB_VOLATILE;
                    p_elem->addresslen = ENT_NADR_LENGTH;

                    COPY_NADR(p_multi->m_addr, p_elem->address);

                    length -= elem_len;
                    arg += elem_len;
                    count++;
                }
                else
                {
                    rc = E2BIG;  /* user table is not big enough */
                    break;
                }

                p_multi = p_multi->next;
            }

            /* put the final count into the buffer */
            p_table->count = count;

            break;
        }

        /* Add an asynchronous status filter, i.e. 'bad frame' receptions. */

        case NDD_ADD_STATUS: 
        {
            ns_com_status_t *p_stat = (ns_com_status_t *)arg;

            /* Check the adapter state. */
            if (WRK.adap_state != OPEN_STATE)
            {
                if (WRK.adap_state == DEAD_STATE)
                {
                    rc = ENETDOWN;
                }
                else
                {
                    rc = ENETUNREACH;
                }
                break;
            }

            if (p_stat->filtertype == NS_STATUS_MASK)
            {
                if (p_stat->mask & NDD_BAD_PKTS)
                {
                    if (!WRK.badframe_count)
                    {
                        p_ndd->ndd_flags |= ENT_RCV_BAD_FRAME;

                        /*
                        ** Configure the adapter to receive bad packets.
                        ** Set the 'bad frame mode' bit in the receive config
                        ** register, with the following sequence of commands.
                        */

                        /* Attach to the IO-bus. */

                        ioaddr = iomem_att(dd_ctrl.io_map_ptr);

                        /* First stop the NIC. */
                        ient_stop_NIC(p_dds, ioaddr);

                        /* Set page. #2. */
                        *(ioaddr + DDI.io_addr + PAGE0_COMMAND) =
                            IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC | IEN_ISA_PAGE2;
                        eieio();

                        /* Read the current value of 'RCR' from page 2. */
                        cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
                        eieio();

                        /* Switch back to the first page. */
                        *(ioaddr + DDI.io_addr + PAGE0_COMMAND) =
                            IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC;
                        eieio();

                        /* Set the 'bad frame mode' bit. */
                        cur_rcr |= IEN_ISA_RCV_ERRORS;
                        *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
                        eieio();

                        /* Re-start the NIC. */
                        ient_start_NIC(p_dds, ioaddr);

                        /* Deattach from the bus. */
                        iomem_det((void *) ioaddr);

                    }

                    WRK.badframe_count++; /* Increment the reference count. */
                }
                else  /* if (p_stat->mask... */
                {
                    WRK.otherstatus++; /* Increment the reference count. */
                }

            }
            else      /* if (p_stat->filtertype... */
            {
                rc = EINVAL;
            }
            break;
        }

        /* Delete an asynchronous status filter, i.e. 'bad frames'. */

        case NDD_DEL_STATUS: 
        {
            ns_com_status_t *p_stat = (ns_com_status_t *)arg;

            if (p_stat->filtertype == NS_STATUS_MASK)
            {
                if (p_stat->mask & NDD_BAD_PKTS)
                {
                    if (!WRK.badframe_count)
                    {
                        rc = EINVAL;
                        break;
                    }

                    WRK.badframe_count--; /* dec reference count */

                    if (!WRK.badframe_count)
                    {
                        p_ndd->ndd_flags &= ~ENT_RCV_BAD_FRAME;

                        /*
                        ** Configure the adapter not to receive bad packets.
                        ** Set the 'bad frame mode' bit in the receive config
                        ** register, with the following sequence of commands.
                        */

                        /* Attach to the IO-bus. */
                        ioaddr = iomem_att(dd_ctrl.io_map_ptr);

                        /* First stop the NIC. */
                        ient_stop_NIC(p_dds, ioaddr);

                        /* Set page. #2. */
                        *(ioaddr + DDI.io_addr + PAGE0_COMMAND) =
                          IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC | IEN_ISA_PAGE2;
                        eieio();

                        /* Read the current value of 'RCR' from page 2. */
                        cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
                        eieio();

                        /* Switch back to the first page. */
                        *(ioaddr + DDI.io_addr + PAGE0_COMMAND) =
                          IEN_ISA_NO_DMA | IEN_ISA_STOP_NIC;
                        eieio();

                        /* Reset the 'bad frame mode' bit. */
                        cur_rcr &= ~(IEN_ISA_RCV_ERRORS);
                        *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
                        eieio();

                        /* Re-start the NIC. */
                        ient_start_NIC(p_dds, ioaddr);

                        /* Deattach from the bus. */
                        iomem_det((void *) ioaddr);
                    }
                }
                else  /* if (p_stat->mask.... */
                {
                    if (!WRK.otherstatus)
                    {
                        rc = EINVAL;
                        break;
                    }

                    WRK.otherstatus--; /* dec reference count */
                }
            }
            else      /* if (p_stat->filtertype.... */
            {
                rc = EINVAL;
            }
            break;
        }

        /* Add a multicast address to the multicast filter.  */

        case NDD_ENABLE_ADDRESS: 
        {
            /* Check the adapter state. */
            if (WRK.adap_state != OPEN_STATE)
            {
                if (WRK.adap_state == DEAD_STATE)
                {
                    rc = ENETDOWN;
                }
                else
                {
                    rc = ENETUNREACH;
                }
                break;
            }

            if (length == ENT_NADR_LENGTH)
            {
                if ((rc = ient_multi_add(p_dds, arg)) == 0)
                {
                    p_ndd->ndd_flags |= NDD_ALTADDRS;
                }
            }
            else
            {
                rc = EINVAL;
            }

            break;
        }

        /* Delete a multicast address from the multicast filter table. */

        case NDD_DISABLE_ADDRESS: 
        {
            /* Check the adapter state. */
            if (WRK.adap_state != OPEN_STATE)
            {
                if (WRK.adap_state == DEAD_STATE)
                {
                    rc = ENETDOWN;
                }
                else
                {
                    rc = ENETUNREACH;
                }
                break;
            }

            if (length == ENT_NADR_LENGTH)
            {
                if ((rc = ient_multi_del(p_dds, arg)) == 0)
                {
                    /*
                    ** Address has been successfully deleted and
                    ** the adapter has been configured accordingly.
                    */
                    p_ndd->ndd_flags &= ~NDD_ALTADDRS;
                }
            }
            else
            {
                rc = EINVAL;
            }

            break;
        }

        default:
            rc = EOPNOTSUPP;
            TRACE_SYS(HKWD_IEN_ISA_ERR, "IOC3", rc, cmd, 0);
            break;
    }

    unlock_enable(ipri,&CMD_LOCK);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "IOCe", rc, 0, 0);
    return(rc);
}


/*****************************************************************************/
/*
 * NAME:     ient_getmib
 *
 * FUNCTION: Gather the current statistics from the adapter to the MIB table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *            ient_ctl
 *
 * INPUT:
 *            p_dds       - pointer to the device control area.
 *
 * RETURNS:  
 *            0 - OK
 *            ENETDOWN - device is down
 *            ENETUNREACH - device is currently unreachable
 */
/*****************************************************************************/

int
ient_getmib(dds_t *p_dds)    /* Device structure ptr. */
{
    int   ipri;
    int   temp;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MIBb", (ulong)p_dds, 0, 0);

    /* Check the device state. */

    if (WRK.adap_state != OPEN_STATE)
    {
        if (WRK.adap_state == DEAD_STATE)
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "MIB1", ENETDOWN, WRK.adap_state, 0);
            return(ENETDOWN);
        }
        else
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "MIB2", ENETUNREACH, WRK.adap_state, 0);
            return(ENETUNREACH);
        }
    }

    MIB.Ethernet_mib.Dot3StatsEntry.fcs_errs    = ENTSTATS.fcs_errs;
    MIB.Ethernet_mib.Dot3StatsEntry.align_errs  = ENTSTATS.align_errs;
    MIB.Ethernet_mib.Dot3StatsEntry.mac_rx_errs = ENTSTATS.rx_drop +
        ENTSTATS.overrun + ENTSTATS.rx_collisions + ENTSTATS.no_resources;
    MIB.Ethernet_mib.Dot3StatsEntry.long_frames = 0;
    MIB.Ethernet_mib.Dot3StatsEntry.excess_collisions = 
        ENTSTATS.excess_collisions;
    MIB.Ethernet_mib.Dot3StatsEntry.carriers_sense = ENTSTATS.carrier_sense;
    MIB.Ethernet_mib.Dot3StatsEntry.mac_tx_errs = ENTSTATS.underrun +
        ENTSTATS.defer_tx + ENTSTATS.late_collisions + ENTSTATS.tx_timeouts;
    MIB.Ethernet_mib.Dot3StatsEntry.s_coll_frames = ENTSTATS.s_coll_frames;
    MIB.Ethernet_mib.Dot3StatsEntry.m_coll_frames = ENTSTATS.m_coll_frames;
    MIB.Generic_mib.RcvAddrTable = WRK.multi_count + 2;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "MIBe", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:                  ient_getstat
 *
 * FUNCTION:              Gather the current statistics from the adapter 
 *                        to the device stats table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:               0 - OK
 *                        ENETDOWN - device is down
 *                        ENETUNREACH - device is currently unreachable
 */
/*****************************************************************************/

int
ient_getstat(dds_t *p_dds)    /* Device structure pointer. */
{
    int   i;                       /* Simple loop counter. */
    volatile uchar *ioaddr;        /* IO-handle.           */

    TRACE_SYS(HKWD_IEN_ISA_OTHER,"GETb",(ulong)p_dds, 0, 0);

    /* Check the device state first, only return stats if we are up running. */

    if (WRK.adap_state != OPEN_STATE)
    {
        if (WRK.adap_state == DEAD_STATE)
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "GET1", (ulong)ENETDOWN,
                     (ulong)WRK.adap_state, 0);
            return(ENETDOWN);
        }
        else
        {
            /* In between open and dead states. */
            TRACE_SYS(HKWD_IEN_ISA_ERR,"GET2",(ulong)ENETUNREACH,
                      (ulong)WRK.adap_state,(ulong)0);
            return(ENETUNREACH);
        }
    }

    NDD.ndd_genstats.ndd_elapsed_time = ELAPSED_TIME(WRK.ndd_stime);
    ENTSTATS.dev_elapsed_time         = ELAPSED_TIME(WRK.dev_stime);
    ENTSTATS.ndd_flags                = NDD.ndd_flags;
    ENTSTATS.sw_txq_len               = WRK.xmits_queued;
    ENTSTATS.hw_txq_len               = CUR_NUM_XMIT_BUFS;
    NDD.ndd_genstats.ndd_xmitque_cur  = ENTSTATS.sw_txq_len + ENTSTATS.hw_txq_len;

    /* Return the hw. address. */

    for (i = 0; i < ENT_NADR_LENGTH; i++)
    {
        ENTSTATS.ent_nadr[i] = WRK.net_addr[i];
    }

    ENTSTATS.device_type       = ENT_IEN_ISA;
    ENTSTATS.restart_count     = 0;
    DEVSTATS.multi_promis_mode = 0;

    /* Attach to IO-space. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /* Frame Check Sequence errors. */
    ENTSTATS.fcs_errs += *(ioaddr + DDI.io_addr + PAGE0_CNTR1);
    MIB.Ethernet_mib.Dot3StatsEntry.fcs_errs = ENTSTATS.fcs_errs;

    /* Alignment errors. */
    ENTSTATS.align_errs  += *(ioaddr + DDI.io_addr + PAGE0_CNTR0);
    MIB.Ethernet_mib.Dot3StatsEntry.align_errs = ENTSTATS.align_errs;

    /* Detach from IO-space. */
    iomem_det((void *) ioaddr);

    /*
    ** Overrun, short-frames, long-frames, no-resources, rx-drop, rx-coll
    ** carrier sense, fifo underruns, excess collisions, tx-timeouts, s_colls,
    ** and m_colls are always incremented in the ENTSTATS record, during normal 
    ** execution.
    */

    ENTSTATS.start_rx = ENTSTATS.no_resources;  /* Meaning-less. */
    ENTSTATS.cts_lost = 0;

    ENTSTATS.mcast_rx_ok =  MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok;
    ENTSTATS.bcast_rx_ok =  MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok;
    ENTSTATS.mcast_tx_ok =  MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok;
    ENTSTATS.bcast_tx_ok =  MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "GETe", 0, 0, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_clrstat
 *
 * FUNCTION: Clear all the network statistics and device statistics. 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *           ient_ctl
 *
 * INPUT:
 *            p_dds       - pointer to the device control area.
 *
 * RETURNS:  
 *            0 - OK
 *            ENETDOWN - device is down
 *            ENETUNREACH - device is currently unreachable
 */
/*****************************************************************************/

int
ient_clrstat(dds_t *p_dds)    /* device structure ptr. */
{
    uchar          not_used;       /* Dummy variable. */
    volatile uchar *ioaddr;        /* IO-handle.      */

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CLRb", (ulong)p_dds, 0, 0);

    /*
    ** Check the device state, make sure that the error recovery is not
    ** occurring.
    */

    if (WRK.adap_state != OPEN_STATE)
    {
        if (WRK.adap_state == DEAD_STATE)
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "CLR1", (ulong)ENETDOWN,
                     (ulong)WRK.adap_state, 0);
            return(ENETDOWN);
        }
        else
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "CLR2",(ulong)ENETUNREACH,
                      (ulong)WRK.adap_state,(ulong)0);
            return(ENETUNREACH);
        }
    }

    /* reset the start time for both ndd and device */
    WRK.ndd_stime = WRK.dev_stime = lbolt;

    /* clear all ndd_genstats */
    bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));

    /* Clear the entstats counters. */
    ENTSTATS.mcast_rx_ok = 0;
    ENTSTATS.bcast_rx_ok = 0;
    ENTSTATS.mcast_tx_ok = 0;
    ENTSTATS.bcast_tx_ok = 0;

    /* Clear all the counters on the adapter by reading them. */
    /* Attach to IO-space. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /* Alignment errors. */
    not_used = *(ioaddr + DDI.io_addr + PAGE0_CNTR0);

    /* Frame Check Sequence errors. */
    not_used = *(ioaddr + DDI.io_addr + PAGE0_CNTR1);

    /* Missed packets. */
    not_used = *(ioaddr + DDI.io_addr + PAGE0_CNTR2);

    /* Number of collisions. */
    not_used = *(ioaddr + DDI.io_addr + PAGE0_NCR);

    /* Detach from IO-space. */
    iomem_det((void *) ioaddr);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CLRe", 0, 0, 0);
    return(0);
}
