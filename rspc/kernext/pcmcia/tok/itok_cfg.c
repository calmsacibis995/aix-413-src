static char sccsid[] = "@(#)53  1.11  src/rspc/kernext/pcmcia/tok/itok_cfg.c, pcmciatok, rspc41J, 9517A_all 4/24/95 20:53:52";
/*
static char sccsid[] = "@(#)40	1.8  src/rspc/kernext/isa/itok/itok_cfg.c, isatok, rspc41J 1/6/95 15:58:58";
 */
/*
** COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
**
** FUNCTIONS: tokconfig(), config_init(), config_term(), tok_cfg_dds(), 
**            record_adapter_presense(), extract_aip_options()
**
** ORIGINS: 27
**
** IBM CONFIDENTIAL -- (IBM Confidential Restricted when
** combined with the aggregated modules for this product)
**                  SOURCE MATERIALS
** (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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
#ifdef PCMCIATOK
    int           ipri;               /* privious interrupt priority      */
    int           err;
#endif

    /* Lock to serialize access to the config entry point. */

    if (rc = lockl(&CFG_LOCK, LOCK_SHORT))
    {
        return(rc);
    }

    rc = 0;

    if (!dd_ctrl.initialized)         /* First time around ? */
    {
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
        dd_ctrl.io_map_ptr->size = 0x1000;

        /*
        ** The bus value should be passed down from the config method,
        ** this will change later on.
        */
        dd_ctrl.io_map_ptr->bid = BID_VAL(IO_ISA, ISA_IOMEM, 0);
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
        dd_ctrl.mem_map_ptr->size  = 0x2000;

        /*
        ** The bus value should be passed down from the config method, this will
        ** change later on.
        */

        dd_ctrl.mem_map_ptr->bid     = BID_VAL(IO_ISA, ISA_BUSMEM, 0);
        dd_ctrl.mem_map_ptr->busaddr = 0;

        /* initialize component dump table */

        dd_ctrl.cdt.header._cdt_magic = DMP_MAGIC;
        strncpy(dd_ctrl.cdt.header._cdt_name, DD_NAME_STR,
                sizeof(dd_ctrl.cdt.header._cdt_name));

        dd_ctrl.cdt.header._cdt_len = sizeof(dd_ctrl.cdt.header);

        /* add the device structure to the component dump table */
        tok_add_cdt("ddctrl",(char *)(&dd_ctrl), (int)sizeof(dd_ctrl));

        dd_ctrl.initialized = TRUE;

    } /* End of first time around. */

    /*
    ** Copy in the ndd_config_t structure.
    */

    if (rc = uiomove((caddr_t) &ndd_config, sizeof(ndd_config_t), 
                     UIO_WRITE, p_uio))
    {
        lock_free(&TRACE_LOCK);
        lock_free(&DD_LOCK);
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

    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFGb", (ulong) p_dds,
               (ulong) ndd_config.seq_number, (ulong) cmd);

    switch (cmd)
    {
      case CFG_INIT:

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

        /* Get temporary copy of ddi from user space */

        p_ddi = (itok_ddi_t *) ndd_config.dds;

        if (rc = copyin(p_ddi, &tempddi, sizeof(itok_ddi_t)))
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFI3", (ulong) EINVAL,
                                                   (ulong) rc,
                                                   (ulong) 0);
            rc = EINVAL;
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
#ifndef PCMCIATOK
        WRK.minor_no   = dd_ctrl.num_devs;  /* So we can find the adap_cfg. */
#endif
        /*
        ** Populate the DDI with the DDI variables from the config database.
        */
        bcopy (&tempddi, &(DDI), sizeof(DDI));
        WRK.alloc_size = dds_size;
#ifdef PCMCIATOK
        if( p_dds->ddi.io_addr == TOK_PRIMARY_ADDR )
            WRK.minor_no = TOK_PRIMARY_ADAPTER;
        else
            WRK.minor_no = TOK_SECONDARY_ADAPTER;
#endif

        /* Initialize the locks in the dds */
        lock_alloc(&TX_LOCK, LOCK_ALLOC_PIN, MON_TX_LOCK, -1);
        lock_alloc(&SLIH_LOCK, LOCK_ALLOC_PIN, MON_SLIH_LOCK, -1);

        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_A",&TX_LOCK,(ulong)0,(ulong)0);
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SL_A",&SLIH_LOCK,(ulong)0,(ulong)0);

        simple_lock_init(&TX_LOCK);
        simple_lock_init(&SLIH_LOCK);

#ifdef PCMCIATOK
        if( CheckCS() ) {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFIA",(ulong)0,(ulong)0,(ulong)0);
            rc = EIO;
            break;
        }

        p_dds->no_card    = TRUE;
        p_dds->event_word = EVENT_NULL;
        p_dds->reg_comp   = FALSE;

        if( RegisterClient( p_dds ) ) {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFIB",(ulong)0,(ulong)0,(ulong)0);
            rc = EIO;
            break;
        }

        ipri = i_disable( INTMAX );
        if ( p_dds->reg_comp == FALSE )
           err = e_sleep( &p_dds->event_word, EVENT_SIGRET );
        i_enable( ipri );

        if(err == EVENT_SIG || p_dds->return_callback || p_dds->no_card) {
           TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFIC",
              (ulong)err,(ulong)p_dds->return_callback,(ulong)p_dds->no_card );
           if( CardRemoval( p_dds )) {
               rc = EBUSY;
	       break;
	   }
           if( err = DeregisterClient( p_dds ))
	       rc = EBUSY;
	   else
               rc = ENXIO;
	   break;
        }
#endif
        /* Initialize the dds. */
        if ((rc = config_init(p_dds)) != 0)
        {
#ifdef PCMCIATOK
           TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFID",(ulong)rc,(ulong)0,(ulong)0);
           if( CardRemoval( p_dds )) {
               rc = EBUSY;
	       break;
	   }
           if( err = DeregisterClient( p_dds )) {
	       rc = EBUSY;
	       break;
	   }
#endif
            config_term(p_dds);
            rc = EIO;
            break;
        }

        /* Now issue the ns_attach call to get in the chain. */

        WRK.adap_state = CLOSED_STATE;

        if (rc = ns_attach(&NDD))
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFI5",(ulong)rc,(ulong)0,(ulong)0);
            break;
        }
#ifdef PM_SUPPORT
        /* Initialize Power Management handle */
        p_dds->pm.activity             = PM_ACTIVITY_NOT_SET;
        p_dds->pm.mode                 = PM_DEVICE_FULL_ON;
        p_dds->pm.device_idle_time     = DDI.pm_dev_itime;
        p_dds->pm.device_standby_time  = DDI.pm_dev_stime;
        p_dds->pm.idle_counter         = 0;
        p_dds->pm.handler              = tok_pm_handler;
        p_dds->pm.private              = (caddr_t)p_dds;
        p_dds->pm.devno                = WRK.minor_no;
        p_dds->pm.attribute            = 0;
        p_dds->pm.next1                = (struct pm_handle *)NULL;
        p_dds->pm.next2                = (struct pm_handle *)NULL;
        p_dds->pm.device_idle_time1    = 0;
        p_dds->pm.device_idle_time2    = 0;
        p_dds->pm.device_logical_name  = DDI.lname;
	bzero( p_dds->pm.reserve, sizeof( p_dds->pm.reserve ));
        p_dds->pm.pm_version           = 0x0100;
        p_dds->pm.extension            = NULL;
  
        /* Register Power Management handle */
        if( rc = pm_register_handle(&(p_dds->pm), PM_REGISTER)) 
	    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_1",(ulong)rc,(ulong)0,(ulong)0);
#endif /* PM_SUPPORT */

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

#ifdef PCMCIATOK
	rc = 0;
        if ((WRK.adap_state != NULL_STATE && WRK.adap_state != CLOSED_STATE) ||
            ( rc = ns_detach(&NDD)) != 0 ) {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFT2",(ulong)EBUSY,
                       (ulong)WRK.adap_state,(ulong)0);
	    if( rc != ENOENT ) {
		rc = EBUSY;
                break;
	    }
        }
#else
        if ((WRK.adap_state != NULL_STATE && WRK.adap_state != CLOSED_STATE) ||
            ns_detach(&NDD))
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CFT2",(ulong)EBUSY,
                       (ulong)WRK.adap_state,(ulong)0);
            rc = EBUSY;
            break;
        }
#endif
#ifdef PCMCIATOK
        {
            int zero = 0;

            for( ; ; ){
                compare_and_swap( &p_dds->cb_flag, &zero, -1 );
                if( p_dds->cb_flag == -1 ) 
		    break;
                delay( 1 );
            }
        }
        if( rc = CardRemoval( p_dds ) ) {
            rc = EBUSY;
            break;
        }
        if( rc = DeregisterClient( p_dds )) {
	    rc = EBUSY;
	    break;
	}
#endif

#ifdef PM_SUPPORT
       /* Unregister Power Management handle */
       if( rc = pm_register_handle(&(p_dds->pm), PM_UNREGISTER)) 
	    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_2",(ulong)rc,(ulong)0,(ulong)0);
#endif
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
#ifndef PCMCIATOK	/* Set into the RequestIRQ */
    IHS.flags    = INTR_NOT_SHARED;
#endif
    IHS.level    = DDI.intr_level;
    IHS.priority = DDI.intr_priority;

    /* IHS.bid      = DDI.bus_id;  */
     IHS.bid =  BID_VAL(IO_ISA, ISA_IOMEM, 0);

    /* we will use the bus id for shared memory accesses only. */
    /* DDI.bus_id = BID_VAL(IO_ISA, ISA_BUSMEM, 0); */
    DDI.bus_id += 0xc;

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
    }
    else
    {
        NDD.ndd_mtu = TOK_4M_MAX_DATA;
    }

    NDD.ndd_mintu      = CTOK_MIN_PACKET;
    NDD.ndd_type       = NDD_ISO88025;
    NDD.ndd_addrlen    = CTOK_NADR_LENGTH;
    NDD.ndd_hdrlen     = CTOK_MIN_PACKET;
    NDD.ndd_specstats  = (caddr_t)&(TOKSTATS);
    NDD.ndd_speclen    = sizeof(TOKSTATS) + sizeof(DEVSTATS);

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
        return(rc);
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

    if (!(rc = record_adapter_presense(p_dds)))
    {
        /* No IBM 16/4 Tokenring Adapters Found. */
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ini1",(ulong)p_dds,(ulong)0,(ulong)0);
        return(ENXIO);
    }

    /*
    ** Extract information from the adapter, i.e. do some checksum tests,
    ** get tokenring address and available options.
    */

    if (rc = extract_aip_options(p_dds))
    {
        /* The checksum is wrong, we have a bad card. */
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ini2",(ulong)rc,(ulong)0,(ulong)0);
        return(EFAULT);
    }

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"INIe",(ulong)p_dds,(ulong)0,(ulong)0);
    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  record_adapter_presense
**
** FUNCTION:              Walk the two possible I/O-base addresses where
**                        IBM 16/4 adapters can be located and record their
**                        hw-configurations
**
** EXECUTION ENVIRONMENT: process level
**
** NOTES:
**
** RETURNS:               number of token-ring adapters found
**
*/
/*****************************************************************************/

int
record_adapter_presense(dds_t *p_dds)                /* Device structure ptr.  */
{
    int            i,j,k;                            /* Simple loop counters.  */
    char           bios_str[IBM_ADAPTER_ID_LENGTH+1];/* Unique adapter string. */
    uint           adapter_setup;                    /* Dip-switch setting.    */
    volatile uchar *ioaddr;                          /* I/O space offset.      */
    volatile uchar *memaddr;                         /* Shared mem. offset.    */
    volatile uchar *aip_area, *cur_pos;              /* Shared mem. areas.     */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"RAPb",(ulong)0,(ulong)0,(ulong)0);

    dd_ctrl.num_adapters = 0;

#ifdef PCMCIATOK
    if( p_dds->ddi.io_addr == TOK_PRIMARY_ADDR ) {
        dd_ctrl.num_adapters = k = TOK_PRIMARY_ADAPTER;
        dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base = TOK_PRIMARY_ADDR;
    }
    else {
        dd_ctrl.num_adapters = k = TOK_SECONDARY_ADAPTER;
        dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base = TOK_SECONDARY_ADDR;
    }
#else
    /* Loop for the two possible adapters. */

    for (k = 0; k < 2; k++)
    {
        if (k)
        {
            /* Second time around. */
            dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base = TOK_SECONDARY_ADDR;
        }
        else /* Look at the primary address. */
            dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base = TOK_PRIMARY_ADDR;
#endif
        /* Read the configuration byte (shows BIOS addr., Intr. level. */
        ioaddr = iomem_att(dd_ctrl.io_map_ptr);

#ifdef PCMCIATOK
        *(ioaddr + dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base ) = 
            0x00 | (( p_dds->ddi.bios_addr >> 16 ) & 0xF );
        *(ioaddr + dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base ) = 
            0x10 | (( p_dds->ddi.bios_addr >> 12 ) & 0xF );
        *(ioaddr + dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base ) = 
            0x26;
        *(ioaddr + dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base ) = 
            0x30 | 0x0C | ( DDI.ring_speed << 1 ) | k ;
        *(ioaddr + dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base ) = 
            0x4f;
#endif
        adapter_setup = *(ioaddr +dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base);

        iomem_det(ioaddr);

#ifdef PCMCIATOK
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RAP1",
	    (ulong)adapter_setup,(ulong)dd_ctrl.adap_cfg[dd_ctrl.num_adapters].io_base, (ulong)0);
#endif
        if (adapter_setup != 0xff)
        {

            TRACE_DBG(HKWD_ITOK_ISA_OTHER,"RAPa",(ulong)adapter_setup,(ulong)0,
                     (ulong)0);

            /* Calculate the BIOS address. */
            dd_ctrl.adap_cfg[dd_ctrl.num_adapters].bios_addr = 
                          LOWEST_BIOS_BASE + ((adapter_setup & GET_BIOS_BITS)
                                               << BIOS_ADDR_ADJUST);

            /* Map in the aip area of the adapter. */
            memaddr = iomem_att(dd_ctrl.mem_map_ptr);

            aip_area = memaddr + dd_ctrl.adap_cfg[dd_ctrl.num_adapters].bios_addr 
                                                  + AIP_AREA_OFFSET;
#ifdef DEBUG_MAC
       printf("####################### MAC Address ####################\n");
       for (i = 0, j = 0, cur_pos = aip_area; i < 0x16; i++, j += 4) {
          printf("[%02x]\n", *(cur_pos + i));
       }
#endif
            /* Check for the IBM AT-bus identifier. */
            for (i = 0, j = 0, cur_pos = aip_area + CHANNEL_ID_OFFSET;
                 i < IBM_ADAPTER_ID_LENGTH; i++, j += 4)
            {
                bios_str[i] = (((*(cur_pos + j)) & 0xf) << 4) +
                               ((*(cur_pos + j + 2)) & 0xf);
            }
            if (!strncmp(bios_str, IBM_16_4_ADAPTER_ID, IBM_ADAPTER_ID_LENGTH))
            {
                /* We found an adapter, now get the intr. level. */
                dd_ctrl.adap_cfg[dd_ctrl.num_adapters].adapter_present = TRUE;
                dd_ctrl.adap_cfg[dd_ctrl.num_adapters].intr_priority = 
                       (adapter_setup & 0x2) ? ((adapter_setup & 0x1) ? 7 : 6) :
                       (adapter_setup & 0x1) ? 3 : 2;

                /* Get the built-in tokenring address. */
                for (i = 0; i < CTOK_NADR_LENGTH; i++)
                {
                    dd_ctrl.adap_cfg[dd_ctrl.num_adapters].aip.encoded_hw_addr[i] = 
                                      ((*(aip_area + i * 4) & 0xf) << 4) +
                                       (*(aip_area + 2 + i * 4));
                }

                /* Store the address in the 'vpd' structure as well. */
                for (i = 0; i < CTOK_NADR_LENGTH; i++)
                    VPD.vpd[i] =
                        dd_ctrl.adap_cfg[dd_ctrl.num_adapters].aip.encoded_hw_addr[i];
                                 VPD.vpd_len = CTOK_NADR_LENGTH;
#ifndef PCMCIATOK
                dd_ctrl.num_adapters++;
#endif
            }

            /* Release the aip_area. */
            iomem_det(memaddr);

        }
#ifdef PCMCIATOK
      dd_ctrl.num_adapters++;
#else
    } /* For 'each possible adapter' loop. */
#endif

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"RAPe",(ulong)dd_ctrl.num_adapters,(ulong)0,
              (ulong)0);

    return(dd_ctrl.num_adapters);
}


/*****************************************************************************/
/*
 * NAME:                   extract_aip_options
 *
 * FUNCTION:               Extract the available hw-options that the adapter(s)
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
    int           cur_adap;                    /* Current adapter index. */
    uint          checksum;                    /* Adapter ROM checksum.  */
    volatile uchar *memaddr;                   /* Shared mem. offset.    */
    volatile char *aip_area, *aca_area;        /* Shared mem. areas.     */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"EAOb",
	(ulong)dd_ctrl.num_adapters,(ulong)p_dds->ddi.io_addr, (ulong)0);

#ifdef PCMCIATOK
    /* for target adapters */
    if( p_dds->ddi.io_addr == TOK_PRIMARY_ADDR )
        cur_adap = TOK_PRIMARY_ADAPTER;
    else
        cur_adap = TOK_SECONDARY_ADAPTER;
#else
    /* Loop through for all adapters found. */

    for (cur_adap = 0; cur_adap < dd_ctrl.num_adapters; cur_adap++)
    {
#endif
        /* Map in the aip area of the adapter. */
        memaddr = iomem_att(dd_ctrl.mem_map_ptr);
        aip_area = memaddr + dd_ctrl.adap_cfg[cur_adap].bios_addr + 
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
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"EAO1",(ulong)checksum,(ulong)cur_adap,
                       (ulong)0);
            iomem_det(memaddr);
            return(EIO);
        }

        /* Read the adapter type. F = 1st., E = 2nd., .. , O = 16th. */

        dd_ctrl.adap_cfg[cur_adap].aip.adapter_type = 
                                  (*(aip_area + ADAPTER_TYPE_OFFSET)) & 0xf;

#ifdef PCMCIATOK
    	TRACE_DBG(HKWD_ITOK_ISA_OTHER,"EAO3",
	    (ulong)dd_ctrl.num_adapters,
	    (ulong)dd_ctrl.adap_cfg[cur_adap].aip.adapter_type, 
	    (ulong)0 );
#endif
        /* From the adapter type setup the data width of the tx & rx func's. */

        if (dd_ctrl.adap_cfg[cur_adap].aip.adapter_type == ADAPTER_IS_16BIT_WIDE)
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

        dd_ctrl.adap_cfg[cur_adap].aip.data_rate =
                                   (*(aip_area + DATA_RATE_OFFSET)) & 0xf;

        /* Read the "can do early token release" option. */

        dd_ctrl.adap_cfg[cur_adap].aip.early_token_release = 
                                   (*(aip_area + DATA_RATE_OFFSET)) & 0xf;

        /* DHB size possible at 4 Mbps. , F = 2048, E = 4096, D = 4464. */

        dd_ctrl.adap_cfg[cur_adap].aip.DHB_size_4_mbps = 
                                   (*(aip_area + DHB_4MB_SIZE_OFFSET)) & 0xf;

        /* DHB size possible at 16 Mbps. , F = 2048, E = 4096, B = 17960. */
        dd_ctrl.adap_cfg[cur_adap].aip.DHB_size_16_mbps = (*(aip_area +
                                   DHB_16MB_SIZE_OFFSET)) & 0xf;

        /*
        ** How much RAM is on the adapter and do we have to reset the upper 512
        ** bytes of it.
        */

        dd_ctrl.adap_cfg[cur_adap].shared_ram_width = 
                                   (*(aip_area + RAM_SIZE_OFFSET)) & 0xf;

        if (dd_ctrl.adap_cfg[cur_adap].shared_ram_width <= RAM_64K_AVAILABLE)
        {
            /* We have either 64 or 63.5k bytes. */
            if (dd_ctrl.adap_cfg[cur_adap].shared_ram_width & 0x01)
            {
                /* We have to initialize the uppermost 512 bytes. */
                dd_ctrl.adap_cfg[cur_adap].aip.initialize_RAM_area = TRUE;
            }
            dd_ctrl.adap_cfg[cur_adap].shared_ram_width = 64;
        }
        else
        {
            /* Map in the ACA area of the adapter. */
            aca_area = memaddr + dd_ctrl.adap_cfg[cur_adap].bios_addr
                               + ACA_AREA_OFFSET;

            /* We got either 8K, 16K or 32K RAM available. */
            dd_ctrl.adap_cfg[cur_adap].shared_ram_width = 1 <<
                          (((*(aca_area + RRR_ODD_OFFSET) & 0x0C) >> 2) + 3);
        }

        /* Is RAM paging supported, if so what page size. */

        dd_ctrl.adap_cfg[cur_adap].aip.RAM_paging = 
                                    (*(aip_area + RAM_PAGING_OFFSET)) & 0xf;

        dd_ctrl.adap_cfg[cur_adap].aip.RAM_paging = 
                  (~(dd_ctrl.adap_cfg[cur_adap].aip.RAM_paging & 0x01)) & 0x01;

        /* Second part of the BIOS checksum check. */
        for (i = 0, checksum = 0; i <= AIP_CHECKSUM2_RANGE; i += 2)
            checksum += (*(aip_area + i)) & 0x0f;

        if (checksum & 0xf)
        {
            TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"EAO2",(ulong)checksum,(ulong)cur_adap,
                       (ulong)0);
            iomem_det(memaddr);
            return(EIO);
        }

        /*
        ** Release the AIP area. We don't need it until the next time we
        ** restart.
        */
        iomem_det(memaddr);
#ifndef PCMCIATOK
    } /* End of for loop. */
#endif
    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"EAOe",(ulong)0,(ulong)0,(ulong)0);
    return(0);
}
#ifdef PM_SUPPORT
/*****************************************************************************/
/*
 * NAME:                   tok_pm_handler
 *
 * FUNCTION:               PCMCIATOK - IBM 16/4 PowerPC Token-ring driver      
 *                         Power Management function.
 *
 * EXECUTION ENVIRONMENT:  process level
 *
 * NOTES:
 *
 * RETURNS:                int  PM_SUCCESS (for success).
 *                         int  PM_ERROR   (for not success).
 *
 */
/*****************************************************************************/

int tok_pm_handler(caddr_t private, int mode)
{
        dds_t   *p_dds = ((struct dds *)private);
	int	ret = PM_SUCCESS;
	int	devid;

	devid = PMDEV_PCMCIA00 | DDI.bus_num << 4 | DDI.logical_socket;
	TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_b",
	    (ulong)mode,(ulong)p_dds->pm.mode, (ulong)devid );

        switch( mode ) {
        case PM_DEVICE_FULL_ON :
	    if( p_dds->pm.mode != PM_DEVICE_ENABLE ) {
		ret = PM_ERROR;
	    }
	    else {
                p_dds->pm.mode = mode;
	    }
	    break;
        case PM_DEVICE_IDLE :	/* Not support */
	    ret = PM_ERROR;
	    break;
        case PM_DEVICE_ENABLE :
            switch( p_dds->pm.mode ) {
	    case PM_DEVICE_FULL_ON :
                p_dds->pm.mode = mode;
		break;
            case PM_DEVICE_SUSPEND :
            case PM_DEVICE_HIBERNATION :
  		if( !(ret = pm_planar_control( 
		     (dev_t)p_dds->pm.devno, devid, PM_PLANAR_ON ))) {
		    if( !(ret = tok_request( p_dds ))) {
                        p_dds->pm.mode     = mode;
			p_dds->pm.activity = PM_ACTIVITY_NOT_SET;
		        ret = PM_SUCCESS;
		    }
		    else {
		        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_1",
	                    (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
		        ret = PM_ERROR;
		    }
		}
		else {
		    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_2",
	                (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
		}
		break;
            default :
                ret = PM_ERROR;
	    }
            break;
        case PM_DEVICE_SUSPEND :
        case PM_DEVICE_HIBERNATION :
	    if( p_dds->pm.mode != PM_DEVICE_ENABLE ) {
		ret = PM_ERROR;
		break;
	    }
  	    if( !( ret = tok_release( p_dds ))) {
   	        if( !( ret = pm_planar_control( 
		    (dev_t)p_dds->pm.devno, devid, PM_PLANAR_OFF))) {
                    p_dds->pm.mode     = mode;
		    p_dds->pm.activity = PM_ACTIVITY_NOT_SET;
		    ret = PM_SUCCESS;
		}
		else {
		    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_3",
	                (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
		}
  	    }
	    else {
		TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_4",
	            (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
	        ret = PM_ERROR;
	    }
            break;
        case PM_PAGE_FREEZE_NOTICE:
	case PM_PAGE_UNFREEZE_NOTICE:
	    ret = PM_SUCCESS;
            break;
        default :
	    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_5",
	        (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
            ret = PM_ERROR;
        }
	TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"PM_e",
	    (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode );
        return( ret );
}

/*****************************************************************************/
/*
 * NAME:                   tok_release
 *
 * FUNCTION:               Release the device resource for suspend or 
 *                         hibernation.                    
 *
 * EXECUTION ENVIRONMENT:  process level
 *
 * NOTES:
 *
 * RETURNS:                int  PM_SUCCESS ( for success ).
 *                         int  ERROR_NUM  ( error number form CardRemoval ).
 *
 */
/*****************************************************************************/
tok_release( dds_t *p_dds )
{
	int		ret = PM_SUCCESS;

	TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RELb",
	    (ulong)WRK.adap_state,(ulong)0,(ulong)0);

	p_dds->configured = 1;
	p_dds->net_status = 0;

	if( WRK.adap_state == OPEN_STATE ) {
	    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"REL1",
		(ulong)ret ,(ulong)WRK.adap_state,(ulong)0);
	    p_dds->net_status = NDD_UP;
	    ret = tokclose( &NDD );
	}
	if( ret == PM_SUCCESS ) {
	    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"REL2",
		(ulong)ret ,(ulong)WRK.adap_state,(ulong)0);
	    ret = CardRemoval( p_dds );
	}

	TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RELe",
	    (ulong)ret,(ulong)WRK.adap_state,(ulong)p_dds->net_status );
	return( ret );
}

/*****************************************************************************/
/*
 * NAME:                   tok_request
 *
 * FUNCTION:               Request the device resource.
 *
 * EXECUTION ENVIRONMENT:  process level
 *
 * NOTES:
 *
 * RETURNS:                int  PM_SUCCESS   (for success).
 *                         int  PM_ERROR     (for not success).
 *                         int  CSR_BAD_TYPE ( Bad Card ).
 *                         int  ERROR_NUM    ( Error number from tok_cfg_dds ).
 *
 */
/*****************************************************************************/
tok_request( dds_t *p_dds )
{
	int	ret = PM_SUCCESS;
	int	code;

	TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"REQb",
	    (ulong)WRK.adap_state,(ulong)p_dds->net_status, (ulong)0);

	if( !( ret = CheckCard( p_dds->ddi.logical_socket ))) {
	    if( !CardInsertion( p_dds )) {
	        ret = tok_cfg_dds( p_dds );
	    }
	    else {
	        ret = PM_ERROR;
	    }
	}
	if(( ret == PM_SUCCESS ) && ( p_dds->net_status == NDD_UP )) {
	    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"REQ1",
		(ulong)ret,(ulong)WRK.adap_state,(ulong)0);
	    ret = tokopen( &NDD );
	}

	TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"REQe",
	    (ulong)ret,(ulong)0,(ulong)0);
	return( ret );
}
#endif /* PM_SUPPORT */
