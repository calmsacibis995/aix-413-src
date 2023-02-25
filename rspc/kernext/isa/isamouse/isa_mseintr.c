static char sccsid[] = "@(#)78  1.4  src/rspc/kernext/isa/isamouse/isa_mseintr.c, isamouse, rspc41J, 9521A_all 5/22/95 03:09:31";
/*
 *   COMPONENT_NAME: isamouse
 *
 *   FUNCTIONS: mseintr
 *              mouse_proc
 *              mouse_frame
 *              mouse_err
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
#define ABS(x) ((x<0) ? -(x) : (x))

#define MOUSE_RPT local->mouse_rpt

extern struct io_map  *io_map_ptr;
volatile uchar  *ioaddr;

#ifdef PM_SUPPORT
#include <sys/time.h>
time_t last_time_stamp = 0;
time_t curr_time_stamp;
#endif /* PM_SUPPORT */

/*****************************************************************************/
/*                                                                           */
/* NAME:             mseintr                                                 */
/*                                                                           */
/* FUNCTION:         This module is the interrupt handler for the mouse,     */
/*                   is called by the First Level Interrupt Handle when an   */
/*                   interrupt is detected from one of the devices on our    */
/*                   interrupt level.  This module determines if the mouse   */
/*                   caused the interrupt.                                   */
/*                                                                           */
/* INPUTS:           intr_ptr = pointer to our interrupt data                */
/*                                                                           */
/* OUTPUTS:          INTR_SUCC if an mouse interrupt, INTR_FAIL otherwise    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int mseintr(struct intr *intr_ptr)
{
    struct common  *com;
    struct mouse_port  *port;
    int rc;
    static int		phase = 0;
    char		data;

    KTSMDTRACE0(mseintr, enter);

    com = &local->com;                 /* pointer to common data area        */
    rc = INTR_FAIL;                         
                                       /* get access to I/O bus              */

    ioaddr = iomem_att(io_map_ptr);

    data = BUSIO_GETC(ioaddr+KM_CMD_STATUS_PORT);

    if (data & M_DATA_WAITING != M_DATA_WAITING) {
      i_reset(intr_ptr);               /*   reset system interrupt           */
      iomem_det( (void *)ioaddr);      /* detach from bus                    */
      return(INTR_SUCC);               /* This is because we don't share     */
                                       /* interrupts.                        */
    }

    data = BUSIO_GETC(ioaddr+KM_DATA_PORT);

    if (com->in_progress & MSE_ACK_RQD ||
        com->in_progress & RCV_MSE) {
      local->mouse_rpt = data;
      mouse_frame(com, port);
      phase=0;
    }
    else {

#ifdef PM_SUPPORT
      if (com->pm_in_progress) {       /* mouse report is ignored while pm   */
                                       /* event is being processed           */
        phase = 0;
        local->mouse_rpt = 0;
      }
      else {

        switch( phase ) {
          case 0:
            local->mouse_rpt = data<<16;
            break;
          case 1:
            local->mouse_rpt = local->mouse_rpt | data<<8;
            break;
          case 2:
            local->mouse_rpt = local->mouse_rpt | data;
            break;
        }

        phase++;
        if (phase == 3) {
                                       /* we got a full packet               */
          phase = 0;
          mouse_proc();
        }
      }
#else
      switch( phase ) {
        case 0:
          local->mouse_rpt = data<<16;
          break;
        case 1:
          local->mouse_rpt = local->mouse_rpt | data<<8;
          break;
        case 2:
          local->mouse_rpt = local->mouse_rpt | data;
          break;
      }

      phase++;
      if (phase == 3) {
                                       /* we got a full packet               */
        phase = 0;
        mouse_proc();
      }
#endif /* PM_SUPPORT */

    }
    i_reset(intr_ptr);                /*   reset system interrupt            */
    rc = INTR_SUCC;                   /*   indicate this was mouse interrupt */
    
    iomem_det( (void *)ioaddr);
    KTSMTRACE(mseintr, exit, rc, MOUSE_RPT, com->cur_frame,
              com->in_progress, 0);
    return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             mouse_proc                                              */
/*                                                                           */
/* FUNCTION:         Process mouse report                                    */
/*                                                                           */
/* INPUTS:           none                                                    */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void mouse_proc(void)
{
   struct ir_mouse event;
   uchar buttons;


   buttons = 0;                        /* zero out button status             */

                                       /* dump event if overflow             */
   if (!(MOUSE_RPT & M_X_DATA_OVERFLOW) &&
       !(MOUSE_RPT & M_Y_DATA_OVERFLOW)) {

                                       /* check if left button is down       */
      if (MOUSE_RPT & M_L_BUTTON_STATUS)
         buttons |= MOUSEBUTTON1;

                                       /* check if center button is down     */
      if (MOUSE_RPT & M_C_BUTTON_STATUS)
         buttons |= MOUSEBUTTON2;

                                       /* check if right button is down      */
      if (MOUSE_RPT & M_R_BUTTON_STATUS)
         buttons |= MOUSEBUTTON3;

                                       /* update x accumulations             */
      if (MOUSE_RPT & M_X_DATA_SIGN)
         local->mouse_hor_accum -= (short)((~(MOUSE_RPT >> 8)+1) & 0xff);
      else
         local->mouse_hor_accum += (short)((MOUSE_RPT >> 8) & 0xff);

                                       /* update y accumulations             */
      if (MOUSE_RPT & M_Y_DATA_SIGN)
         local->mouse_ver_accum -= (short)((~MOUSE_RPT+1) & 0xff);
      else
         local->mouse_ver_accum += (short)(MOUSE_RPT & 0xff);

#ifdef PM_SUPPORT
#define TIME_THRESHOLD   1
#define MOVE_THRESHOLD   2
                                       /* if button status is changed,       */
                                       /* movement is larger than threshold  */
                                       /* or two events occur within 1-2 sec */
      curr_time_stamp = time;
      if ( (local->button_status != buttons) ||
           ( !(MOUSE_RPT & M_X_DATA_SIGN) &&
             (((MOUSE_RPT >> 8) & 0xff) > MOVE_THRESHOLD) ) ||
           ( (MOUSE_RPT & M_X_DATA_SIGN) && 
             (((~(MOUSE_RPT >> 8) + 1) & 0xff) > MOVE_THRESHOLD) ) ||
           ( !(MOUSE_RPT & M_Y_DATA_SIGN) &&
             ((MOUSE_RPT & 0xff) > MOVE_THRESHOLD) ) ||
           ( (MOUSE_RPT & M_Y_DATA_SIGN) &&
             (((~MOUSE_RPT + 1) & 0xff) > MOVE_THRESHOLD) ) ||
           ((curr_time_stamp - last_time_stamp) <= TIME_THRESHOLD)
         ) {
                                       /* then this is not                   */
                                       /* trackpoint's drifting movement     */
        local->pm.activity = 1;        /* turn on the PM actibity flag       */
      }
      last_time_stamp = curr_time_stamp;
#endif /* PM_SUPPORT */
 
      KTSMDTRACE(mseproc, event, buttons, local->mouse_hor_accum,
           local->mouse_ver_accum, local->mouse_hor_thresh,
           local->mouse_ver_thresh);

                                       /* if accumulations exceed thresholds */
                                       /* or there is change in button status*/
                                       /* then                               */
      if ((ABS(local->mouse_hor_accum) >=  local->mouse_hor_thresh) ||
         (ABS(local->mouse_ver_accum) >=  local->mouse_ver_thresh) ||
         (local->button_status != buttons))  {
                                       /* put event on input ring            */
         event.mouse_header.report_size = sizeof(event);
         event.mouse_deltax = local->mouse_hor_accum;
         event.mouse_deltay = local->mouse_ver_accum;
         event.mouse_status = buttons;
         ktsm_putring(&local->rcb, (struct ir_report *) &event, NULL);

                                       /* update saved status                */
         local->button_status = buttons;
         local->mouse_hor_accum   = 0;
         local->mouse_ver_accum   = 0;
      }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             mouse_frame                                             */
/*                                                                           */
/* FUNCTION:         Process one frame response                              */
/*                                                                           */
/* INPUTS:           com = pointer to commone area                           */
/*                   port = pointer to attached I/O ports                    */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            RESEND response from mouse is ignored. If we are sending*/
/*                   frame, watch dog timer will pop and frame will be       */
/*                   resent.                                                 */
/*                                                                           */
/*****************************************************************************/

void mouse_frame(struct common *com, struct mouse_port  *port)
{

                                       /* enqueue frame to input queue if    */
                                       /* in receive mode                    */
   if (com->in_progress & RCV_MSE) {
       put_iq(com, (IFRAME) (MOUSE_RPT));
   }
   else {                              /* if expecting "ACK"                 */
     if (com->in_progress & MSE_ACK_RQD) {
                                       /* and "ACK" received then            */
	 if ((MOUSE_RPT) == M_ACK) { 
	     tstop(com->wdtimer);          /* stop watch dog timer               */
	     /*  if ACK from RESET or READ STATUS  */
	     /*  cmd then                          */
	     if ((com->cur_frame == M_RESET) ||
		 (com->cur_frame == M_STATUS_REQ)) {
		 /*   flush input queue                */
		 com->inq.head = com->inq.elements;
		 com->inq.tail = com->inq.elements;
		 /*   switch to receive mode           */
		 com->in_progress = RCV_MSE;
		 com->retry_cnt = 0;        /*   reset retry counter              */
		 e_wakeup(&com->asleep);     /*    wake up any waiting process     */
	     }
	     else {                        /*  else                              */
		 send_q_frame(com);        /*     send nxt frame to mouse        */
	     }
	 }
	 else {                          /* if resend received instead of ack  */
	     /* then indicate bad frame rcv'ed     */
	     /* (see note - watch dog will resend) */
	     if (((MOUSE_RPT) == M_INV) ||
		 ((MOUSE_RPT) == M_RESEND)) {
		 com->in_progress |= OP_RCV_ERR;  
	     }
	 }
     }
 }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             mouse_err                                               */
/*                                                                           */
/* FUNCTION:         Process interface errors                                */
/*                                                                           */
/* INPUTS:           com = pointer to commone area                           */
/*                   port = pointer to attached I/O ports                    */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            Errors that occur while sending frames are ignored.     */
/*                   Recovery is performed when watch dog timer pop's        */
/*                                                                           */
/*                   If retries have been exhausted, code just exits and     */
/*                   lets watch dog timer routine or wait_iq() clean up      */
/*                                                                           */
/*                   An assumption is made that if mouse is not sending      */
/*                   commands or rcv'ing their responses then the adapter    */
/*                   is blocked                                              */
/*                                                                           */
/*****************************************************************************/

void mouse_err(struct common *com, struct mouse_port  *port)
{

   int   count;
   char data;
   long ldata;

                                       /* if receiving frames from mouse     */
                                       /*   send RESEND to mouse if retries  */
                                       /*   have not been exhausted          */
                                       /*   NOTE: flush input queue as mouse */
                                       /*     resends entire response which  */
                                       /*     may consist of multiple frames */
   if (com->in_progress & RCV_MSE) {
       com->in_progress |= OP_RCV_ERR;   /*    <-- indicate bad frame rcv'ed   */
       if (com->retry_cnt < M_RETRY) {
	   com->retry_cnt++;
	   com->inq.head = com->inq.elements;
	   com->inq.tail = com->inq.elements;
	   BUSIO_PUTC(ioaddr+KM_CMD_STATUS_PORT, M_RESEND);
	   __iospace_sync();
       }
   }
   else {                              /* if sending frames to mouse let     */
                                       /*   watch dog timer handle error but */
                                       /*   change timer value if RESET cmd  */
                                       /*   sent just in case mse executed   */
                                       /*   the RESET cmd                    */
     if (com->in_progress & MSE_ACK_RQD) {
       com->in_progress |= OP_RCV_ERR; /*    <-- indicate bad frame rcv'ed   */
       if (com->cur_frame == M_RESET) {
         startwd(com, (void *) watch_dog, RSTWD);
       }
     }
     else {
                                      /* if interface error then most likely */
                                      /* mouse has been unplugged but will   */
                                      /* attempt to recover anyway ...       */
       if (MOUSE_RPT & M_FACE_ERR) {
                                      /* but first log error                 */
         io_error("mouse_err", FALSE, RCV_ERROR, 1, "%08x", MOUSE_RPT);



      /* fell out of loop...assume that block mode is disabled. If not, the  */
      /* rest of this code will not be effective but will not be fatal either*/

                                      /* flush adapter's fifo's              */
	 ldata = BUSIO_GETC(ioaddr+KM_DATA_PORT);
	 ldata = BUSIO_GETC(ioaddr+KM_DATA_PORT);
	 ldata = BUSIO_GETC(ioaddr+KM_DATA_PORT);
	 ldata = BUSIO_GETC(ioaddr+KM_DATA_PORT);

	 BUSIO_PUTC(ioaddr+KM_CMD_STATUS_PORT, M_ENABLE);
	 __iospace_sync();
     }
       else {                          /* else treat all other conditions as */
                                       /* data error ...                     */

                                       /*   send RESEND cmd so mouse         */
                                       /*   will re-xmit report              */
	   BUSIO_PUTC(ioaddr+KM_CMD_STATUS_PORT, M_RESEND);
	   __iospace_sync();
                                       /*   bump error counter               */
	   if ((com->rcv_err_cnt++) > RCV_ERR_THRESHOLD) {
	       /*   if over threshold, log error     */
	       io_error("mouse_err", FALSE, RCV_ERROR, 2, "%08x", MOUSE_RPT);
	       com->rcv_err_cnt = 0;       /*   and reset counter                */
	   }
       }
   }
 }
}


