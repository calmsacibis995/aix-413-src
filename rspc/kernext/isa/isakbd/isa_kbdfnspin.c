static char sccsid[] = "@(#)71  1.18  src/rspc/kernext/isa/isakbd/isa_kbdfnspin.c, isakbd, rspc41J, 9521A_all 5/23/95 09:06:21";
/*
 *   COMPONENT_NAME: isakbd
 *
 *   FUNCTIONS: send_q_frame
 *              watch_dog
 *              io_error
 *              reg_intr
 *              ureg_intr
 *              sv_rflush
 *              sv_sak
 *              sv_alarm
 *              put_sq
 *              next_sound
 *              stop_sound
 *              mem_dump
 *              service_vector
 *              my_cdt
 *              send_8042_cmd
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


#include "kbd.h"

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
/* NOTES:       Frames are stored on the output queue using format defined   */
/*              by the hardware for the RS1/RS2 platforms. The actual        */
/*              keyboard frame is located in the high order byte.            */
/*                                                                           */
/*              No check is made for data in 8042 output buffer. The         */
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

    volatile uchar      *ioaddr;
    uchar   data;

    KTSMDTRACE1(k_send_q_frame, enter, com->outq.out_cnt);

    if (com->outq.out_cnt == 0) {      /* if entire transmission group sent  */
                                       /*   point head to start of nxt group */
      com->outq.head = com->outq.dq_ptr;
                                       /* if nothing on queue then           */
      if (com->outq.head == com->outq.tail) {
        com->in_progress=NO_OP;        /*   indicate nothing going on        */
        km_io_lock->lock = NOT_LOCKED; /*   unlock adapter                   */
        e_wakeup(&com->asleep);        /*   wake up any waiting process      */
        return;                        /*   exit                             */
      }
      else {                           /* if queue is not empty then         */
                                       /*   get frame cnt for next group     */
        com->outq.out_cnt = get_oq(com);
        com->retry_cnt = 0;            /*   clear retry counter              */
      }
    }

    ioaddr = iomem_att(io_map_ptr);    /* get access to bus                  */
                                       /* get adapter status                 */
    data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT);
                                       /* if adapter is locked by mouse      */
                                       /* or 8042 input buffer is full then  */
                                       /* sleep for 200ms and retry          */
    if ((km_io_lock->lock & LOCKED_BY_MSE) |
       (data & TX_FULL)) {
      startwd(com, (void *) watch_dog, WDOGFULL);
                                       /* indicate that driver is waiting    */
      com->in_progress = UNLOCK_WAIT;  /* for adapter to unlock              */
    }
    else {
                                       /* lock adapter                       */
      km_io_lock->lock = LOCKED_BY_KBD;
      com->cur_frame = get_oq(com);    /* get frame from queue               */
                                       /* write frame to adapter             */
      BUSIO_PUTC(ioaddr+KM_DATA_PORT, (char) (com->cur_frame >> 8));
                                       /* start 50ms watchdog timer          */
      startwd(com, (void *) watch_dog, WDOGTIME);
      com->in_progress = KBD_ACK_RQD;  /* indicate I/O in progress           */
    }

    iomem_det( (void *)ioaddr);

    KTSMTRACE(k_send_q_frame, exit, com->cur_frame,
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

    com = &local->com;                 /* pointer to common area             */

    KTSMTRACE(k_watch_dog, enter, com->cur_frame, com->in_progress,
              com->retry_cnt, local->status, local->data);

                                       /* if waiting for ACK or adapter      */
                                       /* to unlock then ...                 */
    if (com->in_progress & (KBD_ACK_RQD | UNLOCK_WAIT)) {

      if (com->retry_cnt < K_RETRY) {  /* if retry count is not exceeded     */
        com->retry_cnt++;              /*   update retry count               */
        com->outq.dq_ptr =             /*   reset ptr back to start of       */
        com->outq.head;                /*     transmission group             */
                                       /*   get number of frames in group    */
        com->outq.out_cnt = get_oq(com);
      }
      else {                           /* else retries are exhausted so      */
#ifndef GS_DEBUG_TRACE
        if (com->cur_frame != KBD_RESET_CMD)
#endif
                                       /*   log error                        */
          io_error("watch_dog", FALSE, KIO_ERROR, 0, "%02x %02x %04x %04x",
                 local->status, local->data, com->cur_frame, com->in_progress);
        while(com->outq.out_cnt) {     /* flush transmission group           */
          get_oq(com);
        }
        com->outq.error = TRUE;        /* indicate that there was a problem  */
      }
      send_q_frame(com);               /* process next frame                 */
    }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        io_error                                                     */
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
    struct common *com;

    va_start(arg,fmt);                 /* log error                          */
    _vsprintf(ss, fmt, arg);
    ktsm_log(RES_NAME_KBD, rout, err_code, loc, ss);

    if (perm) {                        /* if hard error then ...             */
        com = &local->com;             /*   pointer to common data struct    */
        flush_q(com);                  /*   reset I/O queues                 */
        com->perr = TRUE;              /*   remember we're hosed             */
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
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int reg_intr(int adpt_rst)
{
  int    rc;
  char   tmp;
  struct common  *com;
  volatile uchar *ioaddr;

  com = &local->com;                   /* pointer to common area             */
  if (com->perr) {
    return(EIO);                       /* exit if permanent error posted     */
  }

                                       /* pin bottom half of driver          */
  if (rc = pincode((int (*) ())kbdintr)) {
    return(rc);                        /* exit if error                      */
  }

  flush_q(com);                        /* initialize I/O queues              */
  com->outq.error = FALSE;             /* no errors yet                      */

                                       /* tell kernel about slih             */
  com->intr_data.next     = 0;
  com->intr_data.handler  = (int (*) ())kbdintr;
  com->intr_data.bus_type = BUS_BID;
  com->intr_data.flags    = 0;
  com->intr_data.level    = com->intr_level;
  com->intr_data.priority = com->intr_priority;
  com->intr_data.bid      = com->bus_id;

  if (i_init(&com->intr_data) != INTR_SUCC) {
    unpincode((int (*) ())kbdintr);    /* if error, unpin bottom half and    */
    return(EPERM);                     /*   return                           */
  }
                                       /* enable kbd interface               */
  if (send_8042_cmd(EN_KBD_INTR)) {
                                       /* if error ...                       */
    i_clear(&com->intr_data);          /* undefine interrupt handler         */
    unpincode((int (*) ())kbdintr);    /* unpin bottom half and return       */
    return(EIO);
  }

  local->ih_inst = TRUE;               /* interrupt handler is installed     */
  dmp_add(mem_dump);                   /* register system crash local memory */
                                       /* dump routine                       */

                                       /* make sure alarm hardware is off..  */
  ioaddr = iomem_att(io_map_ptr);      /* get access to bus                  */
                                       /* get value that's out there         */
  tmp = BUSIO_GETC(ioaddr+SPEAKER_ENABLE_PORT);
  tmp = (tmp & 0x0c) | SPEAKER_OFF;    /* turn speaker off                   */
  BUSIO_PUTC(ioaddr+SPEAKER_ENABLE_PORT, tmp);
  iomem_det( (void *)ioaddr);
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

    com = &local->com;                 /* pointer to common area             */
    stop_sound();                      /* stop alarm and clear queue         */
    wait_oq(com);                      /* wait for all I/O to complete       */

    if (local->ih_inst) {              /* if interrupt handler installed     */
                                       /* disable kbd interface              */
       send_8042_cmd(DIS_KBD_INTERFACE);
       i_clear(&com->intr_data);       /* undefine interrupt handler         */
       dmp_del(mem_dump);              /* unregister local memory dump rout. */
       unpincode((int (*) ())kbdintr); /* unpin bottom half                  */
       local->ih_inst = FALSE;         /* interrupt handler is not installed */
    }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        sv_rflush                                                    */
/*                                                                           */
/* FUNCTION:    This module flushes the input ring. It is called via         */
/*              the service vector                                           */
/*                                                                           */
/* INPUTS:      devno = device number (ignored)                              */
/*              arg   = pointer to NULL (ignored)                            */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              -1 = kernel has not opened channel                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int sv_rflush(dev_t devno, caddr_t arg)
{
    int rc = -1;
    if (local) {
        rc = sv_proc(&local->com, &local->key, KSVRFLUSH, arg);
    }
    return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sv_sak                                                       */
/*                                                                           */
/* FUNCTION:    Enable/disable SAK. This routine is called via               */
/*              the service vector                                           */
/*                                                                           */
/* INPUTS:      devno = device number (ignored)                              */
/*              arg = pointer to int containing:                             */
/*                    KSSAKDISABLE = disable SAK                             */
/*                    KSSAKENABLE  = enable SAK                              */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EINVAL = invalid parameter                                   */
/*              -1 = kernel has not opened channel                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       No attempt is made to recover key events if in SAK and       */
/*              disable is received                                          */
/*                                                                           */
/*****************************************************************************/

int sv_sak(dev_t devno, caddr_t arg)
{
    int rc = -1;
    if (local) {
        rc = sv_proc(&local->com, &local->key, KSVSAK, arg);
    }
    return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sv_alarm                                                     */
/*                                                                           */
/* FUNCTION:    Sound alarm. This routine is called via the service          */
/*              vector                                                       */
/*                                                                           */
/* INPUTS:      devno = device number (ignored)                              */
/*              arg = pointer to ksalarm structure                           */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EINVAL = invalid parameter                                   */
/*              EBUSY = queue is full, request ignored                       */
/*              EIO = I/O error                                              */
/*              -1 = kernel has not opened channel                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int sv_alarm(dev_t devno, caddr_t arg)
{
    int rc = -1;
    if (local) {
        rc = sv_proc(&local->com, &local->key, KSVALARM, arg);
    }
    return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        put_sq                                                       */
/*                                                                           */
/* FUNCTION:    Put request on sound queue                                   */
/*                                                                           */
/* INPUTS:      arg   = pointer to ksalarm structure                         */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EINVAL = invalid parameter                                   */
/*              EBUSY = queue is full                                        */
/*              EIO = I/O error                                              */
/*              EPERM = could not allocate timer                             */
/*                                                                           */
/* ENVIRONMENT: Process or interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int put_sq(struct ksalarm *arg)
{
    int rc;
    int tmp;
    int freq;
    struct sound_q *sqe;


    KTSMDTRACE(put_sq, enter, arg->duration, arg->frequency, 0,0,0);

                                       /* make sure parms are valid          */
    if ((arg->duration > MAX_ALARM_DURATION) ||
        (arg->frequency > MAX_ALARM_FREQ) ||
        (arg->frequency < MIN_ALARM_FREQ)) {
      rc = EINVAL;
    }
    else {                             /* sound alarm only if some volume    */
                                       /* and some time                      */
      if ((local->key.volume == SPEAKER_OFF) ||
          (arg->duration == 0)) {
        rc = 0;
      }
      else {
                                       /* return busy if queue is full       */
        if ((local->sq_tail + 1 ) == local->sq_head) {
          rc = EBUSY;
        }
        else {                         /* put request on queue               */
          sqe = &local->sound_q[local->sq_tail];

          sqe->sec =  arg->duration/128;
          sqe->nsec = (arg->duration % 128) * (NS_PER_SEC/128);

          freq = (COUNTER_CLOCK_FREQ / arg->frequency);
          sqe->locmd = (uchar )(freq);
          sqe->hicmd= (uchar)(freq>>8);

          KTSMTRACE(put_sq, qe, sqe->sec, sqe->nsec,
                    sqe->hicmd, sqe->locmd, local->sq_tail);

                                       /* update sound queue tail index      */
          local->sq_tail = (local->sq_tail < (SOUND_Q_SIZE-2)) ?
                           (local->sq_tail+1) : 0;

                                       /* start critical section             */
          tmp = i_disable(local->com.intr_priority);

                                       /* if not sounding alarm              */
          if (!local->alarm_active) {
            rc = next_sound();         /*   start sound request              */
          }
          else {                       /* else do nothing more               */
            rc = 0;
          }
          i_enable(tmp);               /* end critical section               */
        }
      }
    }

    return(rc);

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        next_sound                                                   */
/*                                                                           */
/* FUNCTION:    process next sound request on queue                          */
/*                                                                           */
/* INPUTS:      none                                                         */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EIO = I/O error                                              */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       This routine is called directly by put_sq() when the alarm   */
/*              is off and it is called when the sound timer pop's.          */
/*                                                                           */
/*****************************************************************************/

int next_sound(void)
{

    int rc = EIO;
    char tmp;
    struct kbd_port *port;
    volatile uchar      *ioaddr;
    struct sound_q *sqe;

    ioaddr = iomem_att(io_map_ptr);    /* get access to bus */

                                       /* if queue is empty                  */
    if (local->sq_head == local->sq_tail)  {
                                       /* get value that's out there */
      tmp = BUSIO_GETC(ioaddr+SPEAKER_ENABLE_PORT);
                                       /* turn speaker off   */
      tmp = (tmp & 0x0c) | SPEAKER_OFF;
      BUSIO_PUTC(ioaddr+SPEAKER_ENABLE_PORT, tmp);

                                       /* alarm is off                       */
      local->alarm_active = FALSE;
      rc = 0;                          /* ok return code                     */
    }
    else {                             /* else process next request ...      */
                                       /* write sound data to adapter        */
      BUSIO_PUTC(ioaddr+CONTROL_WORD_PORT, MODE3);
      sqe = &local->sound_q[local->sq_head];
      BUSIO_PUTC(ioaddr+SPEAKER_TONE_PORT, sqe->locmd);
      __iospace_sync();
      BUSIO_PUTC(ioaddr+SPEAKER_TONE_PORT, sqe->hicmd);

                                       /* get value that's out there */
      tmp = BUSIO_GETC(ioaddr+SPEAKER_ENABLE_PORT);
      tmp = (tmp & 0x0c) | SPEAKER_ON; /* turn speaker on */
      BUSIO_PUTC(ioaddr+SPEAKER_ENABLE_PORT, tmp);

                                       /* alarm is on                        */
      local->alarm_active = TRUE;
                                       /* zero out timer structure           */
      bzero (local->sdd_time_ptr, sizeof(struct trb));
                                       /* set time interval                  */
      local->sdd_time_ptr->timeout.it_value.tv_sec = sqe->sec;
      local->sdd_time_ptr->timeout.it_value.tv_nsec = sqe->nsec;
      local->sdd_time_ptr->flags |= T_INCINTERVAL;
                                       /* timer interrupt handler            */
      local->sdd_time_ptr->func = (void (*) ()) next_sound;
                                       /* timer interrupt priority           */
      local->sdd_time_ptr->ipri = local->com.intr_priority;

                                       /* start timer                        */
      tstart(local->sdd_time_ptr);
                                       /* update sound queue head index      */
      local->sq_head = (local->sq_head < (SOUND_Q_SIZE-2)) ?
            (local->sq_head+1) : 0;

      rc = 0;                          /* all ok                             */
    }
    iomem_det( (void *)ioaddr);

    KTSMTRACE(next_sound, exit, rc, local->sq_head,
              local->sq_tail, 0, 0);

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        stop_sound                                                   */
/*                                                                           */
/* FUNCTION:    Stop current sound, flush sound queue, and free timer        */
/*                                                                           */
/* INPUTS:      none                                                         */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void stop_sound(void)
{
    int tmp;

                                       /* start critical section             */
    tmp = i_disable(local->com.intr_priority);

    if (local->alarm_active) {         /* if alarm is on then                */
      tstop(local->sdd_time_ptr);      /*    stop alarm timer                */
      local->sq_head = 0;              /*    flush alarm queue               */
      local->sq_tail = 0;
      next_sound();                    /*    turn off alarm                  */
    }
    i_enable(tmp);                     /* end critical section               */
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
/* NAME:        service_vector                                               */
/*                                                                           */
/* FUNCTION:    externalize certain routines                                 */
/*                                                                           */
/*****************************************************************************/

void * service_vector[] =  {
    (void *) sv_alarm,                 /* routine to sound alarm             */
    (void *) sv_sak,                   /* routine to enable/disable sak      */
    (void *) sv_rflush                 /* routine to flush queue             */
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
  {DMP_MAGIC,                          /*   magic number                     */
                                       /*   component dump name              */
  {'k', 'b', 'd', 'd', 'd', ' ', ' ', ' ',
   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
   sizeof(struct cdt)},                /*   length                           */
                                       /* cdt entry:                         */
                                       /*   name of data area                */
  {{{'l', 'o', 'c', 'a', 'l', ' ', ' ', ' '},
    sizeof(struct local),              /*   length of data area              */
    NULL,                              /*   address of area                  */
    {0},                               /*   cross memory parms               */
    }}};


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
                                       /* synchronization with mouse dd      */
  lvl = i_disable(com->intr_priority);
                                       /* retry 5 K_RETRY times              */
  for(retry = 0; retry< K_RETRY; retry++) {

                                       /* get adapter status                 */
    data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT);
                                       /* if adapter is locked by mouse      */
                                       /* or 8051 input buffer is full then  */
                                       /* sleep for 200ms and retry          */
    if ((km_io_lock->lock & LOCKED_BY_MSE) |
       (data & TX_FULL)) {
      ktsm_sleep(com, FRAME_WAIT);
    }
    else {                             /* lock adapter                       */
      km_io_lock->lock = LOCKED_BY_KBD;
                                       /* tell 8051 that we want to update   */
                                       /* command register                   */
      BUSIO_PUTC(ioaddr+KM_CMD_STATUS_PORT, WRITE_CCB_CMD);
                                       /* wait 30ms for request to be        */
      ktsm_sleep(com, RCVTO);          /* processed                          */
                                       /* if request processed then give     */
                                       /* adapter new cmd byte               */
      if (!((data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT)) & TX_FULL)) {
        km_io_lock->cmd_save = (km_io_lock->cmd_save &
              ~(DIS_KBD_INTERFACE | EN_KBD_INTR )) | cmd;
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
    io_error("send_8042", FALSE, KIO_ERROR, 0, "%02x %02x %02x %02x",
             cmd, km_io_lock->cmd_save, data, retry);
  }

  KTSMTRACE(k_send_8042_cmd, exit, rc, cmd, km_io_lock->cmd_save, data, 0);

  return(rc);
}


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
  struct kbdext *key;
  ushort save_kbdstat = 0;
  int    pm_effect_int, pm_effect_ext;

                                       /* get lock                            */
  if (lockl(&kbd_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = PM_ERROR;
  }
  else {
    com = &local->com;
    key = &local->key;
    com->pm_in_progress = 1;           /* pm event is being processed...      */

    if (local->pm.mode != mode) {      /* if requested mode is not same as    */
                                       /* current mode,                       */
      switch (mode) {
        case PM_DEVICE_FULL_ON:
        case PM_DEVICE_IDLE:
                                       /* if previous mode is not 'enable',   */
          if (local->pm.mode != PM_DEVICE_ENABLE) {
            rc = PM_ERROR;             /* set error ...                       */
          }
          break;

        case PM_DEVICE_ENABLE:

          switch (local->pm.mode) {
            case PM_DEVICE_SUSPEND:
                                       /* If embedded keyboard is powered off */
                                       /* then turn on keyboard power         */
              pm_effect_int = pm_planar_control(key->kbd_devno,
                                                PMDEV_INTKBD,
                                                PM_PLANAR_QUERY);
              if (pm_effect_int & PM_PLANAR_ON) {
                pm_planar_control(key->kbd_devno, PMDEV_INTKBD, PM_PLANAR_ON);
              }
                                       /* If external keyboard is powered off */
                                       /* then turn on keyboard power         */
              pm_effect_ext = pm_planar_control(key->kbd_devno,
                                                PMDEV_EXTKBD,
                                                PM_PLANAR_QUERY);
              if (pm_effect_ext & PM_PLANAR_ON) {
                pm_planar_control(key->kbd_devno, PMDEV_EXTKBD, PM_PLANAR_ON);
              }
                                       /* If embedded or external keyboard    */
                                       /* was powered on then wait for kbd    */
                                       /* to complete power up sequence       */
              if ( (pm_effect_int & PM_PLANAR_ON) ||
                   (pm_effect_ext & PM_PLANAR_ON) ) {
                ktsm_sleep(com,POR_WAIT);
              }
                                       /* fall into case PM_DEVICE_HIBERNATION*/
                                       /* to reset and initialize keyboard    */

            case PM_DEVICE_HIBERNATION:

              if (reg_intr(FALSE)) {   /* define interrupt handler and        */
                                       /* enable keyboard interface           */
                rc = PM_ERROR;
              }
              else {
                                       /* reset keyboard (fixes H8 sequence   */
                get_kbd_id(com,key);   /* problem seen on laptops)            */

                                       /* if no channels are open then        */
                                       /* disable interface again             */
                if (key->act_ch == NO_ACT_CH) {
                  ureg_intr();
                }
                else {                 /* at least one channel is open ...    */
                                       /* save previous shift status          */
                  save_kbdstat = key->kbdstat;

                                       /* initialize keyboard and enable      */
                                       /* keyboard scan                       */
                                       /* if keyboard can not wake up here    */
                                       /* return error                        */
                  rc = keysetup(com,key);
                                       /* restore shift and led status.       */
                                       /* do nothing if return code is bad    */
                                       /* because error return code stops     */
                                       /* the system from resuming.           */
                  key_stat( com, key,
                           (ushort)(KATAKANA | CAPS_LOCK | NUM_LOCK),
                           (ushort)save_kbdstat);
                }
              }

              if (!rc)                 /* if all ok,                          */
                local->pm.activity = 0;/* reset activity flag to 0 so that    */
                                       /* pm core can touch the activity      */
                                       /* flag.                               */
              break;

            default:                   /* if previous mode is not suspend     */
              break;                   /* or hibernation, just break          */
          }
          break;

        case PM_DEVICE_SUSPEND:
        case PM_DEVICE_HIBERNATION:
                                       /* if previous mode is not 'enable',   */
          if (local->pm.mode != PM_DEVICE_ENABLE) {
            rc = PM_ERROR;             /* set error ...                       */
          }
          else {                       /* if previous mode is 'enable',       */

                                       /* if keyboard is open,                */
            if (key->act_ch != NO_ACT_CH) {
                                       /* turn off led lights                 */
              put_oq2( com, (OFRAME) SET_LED_CMD,
                      (OFRAME)( (((ushort)ALL_LEDS_OFF)<<8) | WRITE_KBD_CMD) );

              ureg_intr();             /* disable keyboard interface          */
            }
                                       /* whether pm_planar_control() is      */
                                       /* effective or not, call it anyway... */
            pm_planar_control(key->kbd_devno, PMDEV_INTKBD, PM_PLANAR_OFF);
            pm_planar_control(key->kbd_devno, PMDEV_EXTKBD, PM_PLANAR_OFF);

            local->pm.activity = -1;   /* set activity flag to -1 so that     */
                                       /* keyboard driver can get 'enable'    */
                                       /* command at resume                   */
          }
          break;

        case PM_PAGE_FREEZE_NOTICE:    /* pm related data area is pinned      */
          break;                       /* at config, and pm related codes     */
                                       /* are pinned at device open.          */
                                       /* So just return with PM_SUCCESS      */

        case PM_PAGE_UNFREEZE_NOTICE:  /* pm related codes are unpinned       */
          break;                       /* at device close, and pm related     */
                                       /* data area is unpinned at config.    */
                                       /* So just return with PM_SUCCESS      */

        default:                       /* unknown/invalid request coming      */
          rc = PM_ERROR;               /* set error ...                       */
          break;
      }
    }

    if (!rc) {                         /* if all ok and request is not        */
                                       /* page freeze/unfreeze, update        */
                                       /* device mode                         */
      if ( (mode != PM_PAGE_FREEZE_NOTICE) &&
           (mode != PM_PAGE_UNFREEZE_NOTICE) ) {
          local->pm.mode = mode;
      }
    }
    else {
      rc = PM_ERROR;                   /* error...                            */
    }

    com->pm_in_progress = 0;           /* pm event processing is done         */
    unlockl(&kbd_lock);                /* release lock                        */
  }
  return(rc);
}
#endif /* PM_SUPPORT */

