static char sccsid[] = "@(#)73  1.7  src/rspc/kernext/isa/isakbd/isa_kbdconfig.c, isakbd, rspc41J, 9519A_all 5/9/95 07:55:08";
/*
 *   COMPONENT_NAME: isakbd
 *
 *   FUNCTIONS: addswitch
 *              cleanup
 *              initadpt
 *              kbdconfig
 *              qvpd
 *              reg_pm
 *              ureg_pm
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include  "kbd.h"
lock_t  kbd_lock=LOCK_AVAIL;           /* process level lock                */

extern struct io_map   *io_map_ptr;
extern struct _km_io_lock *km_io_lock;

/*****************************************************************************/
/*                                                                           */
/* NAME:             kbdconfig                                               */
/*                                                                           */
/* FUNCTION:         This module manages the task of allocating storage for  */
/*                   and configuring the keyboard.                           */
/*                                                                           */
/* INPUTS:           devno = major and minor device number                   */
/*                   cmd   = command, i.e. what to do: initialize, terminate,*/
/*                              or query vital product data                  */
/*                   uiop  = pointer to a uio structure containing the dds   */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   EINTR  = failure                                        */
/*                   ENOSPC                                                  */
/*                   EFAULT                                                  */
/*                   ENOMEM                                                  */
/*                   EINVAL                                                  */
/*                   EPERM                                                   */
/*                   EIO                                                     */
/*                   ENXIO                                                   */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  kbdconfig(dev_t devno, long cmd, struct uio *uiop)
{

  struct ktsmdds    dds;               /* local copy of dds                  */
  int rc = 0;

  KTSMDTRACE(kbdconfig, enter, devno, cmd, 0, 0, 0);

                                       /* get lock                           */
  if (lockl(&kbd_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {

    switch(cmd) {                      /* switch on command                  */

                                       /*------------------------------------*/
      case CFG_INIT:                   /* case: initialize                   */
                                       /*------------------------------------*/

                                       /* move dds to kernel memory          */
        if (uiop == (struct uio *) NULL) {
          rc = EINVAL;                 /* ...error                           */
          break;
        }
        if (rc = uiomove((caddr_t) &dds, sizeof(struct ktsmdds),
              UIO_WRITE, uiop)) {
          if (rc < 0) rc = EPERM;      /* ...error                           */
          break;
        }

        if (!local) {                  /* allocate local storage             */
          local = (struct local *)
                xmalloc(sizeof(struct local), 2, pinned_heap);
          if (!local) {                /* ...error                           */
            rc = ENOMEM;
            break;
          }
                                       /* clear out local storage            */
          bzero ((char *) local, sizeof(struct local));

                                       /* allocate I/O mem attach parm block */
          io_map_ptr = (struct io_map *)
                xmalloc(sizeof(struct io_map), 2, pinned_heap);
          if (!io_map_ptr) {           /* ...error                           */
            rc = ENOMEM;
            break;
          }
                                       /* allocate I/O wait timer            */
          if ((local->com.stimer = talloc()) == NULL ) {
            rc = EPERM;                /* ...error                           */
            break;
          }
                                       /* allocate watch dog timer           */
          if ((local->com.wdtimer = talloc()) == NULL ) {
            rc = EPERM;                /* ...error                           */
            break;
          }
                                       /* allocate alarm timer               */
          if ((local->sdd_time_ptr = talloc()) == NULL) {
            rc = EPERM;                /* ...error                           */
            break;
          }
                                       /* initialize sleep anchor            */
          local->com.asleep = EVENT_NULL;
                                       /* no active keyboard channels        */
          local->key.act_ch = NO_ACT_CH;
        }

                                       /* configuring adapter                */
        if (dds.device_class == ADAPTER) {
          rc = initadpt(&dds, devno);
        }
        else {                         /* configuring keyboard               */
          if (dds.device_class == KBD) {
            rc = initkbd(&local->com, &local->key, &dds, devno);
#ifdef PM_SUPPORT
            if (!rc) {                 /* register pm handle structure to pm */
              reg_pm(&dds, devno);     /* core to notify that keyboard is pm */
            }                          /* aware                              */
#endif /* PM_SUPPORT */
          }
          else {
            rc = EINVAL;               /* invalid device class               */
          }
        }
        break;


                                       /*------------------------------------*/
      case CFG_TERM:                   /* case: terminate                    */
                                       /*------------------------------------*/

        if (!local) {                  /* error if not configured            */
          rc = ENXIO;
        }
        else {                         /* terminating adapter                */
          if (devno == local->com.adpt_devno) {
                                       /*   error if keyboard configured     */
            if (local->key.kbd_devno) {
                rc = EBUSY;
            }
            else {                     /*   mark adapter as termianted       */
              local->com.adpt_devno = 0;
            }
          }
          else {                       /* terminating keyboard               */
            if (devno == local->key.kbd_devno) {
                                       /*   error if channel open            */
              if (local->key.act_ch != NO_ACT_CH) {
                rc = EBUSY;
              }
              else {                   /*   mark keyboard as terminated      */
                local->key.kbd_devno = 0;
#ifdef PM_SUPPORT
                                       /* unregister pm handle structure if  */
                                       /* registered                         */
                ureg_pm();
#endif /* PM_SUPPORT */
              }
            }
            else {                     /* error if not any known device      */
              rc = ENXIO;
            }
          }
        }
        break;

                                       /*------------------------------------*/
      case CFG_QVPD:                   /* case: query vital product data     */
                                       /*------------------------------------*/

        if (!local) {                  /* error if not configured            */
          rc = ENXIO;
        }
        else {                         /* only valid if for adapter          */
          if (devno != local->com.adpt_devno) {
            rc = EINVAL;
          }
          else {                       /* error if no uio structure          */
            if (uiop == (struct uio *) NULL) {
              rc = EINVAL;
            }
            else {
              rc = qvpd(uiop);
            }
          }
        }
        break;

                                       /*------------------------------------*/
      default:                         /* default: unknown command           */
                                       /*------------------------------------*/

        rc = EINVAL;
        break;

    }

    cleanup(devno);                    /* release resource                   */
    unlockl(&kbd_lock);                /* release lock                       */
  }

  KTSMTRACE(kbdconfig, exit, rc, devno, cmd, 0, 0);
  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             cleanup                                                 */
/*                                                                           */
/* FUNCTION:         this module frees all local resources                   */
/*                                                                           */
/* INPUTS:           devno = major and minor device numbers                  */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:            deallocation of adapter locking structure assumes       */
/*                   that configuration is serialized. This is certainly     */
/*                   true when mkdev, rmdev, cfgmgr commands are used as     */
/*                   they guarantee serialization via an ODM lock.           */
/*                                                                           */
/*****************************************************************************/

void cleanup(dev_t devno)
{

   if (local) {                        /* if resources allocated and         */
                                       /* nothing configured then            */
     if ((!local->com.adpt_devno) &&
         (!local->key.kbd_devno)) {

       devswdel(devno);                /* clear switch table                 */

       if (local->com.stimer)          /* free I/O wait timer                */
         tfree(local->com.stimer);
       if (local->com.wdtimer)         /* free watch dog timer               */
         tfree(local->com.wdtimer);
       if (local->sdd_time_ptr)        /* free alarm timer                   */
         tfree(local->sdd_time_ptr);
       
                                       /* free local memory                  */
       xmfree((caddr_t) local, pinned_heap);
       local = NULL;
                                       /* deallocate I/O mem attach parm blk */
       if (io_map_ptr) {            
         xmfree((caddr_t) io_map_ptr, pinned_heap);
         io_map_ptr = NULL;
       }
                                       /* deallocate adapter locking struct  */
       if (km_io_lock) {               /* if mouse dd not using it           */
         if (km_io_lock->other == OTHER_DEVICE_USING)
           km_io_lock->other = NO_OTHER_DEVICE_USING;
         else {
           xmfree((caddr_t) km_io_lock, pinned_heap);
           km_io_lock = NULL;
         }
       }
     }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             initadpt                                                */
/*                                                                           */
/* FUNCTION:         This module initializes the adapter                     */
/*                                                                           */
/* INPUTS:           devno = device number                                   */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:            allocation of adapter locking structure assumes         */
/*                   that configuration is serialized. This is certainly     */
/*                   true when mkdev, rmdev, cfgmgr commands are used as     */
/*                   they guarantee serialization via an ODM lock.           */
/*                                                                           */
/*****************************************************************************/

int initadpt(struct ktsmdds *dds_ptr, dev_t devno)
{
  int rc = 0;
  uint status;
  caddr_t p;

  if (!local->com.adpt_devno) {        /* skip if already configured         */
                                       /* save interrupt level               */
    local->com.intr_level = dds_ptr->bus_intr_lvl;
                                       /* save interrupt priority            */
    local->com.intr_priority = dds_ptr->intr_priority;
                                       /* save bus ID                        */
    local->com.bus_id = dds_ptr->bus_id;
                                    
    io_map_ptr->key = IO_MEM_MAP;      /* isa I/O mem attach parm block      */
    io_map_ptr->flags = 0;             /*    4K block starting at 0x0000     */
    io_map_ptr->size = IOMEM_ATT_SIZE;
    io_map_ptr->bid = dds_ptr->bus_id;  
    io_map_ptr->busaddr = 0;


/* devno of mouse adapter if Available is passed to keyboard driver in       */
/* devno_link field of dds. The driver uses this to get pointer to adapter   */
/* locking structure (from mouse's devsw table).                             */
/* If devno_link is -1, then the keyboard driver is the leader and it must   */
/* allocate the adapter locking structure                                    */
                                    
                                       /* keyboard is the leader so ...      */
    if (dds_ptr->devno_link == -1 ) {
                                       /* malloc locking structure           */
       km_io_lock = (struct _km_io_lock *)
         xmalloc(sizeof(struct _km_io_lock), 2, pinned_heap);
                                       /* error if malloc fails              */
       if (!km_io_lock) return(ENOMEM);
       km_io_lock->lock = NOT_LOCKED;  /* adapter is not locked              */
                                       /* mouse and keyboard disabled        */
       km_io_lock->cmd_save = DIS_AUX_INTERFACE | DIS_KBD_INTERFACE;
        
                                       /* no other device using structure    */
       km_io_lock->other = NO_OTHER_DEVICE_USING;
    }
    else {                             /* keyboard is not the leader so ...  */
                                       /* get pointer to locking structure   */
                                       /* from mouse device switch table     */
      if (!devswqry(dds_ptr->devno_link, &status, &p) &&
        (status & DSW_DEFINED)) {
         km_io_lock = (struct _km_io_lock *) p;
                                       /* keyboard is using structure        */
         km_io_lock->other = OTHER_DEVICE_USING;
      }
      else                             /* hosed...devsw is not valid         */
         return(EPERM);
    }


    rc = addswitch(devno);             /* add device to switch table         */
    if (!rc)                           /* if all ok then mark adapter        */
      local->com.adpt_devno = devno;   /* configured by saving our devno     */
  }
 
  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             addswitch                                               */
/*                                                                           */
/* FUNCTION:         This module adds the device driver to the device        */
/*                   switch table.                                           */
/*                                                                           */
/* INPUTS:           devno = major and minor device numbers                  */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   ENOMEM = failure                                        */
/*                   EINVAL                                                  */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  addswitch(dev_t devno)
{

   int           rc;
   struct devsw  tmp_devsw;

   void nodev(void);
                                       /* copy pointers to functions into    */
                                       /* the local device switch table      */
   tmp_devsw.d_open     = (int (*) ()) kbdopen;
   tmp_devsw.d_close    = (int (*) ()) kbdclose;
   tmp_devsw.d_read     = (int (*) ()) nodev;
   tmp_devsw.d_write    = (int (*) ()) nodev;
   tmp_devsw.d_ioctl    = (int (*) ()) kbdioctl;
   tmp_devsw.d_strategy = (int (*) ()) nodev;
   tmp_devsw.d_select   = (int (*) ()) nodev;
   tmp_devsw.d_config   = (int (*) ()) kbdconfig;
   tmp_devsw.d_print    = (int (*) ()) nodev;
   tmp_devsw.d_dump     = (int (*) ()) nodev;
   tmp_devsw.d_mpx      = (int (*) ()) kbdmpx;
   tmp_devsw.d_revoke   = (int (*) ()) nodev;
   tmp_devsw.d_dsdptr   = (caddr_t) km_io_lock;
   tmp_devsw.d_ttys     = NULL;
   tmp_devsw.d_selptr   = NULL;
   tmp_devsw.d_opts     = 0;
                                       /* add to the device switch table     */
   return(devswadd(devno, &tmp_devsw));

}


/*****************************************************************************/
/*                                                                           */
/* NAME:             qvpd                                                    */
/*                                                                           */
/* FUNCTION:         This module returns information on what device(s)       */
/*                   are connected to adapter                                */
/*                                                                           */
/* INPUTS:           uio pointer which points to an area of memory where the */
/*                      data will be stored                                  */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   ENOMEM = failure                                        */
/*                   EIO                                                     */
/*                   ENOSPC                                                  */
/*                   EFAULT                                                  */
/*                   EPERM                                                   */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  qvpd(struct uio *uiop )
{
  uchar  data[28];
  int    rc;
  int    index = 0;

                                       /* get keyboard ID if keyboard has    */
  if (!local->key.kbd_devno) {        /* not yet been configured            */
    if (!reg_intr(FALSE)) {
      get_kbd_id(&local->com, &local->key);
      ureg_intr();
    }
  }


  switch (local->key.kbd_type) {
    case KSPS2:
      bcopy("keyboard/std_k/ps2 kbd ", data ,25);
      index = 25;
      break;

    default:
      break;
  }

  data[index] = '\0';                  /* null terminate                     */

                                       /* write data out to user space       */
  rc = uiomove(data, index+1, UIO_READ, uiop);
  if (rc < 0) rc = EPERM;

  return(rc);
}


#ifdef PM_SUPPORT
/*****************************************************************************/
/*                                                                           */
/* NAME:             reg_pm                                                  */
/*                                                                           */
/* FUNCTION:         This routine initializes pm handle structure and        */
/*                   registers it to pm core for Power Management(PM)        */
/*                                                                           */
/* INPUTS:           devno = major and minor device number                   */
/*                   dds_local = pointer to DDS structure                    */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:            Even if an error occurs, keyboard must work without     */
/*                   Power Management(PM)                                    */
/*                                                                           */
/*****************************************************************************/

void reg_pm(struct ktsmdds *dds_local, dev_t devno)
{
  int    rc;
  struct pm_handle *pm;


  if (!local->com.pm_registered) {     /* if pm handle is registered, just   */
                                       /* skip                               */
    pm = &local->pm;
                                       /* zero out pm handle structure       */
    bzero ( (char *)pm, sizeof(struct pm_handle) );

                                       /* make area for logical name string  */
    pm->device_logical_name = (char *)
                    xmalloc(strlen(dds_local->logical_name)+1, 2, pinned_heap);
    if (pm->device_logical_name) {     /* if successfully allocated,         */

                                       /* copy logical name in dds to pm     */
                                       /* structure                          */
      strcpy(pm->device_logical_name, dds_local->logical_name);

                                       /* initialize pm handle structure     */
      pm->mode                = PM_DEVICE_FULL_ON;
      pm->handler             = (int (*)())pm_proc;
      pm->devno               = devno;
      pm->attribute           = PM_GRAPHICAL_INPUT;
      pm->pm_version          = 0x100;

                                       /* register pm structure to pm core   */
      rc = pm_register_handle( pm, PM_REGISTER);
      if (rc == PM_ERROR) {            /* if error,                          */
                                       /* log error...                       */
        io_error("reg_pm", FALSE, PM_REG_ERROR, 0, "");
                                       /* free allocated area                */
        xmfree( (caddr_t)pm->device_logical_name, pinned_heap);
      }
      else {
                                       /* remember pm struct is registered   */
        local->com.pm_registered = TRUE;
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             ureg_pm                                                 */
/*                                                                           */
/* FUNCTION:         This routine unregisters pm handle structure            */
/*                                                                           */
/* INPUTS:           none                                                    */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ureg_pm()
{
  struct pm_handle *pm;


  pm = &local->pm;
                                       /* if registered,                     */
  if (local->com.pm_registered == TRUE) {
                                       /* unregister the pm handle structure */
    pm_register_handle( pm, PM_UNREGISTER);

    local->com.pm_registered = FALSE;  /* remember pm struct is unresigtered */

                                       /* free area for logical name         */
    xmfree((caddr_t) pm->device_logical_name, pinned_heap);
  }
}
#endif /* PM_SUPPORT */

