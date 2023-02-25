static char sccsid[] = "@(#)77  1.12  src/rspc/kernext/isa/isamouse/isa_msefnspin.c, isamouse, rspc41J, 9521A_all 5/23/95 09:05:35";
/*
 *   COMPONENT_NAME: isamouse
 *
 *   FUNCTIONS: send_q_frame
 *              watch_dog
 *              io_error
 *              reg_intr
 *              ureg_intr
 *              mem_dump
 *              send_8042_cmd
 *              mse_button_cmds
 *              my_cdt
 *              pm_proc
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

#include "mse.h"

struct io_map    *io_map_ptr=NULL;
struct  local *local=NULL;             /* local data anchor                  */
struct  _km_io_lock *km_io_lock=NULL;  /* I/O lock structure                 */

int send_8042_cmd(char);

/*****************************************************************************/
/*                                                                           */
/* NAME:        send_q_frame()                                               */
/*                                                                           */
/* FUNCTION:    Send next frame on output queue to device                    */
/*                                                                           */
/* INPUTS:      com = pointer to common data area                            */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/*                                                                           */
/* NOTES:       No check is made for data in 8042 output buffer. The         */
/*              assumption is that upon exit of this routine, the interrupt  */
/*              handler will process the data BEFORE the response generated  */
/*              by the device (for command/data being sent) is received by   */
/*              the 8042. This is really no worse than normal case of having */
/*              both the keyboard and mouse interfaces enabled and assuming  */
/*              that the interrupt handlers will process the data before     */
/*              another event is received.                                   */
/*                                                                           */
/*****************************************************************************/

void send_q_frame(struct common *com)
{

    volatile uchar  *ioaddr;
    uchar   data;

    KTSMDTRACE0(m_send_q_frame, enter);

    if (com->outq.out_cnt == 0) {      /* group complete so ...              */
                                       /* point head to start of nxt group   */
      com->outq.head = com->outq.dq_ptr;
                                       /* if nothing on queue then           */
      if (com->outq.head == com->outq.tail) {
        com->in_progress=NO_OP;        /* indicate nothing going on          */
        km_io_lock->lock = NOT_LOCKED; /* unlock adapter                     */
        e_wakeup(&com->asleep);        /* wake up any waiting process        */
        return;                        /* exit                               */
      }
      else {                           /* if queue is not empty then         */
                                       /* get frame cnt for next group       */
        com->outq.out_cnt = get_oq(com);
        com->retry_cnt = 0;            /* clear retry counter                */
      }
    }


    ioaddr = iomem_att(io_map_ptr);    /* get access to bus                  */
                                       /* get adapter status                 */
    data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT);
                                       /* if adapter is locked by keyboard   */
                                       /* or 8042 input buffer is full then  */
                                       /* sleep for 200ms and retry          */
    if ((km_io_lock->lock & LOCKED_BY_KBD) |
       (data & TX_FULL)) {
      startwd(com, (void *) watch_dog, WDOGFULL);
                                       /* indicate that driver is waiting    */
      com->in_progress = UNLOCK_WAIT;  /* for adapter to unlock              */
    }
    else {
                                       /* lock adapter                       */
      km_io_lock->lock = LOCKED_BY_MSE;
      com->cur_frame = get_oq(com);    /* get frame from queue               */
                                       /* Tell adapter that next command is  */
                                       /* a mouse command                    */
      BUSIO_PUTC(ioaddr+KM_CMD_STATUS_PORT, MOUSE_CMD);
      com->in_progress = ADPT_ACK_RQD; /* indicate mouse prep cmd sent       */
                                       /* sleep for 50ms to give adapter     */
                                       /* time to process command            */
      startwd(com, (void *) watch_dog, WDOGTIME);
    }

    iomem_det( (void *)ioaddr);


    KTSMTRACE(m_send_q_frame, exit, com->cur_frame,
         com->in_progress, data, com->outq.dq_ptr, com->outq.out_cnt);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        watch_dog                                                    */
/*                                                                           */
/* FUNCTION:    Service watch dog timer interrupt                            */
/*                                                                           */
/* INPUTS:      tb = address of watchdog trb                                 */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       This is the I/O watch dog timer service routine. It gets     */
/*              control when the watch dog timer pops because the device     */
/*              did not acknowledge a transmission within the time limit.    */
/*              If the retry count has not been exceeded, this routine resets*/
/*              dq_ptr to the start of the transmission group and calls      */
/*              send_q_frame() to start the re-transmission of all frames    */
/*              in the transmission group.                                   */
/*                                                                           */
/*****************************************************************************/

void watch_dog(struct trb *tb)
{
   struct common *com;
   volatile uchar  *ioaddr;
   uchar data;

   com = &local->com;                  /* pointer to common area             */

   KTSMTRACE(m_watch_dog, enter, com->cur_frame, com->in_progress,
             com->retry_cnt, tb->flags, com->outq.head);

                                       /* if waiting for mouse prep command  */
                                       /* to complete then ...               */
   if(com->in_progress & ADPT_ACK_RQD ) {
     ioaddr = iomem_att(io_map_ptr);   /* get access to bus                  */
                                       /* fatal error if prep cmd has not    */
                                       /*   been processed                   */
     if ((data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT)) & TX_FULL) {
       io_error("watch_dog", TRUE, MIO_ERROR, 0, "%02x %04x",
            data, com->in_progress);
     }
     else {
                                       /* output frame to mouse              */
       BUSIO_PUTC(ioaddr+KM_DATA_PORT, com->cur_frame);
       com->in_progress = MSE_ACK_RQD; /* waiting for ACK from mouse         */
                                       /* start 50ms watch dog timer         */
       startwd(com, (void *) watch_dog, WDOGTIME);
     }
     iomem_det( (void *)ioaddr);
   }
   else {
                                       /* if waiting for mouse ACK or adapter*/
                                       /* to unlock then ...                 */
     if (com->in_progress & (MSE_ACK_RQD | UNLOCK_WAIT)) {
                                       /* if retry count is not exceeded     */
       if (com->retry_cnt < M_RETRY) {
         com->retry_cnt++;             /*   update retry count               */
         com->outq.dq_ptr =            /*   reset ptr back to start of       */
         com->outq.head;               /*     transmission group             */
                                       /*   get number of frames in group    */
          com->outq.out_cnt = get_oq(com);
          send_q_frame(com);           /*   resend first frame in group      */
       }
       else {                          /* else retries are exhausted so      */
                                       /*   log error                        */
#ifndef GS_DEBUG_TRACE
         if (com->cur_frame != M_RESET)
#endif
         io_error("watch_dog", FALSE, MIO_ERROR, 0, "%08x %04x %04x",
             local->mouse_rpt,  com->cur_frame, com->in_progress);
         flush_q(com);                 /* flush queues to wake up top half   */
       }
     }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        io_error                                                     */
/*                                                                           */
/* FUNCTION:    Log I/O error                                                */
/*                                                                           */
/* INPUTS:      rout = name of routine detecting error                       */
/*              perm = TRUE if hard error                                    */
/*              err_code = error code to be logged                           */
/*              loc = location code to be logged                             */
/*              fmt = format string for detailed information (or NULL)       */
/*                    (this is a limited printf like fmt string)             */
/*              ... = detailed information to be formated                    */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process or Interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void io_error(char *rout, int perm, uint err_code, uint loc, char *fmt, ...)
{
    va_list arg;
    char ss[30];

    va_start(arg,fmt);                 /* log error                          */
    _vsprintf(ss, fmt, arg);
    ktsm_log(RES_NAME_MSE, rout, err_code, loc, ss);

    if(perm) {                         /* if hard error then                 */
      flush_q(&local->com);            /* reset I/O queues                   */
      local->com.perr = TRUE;          /* remember we're hosed               */
    }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        reg_intr                                                     */
/*                                                                           */
/* FUNCTION:    Register interrupt handler                                   */
/*                                                                           */
/* INPUTS:      adpt_rst (not used)                                          */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EPERM = failed to register interrupt handler                 */
/*              ENOMEM = unable to pin driver, out of memory                 */
/*              EIO = I/O error                                              */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int reg_intr(int adpt_rst)
{
    struct common *com;
    long rc;


    com = &local->com;                 /* point to common extension          */
    if (com->perr) return(EIO);        /* exit if permanent error posted     */

                                       /* pin bottom half of driver          */
    if (rc = pincode((int (*) ())mseintr))
       return(rc);                     /* exit if error                      */

    flush_q(com);                      /* initialize I/O queues              */
    com->outq.error = FALSE;           /* no errors yet                      */
    local->block_mode = FALSE;         /* not in block mode                  */

                                       /* tell kernel about slih             */
    com->intr_data.next     = 0;
    com->intr_data.handler  = (int (*) ())mseintr;
    com->intr_data.bus_type = BUS_BID;
    com->intr_data.flags    = 0;
    com->intr_data.level    = com->intr_level;
    com->intr_data.priority = com->intr_priority;
    com->intr_data.bid      = com->bus_id;

    if (i_init(&com->intr_data) != INTR_SUCC) {
       unpincode((int (*) ())mseintr); /* if error, unpin bottom half and    */
       return(EPERM);                  /*   return                           */
    }

                                       /* enable mouse(auxillary) interface  */
    if (rc = send_8042_cmd(EN_AUX_INTR)) {
                                       /* if error ...                       */
      i_clear(&com->intr_data);        /* undefine interrupt handler         */
      unpincode((int (*) ())mseintr);  /*  unpin bottom half and return      */
      return(EIO);
    }

    dmp_add(mem_dump);                 /* register system crash local memory */
                                       /* dump routine                       */

    return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ureg_intr                                                    */
/*                                                                           */
/* FUNCTION:    Un-register interrupt handler                                */
/*                                                                           */
/* INPUTS:      None                                                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ureg_intr(void)
{
    struct      common *com;

    com = &local->com;                 /* point to common extension          */
    send_8042_cmd(DIS_AUX_INTERFACE);  /* disable mouse interface            */

    i_clear(&local->com.intr_data);    /* undefine interrupt handler         */
    dmp_del(mem_dump);                 /* unregister local memory dump rout. */
    unpincode((int (*) ())mseintr);    /* unpin bottom half                  */
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        mem_dump                                                     */
/*                                                                           */
/* FUNCTION:    returns address of areas to be included in system dump       */
/*                                                                           */
/* INPUTS:      pass = 1 for start dump                                      */
/*                     2 for end of dump                                     */
/*                                                                           */
/* OUTPUTS:     address of component dump table structure                    */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

struct cdt * mem_dump(int pass)
{
                                       /* address of local data structure    */
    my_cdt.cdt_entry[0].d_ptr = (caddr_t) local;
    return(&my_cdt);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        send_8042_cmd                                                */
/*                                                                           */
/* FUNCTION:    send cmd to adapter                                          */
/*                                                                           */
/* INPUTS:      cmd -- command sent to adapter                               */
/*                                                                           */
/* OUTPUTS:     0     -- success                                             */
/*              non 0 -- failure                                             */
/*                                                                           */
/* ENVIRONMENT: Process only                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int send_8042_cmd(uchar cmd)
{

  volatile uchar *ioaddr;
  int   retry;
  uchar data;
  int rc = 1;
  int   lvl;
  struct common  *com;

  com = &local->com;                   /* pointer to common area             */

  ioaddr = iomem_att(io_map_ptr);      /* get access to bus                  */
                                       /* disable to INTCLASS3 to assure     */
                                       /* synchronization with keyboard dd   */
  lvl = i_disable(com->intr_priority);
                                       /* retry M_RETRY times                */
  for(retry = 0; retry< M_RETRY; retry++) {

                                       /* get adapter status                 */
    data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT);
                                       /* if adapter is locked by keyboard   */
                                       /* or 8051 input buffer is full then  */
                                       /* sleep for 200ms and retry          */
    if ((km_io_lock->lock & LOCKED_BY_KBD) |
       (data & TX_FULL)) {
      ktsm_sleep(com, FRAME_WAIT);
    }
    else {                             /* lock adapter                       */
      km_io_lock->lock = LOCKED_BY_MSE;
                                       /* tell 8051 that we want to update   */
                                       /* command register                   */
      BUSIO_PUTC(ioaddr+KM_CMD_STATUS_PORT, WRITE_CCB_CMD);
                                       /* wait 30ms for request to be        */
      ktsm_sleep(com, RCVTO);          /* processed                          */
                                       /* if request processed then give     */
                                       /* adapter new cmd byte               */
      if (!((data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT)) & TX_FULL)) {
        km_io_lock->cmd_save = (km_io_lock->cmd_save &
              ~(DIS_AUX_INTERFACE | EN_AUX_INTR )) | cmd;
        BUSIO_PUTC(ioaddr+KM_DATA_PORT, km_io_lock->cmd_save);
        rc = 0;
      }
                                       /* unlock adapter                     */
      km_io_lock->lock = NOT_LOCKED;
      break;
    }
  }
  i_enable(lvl);                       /* restore interrupts                 */
  iomem_det( (void *)ioaddr);

                                       /* if failed, then log error          */
  if (rc) {
    io_error("send_8042", FALSE, MIO_ERROR, 0, "%02x %02x %02x %02x",
             cmd, km_io_lock->cmd_save, data, retry);
  }

  KTSMTRACE(m_send_8042_cmd, exit, rc, cmd, km_io_lock->cmd_save, data, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        mse_button_cmds                                              */
/*                                                                           */
/* FUNCTION:    commands to read number of buttons on mouse                  */
/*                                                                           */
/*****************************************************************************/

OFRAME mse_button_cmds[] = {
    6,                                 /* frame count                        */
    M_SET_RES,
    M_RES_1,
    M_RESET_SCALE,
    M_RESET_SCALE,
    M_RESET_SCALE,
    M_STATUS_REQ
    };

/*****************************************************************************/
/*                                                                           */
/* NAME:        my_cdt                                                       */
/*                                                                           */
/* FUNCTION:    define areas to be included in system dump                   */
/*                                                                           */
/*****************************************************************************/

struct cdt my_cdt =  {
                                       /* cdt_head:                          */
          {DMP_MAGIC,                  /*   magic number                     */
                                       /*   component dump name              */
          {'m', 'o', 'u', 's', 'e', 'd', 'd', ' ',
           ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
          sizeof(struct cdt)},         /*   length                           */
                                       /* cdt entry:                         */
                                       /*   name of data area                */
          {{{'l', 'o', 'c', 'a', 'l', ' ', ' ', ' '},
           sizeof(struct local),       /*   length of data area              */
           NULL,                       /*   address of area                  */
           {0},                        /*   cross memory parms               */
           }}};

#ifdef PM_SUPPORT
/*****************************************************************************/
/*                                                                           */
/* NAME:        pm_proc                                                      */
/*                                                                           */
/* FUNCTION:    This routine brings the device driver into a specified mode  */
/*                                                                           */
/* INPUTS:      private = pointer to a private data                          */
/*              mode = one of pm device modes or pm notices                  */
/*                                                                           */
/* OUTPUTS:     PM_SUCCESS = success                                         */
/*              PM_ERROR = failure                                           */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       This routine needs to be pinned to prevent page fault from   */
/*              occuring, because hard disk drive may not be working at the  */
/*              time of resuming.                                            */
/*                                                                           */
/*****************************************************************************/

int pm_proc( caddr_t private, int mode)
{
  int    rc = PM_SUCCESS;
  int    lvl;
  struct common *com;
  int    pm_effect_int, pm_effect_ext;
                                       /* get lock                           */
  if (lockl(&mse_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = PM_ERROR;
  }
  else {
    com = &local->com;
    com->pm_in_progress = 1;           /* now pm event is being processed... */

    if (local->pm.mode != mode) {      /* if requested mode is not same as   */
                                       /* current mode,                      */
      switch (mode) {
        case PM_DEVICE_FULL_ON:
        case PM_DEVICE_IDLE:
                                       /* if previous mode is not 'enable',  */
          if (local->pm.mode != PM_DEVICE_ENABLE) {
                                       /* set error ...                      */
            rc = PM_ERROR;
          }
          break;

        case PM_DEVICE_ENABLE:

          switch (local->pm.mode) {
            case PM_DEVICE_SUSPEND:
                                       /* query if pm_planar_control() is    */
                                       /* effective of not                   */
              pm_effect_int = pm_planar_control(local->mse_devno,
                                                PMDEV_INTMOUSE,
                                                PM_PLANAR_QUERY);
                                       /* if effective, turn on mouse anyway */
              if (pm_effect_int & PM_PLANAR_ON) {
                pm_planar_control(local->mse_devno,PMDEV_INTMOUSE,PM_PLANAR_ON);
              }

              pm_effect_ext = pm_planar_control(local->mse_devno,
                                                PMDEV_EXTMOUSE,
                                                PM_PLANAR_QUERY);
              if (pm_effect_ext & PM_PLANAR_ON) {
                pm_planar_control(local->mse_devno,PMDEV_EXTMOUSE,PM_PLANAR_ON);
              }

                                       /* wait POR if required               */
              if ( (pm_effect_int & PM_PLANAR_ON) ||
                   (pm_effect_ext & PM_PLANAR_ON) ) {
                                       /* wait for possible BAT and ID       */
                ktsm_sleep(com,POR_WAIT);
              }

            case PM_DEVICE_HIBERNATION:

              if (reg_intr(FALSE)) {   /* define interrupt handler and       */
                                       /* enable mouse interface             */
                rc = PM_ERROR;
              }
              else {
                get_id();              /* just for reset                     */

                if (!local->oflag) {   /* if mouse is not open then          */
                  ureg_intr();         /* disable interface again            */
                }
                else {
                                       /* clear status                       */
                  local->mouse_hor_accum    = 0;
                  local->mouse_ver_accum    = 0;
                  local->mouse_overflow     = 0;
                                       /* emulate button released and        */
                                       /* queue up event                     */
                  if (local->button_status) {
                      local->mouse_rpt = 0;
                      mouse_proc();
                  }
                                       /* because mouse is not totally       */
                                       /* broken at this point, return       */
                                       /* with pm success not to stop the    */
                                       /* system from resuming anyway        */

                                       /* set mouse to default condition     */
                  send_cmd((OFRAME) M_SET_DEFAULT, (OFRAME) NO_PARM);

                                       /* resume mse characteristics and     */
                                       /* enable mse report                  */
                  if (local->mouse_resolution != DEFAULT_RESOLUTION) {
                    set_resolution(local->mouse_resolution);
                  }
                  if (local->mouse_scaling != DEFAULT_SCALING) {
                    set_scale_factor(local->mouse_scaling);
                  }
                  if (local->mouse_sample_rate != DEFAULT_SAMPLE_RATE) {
                    set_sample_rate(local->mouse_sample_rate);
                  }
                }
              }

              if (!rc)                 /* if all ok,                         */
                local->pm.activity = 0;/* reset activity flag to 0 so that   */
                                       /* pm core can touch the activity     */
                                       /* flag.                              */
              break;

            default:                   /* if previous mode is not suspend    */
              break;                   /* or hibernation, just break         */
          }
          break;

        case PM_DEVICE_SUSPEND:
        case PM_DEVICE_HIBERNATION:
                                       /* if previous mode is not 'enable',  */
          if (local->pm.mode != PM_DEVICE_ENABLE) {
            rc = PM_ERROR;             /* set error ...                      */
          }
          else {                       /* if previous mode is 'enable',      */

            if (local->oflag) {        /* if mouse is open,                  */
              ureg_intr();             /* disable mouse interface            */
            }
                                       /* whether pm_planar_control() is     */
                                       /* effective of not, call it anyway...*/
            pm_planar_control(local->mse_devno, PMDEV_INTMOUSE, PM_PLANAR_OFF);
            pm_planar_control(local->mse_devno, PMDEV_EXTMOUSE, PM_PLANAR_OFF);

            local->pm.activity = -1;   /* if all ok, set activity flag to    */
                                       /* -1 so that mouse driver can        */
                                       /* get enable command at resume       */
          }
          break;

        case PM_PAGE_FREEZE_NOTICE:    /* pm related data area is pinned     */
          break;                       /* at config, and pm related codes    */
                                       /* are pinned at device open.         */
                                       /* So just return with PM_SUCCESS     */

        case PM_PAGE_UNFREEZE_NOTICE:  /* pm related codes are unpinned      */
          break;                       /* at device close, and pm related    */
                                       /* data area is unpinned at config.   */
                                       /* So just return with PM_SUCCESS     */

        default:                       /* unknown/invalid request coming     */
          rc = PM_ERROR;               /* set error ...                      */
          break;
      }
    }

    if (!rc) {                         /* if all ok and request is not       */
                                       /* page freeze/unfreeze, update       */
                                       /* device mode                        */
        if ( (mode != PM_PAGE_FREEZE_NOTICE) &&
             (mode != PM_PAGE_UNFREEZE_NOTICE) ) {
            local->pm.mode = mode;
        }
    }
    else {
      rc = PM_ERROR;                   /* error...                           */
    }

    com->pm_in_progress = 0;           /* pm event processing is done        */
    unlockl(&mse_lock);                /* release lock                       */
  }
  return(rc);
}
#endif /* PM_SUPPORT */

