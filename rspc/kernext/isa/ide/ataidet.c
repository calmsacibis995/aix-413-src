static char sccsid[] = "@(#)94  1.17  src/rspc/kernext/isa/ide/ataidet.c, isaide, rspc41J, 9518A_all 5/2/95 17:31:48";
/*
 * COMPONENT_NAME: (ISAIDE) IBM IDE Adapter Driver Top Half
 *
 * FUNCTIONS:
 *    lide_config             lide_open               lide_adp_str_init
 *    lide_config_devices
 *    lide_close              lide_clear_open         lide_fail_open
 *    lide_ioctl              lide_build_command      lide_inquiry
 *    lide_start_unit         lide_test_unit_rdy      lide_identify_device
 *    lide_read_ata           lide_start_dev          lide_xlate_mode
 *    lide_stop_dev           lide_issue_reset
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/************************************************************************/
/*                                                                      */
/*                                                                      */
/* NAME:        ataidet.c                                               */
/*                                                                      */
/* FUNCTION:    IBM IDE Adapter Driver Top Half Source File             */
/*                                                                      */
/*      This adapter driver is the interface between an IDE device      */
/*      driver and the actual IDE adapter.  It executes commands        */
/*      from multiple drivers which contain generic IDE device          */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/time.h>
#include <sys/timers.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/ide.h>
#include <sys/pm.h>
#include "ataide_errids.h"
#include "ataidedd.h"
/* END OF INCLUDED SYSTEM FILES  */

/************************************************************************/
/* Global pinned device driver static data areas                        */
/************************************************************************/
/* global pointer for adapter structure                                 */
extern struct	adapter_def	adp_str[MAX_ADAPTERS];
/* global pointer for ide adapter component dump table */
extern struct   lide_cdt_table *lide_cdt;
/* global configuration counter */
extern int	lide_cfg_cnt;
/* global open adapter counter */
extern int	lide_open_cnt;

/************************************************************************/
/* Global pageable device driver static data areas                      */
/************************************************************************/
/* global adapter device driver lock word                               */
lock_t	lide_lock = LOCK_AVAIL;

/************************************************************************/
/*                                                                      */
/* NAME:        lide_config                                             */
/*                                                                      */
/* FUNCTION:    Adapter Driver Configuration Routine                    */
/*                                                                      */
/*      For the INIT option, this routine allocates and initializes     */
/*      data structures for processing of requests from the device      */
/*      driver heads to the ATA task file registers.  The code is       */
/*      set up to handle only a single device instance.                 */
/*                                                                      */
/*      If the TERM option is specified, this routine will    */
/*      delete a previously defined device and free the structures      */
/*      associated with it.  If the QVPD option is specified, this      */
/*      routine will return the adapter vital product data.             */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process.                   */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure                     */
/*      ide_adap_ddi - adapter dependent information structure          */
/*      uio     - user i/o area struct                                  */
/*      devsw   - kernel entry point struct                             */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      op      - operation code (INIT, TERM, or QVPD)                  */
/*      uiop    - pointer to uio structure for data for the             */
/*                specified operation code                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      EFAULT  - return from uiomove                                   */
/*      EBUSY   - on terminate, means device still opened               */
/*      ENOMEM  - memory space unavailable for required allocation      */
/*      EINVAL  - invalid config parameter passed                       */
/*      EIO     - bad operation, or permanent I/O error                 */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      devswadd        devswdel                                        */
/*      bcopy           bzero                                           */
/*      xmalloc         xmfree                                          */
/*      uiomove         pm_register_handle                              */
/*                                                                      */
/************************************************************************/
int
lide_config(
	   dev_t devno,
	   int op,
	   struct uio * uiop)
{
    struct devsw lide_dsw;
    struct ide_adap_ddi local_ddi;
    int     ret_code, rc, i;
    extern int nodev();
    struct adapter_def *ap;

    ret_code = 0;       /* default return code */
    DDHKWD1(HKWD_DD_IDEDD, DD_ENTRY_CONFIG, ret_code, devno);

    rc = lockl(&(lide_lock), LOCK_SHORT);     /* serialize this */
    if (rc == LOCK_SUCC)
    {    /* Lock succeded */
      switch (op)
      {    /* Begin op switch */
      case CFG_INIT:  /* Handle request to initialize adapter */
        /* move adapter configuration data from uio space */
        /* into local area kernel space */
        rc = uiomove((caddr_t) (&local_ddi),
                     (int) sizeof(struct ide_adap_ddi),
                     UIO_WRITE, (struct uio *) uiop);
        if (rc != 0)
        {
          ret_code = errno;
          break;
        }

        /* copy was OK, so */
        /* get index of adapter from DDI data resource name */
        /* assume 'ide0' is adapter 0, and 'ide1' is adapter 1 */
        switch(local_ddi.adap.resource_name[3])
        {
        case '0':
          ap = &adp_str[0];
          break;
        case '1':
          ap = &adp_str[1];
          break;
        default:
          ret_code = ENXIO;
        }

        if (ret_code != 0)
          break;

        if (ap->inited)
        {  /* pointer already initialized */
          ret_code = EALREADY;
          break;
        }

        /* new uninitialized pointer */
        /* clear the structure */
        bzero((char *)ap, sizeof(struct adapter_def));

        /* init adapter to unlocked */
        ap->lock = LOCK_AVAIL;

        /* set up interrupt structures to point to this adapter */
        ap->intr_struct.ap = ap;
        
        /* if this is the first config call to this driver */
        if (lide_cfg_cnt++ == 0)
        {
          /* set up entry points to the driver    */
          /* for the device switch table          */
          lide_dsw.d_open = (int (*) ()) lide_open;
          lide_dsw.d_close = (int (*) ()) lide_close;
          lide_dsw.d_read = nodev;
          lide_dsw.d_write = nodev;
          lide_dsw.d_ioctl = (int (*) ()) lide_ioctl;
          lide_dsw.d_strategy = (int (*) ()) lide_strategy;
          lide_dsw.d_ttys = 0;
          lide_dsw.d_select = nodev;
          lide_dsw.d_config = (int (*) ()) lide_config;
          lide_dsw.d_print = nodev;
          lide_dsw.d_dump = (int (*) ()) lide_dump;
          lide_dsw.d_mpx = nodev;
          lide_dsw.d_revoke = nodev;
          lide_dsw.d_dsdptr = 0;
          lide_dsw.d_selptr = 0;
          lide_dsw.d_opts = 0;

          rc = devswadd(devno, &lide_dsw);
          if (rc != 0)
          {  /* failed to add to device switch table */
            ret_code = EIO;
            break;
          }
        }

        /* copy local ddi to global, static adapter structure */
        bcopy(&local_ddi, &ap->ddi, sizeof(struct ide_adap_ddi));

        /* These parameters aren't set properly in ODM, so override them */
        ap->ddi.adap.bus_type = IO_ISA;
        if (ap->ddi.adap.int_prior < 1) /* Make sure int PRIORITY isn't INTMAX*/
	  ap->ddi.adap.int_prior = 1;

        ap->max_request = MAXREQUEST;
        ap->bus_num     = ap->ddi.adap.bus_id;
        ap->base_addr0  = ap->ddi.adap.base_addr;
        ap->active_device = NO_DEV_ACTIVE;

        /* init IOM area */
        ap->iom.key     = IO_MEM_MAP;
        ap->iom.flags   = 0;
        ap->iom.size    = SEGSIZE;
        ap->iom.busaddr = 0;
        ap->iom.bid     = BID_VAL(IO_ISA, ISA_IOMEM, ap->bus_num);

#ifdef LIDE_TRACE
        ap->current_trace_line = 1;
        ap->trace_ptr = (struct lide_trace_struct *)
          xmalloc(sizeof(struct lide_trace_struct), 4, pinned_heap);
#endif

	/* Initialize disk activity light support flag */
	if (write_operator_panel_light(1) != ENODEV) {
	  ap->light_supported = TRUE;
	  write_operator_panel_light(0);
	}
	else
	  ap->light_supported = FALSE;

	/* Initialize the power management handler */
	ap->pmh.activity = PM_NO_ACTIVITY;
	ap->pmh.mode = PM_DEVICE_FULL_ON;
	ap->pmh.device_idle_time - ap->ddi.pm_dev_itime;
	ap->pmh.device_standby_time = ap->ddi.pm_dev_stime;
	ap->pmh.attribute = 0;
	ap->pmh.devno = devno;
	ap->pmh.handler = lide_pm_handler;
	ap->pmh.private = (caddr_t)ap;
	ap->pmh.device_logical_name = ap->ddi.adap.resource_name;
	ap->pmh.pm_version = PM_PHASE2_SUPPORT;

	/* Register Power Management Handler */
	rc = pm_register_handle((struct pm_handle *)&(ap->pmh), PM_REGISTER);
        DEBUG_1 ("lide_config register pm_handler returns 0x%x\n", rc);

	ap->device_cnt_done = FALSE;
        ap->devno  = devno;
        ap->opened = FALSE;
        ap->inited = TRUE;
        DEBUG_2 ("lide_config done for adapter %s, devno 0x%x\n",
                 local_ddi.adap.resource_name, devno);
        break;


      case CFG_TERM:  /* Handle request to terminate adapter */
        for (i=0; i<MAX_ADAPTERS; i++)
        {
          if (adp_str[i].devno == devno)
          {
            ap = &adp_str[i];
            break;
          }
        }
        if (i == MAX_ADAPTERS)
        {
          ret_code = 0;
          break;
        }

        if (!ap->inited)
        {  /* device already deleted */
          ret_code = 0;
          break;
        }
        if (ap->opened)
        {  /* adapter is still open, so error */
          ret_code = EBUSY;
          break;
        }

	/* Free any allocated dev_info structures */
	for (i=0; i<MAX_DEVICES; i++)
	{
	  if (ap->device_queue_hash[i] != NULL)
            (void) xmfree((void *) ap->device_queue_hash[i], pinned_heap);
	}

	/* Unregister the Power Management handler */
	(void) pm_register_handle((struct pm_handle *)&(ap->pmh),
				  PM_UNREGISTER);

	if (--lide_cfg_cnt == 0)
          (void) devswdel(devno);
#ifdef LIDE_TRACE
        (void) xmfree(ap->trace_ptr, pinned_heap);
#endif

        ap->inited = FALSE;
        break;


      case CFG_QVPD:  /* Handle query for adapter Vital Product Data */
      default:
        ret_code = EINVAL;
      }
    }
    else
    {    /* Lock Failed */
      ret_code = EIO; /* error--kernel service call failed */
      return (ret_code);
    }

    unlockl(&(lide_lock));
    DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_CONFIG, ret_code, devno);
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_open                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Open Routine                             */
/*                                                                      */
/*      This routine opens the IDE Adapter chip and makes it ready.     */
/*      It allocates adapter specific structures and initializes        */
/*      appropriate fields in them.  The adapter is marked as           */
/*      opened.                                                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      devflag - unused                                                */
/*      chan    - unused                                                */
/*      ext     - extended data; this is 0 for normal use, or           */
/*                a value of SC_DIAGNOSTIC selects diagnostic mode      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      EIO     - kernel service failed or invalid operation            */
/*      EPERM   - authority error                                       */
/*      ENOMEM  - not enough memory available                           */
/*      ENODEV  - device is not available                               */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*      lide_fail_open  lide_adp_str_init                               */
/*      lide_turn_chip_on                                               */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      pincode         privcheck                                       */
/*      i_init          xmalloc                                         */
/*      dmp_add         w_init                                          */
/*      i_disable       i_enable                                        */
/*      bzero                                                           */
/*      DIO_INIT        D_MAP_INIT      DIO_FREE                        */
/*                                                                      */
/************************************************************************/
int
lide_open(
	 dev_t devno,
	 ulong devflag,
	 int chan,
	 int ext)
{
    int     rc, ret_code;
    int     i;
    int     undo_level;
    int     old_level;
    struct adapter_def *ap;

    ret_code = 0;
    DDHKWD1(HKWD_DD_IDEDD, DD_ENTRY_OPEN, ret_code, devno);
    undo_level = 0;

    rc = lockl(&(lide_lock), LOCK_SHORT);     /* serialize this */
    if (rc == LOCK_SUCC)
    {    /* Lock succeded */
      /* find out which adapter we want based on devno */
      for (i=0; i<MAX_ADAPTERS; i++)
      {
        if ((adp_str[i].devno == devno) &&
            (adp_str[i].inited))
        {
          ap = &adp_str[i];
          break;
        }
      }
      if (i == MAX_ADAPTERS)
      {
        ret_code = ENODEV;
        lide_fail_open(ap, undo_level, ret_code, devno);
        return (ret_code);
      }

      if ((privcheck(DEV_CONFIG) != 0) && (!(devflag & DKERNEL)))
      {  /* must be normal open */
        ret_code = EPERM;
        lide_fail_open(ap, undo_level, ret_code, devno);
        return (ret_code);
      }
      if (!ap->opened)
      {
        ap->errlog_enable = FALSE;

        ret_code = pincode(lide_intr);
        if (ret_code != 0)
        {   /* pin failed */
            lide_fail_open(ap, undo_level, ret_code, devno);
            return (ret_code);
        }
        undo_level++;

        /* init reset watchdog timer struct */
        ap->reset_watchdog.dog.next = NULL;
        ap->reset_watchdog.dog.prev = NULL;
        ap->reset_watchdog.dog.func = lide_command_watchdog;
        ap->reset_watchdog.dog.count = 0;
        ap->reset_watchdog.dog.restart = RESETWAIT;
        ap->reset_watchdog.ap = ap;
        ap->reset_watchdog.timer_id = LIDE_RESET_TMR;

        /* init restart watchdog timer struct */
        ap->restart_watchdog.dog.next = NULL;
        ap->restart_watchdog.dog.prev = NULL;
        ap->restart_watchdog.dog.func = lide_command_watchdog;
        ap->restart_watchdog.dog.count = 0;
        ap->restart_watchdog.dog.restart = 0;
        ap->restart_watchdog.ap = ap;
        ap->restart_watchdog.timer_id = LIDE_RESTART_TMR;

        /* initialize the system timers */
        w_init(&ap->reset_watchdog.dog);
        w_init(&ap->restart_watchdog.dog);
        undo_level++;

        /* initialize the adapter structure */
        lide_adp_str_init(ap); 
        rc = i_init(&(ap->intr_struct.its));
        if (rc != 0)
        {       /* i_init of ide interrupt handler */
            ret_code = EIO;
            DEBUG_1 ("lide_open i_init() failed, rc = 0x%x\n",rc)
            lide_fail_open(ap, undo_level, ret_code, devno);
            return (ret_code);
        }
        undo_level++;

#ifdef EPOW_SUPPORTED
        INIT_EPOW(&epow_struct, (int (*) ()) lide_epow,
                  ap->ddi.adap.bus_id);
        rc = i_init(&epow_struct);
        if (rc != 0)
        {       /* i_init of epow structure failed */
            ret_code = EIO;
            lide_fail_open(ap, undo_level, ret_code, devno);
            return (ret_code);
        }
#endif
        undo_level++;

        if (lide_open_cnt == 0)
	{
          /* allocate and set up the component dump table entry */
          lide_cdt = (struct lide_cdt_table *) xmalloc(
                              (uint) sizeof(struct lide_cdt_table),
                                           (uint) 2, pinned_heap);
          if (lide_cdt == NULL)
          {       /* error in dump table memory allocation */
              ret_code = ENOMEM;
              lide_fail_open(ap, undo_level, ret_code, devno);
              return (ret_code);
          }
	}
        undo_level++;

        if (lide_open_cnt == 0)
	{
          /* initialize the storage for the dump table */
          bzero((char *) (lide_cdt), sizeof(struct lide_cdt_table));
          rc = dmp_add(lide_cdt_func);
          if (rc != 0)
          {
              ret_code = ENOMEM;
              lide_fail_open(ap, undo_level, ret_code, devno);
              return (ret_code);
          }
	}
        undo_level++;

        /* initialize the table to unused */
        ap->errlog_enable = TRUE;

        /* allocate the transfer area */
        ap->xfer_area = (char *)xmalloc( (uint)MAXSECTOR,
                                             (uint) PGSHIFT, pinned_heap);
        if (ap->xfer_area == NULL)
        {       /* error in dump table memory allocation */
            ret_code = ENOMEM;
            lide_fail_open(ap, undo_level, ret_code, devno);
            return (ret_code);
        }
        undo_level++;

        DIO_INIT(&ap->dio, 32);     /* space for user buffer map */
        if (ap->dio.total_iovecs == 0)
        {       /* error in iovec table memory allocation */
            ret_code = ENOMEM;
            lide_fail_open(ap, undo_level, ret_code, devno);
            return (ret_code);
        }
        DEBUG_2 ("dma_lvl=%x, busid=%x\n",
                         ap->ddi.adap.dma_lvl, ap->ddi.adap.bus_id)
        ap->handle = D_MAP_INIT(ap->ddi.adap.bus_id,DMA_SLAVE,0,
                                ap->ddi.adap.dma_lvl);
        if ((int)ap->handle == DMA_FAIL)
        {       /* failed to init the dma channel */
            DIO_FREE(&ap->dio);
            ret_code = EIO;
            lide_fail_open(ap, undo_level, ret_code, devno);
            return (ret_code);
        }
        undo_level++;

        /* find out how many devices are attached to the adapter  */
	/* so that we can allocate 'dev_info' structures for each.*/
	/* Only do this on the very first open.                   */
	if (!ap->device_cnt_done) {
	  lide_count_devices(ap);

          for (i = 0; i < ap->device_cnt; i++) {
	    ap->device_queue_hash[i] = xmalloc(
                            (uint) sizeof(struct dev_info), 4, pinned_heap);
	    if (ap->device_queue_hash[i] == NULL)
	    {   /* malloc failed */
		ret_code = ENOMEM;
		break;
	    }
	    bzero(ap->device_queue_hash[i], (int) sizeof(struct dev_info));
          }
	  if (ret_code) {
	    /* then clean up any successfull allocations */
	    for (i = 0; i < ap->device_cnt; i++) {
	      if (ap->device_queue_hash[i] != NULL)
                (void) xmfree((void *) ap->device_queue_hash[i], pinned_heap);
	    }
	    /* and fail the open */
	    lide_fail_open(ap, undo_level, ret_code, devno);
	    return (ret_code);
	  }
	}

	/* handle possible PM modes */
        old_level = i_disable(ap->ddi.adap.int_prior);
	switch(ap->pmh.mode) {
	  case PM_DEVICE_FULL_ON:
	  case PM_DEVICE_ENABLE:
            /* just enable IDE interrupts */
            LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x08);
	    break;
	  case PM_DEVICE_IDLE:
	    lide_turn_chip_on(ap, TRUE);
            LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x08);
	    break;
	  default:
	    ret_code = EIO;
	}
        i_enable(old_level);
        if (ret_code)
	{
            lide_fail_open(ap, undo_level, ret_code, devno);
            return (ret_code);
        }

	
        ap->opened = TRUE;
        lide_open_cnt++;

	/* The adapter is now considered to be "OPEN".            */
	/* Only do this on the very first open.                   */
	if (!ap->device_cnt_done) {
	  lide_config_devices(ap, devno);
	  ap->device_cnt_done = TRUE;
	}

      }
    }
    else
    {    /* Lock Failed */
      ret_code = EIO; /* error--kernel service call failed */
      DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_OPEN, ret_code, devno);
      return (ret_code);
    }

    unlockl(&(lide_lock));
    DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_OPEN, ret_code, devno);
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_config_devices                                             */
/*                                                                        */
/* FUNCTION:  Count the number of devices attached to the IDE Adapter     */
/*            and set the proper DMA and PIO modes for those devices      */
/*            based on what the system is known to support.               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine will only allow DMA modes Compatibility, A, B & F.*/
/*         and only allow PIO modes 0, 1, 2, or 3.  The system will also  */
/*         only allow a single PIO mode.  This mode is assumed to be the  */
/*         lessor of the maximum PIO modes supported by each individually */
/*         attached device.  For example, if the Master device supports   */
/*         PIO modes 0 & 1, and the slave supports PIO modes 0, 1 & 2,    */
/*         then the system mode is assumed to be Mode 1, and both devices */
/*         are set to match this.                                         */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA interface unique data structures.               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap          -  pointer to an adapter_def structure                */
/*      devno       -  device major/minor number                          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_start_dev  lide_stop_dev   lide_identify_device              */
/*      lide_xlate_mode                                                   */
/**************************************************************************/
void
lide_config_devices(
                   struct adapter_def *ap,
		   dev_t devno)
{
  int dev_index;
  int rc;
  int i, sys_pio, sys_iordy;
  struct ataide_buf		*bp;
  struct dev_info		*dev_ptr;
  struct identify_device	ident;
  struct ide_identify		ioctl_ident;
  uchar				tchar;
  uchar				*ucp;

  /* At this point, we know the adapter is powered up, and hopefully */
  /* any attached devices are also powered on.  Before we begin      */
  /* serious communication with the devices, we need to know what    */
  /* modes (DMA & PIO) each device supports.  In particular, for     */
  /* Delmar, Southwind and Carolina machines where the IDE adapter   */
  /* is on the mother board, and only one PIO mode may be set for all*/
  /* connected devices, it is important to query all the devices to  */
  /* determine what global PIO mode will work for all of them.  At   */
  /* worst, all Devices are assumed to support PIO mode 0 without    */
  /* flow control (no IOREADY support).                              */

  /* set the current MAX PIO mode to the fastest allowed */
  /* which is PIO mode 3 with IOFLOW control ON          */
  sys_iordy = 1;   /* indicate that IOREADY Flow Control is on */
  sys_pio   = 3;   /* indicate that mode 3 is supported        */

  for (dev_index = 0; dev_index < ap->device_cnt; dev_index++) {
    /* start the device */
    rc = lide_start_dev(ap, devno, dev_index);
    DEBUG_2("lide_config_devices, lide_start_dev(0x%x) returned 0x%x\n",
            dev_index,rc);
    if (rc) {
      break;
    }

    dev_ptr = ap->device_queue_hash[dev_index];

    /* init transfer mode                               */

    dev_ptr->protocol_type = IDE_ID_ATA;
    dev_ptr->chs_only = TRUE;
    dev_ptr->dma_supported = 0;
    dev_ptr->pio_cur_mode = IDE_PIO_MODE_0;
    dev_ptr->pio_cycle_time = IDE_PIO_MODE_0_NS;
    dev_ptr->iordy_enabled = 0;

    /* do identify_device to get device info */
    ioctl_ident.ide_device   = dev_index;
    ioctl_ident.identify_len = sizeof(ident);
    ioctl_ident.identify_ptr = (uchar *)&ident;
    ioctl_ident.flags        = ATA_LBA_MODE;

    if (rc = lide_identify_device(ap, devno, (int)&ioctl_ident, DKERNEL)) {
       DEBUG_1("lide_config_device identify returned 0x%x\n",rc)
       break;
    }

    /* swap each pair of bytes in the returned data structure */

    ucp    = (uchar *)&ident;
    for (i = 0; i < sizeof(ident); i+=2)
    {
      tchar = ucp[1];
      ucp[1] = ucp[0];
      ucp[0] = tchar;
      ucp +=2;
    }

    DEBUG_1("lide_config_devices: ident.gen_config = 0x%x\n",ident.gen_config);


    /* save the Protocol type */

    if ( ((ident.gen_config & ID_ATAPI) >> ID_PROTOCOL_SHIFT) == 0x0002 ) {
	dev_ptr->protocol_type = IDE_ID_ATAPI;
	dev_ptr->chs_only = FALSE;
        dev_ptr->atapi_cmd_drq = (ident.gen_config & ID_CMD_DRQ_MASK)
                                 >> ID_CMD_DRQ_SHIFT;
        dev_ptr->atapi_cmd_size = (ident.gen_config & ID_CMD_PCKT_SIZE_MASK);
    } else {
	dev_ptr->protocol_type = IDE_ID_ATA;

	/* Determine if the media is removable */

	if (!(ident.gen_config & ID_REMOVABLE)) {

            /* Determine whether the device supports LBA. */

            if (ident.capabilities & ID_CAP_LBA) {

                    /* LBA is supported: */

                    dev_ptr->chs_only = FALSE;
            } else {

                    /* LBA is NOT supported: */

                    dev_ptr->chs_only = TRUE;
            }
            dev_ptr->num_heads = ident.num_of_heads;
            dev_ptr->sectors_per_track = ident.sectors_per_track;
	}
    }

    dev_ptr->device_type = (ident.gen_config & ID_DEVICE_TYPE_MASK)
                           >> ID_DEVICE_TYPE_SHIFT;

    /* PIO */

    /* Find the maximum PIO mode which the device can support */
    dev_ptr->pio_max_mode = (ident.pio_data_xfer_cycle_timing &
                             ID_PIO_XFER_CYCLE) >> 8;
    /* max PIO mode supported by the device */
    if ((ident.field_validity & ID_VALID_64_70) &&
        (ident.enhanced_pio_mode & ID_PIO_MODE_3)) {
         dev_ptr->pio_max_mode = IDE_PIO_MODE_3;
    }
    dev_ptr->iordy_supported  = ident.capabilities & ID_CAP_IORDY ? 1 : 0;

    /* If the current devices' best PIO mode is slower than */
    /* the current MAX PIO mode, then set the current MAX   */
    /* PIO mode to the new slower one.                      */
    if (!dev_ptr->iordy_supported)
	sys_iordy = 0;
    if (dev_ptr->pio_max_mode < sys_pio)
	sys_pio = dev_ptr->pio_max_mode;


    /* DMA */

    dev_ptr->dma_supported  = ident.capabilities & ID_CAP_DMA ? 1 : 0;
    if (dev_ptr->dma_supported)
    {
      /* attempt to get dma mode from multi word dma data */
      DEBUG_2(" original DMA mode multi 0x%x, single 0x%x\n",
	      ident.multi_word_dma_xfer, ident.single_word_dma_xfer)
      /* max DMA mode supported by the device */
      if ((dev_ptr->dma_max_mode = lide_xlate_mode(ident.multi_word_dma_xfer))
           >= 0)
      {
        dev_ptr->dma_max_mode |= IDE_DMA_MULTIW_MODE;
        dev_ptr->dma_cur_mode =
           lide_xlate_mode((ident.multi_word_dma_xfer & 0xff00)>>8) |
			    IDE_DMA_MULTIW_MODE;
      }
      else if ((dev_ptr->dma_max_mode = 
               lide_xlate_mode(ident.single_word_dma_xfer)) >= 0)
      {
        dev_ptr->dma_max_mode |= IDE_DMA_SINGLW_MODE;
        dev_ptr->dma_cur_mode =
           lide_xlate_mode((ident.single_word_dma_xfer & 0xff00)>>8) |
			    IDE_DMA_SINGLW_MODE;
      }
      else
      {
          dev_ptr->dma_max_mode = -1;
          dev_ptr->dma_cur_mode = -1;
          dev_ptr->dma_supported = 0;
          DEBUG_2("  ERROR - unable to find valid DMA mode from 0x%x, 0x%x\n",
                  ident.multi_word_dma_xfer, ident.single_word_dma_xfer);
      }
      DEBUG_2("lide_config_dev: current dma = 0x%x, max. dma =0x%x\n",
                dev_ptr->dma_cur_mode, dev_ptr->dma_max_mode);
    }

    /* stop the device */
    rc = lide_stop_dev(ap, dev_index);
    
  }

  /* Now go back through the devices and set them each to:  */
  /*    the max recommended # heads and sectors per track   */
  /*    the current MAX PIO mode                            */
  /*    their own MAX DMA mode                              */
  for (dev_index = 0; dev_index < ap->device_cnt; dev_index++) {

    /* start the device */
    rc = lide_start_dev(ap, devno, dev_index);
    DEBUG_2("lide_config_devices, lide_start_dev(0x%x) returned 0x%x\n",
            dev_index,rc);
    if (rc) {
      break;
    }

    dev_ptr = ap->device_queue_hash[dev_index];

    /* no matter what, use the global settings for the PIO mode */
    dev_ptr->pio_cur_mode = sys_pio;
    dev_ptr->iordy_enabled  = sys_iordy;

    /* set pio_cycle_time based on current pio mode */
    switch (dev_ptr->pio_cur_mode) {
      case IDE_PIO_MODE_3:
        dev_ptr->pio_cycle_time = IDE_PIO_MODE_3_NS;
        break;
      case IDE_PIO_MODE_2:
        dev_ptr->pio_cycle_time = IDE_PIO_MODE_2_NS;
        break;
      case IDE_PIO_MODE_1:
        dev_ptr->pio_cycle_time = IDE_PIO_MODE_1_NS;
        break;
      default:
        dev_ptr->pio_cycle_time = IDE_PIO_MODE_0_NS;
    }


    /* set the device parameters according to the data in the dev_info struct*/
    (void)lide_init_device(ap, dev_index);

    /* retrieve and store current device parameters */

    /* do identify_device to get device info */
    ioctl_ident.ide_device   = dev_index;
    ioctl_ident.identify_len = sizeof(ident);
    ioctl_ident.identify_ptr = (uchar *)&ident;
    ioctl_ident.flags        = ATA_LBA_MODE;

    if (rc = lide_identify_device(ap, devno, (int)&ioctl_ident, DKERNEL)) {
       break;
    }

    /* swap each pair of bytes in the returned data structure */

    ucp    = (uchar *)&ident;
    for (i = 0; i < sizeof(ident); i+=2)
    {
      tchar = ucp[1];
      ucp[1] = ucp[0];
      ucp[0] = tchar;
      ucp +=2;
    }

    /* save the current DMA settings (everything else should be the same) */
    if (dev_ptr->dma_supported) {
      /* attempt to get dma mode from multi word dma data */
      /* max DMA mode supported by the device */
      dev_ptr->dma_cur_mode =
         lide_xlate_mode((ident.multi_word_dma_xfer & 0xff00)>>8) |
	    IDE_DMA_MULTIW_MODE;
      /* if multiword DMA mode wasn't set */
      if (dev_ptr->dma_cur_mode < 0)
      {
        /* attempt to get dma mode form single word dma data */ 
        dev_ptr->dma_cur_mode =
           lide_xlate_mode((ident.single_word_dma_xfer & 0xff00)>>8) |
			    IDE_DMA_SINGLW_MODE;
      }
      /* if no DMA mode was set, but DMA is supported, then assume it's*/
      /* actually in it's highest supported mode.                      */
      if (dev_ptr->dma_cur_mode < 0)
      {
        dev_ptr->dma_cur_mode = dev_ptr->dma_max_mode;
        DEBUG_2("  ERROR - unable to find valid DMA mode from 0x%x, 0x%x\n",
                ident.multi_word_dma_xfer, ident.single_word_dma_xfer);
      }
      DEBUG_2("lide_config_devices: current dma = 0x%x, max. dma =0x%x\n",
                dev_ptr->dma_cur_mode, dev_ptr->dma_max_mode);

    }
    /* stop the device */
    rc = lide_stop_dev(ap, dev_index);
    
  }
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_adp_str_init                                               */
/*                                                                        */
/* FUNCTION:  Initializes adapter structure variables.                    */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called to initialize the adapter structures    */
/*         variables, arrays, etc.                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA interface unique data structures.               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap          -  pointer to an adapter_def structure                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
void
lide_adp_str_init(
                  struct adapter_def *ap)
{
  /* init the adapter interrup handler struct */
  ap->intr_struct.its.next = (struct intr *) NULL;
  ap->intr_struct.its.handler = lide_intr;
  ap->intr_struct.its.bus_type = BUS_BID;
  ap->intr_struct.its.flags = INTR_LEVEL;
  ap->intr_struct.its.level = ap->ddi.adap.int_lvl;
  ap->intr_struct.its.priority = ap->ddi.adap.int_prior;
  ap->intr_struct.its.bid = BID_VAL(IO_ISA, ISA_IOMEM, ap->bus_num);
  DEBUG_5("  bus_type=0x%x, flags=0x%x, lvl=0x%x, prio=0x%x, bid=0x%x\n",
          ap->intr_struct.its.bus_type,
          ap->intr_struct.its.flags,
          ap->intr_struct.its.level,
          ap->intr_struct.its.priority,
          ap->intr_struct.its.bid);

  /* initialize other basic global variables to the adp_str */
  ap->base_addr0 = ap->ddi.adap.base_addr;
  ap->dump_inited = FALSE;
  ap->dump_started = FALSE;
  ap->dump_pri = 0;
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_close                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Close Routine                            */
/*                                                                      */
/*      This routine closes the IDE Adapter instance and releases any   */
/*      resources (as well as unpins the code) that were setup at open  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      chan    - 0; unused                                             */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      0       - successful                                            */
/*      EIO     - kernel service failed or invalid operation            */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*      lide_stop_dev   LIDE_READ_ATA   lide_clear_open                 */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*                                                                      */
/************************************************************************/
int
lide_close(
	  dev_t devno,
	  int chan)
{
    int     ret_code, rc;
    int     i;
    struct adapter_def *ap;

    ret_code = 0;	/* default to good completion */
    DDHKWD1(HKWD_DD_IDEDD, DD_ENTRY_CLOSE, ret_code, devno);

    rc = lockl(&(lide_lock), LOCK_SHORT);        /* serialize this */
    if (rc != LOCK_SUCC)
    {   /* lock kernel call failed */
        ret_code = EIO;
        DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_CLOSE, ret_code, devno);
        return (ret_code);
    }

    /* find out which adapter we want based on devno */
    for (i=0; i<MAX_ADAPTERS; i++)
    {
      if (adp_str[i].devno == devno)
      {
        ap = &adp_str[i];
        break;
      }
    }
    if ((i == MAX_ADAPTERS) ||   /* if adapter not found    */
	(!ap->opened))           /* or adapter never opened */
    {
      ret_code = EIO;
      DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_CLOSE, ret_code, devno);
      unlockl(&(lide_lock));
      return (ret_code);
    }

    /* Loop through the device table and stop devices */
    /* that are still started.                        */
    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (ap->device_queue_hash[i]->opened == TRUE)
        {       /* If there's a device still started */
           lide_stop_dev(ap,i);    /* stops and frees resources */
        }
    }


    /* Disable dma and IDE interrupts from the chip */
    /* (if PM handler hasn't already done it...)    */
    if ((ap->power_state == 0) &&
	(ap->pmh.mode != PM_DEVICE_IDLE)) {
	LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0A );
    }

    ap->opened = FALSE;

    lide_open_cnt--;

    lide_clear_open(ap, 8);
 
    unlockl(&(lide_lock));
    DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_CLOSE, ret_code, devno);
    return (ret_code);

}  /* end lide_close */

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_clear_open                                                 */
/*                                                                        */
/* FUNCTION:  clears/frees the adapter resources on close or failed open  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to free storage, unpin code and do      */
/*         general cleanup on a close, or failed open command.            */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA/IDE Adapter unique data structure               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap          -  pointer to an adapter_def structure                */
/*      undo_level  -  value of amount of cleanup required                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      None                                                              */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      None                                                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      unpincode       xmfree        dmp_del                             */
/*      i_clear         w_clear                                           */
/*                                                                        */
/**************************************************************************/
void
lide_clear_open(
                struct adapter_def *ap,
                int undo_level)
{


  switch (undo_level)
  {
  case 8:
    /* clear dma handles */
    D_MAP_CLEAR(ap->handle);
    DIO_FREE(&ap->dio);
  case 7:
    if (lide_open_cnt == 0)
    (void) xmfree((void *) ap->xfer_area, pinned_heap);
  case 6:
    /* clear the cdt table */
    if (lide_open_cnt == 0)
      (void) dmp_del(lide_cdt_func);
  case 5:
    /* free the cdt table */
    if (lide_open_cnt == 0)
      (void) xmfree((void *) (lide_cdt), pinned_heap);
  case 4:
    /* clear the EPOW structure */
#ifdef EPOW_SUPPORTED
    i_clear(&epow_struct);
#endif
  case 3:
    /* clear out ATA/IDE adapter interrupt handler */
    i_clear(&(ap->intr_struct.its));
  case 2:
    /* clear all system timers */
    w_clear(&ap->reset_watchdog.dog);
    w_clear(&ap->restart_watchdog.dog);
  case 1:
    /* unpin the device driver */
    (void) unpincode(lide_intr);
  default:
    break;
  }
  return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_fail_open                                                  */
/*                                                                        */
/* FUNCTION:  Cleans up during failed open processing.                    */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to free storage, unpin code and do      */
/*         general cleanup on a failed open command.                      */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA/IDE Adapter unique data structure               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap          -  pointer to an adapter_def structure                */
/*      undo_level  -  value of amount of cleanup required                */
/*      ret_code    -  return value to be logged in trace                 */
/*      devno       -  device major/minor number to be logged in trace    */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      None                                                              */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_clear_open                                                   */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      unlockl                                                           */
/*                                                                        */
/**************************************************************************/
void
lide_fail_open(
                struct adapter_def *ap,
                int undo_level,
                int ret_code,
                dev_t devno)
{
  lide_clear_open(ap, undo_level);
  DEBUG_1("Leaving lide_open through lide_fail_open (with errors) undo=0x%x\n",
           undo_level)

  DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_OPEN, ret_code, devno);

  unlockl(&(lide_lock));
  return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_ioctl                                                      */
/*                                                                        */
/* FUNCTION:  IDE Chip Device Driver Ioctl Routine                        */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine will accept commands to perform specific function */
/*         and diagnostic operations on a Coral chip.  Supported commands */
/*         are:                                                           */
/*                                                                        */
/*         IOCINFO     -  Returns information about the adapter device.   */
/*         IDEIOSTART  -  Open an IDE Port                                */
/*         IDEIOSTOP   -  Close an IDE Port                               */
/*         IDEIOINQU   -  Issues an ATAPI Inquiry command to a device.    */
/*         IDEIOSTUNIT -  Issues ATAPI start unit command to a device.    */
/*         IDEIOTUR    -  Issues ATAPI test unit ready command to device. */
/*         IDEIOREAD   -  Issues ATAPI 6-byte read command to device.     */
/*         IDEIOHALT   -  No operation.  Returns EINVAL.                  */
/*         IDEIORESET  -  Issues a Soft Reset to an ATAPI device.         */
/*         IDEIODIAG   -  No operation.  Returns EINVAL.                  */
/*         IDEIOIDENT  -  Issues an Identify Device command to a device.  */
/*         IDEIOTRAM   -  No operation.  Returns EINVAL.                  */
/*         IDEIODNLD   -  No operation.  Returns EINVAL.                  */
/*         IDEIOSTARTTGT - No operation.  Returns EINVAL.                 */
/*         IDEIOSTOPTGT- No operation.  Returns EINVAL.                   */
/*         IDEIOEVENT  - No operation.  Returns EINVAL.                   */
/*         IDEIOGTHW   - No support for gathered writes.  Returns EINVAL. */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    cmd    - code used to determine which operation to perform.         */
/*    arg    - address of a structure which contains values used in the   */
/*             'arg' operation.                                           */
/*    devflag - not used.                                                 */
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - not used.                                                  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Child Device not opened.                                 */
/*             - unsupported IOCTL command or mode                        */
/*      ENOMEM - Could not allocate an ataide_buf for this command.       */
/*      ENODEV - Adapter could not be selected.                           */
/*      ETIMEDOUT - The requested command timed out.                      */
/*      EIO    - scatter/gather not supported                             */
/*             - queue stopping or request exceeds MAX_REQUEST            */
/*             - lockl failure                                            */
/*      EFAULT - Bad copyin or copyout.                                   */
/*      EBUSY  - Dump in progress                                         */
/*      ENXIO  - queue halted(needs RESUME) or queue full (max 1)         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      LIDE_READ_ATA   LIDE_WRITE_ATA  lide_start_dev  lide_stop_dev     */
/*      lide_issue_reset                lide_start_unit lide_inquiry      */
/*      lide_test_unit_rdy              lide_read_ata                     */
/*      lide_identify_device                                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl, unlockl, bcopy, copyout, copyin    */
/*                                                                        */
/**************************************************************************/
int
lide_ioctl(
	  dev_t devno,	/* major and minor device numbers */
	  int cmd,	/* operation to perform */
	  int arg,	/* pointer to the user structure */
	  ulong devflag,	/* not used */
	  int chan,	/* not used */
	  int ext)	/* not used */

{
    int     rc, ret_code;
    int     i;
    struct  devinfo ideinfo;
    struct  adapter_def *ap;
    struct  regs reginfo;

/*  DEBUG_0 ("In lide_ioctl ")
    DEBUG_6("devno=%x,cmd=%x,arg=%x,devflag=%x,chan=%x,ext=%x\n",
	     devno, cmd, arg, devflag, chan, ext) */

    ret_code = 0;       /* default to no errors found  */

    DDHKWD5(HKWD_DD_IDEDD, DD_ENTRY_IOCTL, ret_code, devno,
	    cmd, devflag, chan, ext);

    /* find out which adapter we want based on devno */
    for (i=0; i<MAX_ADAPTERS; i++)
    {
      if ((adp_str[i].devno == devno) &&
	  (adp_str[i].opened))
      {
        ap = &adp_str[i];
        break;
      }
    }
    if (i == MAX_ADAPTERS)
    {
      ret_code = ENODEV;
      DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_IOCTL, ret_code, devno);
      DEBUG_0 ("Leaving lide_ioctl routine. No adapter for devno/not opened.\n")
      return (ret_code);
    }

    /* lock the adapter lock to serialize with open/close/config */
    rc = lockl(&(ap->lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC)
    {
	DEBUG_0 ("Leaving lide_ioctl routine. Lockl FAILED.\n")

	DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_IOCTL, EIO, devno);

	return (EIO);	/* error--kernel service call failed */
    }

    switch (cmd)
    {	/* cmd switch */
#ifdef DEVELOPMENT
    case 55:  /* reg read */

        rc = copyin((char *) arg, &reginfo,
                         sizeof(struct regs));
        if (rc != 0)
                ret_code = EFAULT;
        reginfo.data = LIDE_READ_ATA(ap, reginfo.isa_reg, reginfo.size);
        rc = copyout(&reginfo, (char *) arg,
                         sizeof(struct regs));
        break;
    case 56:  /* reg write */

        rc = copyin((char *) arg, &reginfo,
                         sizeof(struct regs));
        if (rc != 0)
                ret_code = EFAULT;
        LIDE_WRITE_ATA(ap, reginfo.isa_reg, reginfo.size, reginfo.data);
        break;
#endif
    case IOCINFO:	/* get device information */
	ideinfo.devtype = DD_BUS;
	ideinfo.flags = 0;
	ideinfo.devsubtype = DS_IDE;
	ideinfo.un.ide.max_transfer = ap->max_request;
	if (!(devflag & DKERNEL))
	{  /* for a user process */
	    rc = copyout(&ideinfo, (char *) arg,
			 sizeof(struct devinfo));
	    if (rc != 0)
		ret_code = EFAULT;
	}	/* for a user process */
	else
	{  /* for a kernel process */
	    bcopy(&ideinfo, (char *) arg, sizeof(struct devinfo));
	}  /* for a kernel process */
	break;
    case IDEIOSTART:	/* start a device */
	ret_code = lide_start_dev(ap, devno, arg);
        DEBUG_1("  in ioctl, return from lide_start_dev = 0x%x\n",ret_code);
	break;
    case IDEIOSTOP:	/* stop a device */
	ret_code = lide_stop_dev(ap, arg);
	break;
    case IDEIORESET:	/* issue a SCSI abort cmd */
	ret_code = lide_issue_reset(ap, arg);
	break;
    case IDEIOINQU:	/* issue a SCSI device inquiry cmd */
	ret_code = lide_inquiry(ap, devno, arg, devflag);
	break;
    case IDEIOSTUNIT:	/* issue a SCSI device start unit */
	ret_code = lide_start_unit(ap, devno, arg, devflag);
	break;
    case IDEIOTUR:	/* issue  SCSI device test unit ready */
	ret_code = lide_test_unit_rdy(ap, devno, arg, devflag);
	break;
    case IDEIOREAD:	/* issue a SCSI read cmd (6-byte) */
        ret_code = lide_read_ata(ap, devno, arg, devflag);
	break;
    case IDEIOIDENT:     /* issue an IDE identify drive command */
        ret_code = lide_identify_device(ap, devno, arg, devflag);
        break;
    default:	/* catch unknown ioctls and IDIOGTHW here */
	ret_code = EINVAL;
	break;
    }	/* cmd switch */

    unlockl(&(ap->lock));
    DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_IOCTL, ret_code, devno);
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:        lide_build_command                                        */
/*                                                                        */
/* FUNCTION:    Builds an internal command for ioctl routines.            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to initialize fields within the id_buf  */
/*         structure that is allocated via this routine.  This routine    */
/*         is called by ioctl routines that issue a command via the       */
/*         lid_strategy routine.                                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      ataide_buf  - input/output request struct used between the adapter*/
/*                    driver and the calling IDE device driver            */
/*                                                                        */
/* INPUTS:                                                                */
/*      none                                                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      returns a pointer to the ataide_buf, or NULL, if it could not     */
/*      be allocated.                                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      none                                                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      xmalloc         bzero                                             */
/*                                                                        */
/**************************************************************************/
struct ataide_buf *
lide_build_command()
{
    struct ataide_buf *bp;

    /* Allocate an ataide_buf area for this command  */
    bp = (struct ataide_buf *) xmalloc((uint) PAGESIZE,
				   (uint) PGSHIFT, pinned_heap);
    if (bp == NULL)
    {	/* xmalloc failed--return NULL pointer */
	DEBUG_0 (" In lide_build_command (xmalloc FAILED).\n")
	return (NULL);
    }

    /* Clear the ataide_buf structure to insure all */
    /* fields are initialized to zero.          */
    bzero(bp, sizeof(struct ataide_buf));

    /* Initialize other fields of the id_buf.   */
    bp->bufstruct.b_forw = NULL;
    bp->bufstruct.b_back = NULL;
    bp->bufstruct.av_forw = NULL;
    bp->bufstruct.av_back = NULL;
    bp->bufstruct.b_iodone = (void (*) ()) lide_iodone;
    bp->bufstruct.b_vp = NULL;
    bp->bufstruct.b_event = EVENT_NULL;
    bp->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
    bp->bufstruct.b_un.b_addr = (char *) bp + sizeof(struct ataide_buf);

    /* Additional ataide_buf initialization */
    bp->bp = NULL;	/* set for non-spanned command */
    bp->timeout_value = LONGWAIT;	/* set default timeout value */

    return (bp);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_inquiry                                                    */
/*                                                                        */
/* FUNCTION:  Issues a IDE inquiry command to a device.                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to issue an inquiry to a device.        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the IDEIOSTART failed.  Such a failure would indicate  */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ide chip unique data structure                      */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap      - pointer to an adapter_def structure                     */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Device not opened.                                       */
/*      ENOMEM - Could not allocate an ataide_buf for this command.       */
/*      ENODEV - Adapter could not be selected.                           */
/*      ETIMEDOUT - The inquiry command timed out.                        */
/*      EIO    - No data returned from inquiry command.                   */
/*             - scatter/gather not supported                             */
/*             - queue stopping or request exceeds MAX_REQUEST            */
/*      EFAULT - Bad copyin or copyout.                                   */
/*      EBUSY  - Dump in progress                                         */
/*      ENXIO  - queue halted(needs RESUME) or queue full (max 1)         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*       lide_build_command              lide_strategy   lide_logerr      */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, copyout, bcopy    */
/*                                                                        */
/**************************************************************************/
int
lide_inquiry(
            struct adapter_def *ap,
	    dev_t devno,
	    int arg,
	    ulong devflag)
{
    int     ret_code, dev_index, inquiry_length;
    struct ataide_buf *bp;
    struct ide_inquiry ide_inq;
    struct dev_info   *dev_ptr;

    ret_code = 0;

    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
        ret_code = copyin((char *) arg, &ide_inq,
                          sizeof(struct ide_inquiry));
        if (ret_code != 0)
        {
            DEBUG_0 ("lide_inquiry: copyin failed!\n")
            return (EFAULT);
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy((char *) arg, &ide_inq, sizeof(struct ide_inquiry));
    }

    /* Obtain device structure from hash on the device id */
    dev_index = ide_inq.ide_device & 0x01;

    if (((dev_ptr = ap->device_queue_hash[dev_index]) == NULL) ||
	(dev_ptr->opened == FALSE))
    {   /* device queue structure not allocated or device not started*/
        DEBUG_0 ("lide_inquiry: device pointer is NULL!\n")
        return (EINVAL);
    }
    bp = lide_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("lide_inquiry: can't build command!\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    bp->ata.flags = ide_inq.flags;
    bp->ata.command = ATA_ATAPI_PACKET_COMMAND;
    bp->ata.device = dev_index;
    bp->ata.startblk.chs.cyl_lo = ide_inq.inquiry_len;    /* alloc len */
    bp->ata.atapi.length = 12;
    bp->ata.atapi.packet.op_code = ATAPI_INQUIRY;
    if (ide_inq.get_extended)
      bp->ata.atapi.packet.bytes[0] |= 0x01; /*set EVPD bit on */
    bp->ata.atapi.packet.bytes[1] = ide_inq.code_page_num;  /* page num  */
    bp->ata.atapi.packet.bytes[3] = ide_inq.inquiry_len;    /* alloc len */

    bp->bufstruct.b_bcount = ide_inq.inquiry_len;
    bp->bufstruct.b_flags |= B_READ;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = LONGWAIT;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->q_flags = ATA_RESUME;

    /* Call our strategy routine to issue the inquiry command. */
    if (ret_code = lide_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp, pinned_heap); /* release buffer */
        DEBUG_0 ("lide_inquiry: strategy failure!\n")
        return (ret_code);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        ret_code = bp->bufstruct.b_error;
        if (ret_code == ETIMEDOUT)
        {
	  lide_logerr(ap, ERRID_ATAIDE_ERR4, COMMAND_TIMEOUT, 0, 0, dev_ptr,
                      TRUE);
	}
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) &&
        (bp->bufstruct.b_resid == bp->bufstruct.b_bcount))
    {
        ret_code = EIO;
    }
    /* give the caller the lesser of what he asked for, or */
    /* the actual transfer length                          */
    if (ret_code == 0)
    {
        inquiry_length = bp->bufstruct.b_bcount - bp->bufstruct.b_resid;
        if (inquiry_length > ide_inq.inquiry_len)
            inquiry_length = ide_inq.inquiry_len;
        ide_inq.inquiry_len = inquiry_length;
        ide_inq.ata_status = bp->ata.status;
        ide_inq.ata_error = bp->ata.errval;
        /* Copy out the inquiry data. If the buffer resides */
        /* user space, use copyout, else use bcopy.         */
        if (!(devflag & DKERNEL))
        {
            ret_code = copyout(bp->bufstruct.b_un.b_addr,
                               ide_inq.inquiry_ptr, inquiry_length);
            if (ret_code != 0)
            {
                ret_code = EFAULT;
            }
            ret_code = copyout((char *)&ide_inq, (char *) arg,
                                sizeof (struct ide_inquiry));
            if (ret_code != 0)
            {
                ret_code = EFAULT;
            }
        }
        else
        { /* both the arg and inquiry_ptr are in kernel space */
            bcopy(bp->bufstruct.b_un.b_addr, ide_inq.inquiry_ptr,
                  inquiry_length);
            bcopy(&ide_inq, (char *) arg, sizeof(struct ide_inquiry));
        }
    }
    (void) xmfree(bp, pinned_heap);     /* release buffer */

    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_start_unit                                                 */
/*                                                                        */
/* FUNCTION:  Issues a ATAPI start unit command to a device.              */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue a start unit command to a   */
/*         device.                                                        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the IDEIOSTART failed.  Such a failure would indicate  */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap      - pointer to an adapter_def structure                     */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Child Device not opened.                                 */
/*      ENOMEM - Could not allocate an ataide_buf for this command.       */
/*      ENODEV - Adapter could not be selected.                           */
/*      ETIMEDOUT - The start_unit command timed out.                     */
/*      EIO    - scatter/gather not supported                             */
/*             - queue stopping or request exceeds MAX_REQUEST            */
/*      EFAULT - Bad copyin or copyout.                                   */
/*      EBUSY  - Dump in progress                                         */
/*      ENXIO  - queue halted(needs RESUME) or queue full (max 1)         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_strategy   lide_logerr     lide_build_command                */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, bcopy             */
/*                                                                        */
/**************************************************************************/
int
lide_start_unit(
               struct adapter_def *ap,
	       dev_t devno,
	       int arg,
	       ulong devflag)
{
    int     ret_code, dev_index;
    struct ataide_buf *bp;
    struct ide_startunit ide_stun;

    ret_code = 0;

    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
        ret_code = copyin((char *) arg, &ide_stun,
                          sizeof(struct ide_startunit));
        if (ret_code != 0)
        {
            DEBUG_0 ("EFAULT: Leaving lide_start_unit routine.\n")
            return (EFAULT);
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy((char *) arg, &ide_stun, sizeof(struct ide_startunit));
    }

    /* Obtain device structure from hash on the device id */
    /* and lun id.                                      */
    dev_index = ide_stun.ide_device & 0x01;

    if ((ap->device_queue_hash[dev_index] == NULL) ||
	(ap->device_queue_hash[dev_index]->opened == FALSE))
    {   /* device queue structure not allocated or device not started*/
        DEBUG_0 ("Leaving lide_start_unit .EINVAL\n")
        return (EINVAL);
    }
    bp = lide_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("Leaving lide_start_unit ENOMEM.\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    bp->ata.flags = ATA_LBA_MODE;
    bp->ata.command = ATA_ATAPI_PACKET_COMMAND;
    bp->ata.device = dev_index;
    bp->ata.atapi.length = 12;
    bp->ata.atapi.packet.op_code = ATAPI_START_STOP_UNIT;

    bp->ata.atapi.packet.bytes[3] = ide_stun.start_flag ? 0x01 : 0;

    bp->bufstruct.b_bcount = 0;
    bp->bufstruct.b_dev = devno;

    bp->timeout_value = ide_stun.timeout_value; /* set timeout value */

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->q_flags = ATA_RESUME;

    /* Call our strategy routine to issue the start unit cmd. */
    if (ret_code = lide_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp, pinned_heap); /* release buffer */
        DEBUG_0 ("Leaving lide_start_unit: strategy failure\n")
        return (ret_code);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        ret_code = bp->bufstruct.b_error;
        if (ret_code == ETIMEDOUT)
        {
	  lide_logerr(ap, ERRID_ATAIDE_ERR4, COMMAND_TIMEOUT, 1, 0,
                      ap->device_queue_hash[dev_index], TRUE);
	}
    }

    (void) xmfree(bp, pinned_heap);     /* release buffer */

    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_test_unit_rdy                                              */
/*                                                                        */
/* FUNCTION:  Issues an ATAPI test unit ready command to a device.        */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue a test unit ready command   */
/*         to a device.                                                   */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the IDEIOSTART failed.  Such a failure would indicate  */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ata/ide adapter unique data structure               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap      - pointer to an adapter_def structure                     */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Child Device not opened.                                 */
/*      ENOMEM - Could not allocate an ataide_buf for this command.       */
/*      ENODEV - Adapter could not be selected.                           */
/*      ETIMEDOUT - The test_unit_ready command timed out.                */
/*      EIO    - scatter/gather not supported                             */
/*             - queue stopping or request exceeds MAX_REQUEST            */
/*      EFAULT - Bad copyin or copyout.                                   */
/*      EBUSY  - Dump in progress                                         */
/*      ENXIO  - queue halted(needs RESUME) or queue full (max 1)         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_strategy   lide_logerr     lide_build_command                */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, copyout, bcopy    */
/*                                                                        */
/**************************************************************************/
int
lide_test_unit_rdy(
                  struct adapter_def *ap,
		  dev_t devno,
		  int arg,
		  ulong devflag)
{
    int     ret_code, ret_code2, dev_index;
    struct ataide_buf *bp;
    struct ide_ready ide_rdy;

    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
	ret_code = copyin((char *) arg, &ide_rdy,
			  sizeof(struct ide_ready));
	if (ret_code != 0)
	{
	    DEBUG_0 ("Leaving lide_test_unit_rdy routine. EFAULT\n")
	    return (EFAULT);
	}
    }
    else
    { /* buffer is in kernel space */
	bcopy((char *) arg, &ide_rdy, sizeof(struct ide_ready));
    }

    /* Obtain device structure from hash on the device id */
    dev_index = ide_rdy.ide_device & 0x01;
    if ((ap->device_queue_hash[dev_index] == NULL) ||
	(ap->device_queue_hash[dev_index]->opened == FALSE))
    {   /* device queue structure not allocated or device not started*/
        DEBUG_0 ("Leaving lide_test_unit_rdy routine. EINVAL\n")
        return (EINVAL);
    }
    bp = lide_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("Leaving lide_test_unit_rdy routine. ENOMEM\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    bp->ata.flags = ide_rdy.flags;
    bp->ata.command = ATA_ATAPI_PACKET_COMMAND;
    bp->ata.device = dev_index;
    bp->ata.atapi.length = 12;
    bp->ata.atapi.packet.op_code = ATAPI_TEST_UNIT_READY;

    bp->bufstruct.b_dev = devno;

    /* Initialize default status to zero.                    */
    ide_rdy.status_validity = 0;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->q_flags = ATA_RESUME;

    /* Call our strategy routine to issue the test unit ready cmd. */
    if (ret_code = lide_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp, pinned_heap); /* release buffer */
        DEBUG_0 ("Leaving lide_test_unit_rdy strategy error.\n")
        return (ret_code);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        ret_code = bp->bufstruct.b_error;
        if (ret_code == ETIMEDOUT)
        {
	  lide_logerr(ap, ERRID_ATAIDE_ERR4, COMMAND_TIMEOUT, 2, 0,
                      ap->device_queue_hash[dev_index], TRUE);
	}
    }

    /* Copy out the device status to the ide_ready structure      */
    /* passed in by the calling application.                      */

    /* move data from buffer to ide_rdy.tur_data */
    ide_rdy.status_validity = bp->status_validity;
    bcopy(bp->bufstruct.b_un.b_addr, (char *) ide_rdy.tur_data, 128); 

    if (!(devflag & DKERNEL))
    {
        ret_code2 = copyout(&ide_rdy, (char *) arg,
                            sizeof(struct ide_ready));
        if (ret_code2 != 0)
        {
            ret_code = EFAULT;
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy(&ide_rdy, (char *) arg, sizeof(struct ide_ready));
    }
    (void) xmfree(bp, pinned_heap);     /* release buffer */

    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_identify_device                                            */
/*                                                                        */
/* FUNCTION:  Issues an ATA identify device command.                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue an identify device          */
/*         command.                                                       */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the IDEIOSTART failed.  Such a failure would indicate  */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ata/ide adapter unique data structure               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap      - pointer to an adapter_def structure                     */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument, buffer to contain identify data        */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Child Device not opened.                                 */
/*      ENOMEM - Could not allocate an ataide_buf for this command.       */
/*      ENODEV - Adapter could not be selected.                           */
/*             - or Command rejected by device                            */
/*      ETIMEDOUT - The identify_device command timed out.                */
/*      EIO    - scatter/gather not supported                             */
/*             - queue stopping or request exceeds MAX_REQUEST            */
/*      EFAULT - Bad copyin or copyout.                                   */
/*      EBUSY  - Dump in progress                                         */
/*      ENXIO  - queue halted(needs RESUME) or queue full (max 1)         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_strategy   lide_build_command                                */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, copyout, bcopy    */
/*                                                                        */
/**************************************************************************/
int
lide_identify_device(
                  struct adapter_def *ap,
                  dev_t devno,
                  int arg,
                  ulong devflag)
{
    int     ret_code, ret_code2, dev_index;
    struct ataide_buf *bp;
    struct dev_info   *dev_ptr;
    struct ide_identify ide_ident;

    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
        ret_code = copyin((char *) arg, &ide_ident,
                          sizeof(struct ide_identify));
        if (ret_code != 0)
        {
            DEBUG_0 ("Leaving lide_identify_device routine. EFAULT\n")
            return (EFAULT);
        }
    }
    else
    { /* buffer is in kernel space */
        bcopy((char *) arg, &ide_ident, sizeof(struct ide_identify));
    }
    /* Obtain device structure from hash on the device id */
    dev_index = ide_ident.ide_device & 0x01;
    if ((ap->device_queue_hash[dev_index] == NULL) ||
        (ap->device_queue_hash[dev_index]->opened == FALSE))
    {   /* device queue structure not allocated or not started*/
        DEBUG_0 ("Leaving lide_identify_device routine. EINVAL\n")
        return (EINVAL);
    }
    dev_ptr = ap->device_queue_hash[dev_index];
    bp = lide_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("Leaving lide_identify_device routine. ENOMEM\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    if (dev_ptr->protocol_type == IDE_ID_ATAPI)
      bp->ata.command = ATA_ATAPI_IDENTIFY_DEVICE;
    else
      bp->ata.command = ATA_IDENTIFY_DEVICE;
retry:
    bp->ata.sector_cnt_cmd = 1;
    bp->ata.device = dev_index;
    bp->ata.flags = ide_ident.flags;
    bp->timeout_value = LONGWAIT;

    bp->bufstruct.b_flags = B_READ;
    bp->bufstruct.b_bcount = ide_ident.identify_len;
    bp->bufstruct.b_dev = devno;
    bp->bufstruct.b_error = 0;
    bp->bufstruct.b_resid = 0;
    

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the identify is only running single-threaded  */
    /* to this device.                                            */
    bp->q_flags = ATA_RESUME;

    /* Call our strategy routine to issue the identify device cmd. */
    if (ret_code = lide_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp, pinned_heap); /* release buffer */
        DEBUG_1 ("Leaving lide_identify_device strategy error 0x%x\n",ret_code)
        return (ret_code);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

/*  DEBUG_2("  ret from strategy, b_resid = 0x%x, b_flags = 0x%x\n",
            bp->bufstruct.b_resid, bp->bufstruct.b_flags) */
    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        if (bp->status_validity & ATA_ERROR_VALID)
        {
            if (bp->ata.errval & ATA_ABORTED_CMD)
            { /* try ATAPI ident */
              if (bp->ata.command == ATA_IDENTIFY_DEVICE) {
                bp->ata.command = ATA_ATAPI_IDENTIFY_DEVICE;
                goto retry;
              }
              ret_code = ENODEV;
            }
            else
            {
                ret_code = bp->bufstruct.b_error;
            }
        }
        else if (bp->status_validity & ATA_IDE_STATUS) {
            if (bp->ata.status == 0xff) {           /* time out ?? */
                if (bp->ata.command == ATA_IDENTIFY_DEVICE) {
                  /* try ATAPI ident */
                  bp->ata.command = ATA_ATAPI_IDENTIFY_DEVICE;
                  goto retry;
                }
                ret_code = ETIMEDOUT;
            }
            else
            {
                ret_code = bp->bufstruct.b_error;
            }
        }
    }

    if (ret_code)
      ide_ident.ata_atapi = IDE_ID_NODEV;
    else if(bp->ata.command == ATA_IDENTIFY_DEVICE)
      ide_ident.ata_atapi = IDE_ID_ATA;
    else
      ide_ident.ata_atapi = IDE_ID_ATAPI;
    ide_ident.ata_status = bp->ata.status;
    ide_ident.ata_error = bp->ata.errval;
    /* Copy out the device data to the user buffer      */
    /* passed in by the calling application.            */
    ide_ident.identify_len -= bp->bufstruct.b_resid;

    if (!(devflag & DKERNEL))
    {
        ret_code2 = copyout((char *)bp->bufstruct.b_un.b_addr, 
                            (char *) ide_ident.identify_ptr, 
                            ide_ident.identify_len);
        if (ret_code2 != 0)
        {
            ret_code = EFAULT;
        }
        ret_code2 = copyout((char *)&ide_ident, (char *) arg,
                            sizeof (struct ide_identify));
        if (ret_code2 != 0)
        {
            ret_code = EFAULT;
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy(bp->bufstruct.b_un.b_addr, (char *) ide_ident.identify_ptr, 
              ide_ident.identify_len);
        bcopy(&ide_ident, (char *) arg, sizeof(struct ide_identify));
    }
    (void) xmfree(bp, pinned_heap);     /* release buffer */
/*  DEBUG_0 ("Leaving lide_identify_device routine.\n") */
    return (ret_code);
}


/**************************************************************************/
/*                                                                        */
/* NAME:  lide_read_ata                                                   */
/*                                                                        */
/* FUNCTION:  Issues an ATA read command.                                 */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue an ata read command.        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the IDEIOSTART failed.  Such a failure would indicate  */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ata/ide adapter unique data structure               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap      - pointer to an adapter_def structure                     */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument, buffer to contain identify data        */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Child Device not opened.                                 */
/*      ENOMEM - Could not allocate an ataide_buf for this command.       */
/*      ENODEV - Adapter could not be selected.                           */
/*      ETIMEDOUT - The ata_read_sector command timed out.                */
/*      EIO    - scatter/gather not supported                             */
/*             - queue stopping or request exceeds MAX_REQUEST            */
/*      EFAULT - Bad copyin or copyout.                                   */
/*      EBUSY  - Dump in progress                                         */
/*      ENXIO  - queue halted(needs RESUME) or queue full (max 1)         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_strategy   lide_logerr     lide_build_command                */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, copyout, bcopy    */
/*                                                                        */
/**************************************************************************/
int
lide_read_ata(
	struct adapter_def	*ap,
	dev_t			devno,
	int			arg,
	ulong			devflag)
{
	int			ret_code, ret_code2, dev_index;
	struct ataide_buf	*bp;
	struct ide_readblk	ide_rdblk;
	struct dev_info		*dev_ptr;
	uint			t_blkno;
	uint			abs_track;
	ushort			cylinder;

/*	DEBUG_0 ("Entering lide_read_ata routine.\n")*/

	ret_code = 0;

	/* Copy in the arg structure. If the buffer resides */
	/* user space, use copyin, else use bcopy.          */

	if (!(devflag & DKERNEL))
	{
		ret_code = copyin((char *) arg, &ide_rdblk,
		sizeof(struct ide_readblk));
		if (ret_code != 0)
		{
			DEBUG_0 ("Leaving lide_read_ata routine. EFAULT\n")
			return (EFAULT);
		}
	} else {
		/* buffer is in kernel space */
		bcopy((char *) arg, &ide_rdblk, sizeof(struct ide_readblk));
	}


	/* Obtain device structure from hash on the device id */
	dev_index = ide_rdblk.ide_device & 0x01;

	dev_ptr = ap->device_queue_hash[dev_index];
	if ((dev_ptr == NULL) || (dev_ptr->opened == FALSE))
	{
		/* device queue structure not allocated or not started*/
		DEBUG_0 ("Leaving lide_read_ata routine. EINVAL\n")
		return (EINVAL);
	}

	bp = lide_build_command();

	if (bp == NULL)
	{
		DEBUG_0 ("Leaving lide_read_ata routine. ENOMEM\n")
		return (ENOMEM);        /* couldn't allocate command */
	}

	bp->bufstruct.b_flags = B_READ;

	bp->ata.command = ATA_READ_SECTOR;
	bp->ata.device = dev_index;
	bp->ata.sector_cnt_cmd = 1;

	/* Determine start address using the appropriate	*/
	/* format (CHS/LBA)					*/

	bp->ata.flags = ATA_LBA_MODE;
	bp->ata.startblk.lba = ide_rdblk.blkno;
	if (dev_ptr->chs_only) {
		t_blkno = bp->ata.startblk.lba;
		bp->ata.flags = ATA_CHS_MODE;
		bp->ata.startblk.chs.sector =
			(t_blkno % dev_ptr->sectors_per_track)+1;
		abs_track = (t_blkno / dev_ptr->sectors_per_track);
		bp->ata.startblk.chs.head = (uchar) (abs_track %
						dev_ptr->num_heads);
		cylinder = (ushort) (abs_track / dev_ptr->num_heads);
		bp->ata.startblk.chs.cyl_hi = (uchar) (cylinder >> 8);
		bp->ata.startblk.chs.cyl_lo = (uchar) (cylinder &
						0x00ff);
		DEBUG_4("lide_read_ata: C/H/S = %02x%02x/%02x/%02x\n",
				bp->ata.startblk.chs.cyl_hi,
				bp->ata.startblk.chs.cyl_lo,
				bp->ata.startblk.chs.head,
				bp->ata.startblk.chs.sector)
	}

	bp->timeout_value = ide_rdblk.timeout_value;
	bp->bufstruct.b_bcount = ide_rdblk.blksize;
	bp->bufstruct.b_dev = devno;

	/* Set resume flag in case caller is retrying this operation. */
	/* This assumes the identify is only running single-threaded  */
	/* to this device.                                            */

	bp->q_flags = ATA_RESUME;

	/* Call our strategy routine to issue the identify device cmd */

	if (ret_code = lide_strategy(bp))
	{
		/* an error was returned */
		(void) xmfree(bp, pinned_heap); /* release buffer */
		DEBUG_1 ("Leaving lide_read_ata strategy error 0x%x\n",ret_code)
		return (ret_code);
	}

	iowait((struct buf *) bp);  /* Wait for commmand completion */

	/* The return value from the operation is examined to deter- */
	/* what type of error occurred.  Since the calling applica-  */
	/* requires an ERRNO, this value is interpreted here.        */

	if (bp->bufstruct.b_flags & B_ERROR)
	{
		/* an error occurred */
		ret_code = bp->bufstruct.b_error;
        	if (ret_code == ETIMEDOUT)
        	{
		  lide_logerr(ap, ERRID_ATAIDE_ERR4, COMMAND_TIMEOUT, 3,
                              0, dev_ptr, TRUE);
		}
	}

	if (!(devflag & DKERNEL)) {
		ret_code2 = copyout((char *)bp->bufstruct.b_un.b_addr,
		(char *)ide_rdblk.data_ptr,
		ide_rdblk.blksize);
		if (ret_code2 != 0)
			ret_code = EFAULT;
	} else {
		/* buffer is in kernel space */
		bcopy((char *)bp->bufstruct.b_un.b_addr,
		(char *)ide_rdblk.data_ptr,
		ide_rdblk.blksize);
	}

	/* Copy out the device data to the user buffer      */
	/* passed in by the calling application.            */

	if (!(devflag & DKERNEL))
	{
		ret_code2 = copyout((char *)&ide_rdblk, (char *) arg,
		sizeof (struct ide_readblk));
		if (ret_code2 != 0)
			ret_code = EFAULT;
	} else {
		/* buffer is in kernel space */
		bcopy(&ide_rdblk, (char *) arg, sizeof(struct ide_readblk));
	}
	(void) xmfree(bp, pinned_heap);     /* release buffer */
	return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:        lide_start_dev                                            */
/*                                                                        */
/* FUNCTION:  Allocates resources and starts a device                     */
/*                                                                        */
/*      This routine initializes the device queue to prepare for          */
/*      command processing.                                               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - adapter-unique data structure                       */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap      - pointer to an adapter_def structure                     */
/*      devno   - passed device major/minor number                        */
/*      dev_index - index to device information                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION                                                      */
/*      EINVAL - Child Device not opened or invalid.                      */
/*      ENOMEM - Could not allocate an ataide_buf for this command.       */
/*      ENODEV - Adapter could not be selected.                           */
/*             - or Command rejected by device                            */
/*      EALREADY - device already started                                 */
/*      ETIMEDOUT - The identify_device command timed out.                */
/*      EIO    - scatter/gather not supported                             */
/*             - queue stopping or request exceeds MAX_REQUEST            */
/*      EFAULT - Bad copyin or copyout.                                   */
/*      EBUSY  - Dump in progress                                         */
/*      ENXIO  - queue halted(needs RESUME) or queue full (max 1)         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_strategy   lide_build_command                                */
/*      lide_xlate_mode lide_identify_device                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      xmalloc     xmfree      bzero       w_init      iowait            */
/*                                                                        */
/**************************************************************************/

int
lide_start_dev(
              struct adapter_def *ap,
              dev_t devno,
	      int dev_index)
{
    int				i;
    struct dev_info		*dev_ptr;
    struct identify_device	ident;
    struct ide_identify		ioctl_ident;
    uchar			tchar;
    uchar			*ucp;
    struct ataide_buf          *bp;
    int                         rc;

    if ((dev_index < 0) || (dev_index >= ap->device_cnt))
    {   /* invalid device index */
        return (EINVAL);
    }

    if (ap->device_queue_hash[dev_index] == NULL)
    {   /* malloc failed */
        return (ENOMEM);
    }
    dev_ptr = ap->device_queue_hash[dev_index];
    if (dev_ptr->opened == TRUE)
    {   /* device already started */
	return (EALREADY);
    }

    /* Initialize device queue flags.			*/

    dev_ptr->dev_id = dev_index;
    dev_ptr->flags = RETRY_ERROR;

    /* init watchdog timer struct			*/

    dev_ptr->dev_watchdog.dog.next = NULL;
    dev_ptr->dev_watchdog.dog.prev = NULL;
    dev_ptr->dev_watchdog.dog.func = lide_command_watchdog;
    dev_ptr->dev_watchdog.dog.count = 0;
    dev_ptr->dev_watchdog.dog.restart = 0;
    dev_ptr->dev_watchdog.ap = ap;
    dev_ptr->dev_watchdog.timer_id = LIDE_COMMAND_TMR;
    w_init(&(dev_ptr->dev_watchdog.dog));

    /* init command state flags				*/

    dev_ptr->queue_state = ACTIVE;
    dev_ptr->opened = TRUE;

    return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:        lide_xlate_mode                                           */
/*                                                                        */
/* FUNCTION:                                                              */
/*      Discover the highest supported mode as indicated by a "mode byte".*/
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine may be called from a process or interrupt environment*/
/*                                                                        */
/* INPUTS:                                                                */
/*      mode - the mode to be translated                                  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      This routine returns the offset (7-0) of the highest order bit    */
/*      in the lowest order 8 bits of the input integer (mode area).      */
/*      If no bits are on, it returns -1.                                 */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      none                                                              */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      none                                                              */
/*                                                                        */
/**************************************************************************/
static int
lide_xlate_mode(int mode)
{                                     /* this routine returns the offset  */
  int tmode;                          /* of the highest order bit in the  */
  int modenum;                        /* low-order 8 bits of the 'mode'.  */
                                      /* if no bits are on, it returns -1 */
  modenum = 7;
  tmode = 0x00000080;
  mode &= 0x000000ff;
  while (tmode && (!(mode & tmode)))
  {
    tmode >>= 1;
    modenum--;
  }
  return(modenum);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_stop_dev                                                   */
/*                                                                        */
/* FUNCTION:  Stops a device and deallocates resources.                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to stop a device.  Any further command  */
/*         processing for the device will be halted from this point on.   */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_index - index to device information                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - device not opened.                                         */
/*    EIO - unable to pin code                                            */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: i_enable, i_disable, xmfree, w_clear,      */
/*    pincode, unpincode                                                  */
/*                                                                        */
/**************************************************************************/
int
lide_stop_dev(
             struct adapter_def *ap,
             int dev_index)
{
    int     old_level;
    struct dev_info *dev_ptr;

/*  DEBUG_0 ("Entering lide_stop_dev routine.\n") */

    if (((dev_index < 0) || (dev_index >= MAX_DEVICES)) ||
        (ap->device_queue_hash[dev_index] == NULL) ||
	(ap->device_queue_hash[dev_index]->opened == FALSE))
    {   /* device index out of range OR               */
        /* device queue structure already deallocated */
	/* device already stopped */
        return (EINVAL);
    }

    /* Obtain device structure pointer and disable */
    /* interrupts for processing.                 */

    if (pincode(lide_stop_dev) != 0)
    {
        return (EIO);
    }

    dev_ptr = ap->device_queue_hash[dev_index];
    old_level = i_disable(ap->ddi.adap.int_prior);
    dev_ptr->queue_state = STOPPING;

    /* Search the active queue of */
    /* this device structure to determine if there */
    /* are any commands outstanding.              */
    while (dev_ptr->cmd_pend != NULL)
    {
        DEBUG_0 ("   command pending\n")
    }

    DEBUG_0 ("   Reenable ints\n")
    i_enable(old_level);        /* let interrupts in */

    /* Free the device's watchdog timer entry.   */
    w_clear(&(dev_ptr->dev_watchdog.dog));

    dev_ptr->opened = FALSE;
    (void) unpincode(lide_stop_dev);

/*  DEBUG_0 ("Leaving lide_stop_dev routine.\n") */
    return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:    lide_issue_reset                                              */
/*                                                                        */
/* FUNCTION:  Issues a reset to an IDE attached ATAPI device.             */
/*                                                                        */
/*      This performs actions necessary to send an ATAPI Soft Reset       */
/*      to an attached ATAPI device.                                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes an ATAPI Soft reset to be issued to an     */
/*         ATAPI device.  The reset is queued as a normal command to the  */
/*         device.                                                        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the IDEIOSTART failed.  Such a failure would indicate  */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adapter_def - adapter unique data structure                         */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_index - index to device information                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    0 - for good completion, ERRNO value otherwise                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL    - device not opened.                                      */
/*    EIO       - device powered off                                      */
/*    EBUSY     - dump in progress                                        */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*    lide_atapi_soft_reset           lide_turn_chip_on                   */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
lide_issue_reset(
              struct adapter_def *ap,
	      int dev_index)
{
    struct dev_info *dev_ptr;
    int     i, ret_code;
    register int rc;


    DEBUG_0 ("Entering lide_issue_reset routine.\n")
    ret_code = 0;             /* assume success */
    dev_index &= 0x01;        /* make sure dev_index is within range */
    if (((dev_ptr = ap->device_queue_hash[dev_index]) == NULL) ||
	(dev_ptr->opened == FALSE) ||
        (dev_ptr->protocol_type != IDE_ID_ATAPI))
    {   /* device queue structure not allocated */
        DEBUG_0 ("lide_issue_reset: device pointer is NULL!\n")
        return (EINVAL);
    }

    /* don't interrupt the dump */
    if (ap->dump_started)
	return (EBUSY);

    /* make sure we're not powered off */
    if (ap->power_state == 0) {
	if (ap->pmh.mode == PM_DEVICE_IDLE) {
	  lide_turn_chip_on(ap, TRUE);
	}
    } else {
	return (EIO);
    }

    lide_atapi_soft_reset(ap, dev_index);
    DEBUG_1 ("Leaving lide_issue_reset routine. rc=0x%x\n", ret_code);
    return (ret_code);
}  /* lide_issue_reset */
