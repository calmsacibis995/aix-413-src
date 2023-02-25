static char sccsid[] = "@(#)97  1.31  src/rspc/kernext/isa/ide/ataideb.c, isaide, rspc41J, 9525E_all 6/21/95 12:16:31";
/*
 * COMPONENT_NAME: (ISAIDE) IBM Local Bus ATA/IDE Adapter Driver Bottom Half
 *
 * FUNCTIONS:
 * lide_chip_register_reset lide_atapi_soft_reset    lide_reconfig_adapter
 * lide_count_devices       lide_raw_cmd             lide_init_device
 * lide_logerr              lide_read_ATA            lide_write_ATA
 * lide_read_reg            lide_write_reg           lide_strategy
 * lide_start               lide_stop                lide_iodone
 * lide_dump                lide_dumpstrt            lide_dump_intr
 * lide_dumpwrt             lide_cdt_func            lide_delay
 * lide_xfer_func           lide_intr
 * lide_epow                lide_pm_handler          lide_turn_chip_on
 * lide_command_watchdog    lide_command_reset_ide_bus
 * lide_trace_1             lide_trace_2             lide_trace_3
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
/*                                                                      */
/* COMPONENT:   SYSXIDE                                                 */
/*                                                                      */
/* NAME:        ataideb.c                                               */
/*                                                                      */
/* FUNCTION:    IBM ATA/IDE Adapter Driver Bottom Half Source File      */
/*                                                                      */
/*      This adapter driver is the interface between an IDE device      */
/*      driver and the actual ATA/IDE adapter.  It executes commands    */
/*      from multiple drivers which contain ATA and ATAPI device        */
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
#include <sys/iocc.h>
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
#include <sys/mdio.h>
#include <sys/ide.h>
#include <sys/pm.h>
#include "ataide_errids.h"
#include "ataidedd.h"
/* END OF INCLUDED SYSTEM FILES  */

/************************************************************************/
/* Global pinned device driver static data areas                        */
/************************************************************************/
/* global pointer for adapter structure                                 */
struct    adapter_def    adp_str[MAX_ADAPTERS];
/* global pointer for ide adapter component dump table */
struct lide_cdt_table *lide_cdt = NULL;
/* global configuration count */
int lide_cfg_cnt = 0;
/* global open count */
int lide_open_cnt = 0;

/************************************************************************/
/*                                                                      */
/* NAME:  lide_chip_register_reset                                      */
/*                                                                      */
/* FUNCTION:  Resets IDE interface                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called from any        */
/*      other driver routine.                                           */
/*                                                                      */
/* NOTES:  This routine is called to reset the adapter chip after a     */
/*         catastrophic error has occurred.                             */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      None                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:                                                   */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      d_mask          d_clear                                         */
/*      d_master                                                        */
/*                                                                      */
/************************************************************************/
void
lide_chip_register_reset(
                         struct adapter_def *ap)

{

    int     i;
    int     ata_status;

    TRACE_1 ("in chipR",0)

    /* Disable dma and ATA/IDE interrupts from the chip */
    (void) LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0A);

    /* Assert a chip reset by writing to the LIDCTL register */
    (void) LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0C); /* reset ATA */
    lide_delay(ap, 0, 25000);
    /* release reset, and enable interrupts */
    (void) LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x08);
        
    for (i = 0; i<IPL_MAX_SECS; i++)
    {
       ata_status = LIDE_READ_ATA(ap, LISTA, LISTA_SIZE);
       if (!(ata_status & ATA_BUSY))
         break;
       lide_delay(ap, 1, 0);
    }
    TRACE_1 ("outchipR",0)
}

/************************************************************************/
/*                                                                      */
/* NAME:  lide_atapi_soft_reset                                         */
/*                                                                      */
/* FUNCTION:  Resets an ATAPI device                                    */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called from any        */
/*      other driver routine.                                           */
/*                                                                      */
/* NOTES:  This routine is called to reset an ATAPI device after a      */
/*         catastrophic error has occurred,  or it is required to       */
/*         abort an ATAPI command                                       */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      None                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:                                                   */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/
void
lide_atapi_soft_reset(
                         struct adapter_def *ap,
                         int device)

{
  int     i;
  int     ata_status;

  /* access to the desired device's register set */
  LIDE_WRITE_ATA(ap, LIDRVH, LIDRVH_SIZE, 
                      (0xE0 | ((device & 0x01) << 4)) ); 
  /* write ATAPI SOFT RESET command to command register */

  /* put command into command register */
  LIDE_WRITE_ATA(ap, LICMD, LICMD_SIZE, ATA_ATAPI_SOFT_RESET);

        
  for (i = 0; i<IPL_MAX_SECS; i++)
  {
     ata_status = LIDE_READ_ATA(ap, LISTA, LISTA_SIZE);
     if (!(ata_status & ATA_BUSY))
       break;
     lide_delay(ap, 1, 0);
  }
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_reconfig_adapter                                   */
/*                                                                      */
/* FUNCTION:    Adapter Configuration Routine                           */
/*                                                                      */
/*      This internal routine performs actions required to make         */
/*      the driver ready for the first open to the adapter.             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine must be called from the process level only.        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      Global_adapter_ddi                                              */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      None                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0 for successful return                           */
/*                  = EIO for unsuccessful operation                    */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*      lide_chip_register_reset                                        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/
int
lide_reconfig_adapter(
                      struct adapter_def *ap)
{
    int     ret_code;
    int     dev_index;
    struct dev_info       *dev_ptr;

    ret_code = 0;

    /* reset the interface for this adapter */
    if (ap->device_cnt > 0)
      lide_chip_register_reset(ap);

    /* reset all atapi devices on this adapter */
    for (dev_index = 0; dev_index < ap->device_cnt; dev_index++) {
      dev_ptr = ap->device_queue_hash[dev_index];
      if (dev_ptr->protocol_type == IDE_ID_ATAPI) {
	lide_atapi_soft_reset(ap, dev_index);
      }
    }
    
    /* reset the interface for this adapter again */
    if (ap->device_cnt > 0)
      lide_chip_register_reset(ap);

    for (dev_index = 0; dev_index < ap->device_cnt; dev_index++) {
      (void)lide_init_device(ap, dev_index);
    }
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_count_devices                                      */
/*                                                                      */
/* FUNCTION:    Adapter Configuration Routine                           */
/*                                                                      */
/*      This internal routine performs actions required to make         */
/*      the driver ready for the first open to the adapter.             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine must be called from the process level only.        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      Global_adapter_ddi                                              */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      None                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0 for successful return                           */
/*                  = EIO for unsuccessful operation                    */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*      lide_chip_register_reset                                        */
/*      LIDE_READ_ATA                                                   */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/
int
lide_count_devices(
                    struct adapter_def *ap)
{
    int     ret_code;
    int     ata_status;
    int     i;

    ret_code = 0;       /* default to no error */

    ap->device_cnt = 0; /* initialize number of devices */

    ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);
    DEBUG_1("lide_count_devices entry status is 0x%x\n", ata_status)

    if (ata_status & ATA_BUSY)  /* if initial status is busy,       */
       return (ret_code);       /* then assume there are no devices */

    while ( ap->device_cnt < MAX_DEVICES ) {

      /* issue a NO OP command to the specified device */
      ret_code = lide_raw_cmd (ap, ATA_NOP,
					(0xA0 | (ap->device_cnt << 4)),
					0, 0, 0, 0, 0, 1000);

      DEBUG_1("lide_count_devices after NOP return 0x%x'n", ret_code)
      if (ret_code) {
	ret_code = 0;
	break;
      }

      ap->device_cnt++;
    }

    DEBUG_1("lide_count_devices found 0x%x devices\n",ap->device_cnt)

    return (ret_code);

}  /* end lide_count_devices */

/************************************************************************/
/*                                                                      */
/* NAME:        lide_raw_cmd                                            */
/*                                                                      */
/* FUNCTION:    Raw Command to Device Routine                           */
/*                                                                      */
/*      This internal routine executes a simple command to the          */
/*      specified device.  This is a limited function in that no        */
/*      data except that passed in the ATA Task file registers is       */
/*      transfered.  Only commands which expect to return final         */
/*      status immediately are supported by this routine.  These        */
/*      commands include SET FEATURES, INITIALIZE DEVICE PARAMETERS,    */
/*      NO-OP, etc.                                                     */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine may be called from the process or interrupt level, */
/*      but should not be called when any other command is pending on   */
/*      the target adapter unless the intent is to abort that command.  */
/*      Also, the calling routine should be aware that this routine will*/
/*      leave the adapter interrupt mechanism disabled, and that the    */
/*      calling routine must re-enable the adapter interrupts if that   */
/*      is the desired state.                                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      None                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0 for successful return                           */
/*                  = EIO for the adapter not becoming available within */
/*                        IPL_MAX_SECS.                                 */
/*                  = ETIMEDOUT if the command fails to return status   */
/*                              within 'timeout' milliseconds.          */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*      LIDE_READ_ATA   LIDE_WRITE_ATA  lide_delay                      */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/
int
lide_raw_cmd(
             struct adapter_def *ap,
		int			cmd_reg,	/* command register */
		int			drvh_reg,	/* Drive/Head reg.  */
		int			feat_reg,	/* Feature register */
		int			secc_reg,	/* Sector Count reg.*/
		int			secn_reg,	/* Sector Numb. reg.*/
		int			cyll_reg,	/* Cylinder Low reg.*/
		int			cylh_reg,	/* Cylinder High reg*/
		int                     timeout)        /* Time limit(msecs)*/
{
    int     ret_code;
    int     ata_status;
    int     i;

    ret_code = 0;       /* default to no error */

    /* Turn device interrupts off, since we don't want to disturb system */
    LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0A); /* interrupts off */

    for (i = 0; i < IPL_MAX_SECS; i++)
    {
      ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);
      if (!(ata_status & ATA_BUSY))
        break;
      lide_delay(ap, 1, 0);
    }

    if ((ata_status & (ATA_BUSY | ATA_DEVICE_READY)) != ATA_DEVICE_READY)
      return (EIO);

    LIDE_WRITE_ATA(ap, LIDRVH, LIDRVH_SIZE, drvh_reg);
    LIDE_WRITE_ATA(ap, LIFEAT, LIFEAT_SIZE, feat_reg);
    LIDE_WRITE_ATA(ap, LISECC, LISECC_SIZE, secc_reg);
    LIDE_WRITE_ATA(ap, LISECN, LISECN_SIZE, secn_reg);
    LIDE_WRITE_ATA(ap, LICYLL, LICYLL_SIZE, cyll_reg);
    LIDE_WRITE_ATA(ap, LICYLH, LICYLH_SIZE, cylh_reg);
    LIDE_WRITE_ATA(ap, LICMD,  LICMD_SIZE,  cmd_reg);

    for (i = 0; i < timeout; i++)
    {
      lide_delay(ap, 0, 1000000);  /* wait 1 millisecond */
      ata_status = LIDE_READ_ATA(ap, LISTA, LISTA_SIZE);
      if (!(ata_status & ATA_BUSY))
        break;
    }

    if ((ata_status & (ATA_BUSY | ATA_DEVICE_READY)) != ATA_DEVICE_READY)
      ret_code = ETIMEDOUT;

    return (ret_code);

}  /* end lide_raw_cmd */

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_init_device                                                */
/*                                                                        */
/* FUNCTION:  Initialize an attached device to the proper PIO and DMA     */
/*            modes, and, for hard disks, set the number of default       */
/*            heads and sectors per track.  These parameters are set      */
/*            using data already set in the 'dev_info' structures at      */
/*            the initial adapter open.                                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process or from the interrupt     */
/*      environment.  However, care must be taken to insure that there    */
/*      are no active commands on the adapter when this routine is called.*/
/*      This routine is normally only called during adapter initialization*/
/*      or during PM adapter re-initialization after power restoration.   */
/*                                                                        */
/* NOTES:  This routine will leave the IDE chip interrupts disabled.      */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA interface unique data structures.               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap          -  pointer to an adapter_def structure                */
/*      dev_index   -  device index (0=Master, 1=Slave)                   */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
lide_init_device(
                 struct adapter_def *ap,
                 int dev_index)
{
  int rc;
  int ret_code;
  int secc;                         /* temp sector count reg */
  struct dev_info       *dev_ptr;

  ret_code = 0;
  dev_ptr = ap->device_queue_hash[dev_index];


  if (dev_ptr->protocol_type == IDE_ID_ATA)
  {
    /* issue INITIALIZE DRIVE PARAMETERS command to set the */
    /* number of heads and sectors-per-track for the disk   */

    DEBUG_2("lide_config_dev: # heads = 0x%x, # sectors/track =0x%x\n",
             dev_ptr->num_heads, dev_ptr->sectors_per_track );

    rc = lide_raw_cmd(ap, ATA_INITIALIZE_DEVICE_PARAMETERS,
                      ( 0xA0 | (dev_index << 4) | (dev_ptr->num_heads - 1)),
                      0, dev_ptr->sectors_per_track, 0, 0, 0, 5000);
    DEBUG_1("set drive parameters return = 0x%x\n", rc);

    /* issue SET FEATURES to DISABLE WRITE CACHE */
    rc = lide_raw_cmd(ap, ATA_SET_FEATURES,
		      ( 0xA0 | (dev_index << 4) ),
		      0x82, 0, 0, 0, 0, 5000);
    DEBUG_1("disable write cache return = 0x%x\n", rc);
  }

  /* do SET FEATURES for current MAX PIO mode */
  DEBUG_2("lide_config_dev: current pio = 0x%x, max. pio =0x%x\n",
            dev_ptr->pio_cur_mode, dev_ptr->pio_max_mode);


  if (dev_ptr->iordy_enabled)
    secc = 0x08 | dev_ptr->pio_cur_mode;
  else
    secc = 0x01; /* default mode & disable IORDY */
  DEBUG_1("setting pio mode to 0x%x\n",secc);
  rc = lide_raw_cmd(ap, ATA_SET_FEATURES,
                    ( 0xA0 | (dev_index << 4) ),
                    3, secc, 0, 0, 0, 5000);
  DEBUG_1("set pio mode returns 0x%x\n",rc);

  /* do SET FEATURES for devices' MAX DMA mode */
  if (dev_ptr->dma_supported) {
    DEBUG_2("lide_config_dev: current dma = 0x%x, max. dma =0x%x\n",
              dev_ptr->dma_cur_mode, dev_ptr->dma_max_mode);
    if (dev_ptr->dma_max_mode != dev_ptr->dma_cur_mode)
    {
      DEBUG_0("lide_config_dev: update dma mode needed!\n");
      rc = lide_raw_cmd(ap, ATA_SET_FEATURES,
                        ( 0xA0 | (dev_index << 4) ),
                        3, dev_ptr->dma_max_mode, 0, 0, 0, 5000);

      if (rc)
      {
        DEBUG_0("Error setting dma mode\n");
        if (dev_ptr->dma_cur_mode != -1) {
          dev_ptr->dma_max_mode = dev_ptr->dma_cur_mode;
        }
      }
    }
  }
  return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_logerr                                             */
/*                                                                      */
/* FUNCTION:    Adapter Driver Error Logging Routine                    */
/*                                                                      */
/*      This routine provides a common point through which all driver   */
/*      error logging passes.                                           */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called from any        */
/*      other driver routine.                                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      error_log_def - adapter driver error logging information        */
/*                      structure                                       */
/*                                                                      */
/* INPUTS:                                                              */
/*      errid   - the error log unique id for this entry                */
/*      add_hw_status - additional hardware status for this entry       */
/*      errnum  - a unique value which identifies which error is        */
/*                causing this call to log the error                    */
/*      data1   - additional, error dependent data                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      bcopy           bzero                                           */
/*      errsave                                                         */
/*                                                                      */
/************************************************************************/
void
lide_logerr(
           struct adapter_def *ap,
	   int errid,
	   int add_hw_status,
	   int errnum,
	   int data1,
	   struct dev_info * dev_ptr,
	   uchar read_regs)
{
    struct error_log_def log;
    int            old_level;
    volatile char *io;

    /* if a dump is running, don't log anything! */
    if (ap->dump_started)
      return;

    if (ap->errlog_enable)
    {	/* set up info and log */
	bzero(&log, sizeof(struct error_log_def));
	log.errhead.error_id = (uint) errid;
	bcopy(&ap->ddi.adap.resource_name[0], &log.errhead.
	      resource_name[0], 8);
	log.data.diag_validity = 0;
	log.data.diag_stat = 0;
	if (add_hw_status != 0)
	{
	    log.data.ahs_validity = 0x01;
	    log.data.ahs = (uchar) add_hw_status;
	}
        log.data.un.card1.errnum = (uint) errnum;

	/* see if additional data is needed due to add_hw_status value */
	if ((uint) add_hw_status & DMA_ERROR)
	    log.data.sys_rc = (uint) data1;
	/* Get ATA register data, directly read the regs   */
	/* here to avoid another call to error log         */
	/* routine due to error in read functions.         */
	if (read_regs)
	{	/* on an interrupt level, read these regs */
          old_level = i_disable(INTMAX);
          io = (volatile char *)iomem_att(&ap->iom) + ap->base_addr0;
          log.data.un.card1.error_reg = 
             (*((volatile char *)(io + LIERR))) & 0x00ff;
          log.data.un.card1.seccnt_reg = 
             (*((volatile char *)(io + LISECC))) & 0x00ff;
          log.data.un.card1.secnum_reg = 
             (*((volatile char *)(io + LISECN))) & 0x00ff;
          log.data.un.card1.cyllow_reg = 
             (*((volatile char *)(io + LICYLL))) & 0x00ff;
          log.data.un.card1.cylhi_reg = 
             (*((volatile char *)(io + LICYLH))) & 0x00ff;
          log.data.un.card1.drvhd_reg = 
             (*((volatile char *)(io + LIDRVH))) & 0x00ff;
          log.data.un.card1.status_reg =
             (*((volatile char *)(io + LIASTA))) & 0x00ff;
          iomem_det((void *)io);
          i_enable(old_level);

	  if ((dev_ptr != NULL) &&
	      (dev_ptr->cmd_pend != NULL))
	  {	/* found valid ptr to the job the MOST */
		/* likely caused us to go here */
            log.data.un.card1.ata = dev_ptr->cmd_pend->ata;
            log.data.un.card1.cmd_activity_state =
					       dev_ptr->cmd_activity_state;
          }
        }
	/* log the error here */
	errsave(&log, sizeof(struct error_log_def));
    }

}  /* end lide_logerr */

/************************************************************************/
/*                                                                      */
/* NAME:        lide_read_reg                                           */
/*                                                                      */
/* FUNCTION:    Access the specified register.                          */
/*                                                                      */
/*      This routine reads from a selected adapter                      */
/*      register and returns the data.                                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to read a single register; 8,24 or 32 bit                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap - pointer to an adapter_def                                  */
/*      offset  - offset to the selected register                       */
/*      reg_size - size of register in bytes                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 32-bits of data                                   */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
lide_read_reg(
              struct io_map *iom,
	      uint offset,
	      char reg_size)
{
    int     data;
    int     old_level;
    volatile char *io;

    /* disable our interupts while reading registers */
    /* to prevent our interrupts killing the read    */
    old_level = i_disable(INTMAX);
    io = (volatile char *)iomem_att(iom);
    if (reg_size == 1)
	data = *((volatile char *)(io + offset));
    else if (reg_size == 2)
	data = *((volatile short *)(io + offset));
    else
	data = *((volatile int *)(io + offset));
    iomem_det((void *)io);
    i_enable(old_level);
    return (data);
}  /* end lide_read_reg */

/************************************************************************/
/*                                                                      */
/* NAME:        lide_write_reg                                          */
/*                                                                      */
/* FUNCTION:    Store passed data in specified register.                */
/*                                                                      */
/*      This routine loads a selected adapter register with the         */
/*      passed data value, performing appropriate error detection       */
/*      and recovery.                                                   */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to load a single register.                                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap - pointer to an adapter def                                  */
/*      offset  - offset to the selected register                       */
/*      reg_size - size of register in bytes                            */
/*      data    - value to be loaded                                    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code =  0 if good data or recovered error                */
/*                  = -1 if permanent I/O error encountered             */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
lide_write_reg(
               struct io_map *iom,
	       uint offset,
	       char reg_size,
	       int data)
{
    int     old_level;
    volatile char *io;

    /* disable our interupts while writing registers */
    /* to prevent our interrupts killing the read    */
    old_level = i_disable(INTMAX);
    io = (volatile char *)iomem_att(iom);
    if (reg_size == 1)
      *((volatile char *)(io + offset)) = (char)data;
    else if (reg_size == 2)
      *((volatile short *)(io + offset)) = (short)data;
    else
      *((volatile ulong *)(io + offset)) = (ulong)data;
    iomem_det((void *)io);
    i_enable(old_level);
    return(0);

}  /* end lide_write_reg */

/************************************************************************/
/*                                                                      */
/* NAME:        lide_strategy                                           */
/*                                                                      */
/* FUNCTION: ATA/IDE Device Driver Strategy Routine                     */
/*                                                                      */
/*      This routine will accept commands from the device heads         */
/*      and queue them up to the appropriate device structure for       */
/*      execution.  The command, in the form of ataide_bufs, contains   */
/*      a bufstruct at the beginning of the structure and pertinent     */
/*      data appended after cannot exceed the maximum transfer size     */
/*      allowed.  Note that the av_forw field of the bufstruct MUST     */
/*      be NULL as chained commands are not supported.                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process or interrupt       */
/*      handler.  It can page fault only if called under a process      */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      ataide_buf  - input/output request struct used between the      */
/*                adapter driver and the calling ATA/ATAPI device driver*/
/* INPUTS:                                                              */
/*      bp - command buffer pointer                                     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      0 - For good completion                                         */
/*      ERRNO - value otherwise                                         */
/*                                                                      */
/* ERROR DESCRIPTION                                                    */
/*      EIO - Adapter not defined or initialized                        */
/*          - Adapter not opened                                        */
/*          - Device not started                                        */
/*          - Device is being stopped                                   */
/*      ENXIO - Device queue is currently awaiting a resume cmd         */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      i_disable, i_enable                                             */
/*                                                                      */
/************************************************************************/
int
lide_strategy(
	     struct ataide_buf * bp)
{
    int     ret_code;
    int     i;
    dev_t   devno;
    int     dev_index;
    int     new_pri;
    struct dev_info *dev_ptr;
    struct adapter_def *ap;

    ret_code = 0;
    devno = bp->bufstruct.b_dev;

    DDHKWD5(HKWD_DD_IDEDD, DD_ENTRY_STRATEGY, ret_code, devno, bp,
	    bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
	    bp->bufstruct.b_bcount);

    /* find adapter def based on devno */
    for (i = 0; i < MAX_ADAPTERS; i++)
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
	bp->bufstruct.b_error = ENODEV;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	bp->status_validity = 0;
        if (!(ap->dump_started))
	  iodone((struct buf *) bp);

	DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_STRATEGY, ENODEV, devno);

        DEBUG_1 ("Leaving lide_strategy, can't find adap for devno = 0x%x\n",
                  devno);
	TRACE_1 ("out strat", ENODEV)
	return (ENODEV);
    }

    /* passed the open test */
    /* get index into device table for this device */
    dev_index = bp->ata.device & 0x01;
    dev_ptr = ap->device_queue_hash[dev_index];
    TRACE_2 ("in strat", (int) bp, (int) dev_ptr)

    /* if DUMP is in progress, block out everything else */
    if (ap->dump_started)
    {
        bp->bufstruct.b_error = EBUSY;
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
        bp->status_validity = 0;
        return (EBUSY);
    }

    /* Disable interrupt to keep this single threaded. */
    new_pri = i_disable(ap->ddi.adap.int_prior);

    /* miscellaneous validation of request */
    if ((dev_ptr != NULL) &&
	(dev_ptr->opened == TRUE) &&
        (bp->bufstruct.b_bcount <= ap->max_request)
        && (dev_ptr->queue_state != STOPPING)
        && (bp->sg_ptr == 0x0))
    {
        /* passed check of device structure info */

        /* The HALTED flag is set when the adapter driver */
        /* encounters an error.  Do not continue the pro- */
        /* cessing if the caller has not set RESUME flag. */
        if ((dev_ptr->queue_state != HALTED) ||
    	    ((dev_ptr->queue_state == HALTED)
	     && (bp->q_flags & ATA_RESUME)))
        {
    	    /* passed queue_state and RESUME tests */

	    /* if we get here, device is not halted anymore */
	    /* set normal state */
	    dev_ptr->queue_state = ACTIVE;
            dev_ptr->bytes_moved = 0;
	    bp->bufstruct.av_forw = NULL;	/* only accept one cmd */
	    bp->bufstruct.b_work = 0;
	    /* Initialize the following flag to the "no */
	    /* error" state.                            */
	    bp->bufstruct.b_error = 0;
	    bp->bufstruct.b_flags &= ~B_ERROR;
	    bp->bufstruct.b_resid = 0;
	    bp->status_validity = 0;
            bp->ata.status = 0;
            bp->ata.errval = 0;
            bp->ata.sector_cnt_ret = bp->ata.sector_cnt_cmd;
            bp->ata.endblk.lba = bp->ata.startblk.lba;

	    /* Queue this command to the device's pending      */
	    /* queue for processing by lide_start.             */
	    if (dev_ptr->cmd_pend == NULL)
	    {	/* queue is empty  */
	        dev_ptr->cmd_pend = bp;
	        lide_start(ap, dev_ptr);
	    }
	    else
	    {	/* pending queue not empty */
	        bp->bufstruct.b_error = ENXIO;
	        bp->bufstruct.b_flags |= B_ERROR;
	        bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	        bp->status_validity = 0;
		if (!(ap->dump_started))
	          iodone((struct buf *) bp);
	        ret_code = ENXIO;
	    }
        }
        else
        {	/* check queue_state and RESUME flag failed */
    	    bp->bufstruct.b_error = ENXIO;
	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    bp->status_validity = 0;
	    if (!(ap->dump_started))
	      iodone((struct buf *) bp);
	    ret_code = ENXIO;
        }
    }
    else
    {	/* device structure validation failed */
        bp->bufstruct.b_error = EIO;
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
        bp->status_validity = 0;
	if (!(ap->dump_started))
          iodone((struct buf *) bp);
        ret_code = EIO;
    }

    DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_STRATEGY, ret_code, devno); 
    TRACE_1 ("out strat", 0)
    i_enable(new_pri);
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_spanned_xmem_xfer                                  */
/*                                                                      */
/* FUNCTION: ATA/IDE Device Driver Spanned Command Memory Transfer      */
/*                                                                      */
/*      This routine is called from either the 'lide_start' or the      */
/*      'lide_xfer_func' routines.  It is used on PIO read/write        */
/*      operations to transfer a segment of data to/from the user's     */
/*      data buffer from/to a local memory area for immediate PIO access*/
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process or interrupt       */
/*      handler.  It can page fault only if called under a process      */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      ataide_buf  - input/output request struct used between the      */
/*                adapter driver and the calling ATA/ATAPI device driver*/
/*                                                                      */
/* INPUTS:                                                              */
/*      ap        - pointer to an adapter_def structure                 */
/*      bp        - pointer to an I/O request buffer structure          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      NONE                                                            */
/*                                                                      */
/* ERROR DESCRIPTION                                                    */
/*      NONE                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      xmemin        xmemout                                           */
/*                                                                      */
/************************************************************************/
void
lide_spanned_xmem_xfer(
              struct adapter_def * ap,
              struct ataide_buf *bp)
{
  /* An ataide_buf structure may be extended to include multiple buffers*/
  /* by use of the included 'bp' pointer to another 'buf' structure.    */
  /* If 'bp' is NULL, then the entire user data area is specified by    */
  /* the original ataide_buf structure.  If 'bp' is not NULL, then only */
  /* the user buffer length is specified by the original ataide_buf     */
  /* struct, and data area itself is specified by a chain of 'buf'      */
  /* structures anchored by the 'bp' variable in the original ataide_buf*/
  /* structure.  During a PIO operation, it is necessary to keep track  */
  /* of the current position of the transfer in the users' data buffer  */
  /* by recording in a global location the current user buffer segment  */
  /* and offset within that segment.  Each 'buf' struct defines one     */
  /* segment of the user data area.  This routine will transfer a single*/
  /* device segment worth of data, (as defined by the 'segment_len' in  */
  /* the 'dev_info' structure), and update the global pointers to the   */
  /* users' data area.                                                  */

  uchar *lbuf;       /* local buffer address */
  uint   lbuf_ix;    /* current offset within local buffer */
  uint   lbuf_len;   /* length of local buffer */
  uint   move_len;   /* length of current xmem move operation */
  uchar *ubuf;       /* address of current user buffer */
  uint   ubuf_ix;    /* current offset within current usr buffer*/
  uint   ubuf_len;   /* length of current user buffer */

  /* initialize local buf parameters */
    /* the local buf parameters include address, current offset, & length*/
    /* the length of this buffer is set to the device 'segment_len' and  */
    /* indicates the length needed to be transfered in this routine */
    /* the 'lide_start' routine will insure that the device sector_len   */
    /* does not exceed the length of the allocated adapter xfer_area.    */
  lbuf = ap->xfer_area;
  lbuf_ix = 0;
  lbuf_len = ap->device_queue_hash[ap->active_device]->sector_len;
  if (lbuf_len > bp->bufstruct.b_bcount)
    lbuf_len = bp->bufstruct.b_bcount;
  
  /* initialize user buf parameters */
    /* this is actually done in 'lide_start'.  If the user data buffer is*/
    /* exhausted, the address will be set to NULL */
  if (ap->ubuf_ptr) {
    ubuf = ap->ubuf_ptr->b_un.b_addr;
  } else {
    ubuf = NULL;
  }

  /* while there is more data needed and more data available */
  /* (data is needed if the local buf index is less than the local buf len)*/
  /* (data is available is the user buf index is not null) */
  while ((lbuf_ix < lbuf_len) && (ap->ubuf_ptr != NULL))
  {
    ubuf = ap->ubuf_ptr->b_un.b_addr;
    ubuf_len = ap->ubuf_ptr->b_bcount;
    ubuf_ix = ap->ubuf_ix;

    /* length to move is lessor of remainder needed & remainder available*/
    move_len = ((lbuf_len - lbuf_ix) < (ubuf_len - ubuf_ix))?
               lbuf_len - lbuf_ix : ubuf_len - ubuf_ix ;

    /* if this is a READ operation */
    if (bp->bufstruct.b_flags & B_READ) {

      /* use 'xmemout' to transfer data to user buf */
      xmemout(&lbuf[lbuf_ix], &ubuf[ubuf_ix], move_len, &ap->ubuf_ptr->b_xmemd);

    /* else this is a WRITE operation */
    } else {

      /* use 'xmemin' to transfer data to local xfer area */
      xmemin(&ubuf[ubuf_ix], &lbuf[lbuf_ix], move_len, &ap->ubuf_ptr->b_xmemd);

    }

    /* update local buf parameters */
      /* add the length moved to the local index */
    lbuf_ix += move_len;

    /* update user buf parameters */
      /* add the length moved to the user index */
    ap->ubuf_ix += move_len;
      /* if the user index is >= the user buf length */
    if (ap->ubuf_ix >= ubuf_len)
    {
        /* point to next user buffer, & set user index = 0 */
      if (bp->bp == NULL)
        ap->ubuf_ptr = NULL;
      else
        ap->ubuf_ptr = ap->ubuf_ptr->av_forw;
      ap->ubuf_ix = 0;
    }
  /* end of while operation */
  }
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_start                                              */
/*                                                                      */
/* FUNCTION: ATA/IDE Device Driver Start Command Routine                */
/*                                                                      */
/*      This routine is called from the strategy routine when a command */
/*      becomes available for a device that does not currently have a   */
/*      command in the process of executing.  Since the adapter is      */
/*      shared between the Master & Slave devices, the a check is first */
/*      made to see if the adapter is currently active.  If so, the     */
/*      command is left inactive on the device, otherwise the command   */
/*      is started.  Commands not started by this routine will be       */
/*      attempted by again calling this routine from the 'xfer_func'    */
/*      when the 'xfer_func' completes the current command.             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process or interrupt       */
/*      handler.  It can page fault only if called under a process      */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      dev_info    - driver structure which holds information related  */
/*                    a particular device attached to an adapter        */
/*      ataide_buf  - input/output request struct used between the      */
/*                    adapter driver and the calling ATA/ATAPI device   */
/*                    driver.                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap        - pointer to an adapter_def structure                 */
/*      dev_ptr   - pointer to device information structure             */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      NONE                                                            */
/*                                                                      */
/* ERROR DESCRIPTION                                                    */
/*      NONE                                                            */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*      lide_turn_chip_on               lide_logerr     lide_delay      */
/*      LIDE_READ_ATA   LIDE_WRITE_ATA  lide_spanned_mem_xfer           */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_start         iodone          D_MAP_SLAVE                     */
/*                                                                      */
/************************************************************************/
void
lide_start(
          struct adapter_def * ap,
	  struct dev_info * dev_ptr)
{
    register int     x_len;
    register ushort  *sptr;
    register ushort  *end_xfer;
    register struct ataide_buf *bp;

    int     i;
    int     dma_rc;
    int     dma_reqd;           /* 0 = no dma; 1=dma read; 2=dma write */
    int     dma_flags;
    int     ata_status;
    int     atapi_reason;
    int     data_val;
    int     o_run_flag;		/* overrun flag - 0 = no overrun */
    int     o_run;
    int     old_level;
    volatile char *io;
    uchar   head_reg;
    struct buf  *next_bp;
    d_iovec_t    cur_dvec;


    TRACE_1 ("in strt ", (int) dev_ptr)

    if ((ap->power_state == EPOW_PENDING) ||
        (ap->active_device != NO_DEV_ACTIVE))
    {  /* either an EPOW is pending, or the device is powered down, or  */
       /* the device is busy with another command, so just leave the    */
       /* command on the queue and exit                                 */
      TRACE_1 ("out strt", (int) dev_ptr)
      return;
    }

    /* if device is in PM_IDLE state, then turn it on */
    if ((ap->dump_started == 0) &&
        (ap->pmh.mode == PM_DEVICE_IDLE))
      lide_turn_chip_on(ap, TRUE);

    bp = dev_ptr->cmd_pend;
    dma_reqd = NO_DMA_REQD;            /* no DMA needed (yet) */
    dma_flags = 0;
    /* non-EPOW state and there's no commands currently active*/
    /* Write system trace to indicate start of command */
    /* to adapter */

    DDHKWD5(HKWD_DD_IDEDD, DD_ENTRY_BSTART, 0, ap->devno,
            bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
            bp->bufstruct.b_bcount);

    ap->dma_pending = 0;     /* no DMA active (yet) */
    /* set default sector length for transfers */
    dev_ptr->sector_len = 512;

    /* check command to see what next state should be */
    switch (bp->ata.command)
    {
    case ATA_ATAPI_PACKET_COMMAND:
      dev_ptr->cmd_activity_state = EXPECT_ATAPI_CMD;
      dev_ptr->sector_len = bp->ata.startblk.chs.cyl_lo +
                           (bp->ata.startblk.chs.cyl_hi << 8);
      if ((bp->bufstruct.b_bcount) &&
          ((dev_ptr->sector_len == 0) || (dev_ptr->sector_len > MAXSECTOR)))
      {
        if (dev_ptr->sector_len == 0)
        {
          DEBUG_0("lide_start: WARNING!! ATAPI sector length not set!\n")
          dev_ptr->sector_len = 512;
        }
        else
        {
          DEBUG_1("lide_start: WARNING!! ATAPI sector length 0x%x too long!\n",
                  dev_ptr->sector_len)
          dev_ptr->sector_len = MAXSECTOR;
        }
        bp->ata.startblk.chs.cyl_lo = dev_ptr->sector_len & 0xFF;
        bp->ata.startblk.chs.cyl_hi = (dev_ptr->sector_len & 0x0ff00)
                                      >> 8;
      }

      switch(bp->ata.atapi.packet.op_code)
      {
/*
      case ATAPI_INQUIRY:
      case ATAPI_LOG_SENSE:
      case ATAPI_MODE_SENSE:
      case ATAPI_MODE_SENSE_10:
*/
      case ATAPI_READ_6:
      case ATAPI_READ_10:
      case ATAPI_READ_12:
      case ATAPI_READ_CAPACITY:
      case ATAPI_READ_CD:
      case ATAPI_READ_CD_MSF:
      case ATAPI_READ_HEADER:
      case ATAPI_READ_POSITION:
      case ATAPI_READ_SUBCHANNEL:
      case ATAPI_READ_TOC:
/*
      case ATAPI_REQUEST_SENSE:
*/
        if (dev_ptr->dma_supported)
        {
          dma_reqd = READ_DMA_REQD;    /* dma read required */
          bp->ata.feature |= 0x01;     /* set DMA bit in feature register */
        }
        else
          bp->ata.feature &= 0xFE;     /* clr DMA bit in feature register */
        break;
      case ATAPI_WRITE:
      case ATAPI_WRITE_BUFFER:
        if (dev_ptr->dma_supported)
        {
          /* DMA to device uses user buffer(s) */
          dma_reqd = WRITE_DMA_REQD;   /* dma WRITE required */
          bp->ata.feature |= 0x01;     /* set DMA bit in feature register */
        }
	else
          bp->ata.feature &= 0xFE;     /* clr DMA bit in feature register */
        break;
      default:
        break;
      }
      break;
    case ATA_ATAPI_IDENTIFY_DEVICE:
    case ATA_IDENTIFY_DEVICE:
    case ATA_READ_BUFFER:
    case ATA_READ_LONG:
    case ATA_READ_LONG_RETRY:
    case ATA_READ_SECTOR:
    case ATA_READ_SECTOR_RETRY:
    case ATA_READ_MULTIPLE:
      if ((bp->ata.command == ATA_READ_LONG) ||
          (bp->ata.command == ATA_READ_LONG_RETRY))
      {
        dev_ptr->sector_len = 516;
      }
      dev_ptr->cmd_activity_state = EXPECT_DATAIN;
      break;
    case ATA_DOWNLOAD_MICROCODE:
    case ATA_FORMAT_TRACK:
    case ATA_WRITE_BUFFER:
    case ATA_WRITE_LONG:
    case ATA_WRITE_LONG_RETRY:
    case ATA_WRITE_MULTIPLE:
    case ATA_WRITE_SECTOR:
    case ATA_WRITE_SECTOR_RETRY:
    case ATA_WRITE_SAME:
    case ATA_WRITE_VERIFY:
      if ((bp->ata.command == ATA_WRITE_LONG) ||
          (bp->ata.command == ATA_WRITE_LONG_RETRY))
      {
        dev_ptr->sector_len = 516;
      }
      dev_ptr->cmd_activity_state = EXPECT_DATAOUT;

      break;
    case ATA_ATAPI_SOFT_RESET:
      dev_ptr->cmd_activity_state = DEVICE_NOT_ACTIVE;
      break;
    case ATA_WRITE_DMA:
    case ATA_WRITE_DMA_RETRY:
    case ATA_READ_DMA:
    case ATA_READ_DMA_RETRY:
      /* DMA to device uses user buffer(s) */
      if ((bp->ata.command == ATA_WRITE_DMA) ||
          (bp->ata.command == ATA_WRITE_DMA_RETRY))
      {
        dma_reqd = WRITE_DMA_REQD;  /* dma write needed */
      }
      else
        dma_reqd = READ_DMA_REQD;   /* dma read needed  */
      dev_ptr->cmd_activity_state = EXPECT_STATUS;
      break;
    default:
      dev_ptr->cmd_activity_state = EXPECT_STATUS;
    }

    /* set up DMA registers (if required) */
    if (dma_reqd != NO_DMA_REQD)
    {
      /* DMA to/from device uses user buffer(s) */
      if (bp->bp == NULL) {		/* NOT spanned command */

        ap->dio.used_iovecs = 1;
        ap->dio.dvec->iov_base = bp->bufstruct.b_un.b_addr;
        ap->dio.dvec->iov_len = bp->bufstruct.b_bcount;
        ap->dio.dvec->xmp = &bp->bufstruct.b_xmemd;

      } else {

        /* bp points to the total request, must start with 2nd buf */
        next_bp = bp->bp;
        cur_dvec = ap->dio.dvec;
        ap->dio.used_iovecs = 0;

        while (next_bp && ap->dio.used_iovecs < 33) {
	  ap->dio.used_iovecs++;
	  cur_dvec->iov_base = next_bp->b_un.b_addr;
	  cur_dvec->iov_len  = next_bp->b_bcount;
	  cur_dvec->xmp      = &next_bp->b_xmemd;
	  cur_dvec++;
          next_bp=next_bp->av_forw;
        }
      }

      /* set dma flags */
      if ((dev_ptr->dma_cur_mode & 0x0030) == 0x0010)
      {  /* single word DMA transfer */
        dma_flags = CH_SINGLE | CH_ADDR_INC | CH_EOP_OUTPUT | CH_16BIT_BYTES;
        switch (dev_ptr->dma_cur_mode & 0x000F) {
	case 0x0003:
	case 0x0002:
          dma_flags |= CH_TYPE_F;
	  break;
	case 0x0001:
          dma_flags |= CH_TYPE_B;
	  break;
        default:
          dma_flags |= CH_COMPAT;
        }
      }
      else
      {  /* multi word DMA transfer */
        dma_flags = CH_ADDR_INC | CH_EOP_OUTPUT | CH_16BIT_BYTES;
        switch (dev_ptr->dma_cur_mode & 0x000F) {
	case 0x0003:
	case 0x0002:
	case 0x0001:
          dma_flags |= CH_TYPE_F;
	  break;
        default:
          dma_flags |= CH_TYPE_B;
        }
      }

      if (dma_reqd == WRITE_DMA_REQD)    /* if a DMA write is requested */
      {
        dma_rc = D_MAP_SLAVE(ap->handle, 0, dev_ptr->sector_len,
                             &ap->dio, dma_flags);
      }
      else                  /* it's a DMA read operation   */
      {
        dma_rc = D_MAP_SLAVE(ap->handle, DMA_READ,dev_ptr->sector_len,
                             &ap->dio, dma_flags);
      }

      /* If D_MAP_SLAVE failed to map the entire buffer, but was able */
      /* map at least part of the buffer, then pretend it was OK, and */
      /* just transfer the part that we were able to map.             */
      if ((dma_rc == DMA_NORES) && (ap->dio.bytes_done))
      {
	ap->dma_pending |= LIDE_DMA_NORES;
	bp->ata.sector_cnt_cmd = ap->dio.bytes_done / dev_ptr->sector_len;
	dma_rc = DMA_SUCC;
      }

      if (dma_rc != DMA_SUCC)
      {
	DEBUG_1 (" lide_start DMA_ERROR dma_rc = 0x%x\n", dma_rc)
	lide_logerr(ap, ERRID_ATAIDE_ERR2,
	            DMA_ERROR, 0, dma_rc, dev_ptr, TRUE);
	ap->dma_pending = 0;
	dev_ptr->cmd_pend = NULL;
	dev_ptr->cmd_activity_state = DEVICE_NOT_ACTIVE;
	bp->status_validity = ATA_IDE_STATUS | ATA_ERROR_VALID;
	bp->ata.status = ata_status;
	bp->ata.errval = ATA_ABORTED_CMD;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_error = EIO;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	if (ap->dump_started == 0)
	  iodone((struct buf *) bp);
	return;
      }
      ap->dma_pending |= LIDE_DMA_RUNNING;
    }
    else  /* a PIO operation is required */
    {
      if (bp->bp == NULL) {		/* NOT spanned command */
        ap->ubuf_ptr = &bp->bufstruct;
      } else {
        ap->ubuf_ptr = bp->bp;
      }
      ap->ubuf_ix = 0;
    }

    i=0;
    while (((ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE)) &
            ATA_BUSY))
    {

      if(i++ > 5)          /* if it's been more than 5 seconds*/
      {
	DEBUG_1 (" lide_start BUSY! ata_status = 0x%x\n", ata_status)
	if (ap->dump_started == 0)
	lide_logerr(ap, ERRID_ATAIDE_ERR3, UNKNOWN_CARD_ERR,
		    0, 0, dev_ptr, TRUE);
	(void)lide_reconfig_adapter(ap);
	dev_ptr->cmd_pend = NULL;
	dev_ptr->cmd_activity_state = DEVICE_NOT_ACTIVE;
	bp->status_validity = ATA_IDE_STATUS | ATA_ERROR_VALID;
	bp->ata.status = ata_status;
	bp->ata.errval = ATA_ABORTED_CMD;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_error = EIO;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	if (ap->dump_started == 0)
	  iodone((struct buf *) bp);
	return;
      }
      lide_delay(ap, 1, 0); /* delay for 1 sec */
    }

    /* access to the desired device's register set */
    LIDE_WRITE_ATA(ap, LIDRVH, LIDRVH_SIZE, 
                      (0xA0 | ((bp->ata.device & 0x01) << 4)) ); 

    /* put all but command register into ATA Task file */
    LIDE_WRITE_ATA(ap, LIFEAT, LIFEAT_SIZE, bp->ata.feature);
    LIDE_WRITE_ATA(ap, LISECC, LISECC_SIZE, bp->ata.sector_cnt_cmd);
    LIDE_WRITE_ATA(ap, LISECN, LISECN_SIZE,
                   bp->ata.startblk.chs.sector);
    LIDE_WRITE_ATA(ap, LICYLL, LICYLL_SIZE,
                   bp->ata.startblk.chs.cyl_lo);
    LIDE_WRITE_ATA(ap, LICYLH, LICYLH_SIZE,
                   bp->ata.startblk.chs.cyl_hi);
    head_reg = (0xA0 | ((bp->ata.device & 0x01) << 4) |
                (bp->ata.startblk.chs.head & 0x0F));
    if (bp->ata.flags == ATA_LBA_MODE)
    {
      head_reg |= 0x40;   /* set LBA mode bit in DEV Head reg. */
    }
    LIDE_WRITE_ATA(ap, LIDRVH, LIDRVH_SIZE, head_reg);

    /* set active_device to indicate this device */
    ap->active_device = bp->ata.device;

    /* make sure interrupts are enabled */
    if (ap->dump_started == 0) {
    	LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x08);
    }

    /* put command into command register */
    LIDE_WRITE_ATA(ap, LICMD, LICMD_SIZE, bp->ata.command);

    /* turn on the disk activity light (if supported) */
    if (ap->light_supported)
      (void)write_operator_panel_light(1);

    /* Some ATAPI devices generate an interrupt to indicate */
    /* that they're ready for the command packet, others do */
    /* not.  Check to see if we need to wait for interrupt  */
    /* or just do short delay.                              */

    if ((bp->ata.command == ATA_ATAPI_PACKET_COMMAND) &&
        (dev_ptr->atapi_cmd_drq != IDE_CMD_DRQ_INTR))
    {

      if (dev_ptr->atapi_cmd_drq == IDE_CMD_DRQ_50MICS)
        lide_delay(ap, 0, 50000);  /* delay for 50 microsecs */
      else
        lide_delay(ap, 0, 3000000); /* delay for 3 millisecs */
      ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);
      atapi_reason = LIDE_READ_ATA(ap, LISECC, LISECC_SIZE);
      DEBUG_1("atapi command = 0x%x\n", bp->ata.atapi.packet.op_code);
      DEBUG_2("  after atapi command out, ata_status = 0x%x, reason = 0x%x\n",
              ata_status, atapi_reason)
                

      /* now write ATAPI command packet & set EXPECT_DATAIN */
      if (((ata_status & (ATA_BUSY | ATA_DATA_REQUEST )) ==
          ATA_DATA_REQUEST) && (atapi_reason == 0x01))
      {
        DEBUG_2(" ATAPI status = 0x%02x, Intr. Reason = 0x%x\n",
                ata_status, LIDE_READ_ATA(ap, LISECC, LISECC_SIZE))
        DEBUG_2("       Byte count(h/l) = 0x%02x, 0x%02x\n",
                LIDE_READ_ATA(ap, LICYLH, LICYLH_SIZE),
                LIDE_READ_ATA(ap, LICYLL, LICYLL_SIZE));
        sptr = (ushort *)&bp->ata.atapi.packet;
        for(i=0; i < bp->ata.atapi.length; i += 2)
        {
          LIDE_WRITE_ATA(ap, LIDATA, LIDATA_SIZE, *sptr++);
        }
      }
      else
      {
        DEBUG_1("ERROR ERROR atapi didn't start!!! ata_status=0X%x",
                 ata_status);  
	lide_logerr(ap, ERRID_ATAIDE_ERR3, UNKNOWN_CARD_ERR,
		    1, 0, dev_ptr, TRUE);
      }

    }

    /* if we're doing an ATA write, then go ahead and transfer the */
    /* first sector of data so that we can get an interrupt */
    if (dev_ptr->cmd_activity_state == EXPECT_DATAOUT)
    {
      /* check the alternate status reg. */
      ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);

      /* wait for BUSY to go off */
      i=0;
      while (ata_status & ATA_BUSY)  /* what about the error bit? */
      {
        if (i++ > 1000) /* if it's been more than 1 second total */
        {
          DEBUG_1 ("ERROR ERROR write didn't start!!! ata_status=0X%x",
                   ata_status);  
          break;
        }
        lide_delay(ap, 0, 1000000);  /* wait 1 millisecond */
        ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);
      }

      if (((ata_status & (ATA_BUSY | ATA_DATA_REQUEST)) == 
            ATA_DATA_REQUEST)                               &&
           (dev_ptr->bytes_moved < bp->bufstruct.b_bcount))
      {
          /* write a sector */
          o_run_flag = FALSE;               /* assume no data overruns  */

          if ( (dev_ptr->bytes_moved + dev_ptr->sector_len) <=
              bp->bufstruct.b_bcount) {
            x_len = dev_ptr->sector_len;      /* all bytes into buffer */
            o_run = 0;                        /* no overrun bytes     */
          } else {             /* overrun situation */
            /* remaining bytes */
            x_len = bp->bufstruct.b_bcount - dev_ptr->bytes_moved;
            /* overrun bytes */
            o_run = (dev_ptr->bytes_moved + dev_ptr->sector_len) -
                     bp->bufstruct.b_bcount;
          }

          sptr = (ushort *)&ap->xfer_area[0];

          /* copy data from user buffer to local dd xfer_area */
          lide_spanned_xmem_xfer(ap, bp);

          /* disable our interupts while reading data register */
          /* to prevent our interrupts killing the read        */
          old_level = i_disable(ap->ddi.adap.int_prior);

          io = (volatile char *)iomem_att(&ap->iom) + ap->base_addr0 + LIDATA;

          if (dev_ptr->iordy_enabled) {
              /* carefree transfer of bytes from the xfer buffer */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ;
                   sptr < end_xfer ;
                   sptr++) {
                *((volatile short *)(io)) = *sptr;
              }
          } else {
              /* no flow control support, must delay between writes */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ;
                   sptr < end_xfer ;
                   sptr++) {
                *((volatile short *)(io)) = *sptr;
                lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
          }
          if (o_run) {                 /* throw away any overrun words */
              o_run_flag = TRUE;
              DEBUG_1("  DATA OVERRUN - ata_status = 0x%x\n",ata_status)

              for ( ; o_run > 0 ; o_run -= 2) {
                *((volatile short *)(io)) = 0;
                if ( !(dev_ptr->iordy_enabled) )
                   lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
          }
          iomem_det((void *)io);
          i_enable(old_level);

          dev_ptr->bytes_moved += x_len;

	  if (o_run_flag)
	  {
              lide_logerr(ap, ERRID_ATAIDE_ERR3, PIO_WR_OTHR_ERR,
	                  2, 0, dev_ptr, TRUE);
              ASSERT(0);
	  }
      } /* end if */
    } /* end if ATA write */

    if (!(ap->dump_started))
    {
      /* Start the watchdog timer for this command */
      if (bp->timeout_value == 0)
      {
         DEBUG_1("lide_start: WARNING! timeout of 0 is defaulting to 0x%x\n",
                  IPL_MAX_SECS);
         dev_ptr->dev_watchdog.dog.restart = (ulong) IPL_MAX_SECS;
      }
      else
         dev_ptr->dev_watchdog.dog.restart = (ulong)bp->timeout_value + 1;
      w_start(&(dev_ptr->dev_watchdog.dog));

      /* Update the power mgmt. activity field */
      ap->pmh.activity = PM_ACTIVITY_OCCURRED;
    }

    DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_BSTART, 0, ap->devno);

    TRACE_1 ("out strt", (int) dev_ptr)
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_stop                                               */
/*                                                                      */
/* FUNCTION: ATA/IDE Device Driver Stop Command Routine                 */
/*                                                                      */
/*      This routine is called from the interrupt or a watchdog routine */
/*      when a command completes, and needs to be removed from the      */
/*      active cmd_pend queue, its' resources deallocated, and returned */
/*      to its' strategy routine.                                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process or interrupt       */
/*      handler.  It can page fault only if called under a process      */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      ataide_buf  - input/output request struct used between the      */
/*                adapter driver and the calling ATA/ATAPI device driver*/
/*                                                                      */
/* INPUTS:                                                              */
/*      ap - pointer to an adapter_def structure                        */
/*      dev_index - index to device information                         */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      NONE                                                            */
/*                                                                      */
/* ERROR DESCRIPTION                                                    */
/*      NONE                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_start                                                         */
/*                                                                      */
/************************************************************************/
void
lide_stop(
          struct adapter_def * ap,
	  struct dev_info * dev_ptr)
{
    struct ataide_buf *bp;
    int rc_dma;
#ifdef DEBUG
    uchar head_reg;
#endif

    bp = dev_ptr->cmd_pend;
    bp->ata.errval = (uchar)LIDE_READ_ATA(ap, LIERR, LIERR_SIZE);
    bp->ata.sector_cnt_ret = (uchar) LIDE_READ_ATA(ap, LISECC, LISECC_SIZE);
    bp->ata.endblk.chs.sector=(uchar)LIDE_READ_ATA(ap, LISECN, LISECN_SIZE);
    bp->ata.endblk.chs.cyl_lo=(uchar)LIDE_READ_ATA(ap, LICYLL, LICYLL_SIZE);
    bp->ata.endblk.chs.cyl_hi=(uchar)LIDE_READ_ATA(ap, LICYLH, LICYLH_SIZE);
    bp->ata.endblk.chs.head = (uchar)LIDE_READ_ATA(ap, LIDRVH, LIDRVH_SIZE);

    bp->status_validity = ATA_IDE_STATUS;
    if (ap->dma_pending)
    {
      rc_dma = D_UNMAP_SLAVE(ap->handle);
    }
    else
    {
      rc_dma = 0;
    }
    if((bp->ata.status & ATA_ERROR) || (bp->ata.status == 0) || (rc_dma < 0))
    { /* an error occured */
      bp->bufstruct.b_flags |= B_ERROR;
      if (dev_ptr->cmd_activity_state == COMMAND_TIMED_OUT)
      {
        bp->bufstruct.b_error = ETIMEDOUT;
      }
      else
        bp->bufstruct.b_error = EIO;
      if (bp->ata.status & ATA_ERROR)
        bp->status_validity |= ATA_ERROR_VALID;
#ifdef DEBUG
      head_reg = (0xA0 | ((bp->ata.device & 0x01) << 4) |
                  (bp->ata.startblk.chs.head & 0x0F));
      if (bp->ata.flags == ATA_LBA_MODE)
      {
        head_reg |= 0x40;   /* set LBA mode bit in DEV Head reg. */
      }
      DEBUG_3 ("lide_start cmd=%x Fea=%x SecC=%x",
               bp->ata.command, bp->ata.feature, bp->ata.sector_cnt_cmd)
      DEBUG_4(" DevH=%x CH=%x CL=%x SecN=%x\n",
              head_reg, bp->ata.startblk.chs.cyl_hi,
              bp->ata.startblk.chs.cyl_lo, bp->ata.startblk.chs.sector)
      DEBUG_3("  lide_stop status=%x err=%x SecCt=%x",
            bp->ata.status, bp->ata.errval, bp->ata.sector_cnt_ret)
      DEBUG_4(" DevHd=%x CH=%x CL=%x SecN=%x\n",
              bp->ata.endblk.chs.head, bp->ata.endblk.chs.cyl_hi,
              bp->ata.endblk.chs.cyl_lo, bp->ata.endblk.chs.sector);
      DEBUG_2("  lide_stop: dma_pending =0x%x, rc_dma = 0x%x\n",ap->dma_pending,
              rc_dma)
#endif
    }
    if (ap->dma_pending)
    {
      if (rc_dma == 0)
      {
         dev_ptr->bytes_moved = ap->dio.bytes_done;
	 /* check to see if all of the area was mapped on this DMA xfer  */
         /* if not, then indicate so to head driver so it gets remainder */
         if (ap->dma_pending & LIDE_DMA_NORES)
	   bp->status_validity |= ATA_IDE_DMA_NORES;
      }
      else
      {
         lide_logerr(ap, ERRID_ATAIDE_ERR2,
	             DMA_ERROR, 1, rc_dma, dev_ptr, TRUE);
      }
      ap->dma_pending = 0;
    }
    bp->bufstruct.b_resid = bp->bufstruct.b_bcount - dev_ptr->bytes_moved;

    dev_ptr->bytes_moved = 0;
    dev_ptr->cmd_activity_state = DEVICE_NOT_ACTIVE;
    dev_ptr->cmd_pend = NULL;
    if (ap->light_supported)
      (void)write_operator_panel_light(0);
    if (ap->dump_started == 0)
      iodone((struct buf *) bp);
    TRACE_1 ("out stop", (int) dev_ptr)
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        lide_iodone                                             */
/*                                                                      */
/* FUNCTION:    Adapter Driver Iodone Routine                           */
/*                                                                      */
/*      This routine handles completion of commands initiated through   */
/*      the lide_ioctl routine.                                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the IODONE interrupt level, so it can      */
/*      be interrupted by the interrupt handler.                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      ataide_buf  - input/output request struct used between the      */
/*                adapter driver and the calling ATA/ATAPI device driver*/
/*                                                                      */
/* INPUTS:                                                              */
/*      bp      - pointer to the passed ataide_buf                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      e_wakeup                                                        */
/*                                                                      */
/************************************************************************/
int
lide_iodone(struct ataide_buf * bp)
{
    TRACE_1 ("iodone  ", (int) bp)
    e_wakeup(&bp->bufstruct.b_event);
}

/**************************************************************************/
/*                                                                        */
/* NAME: lide_dump                                                        */
/*                                                                        */
/* FUNCTION: Determine what type of dump operation is being sought        */
/*                                                                        */
/*      Allocate necessary segment registers and parse type of            */
/*      dump operation.  Call the specified routine.                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/*      This routine must be called in the interrupt environment.  It     */
/*      can not page fault and is pinned.                                 */
/*                                                                        */
/* (NOTES:) This routine handles the following operations :               */
/*      DUMPINIT   - initializes IDE attached disk as dump device         */
/*      DUMPSTART  - prepares device for dump                             */
/*      DUMPQUERY  - returns the maximum and minimum number of bytes that */
/*                   can be transferred in a single DUMPWRITE command     */
/*      DUMPWRITE  - performs write to disk                               */
/*      DUMPEND    - cleans up device on end of dump                      */
/*      DUMPTERM   - terminates the bus attached disk as dump device      */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*      ataide_buf  - input/output request struct used between the        */
/*                adapter driver and the calling ATA/ATAPI device driver  */
/*      uio         - structure containing information about the data to  */
/*                transfer                                                */
/*      dev_info    - device info structure                               */
/*      dmp_query   - queried transfer information is returned            */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - device major/minor number                               */
/*      uiop    - pointer to uio structure for data for the               */
/*                specified operation code                                */
/*      cmd     - Type of dump operation being requested                  */
/*      arg     - Pointer to dump query structure or buf_struct for cmd   */
/*      chan    - unused                                                  */
/*      ext     - 0 for system dump, or PM_DUMP                           */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/*     lide_dumpstrt                                                      */
/*     lide_dumpwrt                                                       */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/*                                                                        */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned */
/*      and the caller is left responsible to recover from the error.     */
/*                                                                        */
/* RETURNS:                                                               */
/*      EBUSY     -  request sense inuse                                  */
/*      EINVAL    -  Invalid iovec argumenmt,or invalid cmd               */
/*      EIO       -  I/O error, quiesce or scsi writefailed.              */
/*      ENOMEM    -  Unable to allocate resources                         */
/*      ENXIO     -  Not inited as dump device                            */
/*      ETIMEDOUT - adapter not responding                                */
/*                                                                        */
/**************************************************************************/
int
lide_dump(
  	  dev_t devno,
	  struct uio * uiop,
	  int cmd,
	  int arg,
	  int chan,
	  int ext)
{
    /****************************/
    /* Misc. Variables       */
    /****************************/
    struct ataide_buf *bp;
    struct dmp_query *dmp_qry_ptr;
    struct adapter_def *ap;
    int     i;
    int     ret_code;
    int     old_level;
    /****************************/
    DEBUG_0 ("Entering lide_dump\n");
    TRACE_1 ("in lide_dump", 0);

    /* check to make sure adapter is inited and opened */
    ret_code = LIDE_NO_ERR;
    /* see if adapter is valid */
    for (i = 0; i < MAX_ADAPTERS; i++)
    {
      if ((adp_str[i].devno == devno) &&
          (adp_str[i].inited) && (adp_str[i].opened))
        break;
    }
    if (i != MAX_ADAPTERS)
      ap = &adp_str[i];
    else
      return (ENXIO);

    switch (cmd)
    {
    case DUMPINIT:
	ap->dump_inited = TRUE;
	ap->dump_started = FALSE;
	TRACE_2 ("DUMPINIT", (int) &arg, ext)
	break;
    case DUMPQUERY:
	dmp_qry_ptr = (struct dmp_query *) arg;
	dmp_qry_ptr->min_tsize = 512;
	dmp_qry_ptr->max_tsize = ap->max_request;
	TRACE_2 ("DUMPQUERY", (int) &arg, ext)
	break;
    case DUMPSTART:
	TRACE_2 ("DUMPSTART", (int) &arg, ext)
	lide_dumpstrt(ap, arg, ext);
	break;
    case DUMPWRITE:
	old_level = i_disable(INTMAX);
	bp = (struct ataide_buf *) arg;
	TRACE_2 ("DUMPWRITE", (int) &arg, ext)
	ret_code = lide_dumpwrt(ap, bp, ext);
	TRACE_1 ("rc_dmpwrt", ret_code)
	/* enable interrupts */
	i_enable(old_level);
	break;
    case DUMPEND:
	TRACE_2 ("DUMPEND", (int) &arg, ext)
        if (!ap->dump_started)
	    ret_code = EINVAL;
	else
	    ap->dump_started = FALSE;
	break;
    case DUMPTERM:
	TRACE_2 ("DUMPTERM", (int) &arg, ext)
	if (!ap->dump_inited)
	    ret_code = EINVAL;
	else
            ap->dump_inited = FALSE;
	break;
    default:
	ret_code = EINVAL;
    }

    DEBUG_0 ("Leaving lide_dump\n")
    TRACE_1 ("out lide_dump", ret_code)
    return (ret_code);
}  /* end lide_dump */

/**************************************************************************/
/*                                                                        */
/* NAME: lide_dumpstrt                                                    */
/*                                                                        */
/* FUNCTION: Notify device of pending dump                                */
/*                                                                        */
/*        Prepare device for dump, fail existing commands on all devices  */
/*        including the dump device. Set up the device queue to accept    */
/*        dump commands.                                                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/*      This routine must be called in the interrupt environment.  It     */
/*      can not page fault and is pinned                                  */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*      ataide_buf  - input/output request struct used between the        */
/*                adapter driver and the calling ATA/ATAPI device driver  */
/*      dev_info    - device info structure                               */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap       -  adapter def structure for current adapter             */
/*                                                                        */
/* CALLED BY:                                                             */
/*      lide_dump                                                         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned */
/*      and the caller is left responsible to recover from the error.     */
/*                                                                        */
/* RETURNS:                                                               */
/*      EIO       -  Failed quiesce or scsi write                         */
/*      ENOMEM    -  Unable to allocate resources                         */
/*      ETIMEDOUT - adapter not responding                                */
/*                                                                        */
/**************************************************************************/
int
lide_dumpstrt(
              struct adapter_def * ap,
	      int    arg,
	      int    ext)
{
    struct dev_info *dev_ptr;
    struct ataide_buf *bp;
    int dev_index, ret_code;

    DEBUG_0 ("Entering lide_dumpstrt\n")
    TRACE_2 ("in dumpstrt", (int) ap, ext)
    ret_code = 0;

    if (!ap->dump_inited)
	return (EINVAL);

    if (((ext == PM_DUMP) && (ap->dump_started & LIDE_PM_DMP)) ||
	((ext != PM_DUMP) && (ap->dump_started & LIDE_SYS_DMP))) {
        /* dump already started, so return success */
	return (ret_code);
    }

    /* Make sure we can access the adapter */
    if (ext == PM_DUMP) {
        /* only in PM_DUMP if already in hibernate */
        ap->dump_started |= LIDE_PM_DMP;
        /* it's PM, so turn the chip on, but don't change PM state */
	lide_turn_chip_on(ap, FALSE);
    } else { /* it's a normal (non-PM) system dump */
        ap->dump_started |= LIDE_SYS_DMP;
	if ((ap->pmh.mode != PM_DEVICE_FULL_ON) &&
	    (ap->pmh.mode != PM_DEVICE_ENABLE)) {
	  lide_turn_chip_on(ap, TRUE);
	}
    }

    /* abort all commands on all devices attached to this adapter */
    for (dev_index = 0; dev_index < NO_DEV_ACTIVE; dev_index++)
    {
      dev_ptr = ap->device_queue_hash[dev_index];
      if ((dev_ptr != NULL) && (dev_ptr->opened == TRUE)) {
	bp = dev_ptr->cmd_pend;
	if (bp != NULL)
	{
	  if (ap->active_device == dev_index)
	  { /* this command is running, so abort it */
            ap->active_device = NO_DEV_ACTIVE;
	    if (bp->ata.command == ATA_ATAPI_PACKET_COMMAND)
	    { /* it's an ATAPI command, so use atapi reset sequence */
	      /* get access to the desired device's register set */
	      LIDE_WRITE_ATA(ap, LIDRVH, LIDRVH_SIZE, 
			     (0xE0 | ((dev_index & 0x01) << 4)) ); 

	      /* write ATAPI SOFT RESET command to command register */
	      LIDE_WRITE_ATA(ap, LICMD, LICMD_SIZE, ATA_ATAPI_SOFT_RESET);
	    }
	    else
	    { /* not an ATAPI command, so just issue adapter reset*/
	      LIDE_WRITE_ATA(ap, LICMD, LICMD_SIZE, ATA_ATAPI_SOFT_RESET);
	      lide_chip_register_reset(ap);
	    }
	    if (ap->dma_pending)  /* was it a dma command? */
	    { /* then clean up the dma regs. */
	      ret_code = D_UNMAP_SLAVE(ap->handle);
	    }
	    if (ap->light_supported)
	      (void)write_operator_panel_light(0);
	  }
	  /* now just remove command from queue.  There is no need to   */
	  /* perform 'iodone' actions since, during a dump, the calling */
	  /* device is already dead.                                    */
	  dev_ptr->cmd_pend = NULL;
	  dev_ptr->cmd_activity_state = DEVICE_NOT_ACTIVE;
	}
      }

      ret_code = 0;
    }

    DEBUG_0 ("Leaving lide_dumpstrt\n")
    TRACE_1 ("out dumpstrt", ret_code)
    return (ret_code);
}  /* end lide_dumpstrt */

/**************************************************************************/
/*                                                                        */
/* NAME: lide_dump_intr                                                   */
/*                                                                        */
/* FUNCTION: Handle polling for interrupts for dump routines              */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/*      This routine may be call in the interrupt or process environment. */
/*      It can not page fault and is pinned.                              */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*      ataide_buf  - input/output request struct used between the        */
/*                adapter driver and the calling ATA/ATAPI device driver  */
/*      dev_info    - device info structure                               */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/*                                                                        */
/* CALLED BY:                                                             */
/*      psc_dumpwrt, psc_dumpstrt                                         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      lide_delay        	    lide_xfer_func      LIDE_READ_ATA     */
/*                                                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* RETURNS:                                                               */
/*    EIO - register failures                                             */
/*    ETIMEDOUT - command time outs                                       */
/*                                                                        */
/**************************************************************************/
int
lide_dump_intr(
               struct adapter_def * ap,
               struct dev_info * dev_ptr)

{
    register int ata_status;
    int ret_code, loop;
    int time_out_value;

    DEBUG_0 ("Entering lide_dump_intr\n")
    TRACE_1 ("in dumpintr", abort_chip)
    ret_code = 0;
    loop = TRUE;
    while (loop)
    {
	/* loop until DMA is finished */
	while ( 
		( (ata_status=LIDE_READ_ATA(ap, LISTA, LISTA_SIZE)) & ATA_BUSY )
		|| (ap->dma_pending && (ata_status & ATA_DATA_REQUEST)) 
	      )
	{
	}

        if (dev_ptr->cmd_activity_state &
              (EXPECT_DATAIN | EXPECT_DATAOUT | EXPECT_STATUS |
               EXPECT_ATAPI_CMD))
        {
	    /* call the lide_xfer_func to handle the adapter data movement */
	    lide_xfer_func(ap);
        }
        else if (dev_ptr->cmd_activity_state == DEVICE_NOT_ACTIVE)
        {
            loop = FALSE;
        }
        else
        {
            DEBUG_1("   lide_dump_intr: unknown state = 0x%x\n",
                    dev_ptr->cmd_activity_state)
            ret_code = EIO;
            break;
	}
    }	/* while */
    DEBUG_0 ("Leaving lide_dumpintr\n")
    TRACE_1 ("out dintr", ret_code)
    return (ret_code);
}  /* end lide_dump_intr */

/*************************************************************************/
/*                                                                       */
/* NAME: lide_dumpwrt                                                    */
/*                                                                       */
/* FUNCTION: Write to the dump device.                                   */
/*                                                                       */
/*      Implements the DUMPWRITE function, which is used to dump data    */
/*      blocks to the Dump device.  This function receives the data      */
/*      packaged as a standard ataide_buf.  The command is begun using   */
/*      'lide_start', and is completed by calling the 'lide_dump_intr'   */
/*      routine.                                                         */
/*                                                                       */
/* EXECUTION ENVIRONMENT:                                                */
/*                                                                       */
/*      This routine can  be called in the interrupt environment.  It    */
/*      can not page fault and is pinned.                                */
/*                                                                       */
/* DATA STRUCTURES:                                                      */
/*      adapter_def - ATA Task File (register) unique adapter structure  */
/*      ataide_buf  - input/output request struct used between the       */
/*                adapter driver and the calling ATA/ATAPI device driver */
/*      dev_info    - device info structure                              */
/*                                                                       */
/* INPUTS:                                                               */
/*      ap       -  pointer to an adapter_def structure                  */
/*      bp       -  buf scruct for the pending dump command              */
/*                                                                       */
/* CALLED BY:                                                            */
/*      lide_dump                                                        */
/*                                                                       */
/* INTERNAL PROCEDURES CALLED:                                           */
/*      lide_dump_intr                                                   */
/*                                                                       */
/* EXTERNAL PROCEDURES CALLED:                                           */
/*                                                                       */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is return- */
/*      ed and the caller is left responsible to recover from the error. */
/*                                                                       */
/* RETURNS:                                                              */
/*      EBUSY     -  request sense inuse                                 */
/*      EINVAL    -  Invalid iovec argument                              */
/*      EIO       -  I/O error                                           */
/*      ENOMEM    -  Unable to allocate resources                        */
/*      ENXIO     -  Not inited as dump device                           */
/*      ETIMEDOUT - timed out because no interrupt                       */
/*                                                                       */
/*************************************************************************/
int
lide_dumpwrt(
             struct adapter_def * ap,
	     struct ataide_buf * bp,
	     int    ext)
{
    /**********************/
    /* Misc. Variables    */
    /**********************/
    struct dev_info *dev_ptr;
    int     ret_code;

    DEBUG_0 ("Entering lide_dumpwrt\n")
    TRACE_1 ("in dumpwrt", 0)

    if ((!ap->dump_inited) || (!ap->dump_started))
    {
	return (EINVAL);
    }
    dev_ptr = ap->device_queue_hash[bp->ata.device];
    if ((dev_ptr == NULL) || (dev_ptr->opened == FALSE))
    {
	return (ENXIO);
    }
    dev_ptr->cmd_pend = bp;
    lide_start(ap, dev_ptr);
    if (ret_code = bp->bufstruct.b_error)
	return (ret_code);
    ret_code = lide_dump_intr(ap, dev_ptr);

    DEBUG_0 ("Leaving lide_dumpwrt\n")
    TRACE_1 ("out dumpwrt", 0)
    return (ret_code);
}  /* end lide_dumpwrt */

/************************************************************************/
/*                                                                      */
/* NAME:        lide_cdt_func                                           */
/*                                                                      */
/* FUNCTION:    Adapter Driver Component Dump Table Routine             */
/*                                                                      */
/*      This routine builds the driver dump table during a system dump. */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the interrupt level, so it cannot malloc   */
/*      or free memory.                                                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      cdt     - the structure used in the master dump table to keep   */
/*                track of component dump entries.                      */
/*                                                                      */
/* INPUTS:                                                              */
/*      arg     - =1 dump dd is starting to get dump table entries.     */
/*                =2 dump dd is finished getting the dump table entries.*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      Return code is a pointer to the struct cdt to be dumped for     */
/*      this component.                                                 */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      bcopy           bzero                                           */
/*      strcpy                                                          */
/*                                                                      */
/************************************************************************/
struct cdt *
lide_cdt_func(
	      int arg)
{
    int i;

    DEBUG_0 ("Entering lide_cdt_func routine.\n");

    if (arg == 1)
    {
	/* only build the dump table on the initial dump call */

	/* init the table */
	bzero((char *) lide_cdt, sizeof(struct lide_cdt_table));

	/* init the head struct */
	lide_cdt->lide_cdt_head._cdt_magic = DMP_MAGIC;
	strcpy(lide_cdt->lide_cdt_head._cdt_name, "ataide");
	/* _cdt_len is filled in below */

	/* now begin filling in elements */
        for (i=0; i<MAX_ADAPTERS; i++)
	{
	  lide_cdt->lide_entry[i].d_segval = 0;
	  strcpy(lide_cdt->lide_entry[i].d_name, "adp_str0");
	  lide_cdt->lide_entry[i].d_name[7] += i;
	  lide_cdt->lide_entry[i].d_ptr = (char *) &adp_str[i];
	  lide_cdt->lide_entry[i].d_len = (sizeof(struct adapter_def));
	}

	/* fill in the actual table size */
	lide_cdt->lide_cdt_head._cdt_len = sizeof(struct lide_cdt_table);

    }
    return ((struct cdt *) (lide_cdt));

}  /* end lide_cdt_func */

/*************************************************************************/
/*                                                                      */
/* NAME:        lide_delay                                              */
/*                                                                      */
/* FUNCTION:    provide variable delay in 1 usec increments             */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*     process or interrupt                                             */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*                                                                      */
/* INPUTS:                                                              */
/*      delay - number of usecs to delay                                */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  IOCC_ATT, IOCC_DET, BUS_GETCX           */
/*                                                                      */
/************************************************************************/

void
lide_delay(
           struct adapter_def *ap,
           int delay_sec,
           int delay_nsec)
{
    struct timestruc_t     dtime;  /* delta time                        */
    struct timestruc_t     ctime;  /* current time                      */
    struct timestruc_t     etime;  /* ending time                       */

    /* get current time */
    curtime( &ctime );
    if ( delay_nsec < 1000000000 )
    {
      dtime.tv_sec = delay_sec;
      dtime.tv_nsec = delay_nsec;
      /* generate an ending time */
      ntimeradd( dtime, ctime, etime )

      /* now wait for time to pass */
      while (1)
      {
        curtime(&ctime);
        if( ntimercmp(ctime, etime, >) )
          break;
      }
    }
}

/*************************************************************************/
/*                                                                      */
/* NAME:        lide_xfer_func                                          */
/*                                                                      */
/* FUNCTION:    ATA/IDE Adapter Driver Transfer Function                */
/*                                                                      */
/*      This routine processes adapter signals.                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called either directly from the interrupt       */
/*      routine for DMA cleanups, or indirectly from the transfer thread*/
/*      for longer PIO transfers.                                       */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      ataide_buf  - input/output request struct used between the      */
/*                adapter driver and the calling ATA/ATAPI device driver*/
/*      dev_info - driver structure which holds information related to  */
/*                 a particular device and hence a command              */
/*      intr    - kernel interrupt information structure                */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap        - a pointer to an adapter_def structure               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* INTERNAL PROCEDURES CALLED:                                          */
/*      LIDE_READ_ATA   LIDE_WRITE_ATA  lide_start      lide_stop       */
/*      lide_spanned_xmem_xfer          lide_logerr                     */
/*      lide_delay                                                      */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_stop          w_start         iomem_att       iomem_det       */
/*      i_enable        i_disable                                       */
/*                                                                      */
/************************************************************************/
void
lide_xfer_func(
               struct adapter_def *ap)
{
    register int     x_len;
    register ushort  *sptr;
    register ushort  *end_xfer;
    register int     i;
    register struct ataide_buf *bp;
    register struct dev_info *dev_ptr;
    register struct dev_info *new_dev;
    register int ata_status;
    int     atapi_reason;
    int     data_val;
    int     dev_byte_count;
    int     o_run_flag;
    int     o_run;
    int     odd_move;
    int     old_level;
    volatile char *io;

    /* If we're not currently doing a PM commanded dump        */
    /* update PM Activity indicator so we don't get PM timeout */
    if (!(ap->dump_started & LIDE_PM_DMP)) {
      ap->pmh.activity = PM_ACTIVITY_OCCURRED;
    }

    dev_ptr = ap->device_queue_hash[ap->active_device];

    /* set 'bp' to currently active command */
    bp = dev_ptr->cmd_pend;

    if (!(ap->dump_started)) {
      /* stop the device's watchdog timer for aborts */
      w_stop(&dev_ptr->dev_watchdog.dog);
    }

    ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);

    switch (dev_ptr->cmd_activity_state) {
    case EXPECT_DATAIN:
        if ((ata_status & (ATA_BUSY | ATA_DATA_REQUEST)) == ATA_DATA_REQUEST)
        {
          o_run_flag = FALSE;               /* assume no data overruns  */

          if ( (dev_ptr->bytes_moved + dev_ptr->sector_len) <=
              bp->bufstruct.b_bcount) {
            x_len = dev_ptr->sector_len;      /* all bytes into buffer */
            o_run = 0;                        /* no overrun bytes     */
          } else {             /* overrun situation */
            /* remaining bytes */
            x_len = bp->bufstruct.b_bcount - dev_ptr->bytes_moved;
            /* overrun bytes */
            o_run = (dev_ptr->bytes_moved + dev_ptr->sector_len) -
                     bp->bufstruct.b_bcount;
          }

          sptr = (ushort *)&ap->xfer_area[0];

          /* disable our interupts while reading data register */
          /* to prevent our interrupts killing the read        */
          old_level = i_disable(ap->ddi.adap.int_prior);

          io = (volatile char *)iomem_att(&ap->iom) + ap->base_addr0 + LIDATA;

          if (dev_ptr->iordy_enabled) {
              /* carefree transfer of bytes from the xfer buffer */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ;
                   sptr < end_xfer ;
                   sptr++) {
                *sptr = *((volatile short *)(io));
              }
          } else {
              /* no flow control support, must delay between writes */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ;
                   sptr < end_xfer ;
                   sptr++) {
                *sptr = *((volatile short *)(io));
                lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
          }
          if (o_run) {                 /* throw away any overrun words */
              o_run_flag = TRUE;
              DEBUG_1("  DATA OVERRUN - ata_status = 0x%x\n",ata_status)

              for ( ; o_run > 0 ; o_run -= 2) {
                data_val = *((volatile short *)(io));
                if ( !(dev_ptr->iordy_enabled) )
                   lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
          }
          iomem_det((void *)io);
          i_enable(old_level);

          dev_ptr->bytes_moved += x_len;
          lide_spanned_xmem_xfer(ap, bp);  /* move the data to user area */

	  if (o_run_flag)
	  {
            lide_logerr(ap, ERRID_ATAIDE_ERR3, PIO_RD_OTHR_ERR,
	                3, 0, dev_ptr, TRUE);
	  }

	  /* the ATAPI CD-ROMs, in general, need additional time to 	*/
	  /* get ready for the next command, so completion status for	*/
	  /* command is held off by keeping busy high for ~200 usec.	*/
	  /* Hardware folks suggested a 230 usec delay, which seems	*/
	  /* to work for the 4X (2X seems to need a longer delay, explained */
	  /* below).							*/
	  /* The Mitsumi 2X CD-ROM varies on this in that the BUSY bit  */
	  /* (DRQ high) is initially set low (normally means completion)*/
	  /* then after about ~115 usec goes high for ~135 usec then	*/
	  /* clears the BUSY bit.					*/
          lide_delay(ap,0,300000);	/* set to 300 usec for 2X CD	*/
          ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);

          if ((dev_ptr->bytes_moved < bp->bufstruct.b_bcount) ||
              (ata_status & ATA_BUSY))
          {
            /* command isn't complete, so wait for interrupt for next seg */
            if (bp->timeout_value == 0)
              dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
            else
              dev_ptr->dev_watchdog.dog.restart = (ulong)bp->timeout_value + 1;
            if (!(ap->dump_started)) {
              w_start(&dev_ptr->dev_watchdog.dog);
	    }

            break;  /* leave interrupt routine */
          }
        }
        /* completion of transfer */
        bp->ata.status = (uchar)ata_status;
        lide_stop(ap, dev_ptr);
        break;

    case EXPECT_DATAOUT:

        /* move data from user buffer to ATA/IDE I/F */
        if ((ata_status & (ATA_BUSY | ATA_DATA_REQUEST)) == ATA_DATA_REQUEST)
        {
          o_run_flag = FALSE;               /* assume no data overruns  */

          if ( (dev_ptr->bytes_moved + dev_ptr->sector_len) <=
              bp->bufstruct.b_bcount) {
            x_len = dev_ptr->sector_len;      /* all bytes into buffer */
            o_run = 0;                        /* no overrun bytes     */
          } else {             /* overrun situation */
            /* remaining bytes */
            x_len = bp->bufstruct.b_bcount - dev_ptr->bytes_moved;
            /* overrun bytes */
            o_run = (dev_ptr->bytes_moved + dev_ptr->sector_len) -
                     bp->bufstruct.b_bcount;
          }

          sptr = (ushort *)&ap->xfer_area[0];

          /* copy data from user buffer to local dd xfer_area */
          lide_spanned_xmem_xfer(ap, bp);

          /* disable our interupts while reading data register */
          /* to prevent our interrupts killing the read        */
          old_level = i_disable(ap->ddi.adap.int_prior);

          io = (volatile char *)iomem_att(&ap->iom) + ap->base_addr0 + LIDATA;

          if (dev_ptr->iordy_enabled) {
              /* carefree transfer of bytes from the xfer buffer */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ;
                   sptr < end_xfer ;
                   sptr++) {
                *((volatile short *)(io)) = *sptr;
              }
          } else {
              /* no flow control support, must delay between writes */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ;
                   sptr < end_xfer ;
                   sptr++) {
                *((volatile short *)(io)) = *sptr;
                lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
          }
          if (o_run) {                 /* throw away any overrun words */
              o_run_flag = TRUE;
              DEBUG_1("  DATA OVERRUN - ata_status = 0x%x\n",ata_status)

              for ( ; o_run > 0 ; o_run -= 2) {
                *((volatile short *)(io)) = 0;
                if ( !(dev_ptr->iordy_enabled) )
                   lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
          }
          iomem_det((void *)io);
          i_enable(old_level);

          dev_ptr->bytes_moved += x_len;

	  if (o_run_flag)
	  {
            lide_logerr(ap, ERRID_ATAIDE_ERR3, PIO_WR_OTHR_ERR,
	                4, 0, dev_ptr, TRUE);
	  }

          /* command isn't complete, so wait for interrupt for next seg */
          if (bp->timeout_value == 0)
            dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
          else
            dev_ptr->dev_watchdog.dog.restart = (ulong)bp->timeout_value + 1;
	  if (!(ap->dump_started)) {
            w_start(&dev_ptr->dev_watchdog.dog);
	  }

          break;  /* leave interrupt routine */
        }
        /* completion of transfer */
        bp->ata.status = (uchar)ata_status;
        lide_stop(ap, dev_ptr);
        break;

    case EXPECT_STATUS:
        bp->ata.status = (uchar)ata_status;
        lide_stop(ap, dev_ptr);
        break;

    case EXPECT_ATAPI_CMD:
        ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);
        atapi_reason = LIDE_READ_ATA(ap, LISECC, LISECC_SIZE);

        /* are we in a DATA transfer state? */
        if (((ata_status & (ATA_BUSY | ATA_DATA_REQUEST)) == ATA_DATA_REQUEST)
             && ((atapi_reason == 2) || (atapi_reason == 0)))
        {
          /* read transfer size from ATAPI regs */
          dev_byte_count = LIDE_READ_ATA(ap, LICYLL, LICYLL_SIZE) & 0x00ff;
          dev_byte_count += ((LIDE_READ_ATA(ap, LICYLH, LICYLH_SIZE) & 0x00ff)
                             << 8);

          /* make sure that we don't move too much data */
          if (dev_byte_count < dev_ptr->sector_len) 
            dev_ptr->sector_len = dev_byte_count;

          o_run_flag = FALSE;		    /* assume no overruns    */
          /* handle the move by splitting the transfer into possibly
          ** three phases:
          ** 1. no special handling for 2 byte transfers in/out xfer_area
          ** 2. transfers that are an odd number of bytes, one byte is
          **    moved in/out the xfer_arear
          ** 3. any over run bytes are discarded
          **
          ** assumptions:
          ** 1) the xfer_area is on a two byte alignment, otherwise
	  **    performance may be at issue
	  **
	  ** 2) flow control is supported by device
          */
          if ( (dev_ptr->bytes_moved + dev_ptr->sector_len) <=
              bp->bufstruct.b_bcount) {
            x_len = dev_ptr->sector_len;      /* all bytes into buffer */
            o_run = 0;                        /* no overrun bytes     */
          } else {             /* overrun situation */
            /* remaining bytes */
            x_len = bp->bufstruct.b_bcount - dev_ptr->bytes_moved;
            /* overrun bytes */
            o_run = (dev_ptr->bytes_moved + dev_ptr->sector_len) -
                     bp->bufstruct.b_bcount;
          }

          /* odd byte transfer ? */
          if (x_len % 2) {
            x_len--;          /* makes xfer length a multiple of 2    */
            odd_move = 1;     /* one odd byte to move                 */

	    /* if device reports transfer count in word increments,   */
	    /* need to adjust overrun count.			      */
	    if (o_run) o_run--;          /* one less byte for overrun */
          } else {
            odd_move = 0;     /* no odd bytes to move                 */
          }

	  sptr = (ushort *)&ap->xfer_area[0];

          if (atapi_reason == 0x00)   /* ATAPI_DATAOUT */
          {
	    /* copy data from user buffer to local dd xfer_area */
	    lide_spanned_xmem_xfer(ap, bp);

            /* disable our interupts while writing data register */
            /* to prevent our interrupts killing the write       */
            old_level = i_disable(ap->ddi.adap.int_prior);
            io = (volatile char *)iomem_att(&ap->iom) + ap->base_addr0 + LIDATA;

	    if (dev_ptr->iordy_enabled) {
              /* carefree transfer of bytes from the xfer buffer */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ; 
		   sptr < end_xfer ;
                   sptr++) {
                *((volatile short *)(io)) = *sptr;
              }
	    } else {
	      /* no flow control support, must delay between writes */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ; 
		   sptr < end_xfer ;
                   sptr++) {
                *((volatile short *)(io)) = *sptr;
                lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
            }

            if (odd_move) {              /* handle any odd byte moves */
              *((volatile short *)(io)) = *sptr;
            }
            if (o_run) {                 /* throw away any overrun words */
	      o_run_flag = TRUE;
              DEBUG_1("  DATA OVERRUN - ata_status = 0x%x\n",ata_status)
              
              for ( ; o_run > 0 ; o_run -= 2) {
                *((volatile short *)(io)) = 0;
		if (!(dev_ptr->iordy_enabled)) {
                  lide_delay(ap,0,dev_ptr->pio_cycle_time);
		}
              }
            }
            iomem_det((void *)io);
            i_enable(old_level);

            dev_ptr->bytes_moved += x_len + odd_move ;
          }
          else if (atapi_reason == 0x02) /* ATAPI_DATAIN */
          {
            /* disable our interupts while reading data register */
            /* to prevent our interrupts killing the read        */
            old_level = i_disable(ap->ddi.adap.int_prior);
            io = (volatile char *)iomem_att(&ap->iom) + ap->base_addr0 + LIDATA;

	    if (dev_ptr->iordy_enabled) {
              /* carefree transfer of bytes into the xfer buffer */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ; 
                   sptr < end_xfer ;
                   sptr++) {
                *sptr = *((volatile short *)(io));
              }
	    } else {
	      /* no flow control support, must delay between reads */
              for (end_xfer = (ushort *)&ap->xfer_area[x_len] ; 
                   sptr < end_xfer ;
                   sptr++) {
                *sptr = *((volatile short *)(io));
                lide_delay(ap,0,dev_ptr->pio_cycle_time);
              }
            }

            if (odd_move) {              /* handle any odd byte moves */
              *(uchar *)sptr = (uchar)(*((volatile short *)(io)) >> 8);
            }
            if (o_run) {                 /* throw away any overrun words */
	      o_run_flag = TRUE;
              DEBUG_1("  DATA OVERRUN - ata_status = 0x%x\n",ata_status)
              
              for ( ; o_run > 0 ; o_run -= 2) {
                data_val = *((volatile short *)(io));
		if (!(dev_ptr->iordy_enabled)) {
                  lide_delay(ap,0,dev_ptr->pio_cycle_time);
		}
              }
            }
            iomem_det((void *)io);
            i_enable(old_level);

            dev_ptr->bytes_moved += x_len + odd_move ;
            lide_spanned_xmem_xfer(ap,bp);
	  }
	  if (o_run_flag)
	  {
	    if (atapi_reason == 0x00)      /* ATAPI_DATAOUT */
	    {
              lide_logerr(ap, ERRID_ATAIDE_ERR3, PIO_WR_OTHR_ERR,
	                  5, 0, dev_ptr, TRUE);
	    }
            else                           /* ATAPI_DATAIN  */
	    {
              lide_logerr(ap, ERRID_ATAIDE_ERR3, PIO_RD_OTHR_ERR,
	                  6, 0, dev_ptr, TRUE);
	    }
	  }
        }
        else if (((ata_status & (ATA_BUSY | ATA_DATA_REQUEST))
                  == ATA_DATA_REQUEST) && (atapi_reason == 0x01))
        {  
          DEBUG_2(" ATAPI status = 0x%x, Intr. Reason = 0x%x\n",
                  ata_status, LIDE_READ_ATA(ap, LISECC, LISECC_SIZE))
          DEBUG_3 ("lide_start cmd was=%x Fea=%x SecC=%x",
                     bp->ata.command, bp->ata.feature, bp->ata.sector_cnt_cmd)
          DEBUG_4(" DevH=%x CH=%x CL=%x SecN=%x\n",
                      bp->ata.startblk.chs.head, bp->ata.startblk.chs.cyl_hi,
                      bp->ata.startblk.chs.cyl_lo, bp->ata.startblk.chs.sector)
          DEBUG_2("       Byte count(l/h) = 0x%x, 0x%x\n",
                  LIDE_READ_ATA(ap, LICYLL, LICYLL_SIZE),
                  LIDE_READ_ATA(ap, LICYLH, LICYLH_SIZE));
          DEBUG_1("   input cmd len = 0x%x pack=",bp->ata.atapi.length);

	  /* ATAPI command Packet to transfer */
          sptr = (ushort *)&bp->ata.atapi.packet;
	  end_xfer = sptr + (bp->ata.atapi.length / 2);
          /* disable our interupts while writing data register */
          /* to prevent our interrupts killing the write       */
          old_level = i_disable(ap->ddi.adap.int_prior);
          io = (volatile char *)iomem_att(&ap->iom) + ap->base_addr0 + LIDATA;
	  if (dev_ptr->iordy_enabled) {
            while ( sptr < end_xfer )
            {
              DEBUG_1(" %02x",*sptr);
              *((volatile short *)(io)) = *sptr++;
            }
	  } else {
            while ( sptr < end_xfer )
            {
              DEBUG_1(" %02x",*sptr);
              *((volatile short *)(io)) = *sptr++;
              lide_delay(ap,0,dev_ptr->pio_cycle_time);
            }
	  }
          iomem_det((void *)io);
          i_enable(old_level);

          ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);
          DEBUG_2(" ATAPI status = 0x%x, Intr. Reason = 0x%x\n",
                  ata_status, LIDE_READ_ATA(ap, LISECC, LISECC_SIZE))
        }
        else /* command is complete, so fill in status */
        {
          /* completion of transfer */
          bp->ata.status = (uchar)ata_status;
          lide_stop(ap, dev_ptr);
          break;
        }
        /* command isn't complete, so wait for interrupt for next seg */
        if (bp->timeout_value == 0)
          dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
        else
          dev_ptr->dev_watchdog.dog.restart = (ulong) bp->timeout_value + 1;
	if (!(ap->dump_started)) {
          w_start(&dev_ptr->dev_watchdog.dog);
	}

        break;  /* leave case routine */

    case COMMAND_TIMED_OUT:
        bp->ata.status = (uchar)0xFF;
        lide_stop(ap, dev_ptr);
        break;

    default:
        DEBUG_1("   lide_xfer_func: unknown state = 0x%x\n",
                dev_ptr->cmd_activity_state)
        lide_logerr(ap, ERRID_ATAIDE_ERR3, UNKNOWN_CARD_ERR,
	            7, 0, dev_ptr, TRUE);
        break;
    }

    /* see if done, and if so, check other device for work */
    /* (but only if we're not doing a dump)                */
    if (ap->dump_started != 0) {
      ap->active_device = NO_DEV_ACTIVE;
    }
    else if ((dev_ptr->cmd_activity_state == DEVICE_NOT_ACTIVE))
    {
      ap->active_device++;
      ap->active_device %= MAX_DEVICES;

      /* set 'dev_ptr' to currently active device */
      new_dev = ap->device_queue_hash[ap->active_device];
      ap->active_device = NO_DEV_ACTIVE;

      if ((new_dev != NULL) &&
	  (new_dev->opened == TRUE) &&
          (new_dev->cmd_pend != NULL))
      { /* there is a command waiting on the adapter, so start it */
        lide_start(ap, new_dev);
      }
    }

}

/*************************************************************************/
/*                                                                      */
/* NAME:        lide_intr                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Interrupt Handler                        */
/*                                                                      */
/*      This routine processes adapter interrupt conditions.            */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the interrupt level, therefore, it must    */
/*      only perform operations on pinned data.                         */
/*                                                                      */
/* NOTES:  This routine handles interrupts which are caused by ide      */
/*     devices.  This entails processing the interrupts      */
/*     handling error cleanup and logging, and issuing outstanding      */
/*     commands for other devices                                       */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - ATA Task File (register) unique adapter structure */
/*      ataide_buf  - input/output request struct used between the      */
/*                adapter driver and the calling ATA/ATAPI device driver*/
/*      dev_info - driver structure which holds information related to  */
/*                 a particular device and hence a command              */
/*      intr    - kernel interrupt information structure                */
/*                                                                      */
/* INPUTS:                                                              */
/*      handler - pointer to the intrpt handler structure               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      INTR_FAIL    interrupt was not processed                        */
/*      INTR_SUCC    interrupt was processed                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*     i_reset                                                          */
/*                                                                      */
/************************************************************************/
int
lide_intr(struct lintr * handler)
{
    struct dev_info *dev_ptr;
    struct adapter_def *ap;
    register int ata_status;

    TRACE_1 ("in intr", 0)

    ap = handler->ap;

    if ((ap->pmh.mode != PM_DEVICE_FULL_ON) &&
        (ap->pmh.mode != PM_DEVICE_ENABLE))
    {
      /* can't be ours, because we're powered down...*/
      TRACE_1("intr power down", INTR_FAIL)
      return(INTR_FAIL);
    }

    /* do we need to worry about which device's status register we */
    /* are defaulting to ???                                       */

    ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);

    DDHKWD2(HKWD_DD_IDEDD, DD_ENTRY_INTR, 0, ap->devno, ata_status);

    if ( (ata_status & ATA_BUSY)  ||
	(ap->dma_pending && (ata_status & ATA_DATA_REQUEST)) ||
         (ap->active_device == NO_DEV_ACTIVE))

	/*
	 * If the device is busy, we are awaiting DMA completion and
	 * the DRQ bit is set, or we are not expecting an interrupt...
	 */

    {  /* then it's not really our interrupt */
	/* clear out any spurious interrupt, problem with 2X CDROM */
	if ( !(ata_status & (ATA_BUSY | ATA_DATA_REQUEST)) ) {
	    LIDE_READ_ATA(ap, LISTA, LISTA_SIZE);
	}
#ifdef DEBUG
	if (ap->active_device != NO_DEV_ACTIVE) {
	  DEBUG_2 ("  lide_intr: ATA status = 0x%x  active device = 0x%x\n",
			ata_status, ap->active_device)
	}
#endif
       DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_INTR, INTR_FAIL, ap->devno);
       return (INTR_FAIL);
    }
    else /* else assume the interrupt is for us */
    {
      /* Acknowledge the interrupt */
      ata_status = LIDE_READ_ATA(ap, LISTA, LISTA_SIZE);

      /* set 'dev_ptr' to currently active device */
      dev_ptr = ap->device_queue_hash[ap->active_device];


      if (dev_ptr->cmd_activity_state & (EXPECT_DATAIN | EXPECT_DATAOUT |
                                          EXPECT_STATUS | EXPECT_ATAPI_CMD))
      {
	/* call lide_xfer_function directly */
	lide_xfer_func(ap);
      }
      else
      {
        DEBUG_1("   lide_intr: unknown state = 0x%x\n",
                dev_ptr->cmd_activity_state)
        lide_logerr(ap, ERRID_ATAIDE_ERR3, UNKNOWN_CARD_ERR,
	            9, 0, dev_ptr, TRUE);
      }
    }
    LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x08);   /* make sure intr enable*/
    TRACE_1 ("out intr", 0);
    i_reset(&(ap->intr_struct));

    DDHKWD1(HKWD_DD_IDEDD, DD_EXIT_INTR, INTR_SUCC, ap->devno);

    return (INTR_SUCC);
}  /* end of lide_intr      */

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_epow                                                       */
/*                                                                        */
/* FUNCTION:  ATA/IDE device driver suspend routine.                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called when an EPOW (Early Power Off Warning)  */
/*         has been detected.                                             */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*      intr    -  kernel interrupt handler information structure         */
/*                                                                        */
/* INPUTS:                                                                */
/*      handler - pointer to the intrpt handler structure                 */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      INTR_SUCC - interrupt was not processed                           */
/*      INTR_SUCC - interrupt was processed                               */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      i_disable  i_enable                                               */
/*                                                                        */
/**************************************************************************/
int
lide_epow(struct lintr * handler)
{
    int     old_pri1, old_pri2;
    int     i;
    struct adapter_def *ap;
    struct dev_info *dev_ptr;

    DEBUG_0 ("Entering lide_epow routine.\n")
    ap = handler->ap;
    if ((handler->its.flags & EPOW_SUSPEND) ||
	((handler->its.flags & EPOW_BATTERY) &&
	 (!(ap->ddi.adap.battery_backed))))
    {
        /* Power is going off!!! */
	DEBUG_0 ("lide_epow: suspend\n")
	ap->power_state |= EPOW_PENDING;
	lide_chip_register_reset(ap);
    }	/* endif */
    else  /* restart again after an EPOW has occurred */
    {
        /* is power really coming back on ? */
	if (handler->its.flags & EPOW_RESUME)
	{
	    DEBUG_0 ("lide_epow: resume\n")
	    /* handle a resume command */
	    /* disable to int level during this */
	    old_pri1 = i_disable(ap->ddi.adap.int_prior);

	    /* disable to close window around the test */
	    old_pri2 = i_disable(INTEPOW);

	    if (((!(handler->its.flags & EPOW_BATTERY)) &&
		 (!(handler->its.flags & EPOW_SUSPEND))) &&
		(ap->power_state & EPOW_PENDING))
	    {
		ap->power_state &= ~EPOW_PENDING;	/* reset epow state */
		i_enable(old_pri2);	/* return to lower priority */
                for (i = 0; i < MAX_DEVICES; i++) {
		  if (((dev_ptr = ap->device_queue_hash[i]) != NULL) &&
		      (dev_ptr->opened == TRUE) &&
		      (dev_ptr->cmd_pend != NULL)) {
		    lide_start(ap, dev_ptr);
		    break;
		  }
		}
	    }
	    else
	    {   /* either a SUSPEND has re-occurred, or this */
		/* adap was not put in epow pending state.   */
		/* for these cases--leave adapter as is      */
		i_enable(old_pri2);	/* return to lower priority */
	    }
	    i_enable(old_pri1);	/* re-enable */
	}	/* end if */
    }	/* end else */

    DEBUG_0 ("leaving epow: intr_succeeded\n")
    return (INTR_SUCC);
}  /* end lide_epow */

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_pm_handler                                                 */
/*                                                                        */
/* FUNCTION:  Routine to handle requests from power management core.      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      pm_handler - the PM Handler struct within the adapter_def is chngd*/
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*                                                                        */
/* INPUTS:                                                                */
/*      private - pointer to adapter structure                            */
/*      requested_mode - device pm state requested by the pm core         */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      PM_SUCCESS - transition made successfully                         */
/*      PM_ERROR   - transition not made                                  */
/*                                                                        */
/**************************************************************************/
int
lide_pm_handler(caddr_t private, int requested_mode)
{
    struct adapter_def *ap;
    struct dev_info *dev_ptr;
    int ret_code;
    int old_pri, i, dev_index;
    int cmd_to_service;
    struct ataide_buf *bp;

    ap = (struct adapter_def *)private;   /* fix pointer type */
    ret_code = PM_SUCCESS;                /* assume success   */

    if (ap == NULL)
      return (PM_ERROR);

    TRACE_2("pmh entry", requested_mode, ap->pmh.mode)
    switch (requested_mode) {
      case PM_DEVICE_FULL_ON:
	if (ap->pmh.mode == PM_DEVICE_ENABLE)
	  /* this is a good request, so do it */
	  ap->pmh.mode = requested_mode;
	else
          /* must be in DEVICE ENABLE to make the requested change */
	  ret_code = PM_ERROR;
	break;
      case PM_DEVICE_ENABLE:
	if (pincode(lide_pm_handler) == 0) {
	  old_pri = i_disable(ap->ddi.adap.int_prior);
	  switch(ap->pmh.mode) {
	    case PM_DEVICE_FULL_ON:
	      ap->pmh.mode = requested_mode;
	      break;

	    case PM_DEVICE_IDLE:
	      lide_turn_chip_on(ap, TRUE);
	      break;
	    case PM_DEVICE_SUSPEND:
	    case PM_DEVICE_HIBERNATION:
	      ap->power_state &= ~LIDE_PM_SYS_PWR;
	      TRACE_1("power state", ap->power_state)
	      lide_turn_chip_on(ap, TRUE);

	      for (i=0; i<MAX_DEVICES; i++) {
		if (((dev_ptr = ap->device_queue_hash[i]) != NULL) &&
		    (dev_ptr->opened == TRUE) &&
		    (dev_ptr->cmd_pend != NULL)) {
		  lide_start(ap, dev_ptr);
		  break;
		}
	      }
	      break;
	    default:
	      ret_code = PM_ERROR;
	      break;
          }
	  i_enable(old_pri);
	  unpincode(lide_pm_handler);
	}
	else
	  ret_code = PM_ERROR;
	break;

      case PM_DEVICE_IDLE:
	if (pincode(lide_pm_handler) == 0) {
	  old_pri = i_disable(ap->ddi.adap.int_prior);
	  switch(ap->pmh.mode) {
	    case PM_DEVICE_ENABLE:
	      if (ap->active_device != NO_DEV_ACTIVE)
		ret_code = PM_ERROR;
	      else
	      {
		/* Disable interrupts from adapter */
		LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0A);
		/* Tell Planar Control to take us offline */
		(void) pm_planar_control(ap->devno, PMDEV_ISA00,
					 PM_PLANAR_OFF);
		ap->pmh.mode = requested_mode;
	      }
	      break;
	    case PM_DEVICE_SUSPEND:
	    case PM_DEVICE_HIBERNATION:
	      ap->power_state &= ~LIDE_PM_SYS_PWR;
	      /* Allow any held commands to execute now */
	      /* First, see if there are any */
	      cmd_to_service = FALSE;
	      for (i=0; i<MAX_DEVICES; i++) {
		if (((dev_ptr = ap->device_queue_hash[i]) != NULL) &&
		    (dev_ptr->opened == TRUE) &&
		    (dev_ptr->cmd_pend != NULL)) {
		  cmd_to_service = TRUE;
		  break;
		}
	      }
	      if (!cmd_to_service) {
		/* no commands to run, so all done */
		ap->pmh.mode = requested_mode;
		TRACE_1("no cmds to serve",0)
	      } else {
		/* work to do, get it started */
		lide_turn_chip_on(ap,TRUE);
		lide_start(ap, dev_ptr);
	      }
	      break;
	    case PM_DEVICE_IDLE:
	    case PM_DEVICE_FULL_ON:
	    default:
	      ret_code = PM_ERROR;
	      break;
          }
	  i_enable(old_pri);
	  unpincode(lide_pm_handler);
	}
	else
	  ret_code = PM_ERROR;
	break;

      case PM_DEVICE_SUSPEND:
      case PM_DEVICE_HIBERNATION:
	old_pri = i_disable(ap->ddi.adap.int_prior);
	switch(ap->pmh.mode) {
	  case PM_DEVICE_ENABLE:
	    if (ap->active_device != NO_DEV_ACTIVE) {
	      ret_code = PM_ERROR;
	    } else {
	      /* Disable interrupts from adapter */
	      LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0A);
	      if (requested_mode == PM_DEVICE_SUSPEND) {
		/* Tell Planar Control to take us offline */
		(void) pm_planar_control(ap->devno, PMDEV_ISA00,
					 PM_PLANAR_OFF);
	      }
	      ap->pmh.mode = requested_mode;
              ap->power_state |= LIDE_PM_SYS_PWR;
	    }
	    break;
	  case PM_DEVICE_IDLE:
	    ap->pmh.mode = requested_mode;
            ap->power_state |= LIDE_PM_SYS_PWR;
	    break;
	  case PM_DEVICE_SUSPEND:
	  case PM_DEVICE_HIBERNATION:
	  case PM_DEVICE_FULL_ON:
	  default:
	    ret_code = PM_ERROR;
	    break;
        }
	i_enable(old_pri);
	break;

      case PM_PAGE_FREEZE_NOTICE:
	/* Pin the bottom half of the device driver */
	if (pincode(lide_intr))
	  ret_code = PM_ERROR;
	break;

      case PM_PAGE_UNFREEZE_NOTICE:
	/* unpin the bottom half of the device driver */
	(void) unpincode(lide_intr);
	break;

      case PM_RING_RESUME_ENABLE_NOTICE:
      default:
	ret_code = PM_ERROR;
	break;
    }
    return(ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_turn_chip_on                                               */
/*                                                                        */
/* FUNCTION:  This function is called to handle restoration of power      */
/*            to the IDE Adapter.                                         */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*                                                                        */
/* INPUTS:                                                                */
/*      ap - pointer to an adapter_def structure                          */
/*      change_pmh_mode - flag indicating need to change to PM_ENABLED    */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: None                                         */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
void
lide_turn_chip_on(struct adapter_def *ap, int change_pmh_mode)
{
  int i;

  TRACE_1("turn ide adap on",change_pmh_mode)
  if (change_pmh_mode)
    ap->pmh.mode = PM_DEVICE_ENABLE;

  (void) pm_planar_control(ap->devno, (PMDEV_LOCAL2 + 0x400), PM_PLANAR_ON);
  (void) lide_reconfig_adapter(ap);
  if (ap->opened)
    /* reenable chip interrupts */
    LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x08);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_command_watchdog                                           */
/*                                                                        */
/* FUNCTION:  Command timer routine.                                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called when a command has timed out.  A BDR is */
/*         issued in an attempt to cleanup the device This routines exe-  */
/*         cutes to reset the bus during normal command executtion.       */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*  w - gives you the address of the dev_info structure                   */
/*                                                                        */
/* INPUTS: None                                                           */
/*         w - pointer to watch dog timer structure which timed out       */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: None                                         */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*        w_start  e_wakeup                                               */
/**************************************************************************/
void
lide_command_watchdog(struct watchdog * w)
{
    struct dev_info *dev_ptr;
    struct timer *tdog;
    struct adapter_def *ap;
    struct ataide_buf *bp;
    int     i, old_pri;
    uchar  ata_status;


    tdog = (struct timer *) w;
    ap = tdog->ap;
    old_pri = i_disable(ap->ddi.adap.int_prior);
    TRACE_1 ("in watch", tdog->timer_id)
    if (tdog->timer_id == LIDE_RESET_TMR)
    {
	w_stop(&ap->reset_watchdog.dog);
        /* update the power mgmt. activity field */
	ap->pmh.activity = PM_ACTIVITY_OCCURRED;
	/* if we failed in causing the ide bus to reset.  This   */
	/* guarantees that the chip is hung, so reset the chip.   */
	lide_chip_register_reset(ap);
    }
    else
    {
        DEBUG_0 ("  Fatal command timeout!\n")
        DEBUG_2 ("  devno = 0x%x, active_device = 0x%x\n",
                 ap->devno, ap->active_device);
        if (tdog->dog.count != 0) {
            DEBUG_0 ("Leaving lide_command_watchdog: watchdog already reset\n")
            return;
	}
	/* if here a fatal command timeout occurred */
        if (ap->active_device == NO_DEV_ACTIVE)
        {
            DEBUG_0 ("Leaving lide_command_watchdog: no dev active\n")
	    return;
        }
        dev_ptr = ap->device_queue_hash[ap->active_device];
 	DEBUG_1 ("Command active = %d\n", dev_ptr->cmd_activity_state)
	if (dev_ptr->cmd_activity_state == DEVICE_NOT_ACTIVE)
        {
            DEBUG_0 ("Leaving lide_command_watchdog: state inactive\n")
	    return;
        }
	w_stop(&dev_ptr->dev_watchdog.dog);
	bp = dev_ptr->cmd_pend;
	/* If the BUSY bit is still set or still doing DMA, the command	*/
	/* did not finish in time.  Otherwise the command may have 	*/
	/* finished, but the interrupt for some reason was lost. Go	*/
	/* ahead and let the xfer function try to finish the command.	*/
	ata_status = LIDE_READ_ATA(ap, LIASTA, LIASTA_SIZE);
	if ( (ata_status & ATA_BUSY) ||
	     (ap->dma_pending && (ata_status & ATA_DATA_REQUEST)) ) {
	    dev_ptr->flags |= CAUSED_TIMEOUT;
            dev_ptr->cmd_activity_state = COMMAND_TIMED_OUT;
	    /* now reset & reconfig the devices */
	    lide_reconfig_adapter(ap);
	}
	DEBUG_0 ("lide_command_watchdog: regular cmd \n")
        TRACE_1 ("regular cmd", (int) dev_ptr)

	if (bp != NULL)
	{
	  lide_xfer_func(ap);
        }
    }	/* else dev_ptr */
    TRACE_1 ("outwatch", 0)
    i_enable(old_pri);
}  /* lide_command_watchdog */

/**************************************************************************/
/*                                                                        */
/* NAME:  lide_command_reset_ide_bus                                      */
/*                                                                        */
/* FUNCTION:  Resets the IDE bus lines.                                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine will toggle the IDE bus reset line for 30 micro-  */
/*         seconds to assert a reset on the IDE bus. This routines exe-   */
/*         cutes to reset the bus during normal command execution.        */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*      adapter_def - ATA Task File (register) unique adapter structure   */
/*                                                                        */
/* INPUTS: None                                                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      PSC_NO_ERR - for good completion.                                 */
/*      ERRNO      - value otherwise                                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
void
lide_command_reset_ide_bus(struct adapter_def *ap)
{
    int     rc, i;
    int     old_pri;

    TRACE_1 ("in busrs", 0)
    old_pri = i_disable(ap->ddi.adap.int_prior);
    LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0E);/* assert the ATA reset */
    lide_delay(ap,0,10000000);  /* delay 10 millisecs */
    LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x0A);/* release ATA reset */
    i_enable(old_pri);

    for (i = 0; i<IPL_MAX_SECS; i++)
    {
       rc = LIDE_READ_ATA(ap, LISTA, LISTA_SIZE);
       if (!(rc & ATA_BUSY))
         break;
       lide_delay(ap, 1, 0);  /* delay for 1 second */
    }
    LIDE_WRITE_ATA(ap, LIDCTL, LIDCTL_SIZE, 0x08); /* enable interrupts */
    TRACE_1 ("out busr", 0)
}  /* lide_command_reset_ide_bus */

#ifdef LIDE_TRACE
void
lide_trace_1(char *string, int data)
{
    int     i;

    if (adp_str.current_trace_line > (LIDE_TRACE_SIZE - 0x10))
    {
	adp_str.current_trace_line = 1;
    }
    for (i = 0; i < 13; i++)
	adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.one_val.header1[i] = *(string + i);
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.one_val.data = data;
    adp_str.current_trace_line++;

    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[0] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[1] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[2] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[3] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[4] = 'P';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[5] = 'O';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[6] = 'I';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[7] = 'N';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[8] = 'T';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[9] = 'E';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[10] = 'R';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[11] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[12] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[13] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[14] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[15] = '*';
}
#endif

#ifdef LIDE_TRACE
void
lide_trace_2(char *string, int val1, int val2)
{
    int     i;

    if (adp_str.current_trace_line > (LIDE_TRACE_SIZE - 0x10))
    {
	adp_str.current_trace_line = 1;
    }
    for (i = 0; i < 9; i++)
	adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.two_vals.header2[i] = *(string + i);
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.two_vals.data1 = val1;
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.two_vals.data2 = val2;
    adp_str.current_trace_line++;
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[0] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[1] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[2] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[3] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[4] = 'P';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[5] = 'O';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[6] = 'I';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[7] = 'N';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[8] = 'T';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[9] = 'E';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[10] = 'R';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[11] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[12] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[13] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[14] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[15] = '*';
}
#endif

#ifdef LIDE_TRACE
void
lide_trace_3(char *string, int data1, int data2, int data3)
{
    int     i;

    if (adp_str.current_trace_line > (LIDE_TRACE_SIZE - 0x10))
    {
	adp_str.current_trace_line = 1;
    }
    for (i = 0; i < 5; i++)
	adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.header3[i] = *(string + i);
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.val1 = data1;
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.val2 = data2;
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.val3 = data3;
    adp_str.current_trace_line++;
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[0] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[1] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[2] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[3] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[4] = 'P';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[5] = 'O';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[6] = 'I';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[7] = 'N';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[8] = 'T';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[9] = 'E';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[10] = 'R';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[11] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[12] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[13] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[14] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[15] = '*';
}
#endif
