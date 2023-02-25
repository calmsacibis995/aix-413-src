static char sccsid[] = "@(#)72  1.7  src/rspc/kernext/isa/isakbd/isa_kbdintr.c, isakbd, rspc41J, 9521A_all 5/23/95 09:06:32";
/*
 *   COMPONENT_NAME: isakbd
 *
 *   FUNCTIONS: kbdack
 *              kbderr
 *              kbdintr
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "kbd.h"

volatile uchar          *ioaddr;
extern struct io_map    *io_map_ptr;

/*****************************************************************************/
/*                                                                           */
/* NAME:             kbdintr                                                 */
/*                                                                           */
/* FUNCTION:         This module is the interrupt handler for the keyboard,  */
/*                   is called by the First Level Interrupt Handle when an   */
/*                   interrupt is detected from one of the devices on our    */
/*                   interrupt level.  This module determines if the kybd    */
/*                   caused the interrupt by checking the status of the stat */
/*                   port.  If it is an kybd interrupt then kbdintr must     */
/*                   determine the reason and process it accordingly.        */
/*                                                                           */
/* INPUTS:           intr_ptr = pointer to our interrupt data                */
/*                                                                           */
/* OUTPUTS:          INTR_SUCC if an kybd interrupt, INTR_FAIL otherwise     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  kbdintr(struct intr *intr_ptr)
{
  struct common   *com;
  struct kbd_port *port;

  KTSMDTRACE0(kbdintr, enter);

  com = &local->com;                   /* pointer to common data area        */
                                       /* get access to I/O bus              */
  ioaddr = iomem_att(io_map_ptr);      /* get access to bus                  */

                                       /* read kbd adapter status port       */
  local->status = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT);

                                       /* if no rcv data then not our intr   */
  if ((local->status & M_DATA_WAITING) != RX_FULL) {
    iomem_det( (void *)ioaddr);      
                                       /* isa bus does not share interrupts: */
    i_reset(intr_ptr);                 /*   reset system interrupt           */
    return(INTR_SUCC);                 /*   Don't return failed, just ignore */
  }
  else {                               /* process data when errors detected  */
    if (local->status & KEYERROR) {    /* by the adapter                     */
      kbderr(com,port);              
    }
    else {                             /* if good data rcv'ed from kbd       */
                                       /* read data from adapter port        */
      local->data = BUSIO_GETC(ioaddr+KM_DATA_PORT);
      switch (local->data) {
        case KBD_ACK:                  /* process ACK response               */
          kbdack(com);
          break;

        case OVERRUN:                  /* ignore overrun                     */

        case KBD_ECHO:                 /* should never get ECHO              */
          break;

        case KBD_RESEND:
          /* if PS/2 keyboard, reponse to Read Layout ID command,            */
          /* is RESEND. So in this case, RESEND is regarded as a             */
          /* valid response. If Layout command was not sent then just        */
          /* set error flag and watch dog routine will recover               */
          if (com->in_progress & KBD_ACK_RQD) {
            if (com->cur_frame == LAYOUT_ID_CMD) {
              tstop(com->wdtimer);
                                       /* flush input queue                  */
              com->inq.head = com->inq.elements;
              com->inq.tail = com->inq.elements;
                                       /* not waiting for ACK anymore        */
              com->in_progress = NO_OP;
                                       /* reset retry counter                */
              com->retry_cnt = 0;
                                       /* put RESEND into input queue        */
              put_iq(com, local->data);
              break;
            }

#ifdef DIRTY_PATCH

            /* This dirty code section is for 5576-C01 (kbd w/ track point   */
            /* II, Japanese) keyboard. If the keyboard is attached to the    */
            /* system without this patch, the system may hung up.            */
            /* This problem is caused by hardware bug.(some keys can not be  */
            /* set up correctly.) --keypos 131,132,133                       */

            if ((com->cur_frame == SET_106K_131_SCAN_CODE) ||
                (com->cur_frame == SET_106K_132_SCAN_CODE) ||
                (com->cur_frame == SET_HIRA_KEY_SCAN_CODE)) {
              tstop(com->wdtimer);
              send_q_frame(com);
              break;
            }

#endif /* DIRTY_PATCH */

            com->in_progress |= OP_RCV_ERR;
          }
          break;

        default:                       /* process other responses:           */
                                       /*   if in receive mode then enqueue  */
                                       /*   frame to input queue             */
          if ( com->in_progress & RCV_KBD) {
            put_iq(com, local->data);
          }
          else {                       /* else go process scan code          */
                                       /* if at least one channel is open    */
            if (local->key.act_ch != NO_ACT_CH) {

#ifdef PM_SUPPORT
                                       /* if pm event is not being processed */
              if (!(com->pm_in_progress)) {
                                       /* turn on the activity flag          */
                local->pm.activity = 1;
                keyproc(com, &local->key, local->data);
              }
#else  /* PM_SUPPORT */
              keyproc(com, &local->key, local->data);
#endif /* PM_SUPPORT */

            }
          }
          break;
        }
      }
  }
  iomem_det( (void *)ioaddr);
  i_reset(intr_ptr);                   /* reset interrupt                    */

  KTSMTRACE(kbdintr, exit, local->status, local->data, com->cur_frame,
            com->in_progress, local->key.act_ch);

  return(INTR_SUCC);                   /* return - interrupt serviced        */
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             kbderr                                                  */
/*                                                                           */
/* FUNCTION:         Process line errors                                     */
/*                                                                           */
/* INPUTS:           com = pointer to common area                            */
/*                   port = pointer to attached I/O ports                    */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            A 30ms watch dog timer is started when a frame is       */
/*                   sent to the keyboard. When the timer pop's, the timer   */
/*                   routine resends the first frame in the current          */
/*                   transmission group. The watch dog timer is stopped by   */
/*                   the keyboard interrupt handler when a valid response    */
/*                   is received from the keyboard. If a RESEND response     */
/*                   is received following a frame transmission or if a      */
/*                   receive parity error is detected by the hardware, the   */
/*                   interrupt handler just lets the watchdog timer pop      */
/*                   resulting in the retransmission of the frame.           */
/*                                                                           */
/*****************************************************************************/

void kbderr(struct common *com, struct kbd_port *port)
{

   if (com->in_progress == NO_OP) {    /* if scan code from keyboard         */
                                       /*   send RESEND cmd to keyboard      */
                                       /*   so keyboard will re-xmit frame   */
     BUSIO_PUTC(ioaddr+KM_DATA_PORT, (char) K_RESEND_CMD);
     __iospace_sync();
                                       /*   bump error counter               */
     if ((com->rcv_err_cnt++) > RCV_ERR_THRESHOLD) {
                                       /*   if over threshold, log error     */
       io_error("kbderr", FALSE, RCV_ERROR, 0, NULL);
       com->rcv_err_cnt = 0;           /*   and reset counter                */

     }
   }
   else {                              /* if receiving frames from kbd       */
                                       /*   send RESEND to kbd if retries    */
                                       /*   have not been exhausted          */
     if (com->in_progress & RCV_KBD) {
         com->in_progress |= OP_RCV_ERR;
       if (com->retry_cnt < K_RETRY) {
         com->retry_cnt++;
         BUSIO_PUTC(ioaddr+KM_DATA_PORT, (char) K_RESEND_CMD);
         __iospace_sync();
       }
     }
     else {                            /* if sending frames to kbd let       */
                                       /*   watch dog timer handle error but */
                                       /*   change timer value if RESET cmd  */
                                       /*   sent just in case kbd executed   */
                                       /*   the RESET cmd                    */
       if (com->in_progress & KBD_ACK_RQD) {
           com->in_progress |= OP_RCV_ERR;
         if (com->cur_frame == KBD_RESET_CMD) {
           startwd(com, (void *) watch_dog, RSTWD);
         }
       }
     }
   }                                   /* flush frame that had rcv error     */
   local->data = BUSIO_GETC(ioaddr+KM_DATA_PORT);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             kbdack                                                  */
/*                                                                           */
/* FUNCTION:         Process ACK response from keyboard                      */
/*                                                                           */
/* INPUTS:           com = pointer to commone area                           */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            KBD_RESET and READ_ID_CMD must be last command in       */
/*                   group                                                   */
/*                                                                           */
/*****************************************************************************/

void kbdack(struct common *com)
{

                                       /* if expecting ACK then              */
  if (com->in_progress & KBD_ACK_RQD) {
    tstop(com->wdtimer);               /* stop watch dog timer               */
                                       /* if ACK from RESET or READ ID       */
                                       /* cmd then                           */
    if ((com->cur_frame == KBD_RESET_CMD) ||
        (com->cur_frame == READ_ID_CMD)) {
                                       /*   flush input queue                */
      com->inq.head = com->inq.elements;
      com->inq.tail = com->inq.elements;
                                       /*   switch to receive mode           */
      com->in_progress = RCV_KBD;
      com->retry_cnt = 0;              /*   reset retry counter              */
      e_wakeup(&com->asleep);          /*   wake up any waiting process      */

    }
    else {                             /*  else                              */
      send_q_frame(com);               /*     send nxt frame to keyboard     */
    }
  }
}

