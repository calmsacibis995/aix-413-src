static char sccsid[] = "@(#)40  1.18  src/rspc/kernext/isa/itok/itok_cfg.c, isatok, rspc41J, 9524A_all 6/5/95 18:16:32";
/*
** COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
**
** FUNCTIONS: tokconfig(), config_init(), config_term(), tok_cfg_dds(), 
**            record_adapter_presence(), extract_aip_options()
**
** ORIGINS: 27
**
** IBM CONFIDENTIAL -- (IBM Confidential Restricted when
** combined with the aggregated modules for this product)
**                  SOURCE MATERIALS
** (C) COPYRIGHT International Business Machines Corp. 1994
** All Rights Reserved
**
** US Government Users Restricted Rights - Use, duplication or
** disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

#include "itok_proto.h"

/*
** This is the all important device driver control structure.
** Make sure some of the important fields are set.
*/

dd_ctrl_t dd_ctrl = { LOCK_AVAIL, NULL, 0, 0, 0, 0 };

/*
** This table contains all the MIB's statistics returned on a 
** NDD_MIB_QUERY operation, it's static information, i.e. never changed.
*/

token_ring_all_mib_t itok_isa_mibs_stats = {

    /* Generic Interface Extension Table                                  */
    MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /* ifExtnsChipSet   */
    MIB_NOT_SUPPORTED,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* ifExtnsRevWare   */
    MIB_READ_ONLY,                     /* ifExtnsMulticastsTransmittedOks */
    MIB_READ_ONLY,                     /* ifExtnsBroadcastsTransmittedOks */
    MIB_READ_ONLY,                     /* ifExtnsMulticastsReceivedOks    */
    MIB_READ_ONLY,                     /* ifExtnsBroadcastsReceivedOks    */
    MIB_NOT_SUPPORTED,                 /* ifExtnsPromiscuous              */

    /* Generic Interface Test Table */
    MIB_NOT_SUPPORTED,                 /* ifEXtnsTestCommunity            */
    MIB_NOT_SUPPORTED,                 /* ifEXtnsTestRequestId            */
    MIB_NOT_SUPPORTED,                 /* ifEXtnsTestType                 */
    MIB_NOT_SUPPORTED,                 /* ifEXtnsTestResult               */
    MIB_NOT_SUPPORTED,                 /* ifEXtnsTestCode                 */

    /* Generic Receive Address Table */
    MIB_READ_ONLY,                     /* RcvAddrTable                    */

    /* End of the generic mib statistics. */

    /* Token-Ring status and parameter values group.                      */
    MIB_NOT_SUPPORTED,                 /* dot5Commands                    */
    MIB_READ_ONLY,                     /* dot5RingStatus                  */
    MIB_READ_ONLY,                     /* dot5RingState                   */
    MIB_READ_ONLY,                     /* dot5RingOpenStatus              */
    MIB_READ_ONLY,                     /* dot5RingSpeed                   */
    MIB_READ_ONLY,0,0,0,0,0,           /* dot5UpStream                    */
    MIB_READ_ONLY,                     /* dot5ActMonParticipate           */
    MIB_READ_ONLY,0,0,0,0,0,           /* dot5Functional                  */

    /* Token-Ring Statistics group */
    MIB_READ_ONLY,                     /* dot5StatsLineErrors             */
    MIB_READ_ONLY,                     /* dot5StatsBurstErrors            */
    MIB_READ_ONLY,                     /* dot5StatsACErrors               */
    MIB_READ_ONLY,                     /* dot5StatsAbortTransErrors       */
    MIB_READ_ONLY,                     /* dot5StatsInternalErrors         */
    MIB_READ_ONLY,                     /* dot5StatsLostFrameErrors        */
    MIB_READ_ONLY,                     /* dot5StatsReceiveCongestions     */
    MIB_READ_ONLY,                     /* dot5StatsFrameCopiedErrors      */
    MIB_READ_ONLY,                     /* dot5StatsTokenErrors            */
    MIB_READ_ONLY,                     /* dot5StatsSoftErrors             */
    MIB_READ_ONLY,                     /* dot5StatsHardErrors             */
    MIB_READ_ONLY,                     /* dot5StatsSignalLoss             */
    MIB_READ_ONLY,                     /* dot5StatsTransmitBeacons        */
    MIB_READ_ONLY,                     /* dot5StatsRecoverys              */
    MIB_READ_ONLY,                     /* dot5StatsLobeWires              */
    MIB_READ_ONLY,                     /* dot5StatsRemoves                */
    MIB_READ_ONLY,                     /* dot5StatsSingles                */
    MIB_NOT_SUPPORTED,                 /* dot5StatsFreqErrors             */

    /* Token-Ring Timer group */
    MIB_NOT_SUPPORTED,                 /* dot5TimerReturnRepeat           */
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
};


/*****************************************************************************/
/*
** NAME:                  tokconfig
**
** FUNCTION:              tokconfig entry point from kernel
**
** EXECUTION ENVIRONMENT: process level
**
** NOTES:                 Applies lockl to serialize INIT, TERM and QVPD
**                        operations.
**
** RETURNS:               0 or EINVAL, ENOMEM, EBUSY, EEXIST, EIO.
**
*/
/*****************************************************************************/

int
tokconfig (int         cmd,           /* operation desired (INIT, TERM, QVPD) */
           struct uio *p_uio)         /* uio structure ptr.                   */
{
    int          rc;                  /* Function return code.            */
    int          dds_size;            /* Used when allocating dds memory. */
    ndd_config_t ndd_config;          /* Config information.              */
    dds_t        *p_dds;              /* Device structure pointer.        */
    itok_ddi_t   *p_ddi, tempddi;     /* Database information pointers.   */
    int		 was_first_time;      /* was this the first time around? */

    /* Lock to serialize access to the config entry point. */

    if (rc = lockl(&CFG_LOCK, LOCK_SHORT))
    {
        return(rc);
    }

    rc = 0;
    was_first_time = FALSE;

    /*
    ** Copy in the ndd_config_t structure.
    */

    if (rc = uiomove((caddr_t) &ndd_config, sizeof(ndd_config_t), 
                     UIO_WRITE, p_uio))
    {
        unlockl(&CFG_LOCK);
        return(rc);
    }

    /*
    ** Setup the dds pointer (there can only be two dds's !).
    */

    p_dds = dd_ctrl.p_dds_list;

    while (p_dds)
    {
        if (p_dds->seq_number == ndd_config.seq_number)
        {
            break;
        }
        p_dds = p_dds->next;
    }

    if (!dd_ctrl.initialized)         /* First time around ? */
    {
        /* Get temporary copy of ddi from user space */

        p_ddi = (itok_ddi_t *) ndd_config.dds;

        if (rc = copyin(p_ddi, &tempddi, sizeof(itok_ddi_t)))
        {
            unlockl(&CFG_LOCK);
            return(EINVAL);
    	    
        }
        /*
        ** Pin code and data.
        */
    
        if (rc = pincode(tokconfig))
        {
            unlockl(&CFG_LOCK);
            return(rc);   /* Returns EINVAL or ENOMEM. */
        }

        /* Deploy locks. */
        lock_alloc(&DD_LOCK, LOCK_ALLOC_PIN, MON_DD_LOCK, -1);
        simple_lock_init(&DD_LOCK);

        lock_alloc(&TRACE_LOCK, LOCK_ALLOC_PIN, MON_TRACE_LOCK, -1);
        simple_lock_init(&TRACE_LOCK);

        /* Setup variables in the device control structure. */
        dd_ctrl.num_devs    = 0;
        dd_ctrl.num_opens   = 0;

        /* Now setup the global structure for doing I/O accesses. */
        dd_ctrl.io_map_ptr = (struct io_map *)
                              xmalloc(sizeof(struct io_map), 2, pinned_heap);

        if (!dd_ctrl.io_map_ptr)
        {
            lock_free(&TRACE_LOCK);
            lock_free(&DD_LOCK);
            rc = ENOMEM;
            unpincode(tokconfig);
            unlockl(&CFG_LOCK);
            return(rc);
        }

        /* clear out local storage */
        bzero ((char *) dd_ctrl.io_map_ptr, sizeof(struct io_map));
        dd_ctrl.io_map_ptr->key = IO_MEM_MAP;
        dd_ctrl.io_map_ptr->flags = 0;
        dd_ctrl.io_map_ptr->size = SEGSIZE;

        dd_ctrl.io_map_ptr->bid = BID_ALTREG(tempddi.bus_id, ISA_IOMEM);
        dd_ctrl.io_map_ptr->busaddr = 0;

        /* Now setup the global structure for doing shared memory accesses. */
        dd_ctrl.mem_map_ptr = (struct io_map *)
                              xmalloc(sizeof(struct io_map), 2, pinned_heap);

        if (!dd_ctrl.mem_map_ptr)
        {
            lock_free(&TRACE_LOCK);
            lock_free(&DD_LOCK);
            rc = ENOMEM;

            /* Free the I/O map structure, allocated above. */
            if (dd_ctrl.io_map_ptr)
            {
                bzero ((char *) dd_ctrl.io_map_ptr, sizeof(struct io_map));
                xmfree((caddr_t) dd_ctrl.io_map_ptr, pinned_heap);
                dd_ctrl.io_map_ptr = NULL;
            }

            unpincode(tokconfig);
            unlockl(&CFG_LOCK);
            return(rc);
        }

        /* clear out local storage            */
        bzero ((char *) dd_ctrl.mem_map_ptr, sizeof(struct io_map));
        dd_ctrl.mem_map_ptr->key   = IO_MEM_MAP;
        dd_ctrl.mem_map_ptr->flags = 0;
        dd_ctrl.mem_map_ptr->size  = SEGSIZE;

        dd_ctrl.mem_map_ptr->bid     = BID_ALTREG(tempddi.bus_id, ISA_BUSMEM);
        dd_ctrl.mem_map_ptr->busaddr = 0;

        /* initialize component dump table */

        dd_ctrl.cdt.header._cdt_magic = DMP_MAGIC;
        strncpy(dd_ctrl.cdt.header._cdt_name, DD_NAME_STR,
                sizeof(dd_ctrl.cdt.header._cdt_name));

        dd_ctrl.cdt.header._cdt_len = sizeof(dd_ctrl.cdt.header);

        /* add the device structure to the component dump table */
        tok_add_cdt("ddctrl",(char *)(&dd_ctrl), (int)sizeof(dd_ctrl));

        dd_ctrl.initialized = TRUE;
        was_first_time = TRUE;

    } /* End of first time around. */

    switch (cmd)
    {
      case CFG_INIT:

        if (was_first_time == FALSE) /* didn't already get DDI from user space */
        {

            /* Get temporary copy of ddi from user space */

            p_ddi = (itok_ddi_t *) ndd_config.dds;

            if (rc = copyin(p_ddi, &tempddi, sizeof(itok_ddi_t)))
            {
                rc = EINVAL;
                TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFI3", (ulong) EINVAL,
                                                   (ulong) rc,
                                                   (ulong) 0);
                break;
            }
        } 

        /* does the device already exist */
        if (p_dds)
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFI1",(ulong)EBUSY,(ulong)0,(ulong)0);
            rc = EBUSY;
            break;
        }

        /* Make sure that we don't try to configure too many adapters */

        if (dd_ctrl.num_devs >= TOK_MAX_MINOR)
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFI2", (ulong) EBUSY,
                       (ulong)dd_ctrl.num_devs, (ulong) 0);
            rc = EBUSY;
            break;
        }

        /* Calculate the  size needed for the entire dds */
        dds_size = sizeof(dds_t);
        dds_size += tempddi.xmt_que_size * sizeof(xmt_elem_t);

        /* Allocate the dds. */

        if ((p_dds = (dds_t *) xmalloc( dds_size, 0, pinned_heap )) == NULL)
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER, "CFI4", (ulong)ENOMEM,
                                                    (ulong)0, (ulong)0);
            rc = ENOMEM;
            break;
        }

        bzero (p_dds, dds_size);
        p_dds->seq_number = ndd_config.seq_number;
        WRK.minor_no   = dd_ctrl.num_devs;  /* So we can find the adap_cfg. */

        /*
        ** Populate the DDI with the DDI variables from the config database.
        */
        bcopy (&tempddi, &(DDI), sizeof(DDI));
        WRK.alloc_size = dds_size;

        /* Initialize the locks in the dds */
        lock_alloc(&TX_LOCK, LOCK_ALLOC_PIN, MON_TX_LOCK, -1);
        lock_alloc(&SLIH_LOCK, LOCK_ALLOC_PIN, MON_SLIH_LOCK, -1);

        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_A",&TX_LOCK,(ulong)0,(ulong)0);
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_A",&SLIH_LOCK,(ulong)0,(ulong)0);

        simple_lock_init(&TX_LOCK);
        simple_lock_init(&SLIH_LOCK);

        /* Initialize the dds. */
        if ((rc = config_init(p_dds)) != 0)
        {
            config_term(p_dds);
	    if (rc != EINVAL)
            	rc = EIO;
            break;
        }

        /* With the driver configured, need to register power management */

        if (pm_register_handle(&PMGMT, PM_REGISTER) == PM_ERROR)
        {
             config_term(p_dds);
             rc = EIO;
             break;
        }

        /* Now issue the ns_attach call to get in the chain. */

        WRK.adap_state = CLOSED_STATE;

        if (rc = ns_attach(&NDD))
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFI5",(ulong)rc,(ulong)0,(ulong)0);
	    config_term(p_dds);
            pm_register_handle(&PMGMT, PM_UNREGISTER);
	    rc=EIO;
            break;
        }
        break;

      case CFG_TERM:

        /*
        ** We can't do this unless there has previously been a CFG_INIT
        */

        if ((!dd_ctrl.initialized) || (!p_dds))
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFT1",(ulong)ENODEV,
                       (ulong)dd_ctrl.initialized,(ulong)p_dds);
            rc = ENODEV;
            break;
        }

        /* Check if device can be removed and then call ns_detach */

        if ((WRK.adap_state != NULL_STATE && WRK.adap_state != CLOSED_STATE) ||
            ns_detach(&NDD))
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFT2",(ulong)EBUSY,
                       (ulong)WRK.adap_state,(ulong)0);
            rc = EBUSY;
            break;
        }

        /* unregister power management services */

        pm_register_handle(&PMGMT, PM_UNREGISTER);

        /* Remove the DDS from the dd_ctrl structure and free the resources */
        config_term(p_dds);
        break;

      case CFG_QVPD:
        /* Make sure device exists */
        if (!p_dds)
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFQ1",(ulong)ENODEV,
                                                  (ulong)0,(ulong)0);
            rc = ENODEV;
            break;
        }

        VPD.status  = TOK_VPD_VALID;
        VPD.vpd_len = TOK_VPD_LENGTH;

        /*
        ** Copy out the very limited vital product data that we do have.
        */

        if (rc = copyout(&VPD,ndd_config.p_vpd,MIN(ndd_config.l_vpd,
                         sizeof(VPD))))
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFQ2",(ulong)EIO,(ulong)rc,(ulong)0);
            rc = EIO;
        }

        break;

    default:
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFX1",(ulong)EINVAL,(ulong)cmd,(ulong)0);
        rc = EINVAL;

    } /* end switch (cmd) */

    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFGe",(ulong)rc,(ulong)0,(ulong)0);

    /* Check if we have de-configured the device. */

    if (!dd_ctrl.num_devs)
    {
        /* No more devices, delete the cyclic debug table. */
        tok_del_cdt("ddctrl", (char *)(&dd_ctrl), (int)sizeof(dd_ctrl));

        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_F",(ulong)&DD_LOCK,(ulong)0,(ulong)0);
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_F",(ulong)&TRACE_LOCK,(ulong)0,
                   (ulong)0);

        lock_free(&TRACE_LOCK);
        lock_free(&DD_LOCK);
        dd_ctrl.initialized = FALSE;

        /* Free the I/O map structure. */

        if (dd_ctrl.io_map_ptr)
        {
            bzero ((char *) dd_ctrl.io_map_ptr, sizeof(struct io_map));
            xmfree((caddr_t) dd_ctrl.io_map_ptr, pinned_heap);
            dd_ctrl.io_map_ptr = NULL;
        }

        /* Free the shared mem. map structure. */

        if (dd_ctrl.mem_map_ptr)
        {
            bzero ((char *) dd_ctrl.mem_map_ptr, sizeof(struct io_map));
            xmfree((caddr_t) dd_ctrl.mem_map_ptr, pinned_heap);
            dd_ctrl.mem_map_ptr = NULL;
        }

        /* Finally unpin the driver. */
        unpincode(tokconfig);

    }

    unlockl(&CFG_LOCK); /* Remember this one, if returning earlier. */

    return(rc);

} /* end tokconfig */


/*****************************************************************************/
/*
 * NAME:                  config_init
 *
 * FUNCTION:              process tokconfig entry with cmd of INIT
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:                 Don't intialize the adapter here, do that in
 *                        the open routine instead.
 *
 * RETURNS:               0 or errno
 *
 */
/*****************************************************************************/

int
config_init(dds_t *p_dds)               /* Device structure pointer. */
{
    int rc;                             /* Function return code.            */
    int i;                              /* Simple loop counter.             */
    int saved_intr_level;               /* Old interrupt level to restore.  */
    int tokintr();                      /* Interrupt handler function ptr.  */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"CINb",(ulong)p_dds,(ulong)dd_ctrl.num_devs,
              (ulong)0);

    /* Increment the number of devices configured. */
    dd_ctrl.num_devs++;

    /* set up the interrupt control structure section */
    IHS.next     = (struct intr *) NULL;
    IHS.handler  = (int(*)())tokintr;
    IHS.bus_type = BUS_BID;
    IHS.flags    = INTR_NOT_SHARED;
    IHS.level    = DDI.intr_level;
    IHS.priority = DDI.intr_priority;

    IHS.bid =  BID_ALTREG(DDI.bus_id, ISA_IOMEM);

    /* Set up the NDD structure for this DDS */
    NDD.ndd_name  = DDI.lname;
    NDD.ndd_alias = DDI.alias;
    NDD.ndd_flags = NDD_BROADCAST;

#ifdef DEBUG
    NDD.ndd_flags |= NDD_DEBUG;
#endif

    NDD.ndd_correlator = (caddr_t)&dd_ctrl;
    NDD.ndd_open       = tokopen;
    NDD.ndd_close      = tokclose;
    NDD.ndd_output     = tok_output;
    NDD.ndd_ctl        = tokioctl;

    if (DDI.ring_speed)
    {
        NDD.ndd_mtu   = TOK_16M_MAX_DATA;
        WRK.max_frame_len = TOK_16M_MAX_DATA;
    }
    else
    {
        NDD.ndd_mtu = TOK_4M_MAX_DATA;
        WRK.max_frame_len = TOK_4M_MAX_DATA;
    }


    NDD.ndd_mintu      = CTOK_MIN_PACKET;
    NDD.ndd_type       = NDD_ISO88025;
    NDD.ndd_addrlen    = CTOK_NADR_LENGTH;
    NDD.ndd_hdrlen     = CTOK_MIN_PACKET;
    NDD.ndd_specstats  = (caddr_t)&(TOKSTATS);
    NDD.ndd_speclen    = sizeof(TOKSTATS) + sizeof(DEVSTATS);

    /* initialize the power management structure */

    PMGMT.activity = 0;
    PMGMT.mode = PM_DEVICE_FULL_ON;
    PMGMT.device_idle_time = 0;
    PMGMT.device_standby_time = 0;
    PMGMT.idle_counter = 0;
    PMGMT.handler = (int(*)(unsigned char*,int))tok_pmgmt;
    PMGMT.private = (caddr_t) p_dds;
    PMGMT.devno = 0;
    PMGMT.attribute = 0;
    PMGMT.next1 = 0;
    PMGMT.next2 = 0;
    PMGMT.device_idle_time1 = 0;
    PMGMT.device_idle_time2 = 0;
    PMGMT.device_logical_name = DDI.lname;
    PMGMT.reserve[0] = '\0';
    PMGMT.reserve[1] = '\0';
    PMGMT.pm_version = 0x0100;         /* Hardcoded as defined by */
                                            /*  Power Management */
    PMGMT.extension = 0;
    WRK.pm_event = EVENT_NULL;

    /*
    ** Setup the wait queue and initialize tx. variables in the dds.
    */

    WRK.tx_waitq          = (xmt_elem_t *)((uint)p_dds + sizeof(dds_t));
    WRK.xmits_queued      = 0;
    WRK.tx_srb_waiting    = 0;
    WRK.tx_arb_waiting    = 0;
    WRK.tx_buf_in_use     = 0;
    WRK.tx_stat           = 0;
    WRK.xmit_wdt_opackets = 0;

    /* Initialize the event flags in the DDS. */
    WRK.grp_addr_event   = EVENT_NULL;
    WRK.func_addr_event  = EVENT_NULL;
    WRK.open_wait_event  = EVENT_NULL;
    WRK.sap_open_event   = EVENT_NULL;
    WRK.sap_close_event  = EVENT_NULL;
    WRK.read_log_event   = EVENT_NULL;
    WRK.close_event      = EVENT_NULL;
    WRK.srb_wait_event   = EVENT_NULL;
    WRK.command_to_do    = 0;
    WRK.intr_action      = 0;
    WRK.adap_state       = NULL_STATE;
    WRK.open_status      = OPEN_STATUS0;
    WRK.open_retries     = 0;
    WRK.ndd_stime        = 0;
    WRK.dev_stime        = 0;
    WRK.cmd_event        = 0;
    WRK.network_status   = TR_MIB_NOPROBLEM;
    WRK.last_open        = TR_MIB_NOOPEN;

    /* Reset the reference counts for the functional address bits. */

    for (i = 0; i < 31; i++)
        WRK.func_addr_ref[i] = 0;

    /*
    ** Perform any device-specific initialization of dds.
    */

    if ((rc = tok_cfg_dds(p_dds)))
    {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CIN1",(ulong)rc,(ulong)0,(ulong)0);
    }

    /* Setup the adapter open- and operations-parameters. */
    tok_setup_op_params(p_dds);

    /* Link in the device driver control structure. */
    p_dds->next        = dd_ctrl.p_dds_list;
    dd_ctrl.p_dds_list = p_dds;

    /* Add dds to the component dump table */
    tok_add_cdt("DDS", (char *)p_dds, WRK.alloc_size);

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"CFIe",(ulong)rc,(ulong)0,(ulong)0);
    return(rc);

} /* end config_init */


/*****************************************************************************/
/*
** NAME:                  config_term
**
** FUNCTION:              process tokconfig entry with cmd of TERM
**
** EXECUTION ENVIRONMENT: process level
**
** NOTES:                 Frees the dds memory.
**
** RETURNS:               nothing   
**
*/
/*****************************************************************************/

void
config_term(dds_t *p_dds)            /* Device structure ptr. */
{
    dds_t *prev_ptr;                 /* Points to the previous dd structure. */
    dds_t *index_ptr;                /* Points to the current dd structure.  */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"CTMb",(ulong)p_dds,(ulong)dd_ctrl.num_devs,
              (ulong)0);

    /* Remove this adapter structure from the adapter chain. */

    prev_ptr = index_ptr = dd_ctrl.p_dds_list;

    while (index_ptr != NULL)
    {
        if (index_ptr == p_dds)
        {
            /* We found the one we need to delete. */
            if (dd_ctrl.p_dds_list == index_ptr)
            {
                /* It was the first one in the list. */
                dd_ctrl.p_dds_list = dd_ctrl.p_dds_list->next;
            }
            else
            {
                prev_ptr->next = index_ptr->next;
            }
            break;
        }

        /* Move down the chain. */
        prev_ptr = index_ptr;
        index_ptr = index_ptr->next;
    }

    /* One less device. */
    if (dd_ctrl.num_devs) dd_ctrl.num_devs--;

    /* delete dds from component dump table */
    tok_del_cdt ("DDS",(char *)p_dds, WRK.alloc_size);

    /* Free the dds locks. */
    lock_free(&TX_LOCK);
    lock_free(&SLIH_LOCK);

    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_F",(ulong)&TX_LOCK,(ulong)0,(ulong)0);
    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_F",(ulong)&SLIH_LOCK,(ulong)0,(ulong)0);

    /* give back the dds memory */

    if (xmfree(p_dds, pinned_heap) != 0)
    {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CTM1",(ulong)p_dds,(ulong)0,(ulong)0);
    }

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"CTMe",(ulong)dd_ctrl.num_devs,(ulong)0,
              (ulong)0);

    return;

} /* end config_term */


/*****************************************************************************/
/*
** NAME:                   tok_cfg_dds
**
** FUNCTION:               Perform any device-specific initialization of
**                         the dds.
**
** EXECUTION ENVIRONMENT:  process level
**
** NOTES:                  Records the adapter settings.
**
** RETURNS:                 0 (for success), ENXIO if no adapters are found or
**                          EFAULT if the adapter's checksum is incorrect.
**
*/
/*****************************************************************************/

int
tok_cfg_dds (dds_t *p_dds)             /* Device structure ptr. */
{
    int i;                             /* Simple loop counter.  */
    int rc = 0;                        /* Function return code. */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"INIb",(ulong)p_dds,(ulong)0,(ulong)0);

    /* Try to find an (some) IBM 16/4 adapter. */

    if (rc = record_adapter_presence(p_dds))
    {
        /* Either no IBM 16/4 Tokenring Adapter Found or ODM data doesn't 
	   match adapter switch settings
	*/
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ini1",(ulong)p_dds,(ulong)0,(ulong)0);
        return(rc);
    }
    
    /*
    ** Extract information from the adapter, i.e. do some checksum tests,
    ** get tokenring address and available options.
    */

    if (rc = extract_aip_options(p_dds))
    {
	if (rc == EIO) {
            /* The checksum is wrong, we have a bad card. */
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ini2",(ulong)rc,(ulong)0,(ulong)0);
            return(EFAULT);
	}
    }

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"INIe",(ulong)p_dds,(ulong)0,(ulong)0);
    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  record_adapter_presence
**
** FUNCTION:              Walk the I/O-base addresses where the
**                        IBM 16/4 adapters is located and record it's
**                        hw-configurations
**
** EXECUTION ENVIRONMENT: process level
**
** NOTES:
**
** RETURNS:               0 or errno
**
*/
/*****************************************************************************/

int
record_adapter_presence(dds_t *p_dds)                /* Device structure ptr.  */
{
    int            i,j,k;                            /* Simple loop counters.  */
    char           bios_str[IBM_ADAPTER_ID_LENGTH+1];/* Unique adapter string. */
    char           adapter_setup;                    /* Dip-switch setting.    */
    volatile uchar *ioaddr;                          /* I/O space offset.      */
    volatile uchar *memaddr;                         /* Shared mem. offset.    */
    volatile uchar *supp_intr_area;                  /* Shared mem. offset.    */
    volatile uchar *aip_area, *cur_pos;              /* Shared mem. areas.     */
    int adapter_index;				     /* adapter index          */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"RAPb",(ulong)0,(ulong)0,(ulong)0);

    /* The bus_io_addr tells us if it's the primary token ring or
	the secondary.  Primary has bus_io_addr = 0xA20, secondary
	has bus_io_addr = 0xA24
     */
    if ( DDI.io_addr == TOK_PRIMARY_ADDR) {
	adapter_index = 0;
	if (dd_ctrl.num_adapters < 2) 
		dd_ctrl.num_adapters = 1;
    }
    else if (DDI.io_addr == TOK_SECONDARY_ADDR) {
	adapter_index = 1;
	dd_ctrl.num_adapters = 2;
	}
	else return(EINVAL);

	
    dd_ctrl.adap_cfg[adapter_index].io_base = DDI.io_addr;

    /* Read the configuration byte (shows BIOS addr., Intr. level. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    adapter_setup = *(ioaddr +dd_ctrl.adap_cfg[adapter_index].io_base);

    iomem_det(ioaddr);

    /* no adapter present */
    if (adapter_setup == 0xff)
	return(ENXIO);


    /* Calculate the BIOS address. */
    dd_ctrl.adap_cfg[adapter_index].bios_addr = 
            LOWEST_BIOS_BASE + ((adapter_setup & GET_BIOS_BITS) << BIOS_ADDR_ADJUST);

		
    /* Compare the actual bios addr (ROM) to the user configured one. */
    if (dd_ctrl.adap_cfg[adapter_index].bios_addr != DDI.bios_addr) 
         {
         TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RAP1",(ulong)DDI.bios_addr,
			(ulong)0,(ulong)0);
    	 return(EINVAL);
         }

    /* Map in the aip area of the adapter. */
    memaddr = iomem_att(dd_ctrl.mem_map_ptr);

    aip_area = memaddr + dd_ctrl.adap_cfg[adapter_index].bios_addr 
                                                  + AIP_AREA_OFFSET;

    /* Check for the IBM AT-bus identifier. */
    for (i = 0, j = 0, cur_pos = aip_area + CHANNEL_ID_OFFSET;
      i < IBM_ADAPTER_ID_LENGTH; i++, j += 4)
    { 
        bios_str[i] = (((*(cur_pos + j)) & 0xf) << 4) +
                           ((*(cur_pos + j + 2)) & 0xf);
    }

    /* if no IBM adapter present at given DDI.io_addr */
    if (strncmp(bios_str, IBM_16_4_ADAPTER_ID, IBM_ADAPTER_ID_LENGTH)) {
    	/* Release the aip_area. */
    	iomem_det(memaddr);
	return(ENXIO);
    }
    
    /* We found an adapter */
    dd_ctrl.adap_cfg[adapter_index].adapter_present = TRUE;

    /* We need to know what interrupts the adapter supports, the switch */ 
    /* selectable adapter will support a different set from the software */
    /* configurable adapter. */

    supp_intr_area = memaddr + dd_ctrl.adap_cfg[adapter_index].bios_addr 
							+ SUPP_INTR_OFFSET;

    if ( ((int)*supp_intr_area & 0x0f) == 0xf )
	{
	/* supports original set of interrupts */
	dd_ctrl.adap_cfg[adapter_index].intr_priority = 
		(adapter_setup & 0x2) ? ((adapter_setup & 0x1) ? 7 : 6) :
		(adapter_setup & 0x1) ? 3 : 2;
	}
    else if ( ((int)*supp_intr_area & 0x0f) == 0xe )
	      {
	      /* supports new set of interrupts */
	      dd_ctrl.adap_cfg[adapter_index].intr_priority = 
		      (adapter_setup & 0x2) ? ((adapter_setup & 0x1) ? 11 : 10) :
		      (adapter_setup & 0x1) ? 3 : 2;
	      }
	 else
	      {
              TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RAP2",(ulong)p_dds,
						    (ulong)supp_intr_area,
						    (ulong)adapter_setup);
	      return(ENXIO);
	      }

    /* Compare the actual intr level to the user configured one. */
    
    if (dd_ctrl.adap_cfg[adapter_index].intr_priority != DDI.intr_level){
        if (dd_ctrl.adap_cfg[adapter_index].intr_priority != 2) {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RAP3", (ulong)DDI.intr_priority,0,0); 
    	    /* Release the aip_area. */
    	    iomem_det(memaddr);
            return(EINVAL);
	}
	/* allow hw intr level of 2 to equal sw intr level of 9 */
	if (DDI.intr_level != 9) {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RAP4", (ulong)DDI.intr_priority,0,0); 
    	    /* Release the aip_area. */
    	    iomem_det(memaddr);
            return(EINVAL);
	}
    }


    /* Get the built-in tokenring address. */
    for (i = 0; i < CTOK_NADR_LENGTH; i++)
    {
        dd_ctrl.adap_cfg[adapter_index].aip.encoded_hw_addr[i] = 
                   ((*(aip_area + i * 4) & 0xf) << 4) +
                   (*(aip_area + 2 + i * 4));
    }

    /* Store the address in the 'vpd' structure as well. */
    for (i = 0; i < CTOK_NADR_LENGTH; i++)
        VPD.vpd[i] = dd_ctrl.adap_cfg[adapter_index].aip.encoded_hw_addr[i];

    VPD.vpd_len = CTOK_NADR_LENGTH;

    /* Release the aip_area. */
    iomem_det(memaddr);

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"RAPe",(ulong)adapter_index,(ulong)0,
              (ulong)0);

    return(0);
}


/*****************************************************************************/
/*
 * NAME:                   extract_aip_options
 *
 * FUNCTION:               Extract the available hw-options that the adapter
 *                         offers. The options are located in the AIP area in
 *                         the adapter's ROM area, the two checksum's are
 *                         checked.
 *
 * EXECUTION ENVIRONMENT:  process level
 *
 * NOTES:                  Collects adapter information for all possible
 *                         adapters (max. 2) and verifys the checksums.
 *
 * RETURNS:                 0 (for success), or EIO if the checksum is wrong.
 *
 */
/*****************************************************************************/

int
extract_aip_options(dds_t *p_dds)              /* Device structure ptr. */
{
    int           i;                           /* Simple loop counter.   */
    int           adapter_index;                    /* Current adapter index. */
    uint          checksum;                    /* Adapter ROM checksum.  */
    uint	  ramwidth_bits;	       /* adapter ram width temp - 
						  available physical ram */
    uint	  ramwidth;		       /* dip switch setting for amount
						  of ram to use (may be less 
						  than available ram	 */
    volatile uchar *memaddr;                   /* Shared mem. offset.    */
    volatile char *aip_area, *aca_area;        /* Shared mem. areas.     */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"EAOb",(ulong)dd_ctrl.num_adapters,(ulong)0,
              (ulong)0);

    /* working on which adapter */
    /* The bus_io_addr tells us if it's the primary token ring or
	the secondary.  Primary has bus_io_addr = 0xA20, secondary
	has bus_io_addr = 0xA24
     */
    if ( DDI.io_addr == TOK_PRIMARY_ADDR)
	adapter_index = 0;
    else if (DDI.io_addr == TOK_SECONDARY_ADDR) 
	adapter_index = 1;
	else return(EINVAL);
	
    /* Map in the aip area of the adapter. */
    memaddr = iomem_att(dd_ctrl.mem_map_ptr);
    aip_area = memaddr + dd_ctrl.adap_cfg[adapter_index].bios_addr + 
                         AIP_AREA_OFFSET;

    /*
    ** Do the first (out of two) checksum checks, to insure that the
    ** BIOS is not corrupted.
    */

    for (i = 0, checksum = 0; i <= AIP_CHECKSUM_RANGE; i += 2)
    {
        checksum += (*(aip_area + i)) & 0x0f;
    }

    if (checksum & 0xf)
    {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"EAO1",(ulong)checksum,
		(ulong)adapter_index, (ulong)0);
        iomem_det(memaddr);
        return(EIO);
    }

    /* Read the adapter type. F = 1st., E = 2nd., .. , O = 16th. */

    dd_ctrl.adap_cfg[adapter_index].aip.adapter_type = 
                              (*(aip_area + ADAPTER_TYPE_OFFSET)) & 0xf;

    /* From the adapter type setup the data width of the tx & rx func's. */

    if (dd_ctrl.adap_cfg[adapter_index].aip.adapter_type == ADAPTER_IS_16BIT_WIDE)
    {
        WRK.tx_func = (void *) tok_16bit_tx;
        WRK.rx_func = tok_16bit_rcv;
    }
    else
    {
        WRK.tx_func = (void *) tok_tx;
        WRK.rx_func = tok_rcv;
    }

    /* Read the data rate (16 Mbps or 4 Mbps. F = 4, E = 16, D = Both. */

    dd_ctrl.adap_cfg[adapter_index].aip.data_rate =
                               (*(aip_area + DATA_RATE_OFFSET)) & 0xf;

    /* Read the "can do early token release" option. */

    dd_ctrl.adap_cfg[adapter_index].aip.early_token_release = 
                               (*(aip_area + DATA_RATE_OFFSET)) & 0xf;

    /* DHB size possible at 4 Mbps. , F = 2048, E = 4096, D = 4464. */

    dd_ctrl.adap_cfg[adapter_index].aip.DHB_size_4_mbps = 
                               (*(aip_area + DHB_4MB_SIZE_OFFSET)) & 0xf;

    /* DHB size possible at 16 Mbps. , F = 2048, E = 4096, B = 17960. */
    dd_ctrl.adap_cfg[adapter_index].aip.DHB_size_16_mbps = (*(aip_area +
                               DHB_16MB_SIZE_OFFSET)) & 0xf;

    /*
    ** How much physical RAM is on the adapter and do we have to reset the upper 512
    ** bytes of it.
    */
    ramwidth_bits = (*(aip_area + RAM_SIZE_OFFSET)) & 0xf;

    dd_ctrl.adap_cfg[adapter_index].aip.initialize_RAM_area = FALSE;
     
    switch (ramwidth_bits) {
    case 0xB:
            /* We have to initialize the uppermost 512 bytes. */
            dd_ctrl.adap_cfg[adapter_index].aip.initialize_RAM_area = TRUE;
 	    /* fall through to case A */
    case 0xA:
            /* We have 64k bytes. */
            ramwidth = 0x10000; /*64K*/
            break;
    case 0xC:
            ramwidth = 0x8000; /*32K*/
            break;
    case 0xD:
            ramwidth = 0x4000; /*16K*/
            break;
    case 0xE:
            ramwidth = 0x1000; /*8K*/
            break;

    case 0xF:
    default: ramwidth = 0x10000; 
	break;
    }
    /* Use RRR ODD register to figure out width */
    /* Map in the ACA area of the adapter. */
    aca_area = memaddr + dd_ctrl.adap_cfg[adapter_index].bios_addr
               + ACA_AREA_OFFSET;

    /* We got either 8K, 16K, 32K or 64K  RAM available. */
    /* RRR bits 2,3 indicate size:
   	3|2
	--
	0,0  8K
	0,1  16K
	1,0  32K
	1,1  64K
    */
    dd_ctrl.adap_cfg[adapter_index].shared_ram_width = 1 <<
              (((*(aca_area + RRR_ODD_OFFSET) & 0x0C) >> 2) + 13);


   if (dd_ctrl.adap_cfg[adapter_index].shared_ram_width > ramwidth) {
	iomem_det(memaddr);
	return(EINVAL);
   }

    /* Compare the RAM size to the user configured one. */
    if (dd_ctrl.adap_cfg[adapter_index].shared_ram_width != DDI.shared_mem_leng)
    {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"EA03",DDI.shared_mem_leng,0,0);
	iomem_det(memaddr);
        return(EINVAL);
    }


    /* Is RAM paging supported, if so what page size. */

    dd_ctrl.adap_cfg[adapter_index].aip.RAM_paging = 
                                (*(aip_area + RAM_PAGING_OFFSET)) & 0xf;

    dd_ctrl.adap_cfg[adapter_index].aip.RAM_paging = 
              (~(dd_ctrl.adap_cfg[adapter_index].aip.RAM_paging & 0x01)) & 0x01;

    /* Second part of the BIOS checksum check. */
    for (i = 0, checksum = 0; i <= AIP_CHECKSUM2_RANGE; i += 2)
        checksum += (*(aip_area + i)) & 0x0f;

    if (checksum & 0xf)
    {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"EAO2",(ulong)checksum,
		(ulong)adapter_index, (ulong)0);
        iomem_det(memaddr);
        return(EIO);
    }

    /*
    ** Release the AIP area. We don't need it until the next time we
    ** restart.
    */
    iomem_det(memaddr);

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"EAOe",(ulong)0,(ulong)0,(ulong)0);
    return(0);
}
