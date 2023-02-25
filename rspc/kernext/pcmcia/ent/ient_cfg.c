static char sccsid[] = "@(#)76  1.8  src/rspc/kernext/pcmcia/ent/ient_cfg.c, pcmciaent, rspc41J, 9515A_all 4/10/95 02:59:44";
/*
static char sccsid[] = "@(#)70  1.6.1.1  src/rspc/kernext/isa/ient/ient_cfg.c, isaent, rspc41J 1/6/95 15:46:52";
 */
/*
 * COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet Device Driver
 *
 * FUNCTIONS: ient_config(), ient_cfg_init(), ient_config_term()
 *
 * ORIGINS:        27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

/*************************************************************************/
/*                         Global data structures                        */
/*************************************************************************/

/*
 * The global device driver control area. Initialize the lockl lock 
 * (cfg_lock) in the beginning of the structure to LOCK_AVAIL for 
 * synchronizing the config commands.
 */

dd_ctrl_t dd_ctrl = {LOCK_AVAIL, NULL, 0, 0, 0};

/* This table contains all the MIB's statistics returned on a            */
/* NDD_MIB_QUERY operation, it's static information, i.e. never changed. */

ethernet_all_mib_t ient_mib_status = {
    /* Generic Interface Extension Table */
    MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* ifExtnsChipSet        */
    MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* ifExtnsRevWare        */
    MIB_READ_ONLY,                     /* ifExtnsMulticastsTransmittedOks */
    MIB_READ_ONLY,                     /* ifExtnsBroadcastsTransmittedOks */
    MIB_READ_ONLY,                     /* ifExtnsMulticastsReceivedOks    */
    MIB_READ_ONLY,                     /* ifExtnsBroadcastsReceivedOks    */
    MIB_READ_ONLY,                     /* ifExtnsPromiscuous              */
    /* Generic Interface Test Table */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestCommunity               */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestRequestId               */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestType                    */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestResult                  */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestCode                    */
    /* Generic Receive Address Table */
    MIB_READ_ONLY,                  /* RcvAddrTable                       */
    /* Ethernet-like Statistics group */
    MIB_READ_ONLY,                  /* dot3StatsAlignmentErrors           */
    MIB_READ_ONLY,                  /* dot3StatsFCSErrors                 */
    MIB_READ_ONLY,                  /* dot3StatsSingleCollisionFrames     */
    MIB_READ_ONLY,                  /* dot3StatsMultipleCollisionFrames   */
    MIB_NOT_SUPPORTED,              /* dot3StatsSQETestErrors             */
    MIB_NOT_SUPPORTED,              /* dot3StatsDeferredTransmissions     */
    MIB_NOT_SUPPORTED,              /* dot3StatsLateCollisions            */
    MIB_READ_ONLY,                  /* dot3StatsExcessiveCollisions       */
    MIB_READ_ONLY,                  /* dot3StatsInternalMacTransmitErrors */
    MIB_READ_ONLY,                  /* dot3StatsCarrierSenseErrors        */
    MIB_READ_ONLY,                  /* dot3StatsFrameTooLongs             */
    MIB_READ_ONLY,                  /* dot3StatsInternalMacReceiveErrors  */
    /* Ethernet-like Collision Statistics group */
    MIB_NOT_SUPPORTED,              /* dot3CollCount                      */
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,              /* dot3CollFrequencies                */
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED, MIB_NOT_SUPPORTED
};

/*****************************************************************************/
/*
 * NAME:                  ient_config
 *
 * FUNCTION:              config entry point from kernel
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:                 Applies lockl to serialize INIT, TERM and QVPD
 *                        operations.
 *
 * RETURNS:                0 or EINVAL, ENOMEM, EBUSY, EEXIST, EIO. 
 *              
 */
/*****************************************************************************/

int
ient_config(int cmd, struct uio *p_uio)
{
    int          rc;               /* Function return code.             */
    int          dds_size;         /* Used when allocating dds memory.  */
    ndd_config_t ndd_config;       /* Config information.               */
    dds_t        *p_dds;           /* Device structure pointer.         */
    ddi_t        *p_ddi, tempddi;  /* Database information pointers.    */
#ifdef PCMCIAENT
    int          err;              /* Function error return code.       */
    int          ipri;             /* privious interrupt priority       */
#endif

    /*
    ** Use lockl operation to serialize the execution of the config commands.
    */

    lockl(&CFG_LOCK, LOCK_SHORT);

    rc = 0;

    if (!dd_ctrl.initialized)                 /* Is this the first time??? */
    {
        if (rc = pincode(ient_config))
        {
            unlockl(&CFG_LOCK);
            return(rc);                       /* Returns EINVAL or ENOMEM. */
        }

        /*
        ** Initialize the global device driver lock.
        */

        lock_alloc(&DD_LOCK, LOCK_ALLOC_PIN, MON_DD_LOCK, -1);
        simple_lock_init(&DD_LOCK);

        lock_alloc(&TRACE_LOCK, LOCK_ALLOC_PIN, MON_TRACE_LOCK, -1);
        simple_lock_init(&TRACE_LOCK);

        /* Indicate that we have done this once. */
        dd_ctrl.initialized = TRUE;

        /* Setup variables in the device control structure. */
        dd_ctrl.num_devs  = 0;
        dd_ctrl.num_opens = 0;

        /*
        ** Now setup the global structure for doing I/O accesses.
        */

        dd_ctrl.io_map_ptr = (struct io_map *)
                             xmalloc(sizeof(struct io_map), 2, pinned_heap);
        
        if (!dd_ctrl.io_map_ptr)
        {
            lock_free(&TRACE_LOCK);
            lock_free(&DD_LOCK);
            dd_ctrl.initialized = FALSE;
            unlockl(&CFG_LOCK);
            unpincode(ient_config);
            return(ENOMEM);
        }
        
        bzero ((char *) dd_ctrl.io_map_ptr, sizeof(struct io_map));

        dd_ctrl.io_map_ptr->key = IO_MEM_MAP;
        dd_ctrl.io_map_ptr->flags = 0;
        dd_ctrl.io_map_ptr->size = 0x1000;

        /*
        ** The bus value should be passed down from the config method, this
        ** will change later on.
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

            if (dd_ctrl.io_map_ptr)
            {
                bzero ((char *) dd_ctrl.io_map_ptr, sizeof(struct io_map));
                xmfree((caddr_t) dd_ctrl.io_map_ptr, pinned_heap);
                dd_ctrl.io_map_ptr = NULL;
            }

            dd_ctrl.initialized = FALSE;
            unlockl(&CFG_LOCK);
            unpincode(ient_config);
            return(ENOMEM);
        }
        
        bzero ((char *) dd_ctrl.mem_map_ptr, sizeof(struct io_map));
        dd_ctrl.mem_map_ptr->key   = IO_MEM_MAP;
        dd_ctrl.mem_map_ptr->flags = 0;
        dd_ctrl.mem_map_ptr->size  = 0x4000;

        /*
        ** The bus value should be passed down from the config method, this
        ** will change later on.
        */

        dd_ctrl.mem_map_ptr->bid     = BID_VAL(IO_ISA, ISA_BUSMEM, 0);
        dd_ctrl.mem_map_ptr->busaddr = 0;

#ifdef PCMCIAENT
   	/************ attribute memory attach io_map allocation : for PCMCIA*/
   	/* Now setup the global structure for doing attribute memory access.*/
    	if (!dd_ctrl.attr_map_ptr) {
      	    dd_ctrl.attr_map_ptr = (struct io_map *)
                xmalloc(sizeof(struct io_map), 2, pinned_heap);
      	    if (!dd_ctrl.attr_map_ptr) {
                lock_free(&TRACE_LOCK);
                lock_free(&DD_LOCK);
                rc = ENOMEM;   /* Oh well, not much we can do. */
                /* Free the I/O map structure, allocated above. */
                if (dd_ctrl.io_map_ptr) {
                    bzero ((char *) dd_ctrl.io_map_ptr, sizeof(struct io_map));
                    xmfree((caddr_t) dd_ctrl.io_map_ptr, pinned_heap);
                    dd_ctrl.io_map_ptr = NULL;
            }
            /* Free the shared memory map structure, allocated above. */
            if (dd_ctrl.mem_map_ptr) {
                bzero ((char *) dd_ctrl.mem_map_ptr, sizeof(struct io_map));
                xmfree((caddr_t) dd_ctrl.mem_map_ptr, pinned_heap);
                dd_ctrl.mem_map_ptr = NULL;
            }
            unlockl(&CFG_LOCK);
            return(rc);
        }
        /* clear out local storage            */
        bzero ((char *) dd_ctrl.attr_map_ptr, sizeof(struct io_map));
        dd_ctrl.attr_map_ptr->key   = IO_MEM_MAP;
        dd_ctrl.attr_map_ptr->flags = 0;
        dd_ctrl.attr_map_ptr->size  = 0x1000;
        /* The bus value should be passed down from the config method, 
           this will change later on. */
        dd_ctrl.attr_map_ptr->bid     = BID_VAL(IO_ISA, ISA_BUSMEM, 0);
        dd_ctrl.attr_map_ptr->busaddr = 0x0;
      }
#endif
        /* initialize component dump table */

        dd_ctrl.cdt.head._cdt_magic = DMP_MAGIC;

        strncpy(dd_ctrl.cdt.head._cdt_name, DD_NAME_STR,
                sizeof(dd_ctrl.cdt.head._cdt_name));

        dd_ctrl.cdt.head._cdt_len = sizeof(dd_ctrl.cdt.head);

        /* add the device structure to the component dump table */
        ient_add_cdt("ddctrl",(char *)(&dd_ctrl), (int)sizeof(dd_ctrl));

        dd_ctrl.initialized = TRUE;

    } /* End of first time around. */

    rc = uiomove((caddr_t) &ndd_config, sizeof(ndd_config_t), UIO_WRITE, p_uio);
    if (rc)
    {
        unlockl(&CFG_LOCK);
        return(rc);
    }

    /* Setup the dds pointer. */

    p_dds = dd_ctrl.p_dds_list;
    while (p_dds)
    {
        if (p_dds->seq_number == ndd_config.seq_number)
        {
            break;
        }
        p_dds = p_dds->next;
    }

    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"CFGb",(ulong)p_dds,
               (ulong)ndd_config.seq_number,(ulong)cmd);

    switch (cmd)
    {
      case CFG_INIT:

        /* Does the device already exist. */
        if (p_dds)
        {
            TRACE_BOTH(HKWD_IEN_ISA_ERR, "CFG1", EBUSY, 0, 0);
            rc = EBUSY;
            break;
        }

        /* Make sure that we don't try to config too many devices */

        if (dd_ctrl.num_devs >= IEN_ISA_MAX_ADAPTERS)
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "CFG2", EBUSY,
                      (ulong)dd_ctrl.num_devs, 0);
            rc = EBUSY;
            break;
        }

        /* Get temporary copy of ddi from user space */

        p_ddi = (ddi_t *) ndd_config.dds;

        if (rc = copyin(p_ddi, &tempddi, sizeof(ddi_t)))
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFI3", EINVAL, (ulong)rc, 0);
            rc = EINVAL;
            break;
        }

        /* Calculate the  size needed for the entire dds */
        dds_size = sizeof(dds_t);
        dds_size += tempddi.xmt_que_size * sizeof(xmt_elem_t);

        /* Allocate the dds. */
        if ((p_dds = (dds_t *) xmalloc( dds_size, 0, pinned_heap )) == NULL)
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFI4", ENOMEM, 0, 0);
            rc = ENOMEM;
            break;
        }

        bzero (p_dds, dds_size);
        p_dds->seq_number = ndd_config.seq_number;

        /* Populate the DDI with the DDI variables from the config database. */
        bcopy (&tempddi, &(DDI), sizeof(DDI));
        WRK.alloc_size = dds_size;

        /* Initialize the locks in the dds area */
        lock_alloc(&CMD_LOCK, LOCK_ALLOC_PIN, MON_DD_LOCK, -1);
        lock_alloc(&TX_LOCK, LOCK_ALLOC_PIN, MON_TX_LOCK, -1);
        lock_alloc(&SLIH_LOCK, LOCK_ALLOC_PIN, MON_SLIH_LOCK, -1);
        simple_lock_init(&CMD_LOCK);
        simple_lock_init(&TX_LOCK);
        simple_lock_init(&SLIH_LOCK);

        /* Add the new dds into the dev_list */
        p_dds->next = dd_ctrl.p_dds_list;
        dd_ctrl.p_dds_list = p_dds;
        dd_ctrl.num_devs++;

#ifdef PCMCIAENT
        if( CheckCS() ) {
            ient_config_term(p_dds);
            rc = EIO;
            break;
        }

        p_dds->no_card = TRUE;
        p_dds->event_word = EVENT_NULL;
        p_dds->reg_comp = FALSE;

        if( RegisterClient( p_dds ) ) {
            ient_config_term(p_dds);
            rc = EIO;
            break;
        }
 
        ipri = i_disable( INTMAX );
        if ( p_dds->reg_comp == FALSE ){
            err = e_sleep( &p_dds->event_word, EVENT_SIGRET );
        }
        i_enable( ipri );

        if(err == EVENT_SIG || p_dds->return_callback || p_dds->no_card) {
	    if( CardRemoval( p_dds )) {
		rc = EBUSY;
		break;
	    }
            if( err = DeregisterClient( p_dds )) {
		rc = EBUSY;
		break;
	    }
            ient_config_term(p_dds); 
            rc = ENXIO;
            break;
        }
#endif
        /* Initialize the dds. */
        if (rc = ient_cfg_init(p_dds))
        {
#ifdef PCMCIAENT
           if( CardRemoval( p_dds )) {
               rc = EBUSY;
	       break;
	   }
           if( err = DeregisterClient( p_dds )) {
	       rc = EBUSY;
	       break;
	   }
#endif
            ient_config_term(p_dds);
            rc = EIO;
            break;
        }

        /* Now issue the ns_attach call to get in the chain. */
        WRK.adap_state = CLOSED_STATE;

        if (rc = ns_attach(&NDD))
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFI5", rc, 0, 0);
            break;
        }

#ifdef PM_SUPPORT
        /* Initialize Power Management handle */
        p_dds->pm.activity             = PM_ACTIVITY_NOT_SET;
        p_dds->pm.mode                 = PM_DEVICE_FULL_ON;
        p_dds->pm.device_idle_time     = DDI.pm_dev_itime;
        p_dds->pm.device_standby_time  = DDI.pm_dev_stime;
        p_dds->pm.idle_counter         = 0;
        p_dds->pm.handler              = ent_pm_handler;
        p_dds->pm.private              = (caddr_t)p_dds;
        p_dds->pm.devno                = 2;   /* tmp: org=0 */
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
	    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_1",(ulong)rc,(ulong)0,(ulong)0);
#endif /* PM_SUPPORT */

        break;

      case CFG_TERM:

        /* we can't do this unless there has previously been a CFG_INIT */

          if ((!dd_ctrl.initialized) || (!p_dds))
          {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFT1", ENODEV,
                       dd_ctrl.initialized, (ulong) p_dds);
            rc = ENODEV;
            break;
        }

        /* Check if device can be removed and then call ns_detach */

#ifdef PCMCIAENT
        if ((WRK.adap_state != NULL_STATE && WRK.adap_state != CLOSED_STATE) ||
            ( rc = ns_detach(&NDD)) != 0 ) {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER,"CFT2", EBUSY, WRK.adap_state, 0);
	    if( rc != ENOENT ) {
                rc = EBUSY;
                break;
	    }
        }
#else
        if ((WRK.adap_state != NULL_STATE && WRK.adap_state != CLOSED_STATE) ||
            ns_detach(&NDD))
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER,"CFT2", EBUSY, WRK.adap_state, 0);
            rc = EBUSY;
            break;
        }
#endif
#ifdef PCMCIAENT
        {
            int zero = 0;

            for( ; ; ){
                compare_and_swap( &p_dds->cb_flag, &zero, -1 );
                if( p_dds->cb_flag == -1 ) break;
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
            TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_2",(ulong)rc,(ulong)0,(ulong)0);
#endif
        /* Remove the DDS from the dd_ctrl structure and free the resources */

        ient_config_term(p_dds);
        break;

      case CFG_QVPD:
        
        if (!p_dds)                      /* Make sure device exists */
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFQ1", ENODEV, 0, 0);
            rc = ENODEV;
            break;
        }

        VPD.status  = VPD_VALID;
        VPD.length = MAX_IEN_ISA_VPD_LEN;
        
        /*
        ** Note that this Vital Product Data (VPD), might not be
        ** what you are used to seeing, i.e. it does not contain the
        ** usual POS-register stuff but merely rev. level's and
        ** such as burned into the prom of the ISA-adapter.
        ** Copy out the very limited vital product data that we do have.
        */

        if (rc = copyout(&VPD,ndd_config.p_vpd,MIN(ndd_config.l_vpd,
            sizeof(VPD))))
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFQ2", EIO, rc, 0);
            rc = EIO;
        }

        break;

      default:
        
        TRACE_BOTH(HKWD_IEN_ISA_OTHER,"CFX1", EINVAL, cmd, 0);
        rc = EINVAL;
        
    } /* end switch (cmd) */

    TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFGe", rc, 0, 0);

    /* Check if we have de-configured the device. */
    
    if (!dd_ctrl.num_devs)
    {
        /* No more devices, delete the cyclic debug table. */

        ient_del_cdt("ddctrl", (char *)(&dd_ctrl), (int)sizeof(dd_ctrl));
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
#ifdef PCMCIAENT
        /* Free the attribute mem. map structure. */
        if (dd_ctrl.attr_map_ptr) {
            bzero ((char *) dd_ctrl.attr_map_ptr, sizeof(struct io_map));
            xmfree((caddr_t) dd_ctrl.attr_map_ptr, pinned_heap);
            dd_ctrl.attr_map_ptr = NULL;
        }
#endif
        /* Finally unpin the driver. */
        unpincode(ient_config);
    }

    unlockl(&CFG_LOCK); /* Remember this one, if returning earlier. */
    return(rc);
}

/*****************************************************************************/
/*
 * NAME:                  ient_cfg_init
 *
 * FUNCTION:              perform CFG_INIT function. Initialize the device
 *                        control table and get the adapter VPD data.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:               0 - OK
 *                        EEXIST - device name in use (from ns_attach)
 *                        EINVAL - invalid parameter was passed
 *                        EIO - permanent I/O error
 */
/*****************************************************************************/

int ient_cfg_init(dds_t *p_dds)
{
    int rc;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CINb", (ulong)p_dds, 0, 0);

    /* set up the interrupt control structure section */
    IHS.next     = (struct intr *) NULL;
    IHS.handler  = ient_intr;
    IHS.bus_type = BUS_BID;
#ifndef PCMCIAENT
    IHS.flags    = INTR_NOT_SHARED;
#endif
    IHS.level    = DDI.intr_level;
    IHS.priority = DDI.intr_priority;
    IHS.bid = BID_VAL(IO_ISA, ISA_IOMEM, 0);
    DDI.bus_id += 0xc;

    DDI.shared_mem_size = 0x4000;    /* That would be 16 K. */

    DDI.shared_mem_size -= 256; /* Don't use the last buffer. */


    NDD.ndd_name  = DDI.lname; /* point to the name contained in the dds */
    NDD.ndd_alias = DDI.alias; /* This is the 'TCP/IP' interface name. */

#ifdef DEBUG
    NDD.ndd_flags |= NDD_DEBUG;
#endif

    /* save the dds address in the NDD correlator field */
    NDD.ndd_correlator = (caddr_t)&dd_ctrl;
    NDD.ndd_open      = ient_open;
    NDD.ndd_close     = ient_close;
    NDD.ndd_output    = ient_output;
    NDD.ndd_ctl       = ient_ioctl;

    NDD.ndd_addrlen   = ENT_NADR_LENGTH;
    NDD.ndd_hdrlen    = ENT_HDR_LEN;
    NDD.ndd_physaddr  = WRK.net_addr; /* To be resolved later. */
    NDD.ndd_mtu       = ENT_MAX_MTU;
    NDD.ndd_mintu     = ENT_MIN_MTU;
    NDD.ndd_type      = NDD_ISO88023;
    NDD.ndd_specstats = (caddr_t)&(ENTSTATS);
    NDD.ndd_speclen   = sizeof(ENTSTATS) + sizeof(DEVSTATS);

    /* perform device-specific initialization */
    /* if this routine returns non-zero, the device can't be configured */

    if (rc = ient_init(p_dds))
    {
        TRACE_SYS(HKWD_IEN_ISA_ERR, "CIN1", rc, 0, 0);
        return(rc);
    }

    /* Indicate that we are up. */
    NDD.ndd_flags = NDD_UP | NDD_BROADCAST | NDD_SIMPLEX;

    /* Add the dds to the component dump table */
    ient_add_cdt("DDS",(char *)p_dds, WRK.alloc_size);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CINe", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME: ient_config_term
 *
 * FUNCTION: process ientconfig entry with cmd of TERM
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:  none.
 */
/*****************************************************************************/

int ient_config_term(dds_t *p_dds)    /* dds_t area ptr */
{
    dds_t *p_dev, *p_prev;           /* Work pointers. */

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CTMb", (ulong) p_dds, 0, 0);

    /* Remove the dds_t area from the dds list */
    p_dev = dd_ctrl.p_dds_list;
    p_prev = NULL;

    while (p_dev)
    {
        if (p_dev == p_dds)
        {
            if (p_prev)
            {
                p_prev->next = p_dev->next;
            }
            else
            {
                dd_ctrl.p_dds_list = p_dev->next;
            }
            break;
        }
        p_prev = p_dev;
        p_dev = p_dev->next;
    }

    dd_ctrl.num_devs--;

    /* delete dds from component dump table */
    ient_del_cdt("DDS",(char *)p_dds, WRK.alloc_size);

    /* free the locks in the dds area */
    lock_free(&CMD_LOCK);
    lock_free(&TX_LOCK);
    lock_free(&SLIH_LOCK);

    /* give back the dds memory */
    if (xmfree(p_dds, pinned_heap) != 0)
    {
        TRACE_BOTH(HKWD_IEN_ISA_OTHER,"CTM1",(ulong)p_dds,(ulong)0,(ulong)0);
    }

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CTMe", 0, 0, 0);
    return(0);
}

#ifdef PM_SUPPORT
/*****************************************************************************/
/*
 * NAME:                   ent_pm_handler
 *
 * FUNCTION:               PCMCIAENT - IBM PCMCIA Ethernet Device Driver
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

int ent_pm_handler(caddr_t private, int mode)
{
        dds_t   *p_dds = ((struct dds *)private);
	int	ret = PM_SUCCESS;
	int	devid;

	devid = PMDEV_PCMCIA00 | DDI.bus_num << 4 | DDI.logical_socket;
	TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_b",
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
		    if( !(ret = ent_request( p_dds ))) {
                        p_dds->pm.mode     = mode;
			p_dds->pm.activity = PM_ACTIVITY_NOT_SET;
		        ret = PM_SUCCESS;
		    }
		    else {
		        TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_1",
	                    (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
		        ret = PM_ERROR;
		    }
		}
		else {
		    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_2",
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
    	    if( !( ret = ent_release( p_dds ))) {
   	        if( !( ret = pm_planar_control( 
		    (dev_t)p_dds->pm.devno, devid, PM_PLANAR_OFF))) {
                    p_dds->pm.mode     = mode;
		    p_dds->pm.activity = PM_ACTIVITY_NOT_SET;
		    ret = PM_SUCCESS;
		}
		else {
		    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_3",
	                (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
		}
  	    }
	    else {
		TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_4",
	            (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
	        ret = PM_ERROR;
	    }
            break;
        case PM_PAGE_FREEZE_NOTICE:
	case PM_PAGE_UNFREEZE_NOTICE:
	    ret = PM_SUCCESS;
            break;
        default :
	    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_5",
	        (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
            ret = PM_ERROR;
        }
	TRACE_BOTH(HKWD_IEN_ISA_OTHER,"PM_e",
	    (ulong)ret,(ulong)mode,(ulong)p_dds->pm.mode);
        return( ret );
}

/*****************************************************************************/
/*
 * NAME:                   ent_release
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
ent_release( dds_t *p_dds )
{
	int	ret = PM_SUCCESS;

	TRACE_BOTH(HKWD_IEN_ISA_OTHER,"RELb",
	    (ulong)WRK.adap_state,(ulong)0,(ulong)0);

	p_dds->configured = 1;
	p_dds->net_status = 0;

	if( WRK.adap_state == OPEN_STATE ) {
	    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"REL1",
		(ulong)ret ,(ulong)WRK.adap_state,(ulong)0);
	    p_dds->net_status = NDD_UP;
	    ret = ient_close( &NDD );
	}
	if( ret == PM_SUCCESS ) {
	    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"REL2",
		(ulong)ret ,(ulong)WRK.adap_state,(ulong)0);
	    ret = CardRemoval( p_dds );
	}

	TRACE_BOTH(HKWD_IEN_ISA_OTHER,"RELe",
	    (ulong)ret,(ulong)WRK.adap_state,(ulong)p_dds->net_status );
	return( ret );
}

/*****************************************************************************/
/*
 * NAME:                   ent_request
 *
 * FUNCTION:               Request the device resource.
 *
 * EXECUTION ENVIRONMENT:  process level
 *
 * NOTES:
 *
 * RETURNS:                int  PM_SUCCESS   ( For success).
 *                         int  PM_ERROR     ( For not success).
 *                         int  CSR_BAD_TYPE ( Bad Card ).
 *                         int  ERROR_NUM    ( Error number from tok_cfg_dds ).
 *
 */
/*****************************************************************************/
ent_request( dds_t *p_dds )
{
	int	ret = PM_SUCCESS;
	int	code;

	TRACE_BOTH(HKWD_IEN_ISA_OTHER,"REQb",
	    (ulong)WRK.adap_state,(ulong)p_dds->net_status,(ulong)0);

	if( CheckCard( p_dds->ddi.logical_socket ) != MSK_MATCH_ALL ) {
	    ret = CSR_BAD_TYPE;
	}
	else {
	    if( !CardInsertion( p_dds )) {
		ret = ient_init(p_dds); 
	    }
	    else {
	        ret = PM_ERROR;
	    }
	}

	if(( ret == PM_SUCCESS ) && ( p_dds->net_status == NDD_UP )) {
	    TRACE_BOTH(HKWD_IEN_ISA_OTHER,"REQe",
		(ulong)ret,(ulong)WRK.adap_state,(ulong)0);
	    ret = ient_open( &NDD );
	}

	TRACE_BOTH(HKWD_IEN_ISA_OTHER,"REQe",
	    (ulong)ret,(ulong)0,(ulong)0);
	return( ret );
}
#endif /* PM_SUPPORT */
