static char sccsid[] = "@(#)40  1.3  src/rspc/kernext/pci/stok/stok_cfg.c, pcitok, rspc41J, 9516A_all 4/14/95 05:13:18";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: cfg_pci_regs
 *		init_dev_ctl
 *		stok_config
 *		stok_get_vpd
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 */

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/atomic_op.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/mbuf.h>
#include <sys/err_rec.h>
#include <sys/ndd.h>
#include <sys/mdio.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "stok_dslo.h"
#include "stok_mac.h"
#include "stok_dds.h"
#include "stok_dd.h"
#include "tr_stok_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern int stok_open();
extern int stok_close();
extern int stok_ctl();
extern int stok_output();
extern int stok_pmh();

void init_dev_ctl ();
/*************************************************************************/
/*  Global data structures                                               */
/*************************************************************************/

/*
 * The global device driver control area.
 * Initialize the lockl lock (cfg_lock) in the beginning of the structure
 * to LOCK_AVAIL for synchronizing the config commands.
 */
stok_dd_ctl_t stok_dd_ctl = {LOCK_AVAIL,
				NULL,
				0,
				0};


/*****************************************************************************/
/*
 * NAME:     stok_config
 *
 * FUNCTION: Configure entry point of the device driver
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM:
 *      config method for each adapter detected.
 *
 * CALLS TO:
 * 	init_dev_ctl
 *	stok_get_vpd
 *	
 * INPUT:
 *      cmd     - CFG_INIT, CFG_TERM, CFG_QVPD
 *      p_uio   - uio struct with ndd_config_t structure
 *
 * RETURNS:
 *      0      - successful
 *      EINVAL - invalid parameter was passed
 *
 *      CFG_INIT:
 *              EBUSY  - device already configured
 *              EEXIST - device name in use
 *              EINVAL - invalid parameter was passed
 *              EIO    - permanent I/O error
 *              ENOMEM - unable to allocate required memory
 *      CFG_TERM:
 *              EBUSY  - device is still opened
 *              ENODEV - device is not configured
 *      CFG_QVPD:
 *              ENODEV - device is not configured
 *              EIO    - permanent I/O error
 */
/*****************************************************************************/
stok_config(
int     cmd,                          /* command being process             */
struct uio  *p_uio)                   /* pointer to uio structure          */

{

  int rc = 0;                         /* return code                       */
  int i;                              /* loop index                        */
  ndd_config_t  ndd_config;           /* config information                */
  stok_dev_ctl_t *p_dev_ctl = NULL;    /* pointer to dev_ctl area           */
  stok_dev_ctl_t *p_dev, *p_prev;      /* temporary pointer to dev_ctl area */

  /*
   * Use lockl operation to serialize the execution of the config commands.
   */
  if ((rc = lockl(&CFG_LOCK, LOCK_SHORT)) != LOCK_SUCC) {
        return(EBUSY);
  }

  /*
   * Copys in the ndd_config_t structure 
   */
  if (rc = uiomove((caddr_t)&ndd_config,sizeof(ndd_config_t),UIO_WRITE,p_uio)) {
        unlockl(&CFG_LOCK);
  	return(EINVAL);
  }

  /*
   * Use atomic operation to make sure that the initialization is only
   * done the first time this code is entered.
   */
  if (!stok_dd_ctl.num_devs) {
        lock_alloc(&DD_LOCK, LOCK_ALLOC_PIN, STOK_DD_LOCK, -1);
        lock_init(&DD_LOCK, TRUE);

        lock_alloc(&TRACE_LOCK, LOCK_ALLOC_PIN, STOK_TRACE_LOCK, -1);
        simple_lock_init(&TRACE_LOCK);
        stok_dd_ctl.trace.next_entry = 0;
        strcpy(((char *)(&stok_dd_ctl.trace.table[0])), STOK_TRACE_TOP);
        strcpy(((char *)(&stok_dd_ctl.trace.table[STOK_TRACE_SIZE-4])),
                                                        STOK_TRACE_BOT);
  }

  /*
   * Searchs the device in the dev_list if it is there
   */
  p_dev_ctl = stok_dd_ctl.p_dev_list;
  while (p_dev_ctl) {
    if (p_dev_ctl->seq_number == ndd_config.seq_number) {
        break;
    }
    p_dev_ctl = p_dev_ctl->next;
  }

  TRACE_BOTH(STOK_OTHER, "cfgB", p_dev_ctl, cmd, (ulong)p_uio);

  switch(cmd) {
  case CFG_INIT:

        /*
         * Make sure the devices is not already config
         */
        if (p_dev_ctl) {
                TRACE_BOTH(STOK_ERR, "cfg1", p_dev_ctl,stok_dd_ctl.num_devs,0);
                rc = EBUSY;
                break;
        }

        /*
         *  Allocates memory for the dev_ctl structure
         */
        p_dev_ctl = (stok_dev_ctl_t *)
		xmalloc(sizeof(stok_dev_ctl_t), MEM_ALIGN, pinned_heap);

        if (!p_dev_ctl) {
                TRACE_BOTH(STOK_ERR, "cfg2", p_dev_ctl, ENOMEM, 0);
                rc = ENOMEM;
                break;
        }
        bzero(p_dev_ctl, sizeof(stok_dev_ctl_t));

        /*
         *  Copys in the dds for config manager
         */
        if (copyin(ndd_config.dds, &p_dev_ctl->dds, sizeof(stok_dds_t))) {
                TRACE_BOTH(STOK_ERR, "cfg3", p_dev_ctl, rc, 0);
  		xmfree(p_dev_ctl, pinned_heap);     /* Frees the dev_ctl area */
                rc = EINVAL;
		break;
        }

        init_dev_ctl(p_dev_ctl);

  	/* 
  	 * Reads the vital product data for this adapter 
  	 */
  	if (rc = stok_get_vpd(p_dev_ctl)) {
  		TRACE_BOTH(STOK_ERR, "cfg4", p_dev_ctl, rc, 0);
  		xmfree(p_dev_ctl, pinned_heap);     /* Frees the dev_ctl area */
                rc = EIO;
		break;
  	}

        /*
         * With the driver configured, need to register power management 
         * services
         */

        if (rc = pm_register_handle(&WRK.pmh, PM_REGISTER) == PM_ERROR)
        {
  		TRACE_BOTH(STOK_ERR, "cfg5", p_dev_ctl, rc, 0);
  		xmfree(p_dev_ctl, pinned_heap);     /* Frees the dev_ctl area */
                rc = EIO;
                break;
        }

  	if (rc = ns_attach(&NDD)) {
                TRACE_BOTH(STOK_ERR, "cfg6", p_dev_ctl, rc, 0);
        	pm_register_handle(&WRK.pmh, PM_UNREGISTER);
  		xmfree(p_dev_ctl, pinned_heap);     /* Frees the dev_ctl area */
                rc = EEXIST;
		break;
  	}

        /*
         *  Adds the new dev_ctl into the dev_list
         */
        p_dev_ctl->next = stok_dd_ctl.p_dev_list;
        stok_dd_ctl.p_dev_list = p_dev_ctl;
        stok_dd_ctl.num_devs++;

        p_dev_ctl->seq_number = ndd_config.seq_number;

  	/* 
  	 * Initializes the device & open states 
  	 */
  	p_dev_ctl->device_state = CLOSED;

         break;

  case CFG_TERM:
        /* 
	 * Checks whether the device exist 
	 */
        if (!p_dev_ctl) {
                TRACE_BOTH(STOK_ERR, "cfg6", p_dev_ctl, 0, 0);
                rc = ENODEV;
                break;
        }

        /*
         * Call ns_detach and make sure that it is done without error.
         */
        if (ns_detach(&(NDD))) {
                TRACE_BOTH(STOK_ERR, "cfg7", p_dev_ctl, 0, 0);
                rc = EBUSY;
                break;
        }
        pm_register_handle(&WRK.pmh, PM_UNREGISTER);

  	stok_dd_ctl.num_devs--;

        /*
         * Removes the dev_ctl area from the dev_ctl list
         * and free the resources.
         */
  	p_dev = stok_dd_ctl.p_dev_list;
  	p_prev = NULL;
  	while (p_dev) {
        	if (p_dev == p_dev_ctl) {
                	if (p_prev) {
                        	p_prev->next = p_dev->next;
                	} else {
                        	stok_dd_ctl.p_dev_list = p_dev->next;
                	}
                	break;
        	}
        	p_prev = p_dev;
        	p_dev = p_dev->next;
  	}

  	xmfree(p_dev_ctl, pinned_heap);       /* Frees the dev_ctl area */
        break;

  case CFG_QVPD:

        /* 
	 * Checks whether the device exist 
	 */
        if (!p_dev_ctl) {
                TRACE_BOTH(STOK_ERR, "cfg8", p_dev_ctl, 0, 0);
                rc = ENODEV;
                break;
        }

        if (rc = copyout((caddr_t)p_dev_ctl->vpd, ndd_config.p_vpd,
                        (int)ndd_config.l_vpd)) {
                TRACE_BOTH(STOK_ERR, "cfg9", p_dev_ctl, rc, 0);
                rc = EFAULT;
        }
        break;

  default:
        rc = EINVAL;
        break;

  }

  TRACE_BOTH(STOK_OTHER, "cfgE", p_dev_ctl, rc, 0);

  /* 
   * if we are about to be unloaded, free locks 
   */
  if (!stok_dd_ctl.num_devs) {
    	lock_free(&DD_LOCK);
    	lock_free(&TRACE_LOCK);
  }

  unlockl(&CFG_LOCK);
  return (rc);

}

/*****************************************************************************
*
* NAME: cfg_pci_regs
*
* FUNCTION: Take the information stored in the DDS and set the adapter PCI
*           config registers to proper configure.
*
* EXECUTION ENVIRONMENT:
*      This routine runs only under the process thread.
*
* INPUT:
*      p_dev_ctl       - pointer to the dev_ctl area.
*
* CALLED FROM:
*       stok_open_adapter
*       stok_get_vpd
*
* RETURN:
*      OK    - OK
*      ERROR - ERROR
*
*****************************************************************************/
int cfg_pci_regs (stok_dev_ctl_t  *p_dev_ctl)
{
  ndd_t  *p_ndd = &(NDD);
  int    rc = 0, ioa;
  struct  mdio  md;
  int           md_data;
  ushort        data;

  TRACE_BOTH(STOK_OTHER, "cprB", p_dev_ctl, 0, 0);

  md.md_data = (char *)&md_data;
  md.md_size = 1;
  md.md_incr = MV_WORD;
  md.md_sla = DDS.md_sla;

  /* set IO space */
  md.md_addr = BA0;
  md_data = TOENDIANL(DDS.busio);
  rc = pci_cfgrw(BID_VAL(IO_PCI, PCI_IOMEM, DDS.busid), &md, 1);
  if (rc) {
        TRACE_BOTH(STOK_ERR, "cpr1", p_dev_ctl, rc, 0);
        return (ERROR);
  }

 /*
  * Enables the adapter
  */
  md.md_data = (char *)&data;
  md.md_incr = MV_SHORT;
  md.md_addr = PCR;
  rc = pci_cfgrw(BID_VAL(IO_PCI, PCI_IOMEM, DDS.busid), &md, 0);
  if (rc) {
          TRACE_BOTH(STOK_ERR, "cpr2", p_dev_ctl, rc, 0);
          return (ERROR);
  }

  data |= TOENDIANW(IO_SPACE | MEM_SPACE | BUS_MASTER |
                           MWI_EN | PRTY_ERR | SEER_ENABLE);
  rc = pci_cfgrw(BID_VAL(IO_PCI, PCI_IOMEM, DDS.busid), &md, 1);
  if (rc) {
          TRACE_BOTH(STOK_ERR, "cpr3", p_dev_ctl, rc, 0);
          return (ERROR);
  }


 /*
  * use PCI configuration GPR to set ring speed
  *  +-----+-----+-----+-----+-----+-----+-----+-----+
  *  |     |     |     |     |Ring |Auto |Media|proto|
  *  |     |     |     |     |Speed|Sense|type |type |
  *  +-----+-----+-----+-----+-----+-----+-----+-----+
  *      7        6     5     4     3     2     1     0
  *      Ring Speed:
  *              0 = 4Mb
  *              1 = 16Mb
  *
  *      Auto sennse
  *              0 = no
  *              1 = yes
  *
  *       proto type:
  *               0 = Token Ring
  *
  *       Media type:
  *               0 = STP
  *               1 = UTP
  */
  if (DDS.ring_speed == SIXTEEN_MBS) {
        md_data = TOENDIANW(0x08);
        MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_SIXTEENMEGABIT;
        p_ndd->ndd_flags |= TOK_RING_SPEED_16;
  } else  if (DDS.ring_speed == FOUR_MBS) {
        MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_FOURMEGABIT;
        p_ndd->ndd_flags |= TOK_RING_SPEED_4;
  } else {
        md_data = TOENDIANW(0x04); /* autosense */
        MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_UNKNOWN;
  }
  md.md_addr = GPR;
  rc = pci_cfgrw(BID_VAL(IO_PCI, PCI_IOMEM, DDS.busid), &md, 1);
  if (rc) {
          TRACE_BOTH(STOK_ERR, "cpr4", p_dev_ctl, rc, 0);
          return (ERROR);
  }

  /*
   * Soft reset the adapter to force a known state
   */
  ioa = (int)iomem_att(&WRK.iomap);
  PIO_PUTSRX( ioa + BCTL, SOFT_RST_MSK);
  io_delay(5);  /* Wait for adapter to reset */
  PIO_PUTSRX( ioa + BCTL, 0x0000);
  iomem_det((void *)ioa);              /* restore I/O Bus                */
  if (WRK.pio_rc) {
          TRACE_BOTH(STOK_ERR, "cpr5", p_dev_ctl, WRK.pio_rc, 0);
          return(ERROR);
  }

  TRACE_BOTH(STOK_OTHER, "cprE", p_dev_ctl, rc, 0);
  return (rc);
} /* end cfg_pci_regs                                                        */


/*****************************************************************************/
/*
 * NAME:     init_dev_ctl
 *
 * FUNCTION: Initialize the device control structure 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      stok_config
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *
 */
/*****************************************************************************/
void init_dev_ctl (
  stok_dev_ctl_t *p_dev_ctl)/* pointer to the device control area */

{

  TRACE_BOTH(STOK_OTHER, "cinB", (ulong)p_dev_ctl, 0, 0);

  /* initialize the ndd structure */
  NDD.ndd_alias      = DDS.alias;          /* point to the alias name
                                                contained in the dds       */
  NDD.ndd_name       = DDS.lname;          /* point to the name contained
                                                in the dds                 */
  NDD.ndd_flags      = NDD_BROADCAST;
#ifdef DEBUG
  NDD.ndd_flags	    |= NDD_DEBUG;
#endif
  NDD.ndd_correlator = (caddr_t)p_dev_ctl; 
  NDD.ndd_addrlen    = CTOK_NADR_LENGTH;
  NDD.ndd_hdrlen     = STOK_HDR_LEN;
  NDD.ndd_physaddr   = WRK.stok_addr;       /* point to network address which
                                                will be determined later   */
  NDD.ndd_mintu      = CTOK_MIN_PACKET;

  /*
   * The NDD.ndd.mtu set here only correct when user does not select the
   * autosense.  If autosence is selected then the value of NDD.ndd.mtu
   * will be reset to the correct one after successfully open to the network
   */
  if (DDS.ring_speed == 1)  {
  	NDD.ndd_mtu        = CTOK_16M_MAX_PACKET;
  } else {
  	NDD.ndd_mtu        = CTOK_4M_MAX_PACKET;
  }
  NDD.ndd_type       = NDD_ISO88025;
  NDD.ndd_open       = stok_open;
  NDD.ndd_output     = stok_output;
  NDD.ndd_ctl        = stok_ctl;
  NDD.ndd_close      = stok_close;
  NDD.ndd_specstats  = (caddr_t)&(TOKSTATS);
  NDD.ndd_speclen    = sizeof(TOKSTATS);

  /*
   * Functional address 
   */
  FUNCTIONAL.functional[0] = 0xC0;
  FUNCTIONAL.functional[1] = 0x00;
  FUNCTIONAL.functional[2] = 0x00;
  FUNCTIONAL.functional[3] = 0x00;
  FUNCTIONAL.functional[4] = 0x00;
  FUNCTIONAL.functional[5] = 0x00;

  /*
   * Multi address variable
   */
  WRK.multi_last   = &WRK.multi_table;

  WRK.iomap.key = IO_MEM_MAP;
  WRK.iomap.bid = BID_VAL(IO_PCI, PCI_IOMEM, DDS.busid);
  WRK.iomap.busaddr = DDS.busio;
  WRK.iomap.size = STOK_IO_SIZE; 
  WRK.iomap.flags = 0; 

 /*
  * Sets up Tx1 channel variable 
  */
  TX1.max_buf_list = DDS.pri_xmt_que_sz/2;
  if (TX1.max_buf_list < MAX_TX_LIST) {
	TX1.max_buf_list = MAX_TX_LIST;
  }
  TX1.lfda = TX1LFDA_L;
  TX1.disable   = 0x4000;
  TX1.r_disable = 0x8000;

 /*
  * Sets up Tx2 channel variable 
  */
  TX2.max_buf_list = DDS.xmt_que_sz/2;
  if (TX2.max_buf_list < MAX_TX_LIST) {
	TX2.max_buf_list = MAX_TX_LIST;
  }
  TX2.lfda = TX2LFDA_L;
  TX2.disable   = 0x0400;
  TX2.r_disable = 0x0800;

 /* 
  * initialize the power management structure.
  */
  WRK.pmh.activity = 0;
  WRK.pmh.mode = PM_DEVICE_FULL_ON;
  WRK.pmh.device_idle_time = 0;
  WRK.pmh.device_standby_time = 0;
  WRK.pmh.idle_counter = 0;
  WRK.pmh.handler = (int(*)(unsigned char*,int))stok_pmh;
  WRK.pmh.private = (caddr_t) p_dev_ctl;
  WRK.pmh.devno = 0;
  WRK.pmh.attribute = 0;
  WRK.pmh.next1 = 0;
  WRK.pmh.next2 = 0;
  WRK.pmh.device_idle_time1 = 0;
  WRK.pmh.device_idle_time2 = 0;
  WRK.pmh.device_logical_name = DDS.lname;
  WRK.pmh.reserve[0] = '\0';
  WRK.pmh.reserve[1] = '\0';
  WRK.pmh.pm_version = 0x0100;    /* Hardcoded as defined by Power Management */
  WRK.pmh.extension = 0;
  WRK.pmh_event = EVENT_NULL;

  TRACE_BOTH(STOK_OTHER, "cinE", p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
* NAME: stok_get_vpd
*
* FUNCTION: Read and store the adapter's Vital Product Data
*
* EXECUTION ENVIRONMENT:
*      This routine runs only under the process thread.
*
* INPUT: 
* 	p_dev_ctl - pointer to device control structure.
*
* CALLED FROM: 
*	stok_cfg
*
* CALLS TO: 
*	cfg_pci_regs
*
* RETURNS:
*	OK     - successful
*	ERROR - error
*/
/*****************************************************************************/
int stok_get_vpd (register stok_dev_ctl_t *p_dev_ctl)
{
  int     	rc, ioa, i, pio_rc;
  ushort  	data;
  uchar 	*p_vpd;	
  uchar 	*p_adap_vpd;
  uchar 	vpd[256];

  TRACE_BOTH(STOK_OTHER, "cvpB", p_dev_ctl, 0, 0);

  if (rc = cfg_pci_regs(p_dev_ctl)) {
	return (ERROR);
  }

  /*
   * Gets the vital product data from the adapter 
   */
  ioa = (int)iomem_att(&WRK.iomap);

  PIO_PUTSRX(ioa + LAPE, SEG1);
  PIO_PUTSRX(ioa + LAPA, VPD_ADDR);
  for (i = 0; i < MAX_STOK_VPD_LEN ;) {
        PIO_GETSX(ioa + LAPD_I, &data);
        vpd[i++] = (uchar)(data >> 8);
        vpd[i++] = (uchar)(data & 0xFF);
  }
  iomem_det((void *)ioa);      /* restore I/O Bus */

  if (WRK.pio_rc) {
 	TRACE_BOTH(STOK_ERR, "cvp1", p_dev_ctl, WRK.pio_rc, 0);
        stok_bug_out(p_dev_ctl,NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 0, 0, 0);
  }

  /* Store the VPD in proper format */
  p_vpd = VPD;
  p_adap_vpd = vpd;

  /*    copy in Micro code */
  strncpy(p_vpd,"*LL",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x07;  /* length = 10 characters */
  p_vpd = p_vpd + 1;
  strncpy(p_vpd,"UNKNOWN:  ", 10);
  p_vpd = p_vpd + 10;

  /*    copy in Part Number */
  strncpy(p_vpd,"*PN",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x06;  /* length = 12 characters */
  p_vpd = p_vpd + 1;
  memcpy(p_vpd, p_adap_vpd + 0x20, 8);
  p_vpd = p_vpd + 8;

  /*    copy in EC Level */
  strncpy(p_vpd,"*EC",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x06;  /* length = 12 characters */
  p_vpd = p_vpd + 1;
  memcpy(p_vpd,p_adap_vpd + 0x28, 8);
  p_vpd = p_vpd + 8;

  /*    copy in Card Serial Number */
  strncpy(p_vpd,"*SN",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x06;  /* length = 12 characters */
  p_vpd = p_vpd + 1;
  memcpy(p_vpd,p_adap_vpd + 0x30, 8);
  p_vpd = p_vpd + 8;

  /*    copy in Card FRU */
  strncpy(p_vpd,"*FN",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x06;  /* length = 12 characters */
  p_vpd = p_vpd + 1;
  memcpy(p_vpd,p_adap_vpd + 0x40, 8);
  p_vpd = p_vpd + 8;

  /*    copy in Manufacturer */
  strncpy(p_vpd,"*MF",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x05;  /* length = 10 characters */
  p_vpd = p_vpd + 1;
  memcpy(p_vpd,p_adap_vpd + 0x48, 6);
  p_vpd = p_vpd + 6;

  /*    copy in Network Address */
  strncpy(p_vpd,"*NA",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x05;  /* length = 10 characters */
  p_vpd = p_vpd + 1;
  memcpy(p_vpd, p_adap_vpd + 0x10, 6);
  p_vpd = p_vpd + 6;

  /*    copy in a Brief Description */
  strncpy(p_vpd,"*DS",3);
  p_vpd = p_vpd + 3;
  *p_vpd = 0x24;  /* length = 40 characters */
  p_vpd = p_vpd + 1;

  strncpy(p_vpd,"IBM PCI Token-Ring Adapter (14101800)   ", 40);
  p_vpd = p_vpd + 40;

  /* 
   * Determines which network address to use, either DDS or VPD version 
   */
  if (DDS.use_alt_addr == 0) { 
	/* Use the network address that was passed in the VPD */
	/*
         * Copys the network address 
  	 */
        COPY_NADR (p_adap_vpd + 0x10, WRK.stok_addr);
  } else { /* Use the network address that was passed in the DDS */

        /* 
	 * Copys the network address 
	 */
        COPY_NADR (DDS.alt_addr, WRK.stok_addr);
  } /* endif for which network address to use */

  TRACE_BOTH(STOK_OTHER, "cvpE", p_dev_ctl, 0, 0);
  return (OK);
} /* end stok_get_vpd */
