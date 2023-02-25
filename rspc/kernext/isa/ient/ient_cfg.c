static char sccsid[] = "@(#)70  1.6.1.1  src/rspc/kernext/isa/ient/ient_cfg.c, isaent, rspc41J 1/6/95 15:46:52";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: ient_config(), ient_cfg_init(), ient_config_term()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
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
    int		 was_first_time;   /* was this the first time around ?  */

    /*
    ** Use lockl operation to serialize the execution of the config commands.
    */

    lockl(&CFG_LOCK, LOCK_SHORT);

    rc = 0;
    was_first_time = FALSE;

    rc = uiomove((caddr_t) &ndd_config, sizeof(ndd_config_t), 
				UIO_WRITE, p_uio);
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


    if (!dd_ctrl.initialized)                 /* Is this the first time??? */
    {
        /* Get temporary copy of ddi from user space */

        p_ddi = (ddi_t *) ndd_config.dds;

        if (rc = copyin(p_ddi, &tempddi, sizeof(ddi_t)))
        {
            unlockl(&CFG_LOCK);
            return(rc);
        }

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
            unlockl(&CFG_LOCK);
            unpincode(ient_config);
            return(ENOMEM);
        }
        
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

            if (dd_ctrl.io_map_ptr)
            {
                bzero ((char *) dd_ctrl.io_map_ptr, sizeof(struct io_map));
                xmfree((caddr_t) dd_ctrl.io_map_ptr, pinned_heap);
                dd_ctrl.io_map_ptr = NULL;
            }

            unlockl(&CFG_LOCK);
            unpincode(ient_config);
            return(ENOMEM);
        }
        
        bzero ((char *) dd_ctrl.mem_map_ptr, sizeof(struct io_map));
        dd_ctrl.mem_map_ptr->key   = IO_MEM_MAP;
        dd_ctrl.mem_map_ptr->flags = 0;
        dd_ctrl.mem_map_ptr->size  = SEGSIZE;


        dd_ctrl.mem_map_ptr->bid     = BID_ALTREG(tempddi.bus_id, ISA_BUSMEM);
        dd_ctrl.mem_map_ptr->busaddr = 0;

        /* initialize component dump table */

        dd_ctrl.cdt.head._cdt_magic = DMP_MAGIC;

        strncpy(dd_ctrl.cdt.head._cdt_name, DD_NAME_STR,
                sizeof(dd_ctrl.cdt.head._cdt_name));

        dd_ctrl.cdt.head._cdt_len = sizeof(dd_ctrl.cdt.head);

        /* add the device structure to the component dump table */
        ient_add_cdt("ddctrl",(char *)(&dd_ctrl), (int)sizeof(dd_ctrl));

        /* Indicate that we have done this once. */
        dd_ctrl.initialized = TRUE;
        was_first_time = TRUE;

    } /* End of first time around. */

    switch (cmd)
    {
      case CFG_INIT:

	if (was_first_time == FALSE ) { 
            /* Get temporary copy of ddi from user space */

            p_ddi = (ddi_t *) ndd_config.dds;

            if (rc = copyin(p_ddi, &tempddi, sizeof(ddi_t)))
            {
		rc = EINVAL;
		break;
            }
        }

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

        /* Initialize the dds. */
        if (rc = ient_cfg_init(p_dds))
        {
            ient_config_term(p_dds);
            rc = EIO;
            break;
        }

        /* With the driver configured, need to register power management */

        if (pm_register_handle(&PMGMT, PM_REGISTER) == PM_ERROR)
        {
             ient_config_term(p_dds);
             rc = EIO;
             break;
        }

        /* Now issue the ns_attach call to get in the chain. */
        WRK.adap_state = CLOSED_STATE;

        if (rc = ns_attach(&NDD))
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER, "CFI5", rc, 0, 0);
            ient_config_term(p_dds);
            pm_register_handle(&PMGMT, PM_UNREGISTER);
            rc = EIO;
            break;
        }

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

        if ((WRK.adap_state != NULL_STATE && WRK.adap_state != CLOSED_STATE) ||
            ns_detach(&NDD))
        {
            TRACE_BOTH(HKWD_IEN_ISA_OTHER,"CFT2", EBUSY, WRK.adap_state, 0);
            rc = EBUSY;
            break;
        }

        /* unregister power management services */

        pm_register_handle(&PMGMT, PM_UNREGISTER);

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
    IHS.flags    = INTR_NOT_SHARED;
    IHS.level    = DDI.intr_level;
    IHS.priority = DDI.intr_priority;
    IHS.bid = BID_ALTREG(DDI.bus_id, ISA_IOMEM);

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

    /* initialize the power management structure */

    PMGMT.activity = 0;
    PMGMT.mode = PM_DEVICE_FULL_ON;
    PMGMT.device_idle_time = 0;
    PMGMT.device_standby_time = 0;
    PMGMT.idle_counter = 0;
    PMGMT.handler = (int(*)(unsigned char*,int))ient_pmgmt;
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
