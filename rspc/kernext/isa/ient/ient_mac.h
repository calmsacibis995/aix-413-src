/* @(#)64       1.2.1.2  src/rspc/kernext/isa/ient/ient_mac.h, isaent, rspc41J, 9517A_all 4/21/95 13:06:11 */
/*
 * COMPONENT_NAME: isaent - IBM ISA Bus Ethernet Device Driver
 *
 * FUNCTIONS:      none.
 *
 * ORIGINS:        27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when combined with the
 *                      aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************/
#ifndef _H_IENT_MAC
#define _H_IENT_MAC

#include <sys/time.h>
#include <sys/trcmacros.h>

/* Ioctl cases. */
#define IEN_ISA_CONFIGURE                0x00
#define IEN_ISA_SET_ADDRESS              0x01
#define IEN_ISA_SET_MULTICAST            0x02

/* Packet types, bit-masks. */
#define IEN_ISA_INDIVIDUAL_PACKET        0x00
#define IEN_ISA_BCAST_PACKET             0x01
#define IEN_ISA_MCAST_PACKET             0x02

#define IENT_XMIT_TIMEOUT                  10      /* In seconds. */

/* Command flags and options. */
#define PROMIS_ON     (0x01)           /* CONFIGURE - promiscuous on */
#define BC_DIS        (0x02)           /* CONFIGURE - broadcast disable */
#define SAVE_BP       (0x04)           /* CONFIGURE - save bad packet */

/* Internal trace table size and trace macros */

#ifdef DEBUG
#define IEN_ISA_TRACE_SIZE    1024            /* 4096 bytes, 256 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)    \
                  ient_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)    \
                  ient_trace(hook, tag, arg1, arg2, arg3)
#else
#define IEN_ISA_TRACE_SIZE     256            /* 512 bytes, 32 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)    \
        TRCHKGT((hook << 20)| HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)    
#endif 

#define TRACE_BOTH(hook, tag, arg1, arg2, arg3)    \
                  ient_trace(hook, tag, arg1, arg2, arg3)
/*
 * Trace points defined for the netpmon performance tool
 */

#define TRC_WQUE     "WQUE"     /* write data has just been queued */
#define TRC_WEND     "WEND"     /* write is complete               */
#define TRC_RDAT     "RDAT"     /* a packet was received           */
#define TRC_RNOT     "RNOT"     /* a packet was passed up          */

/* manage transmit buffers */
#define XMIT_BUF_AVAIL \
	  ((WRK.buf0.state == EMPTY && WRK.next_buf_load == BUF0) || \
  	  (WRK.buf1.state == EMPTY && WRK.next_buf_load == BUF1)) 

#define WHICH_XMIT_BUF_AVAIL(x) \
	{if (WRK.buf0.state == EMPTY && WRK.next_buf_load == BUF0) {\
              x = &(WRK.buf0);	            \
	      WRK.buf0.state == LOADING;    \
	 }\
       	 else if (WRK.buf1.state == EMPTY && WRK.next_buf_load == BUF1){ \
               	  x = &(WRK.buf1); \
		  WRK.buf1.state == LOADING;  \	
	      } \
	      else \
		    x = NULL;}

#define XMIT_BUF_READY \
	((WRK.buf0.state == READY_TO_XMIT && WRK.next_buf_xmit == BUF0) || \
	(WRK.buf1.state == READY_TO_XMIT && WRK.next_buf_xmit == BUF1))
		   
	
#define WHICH_XMIT_BUF_READY(x) \
    {if (WRK.buf0.state == READY_TO_XMIT && WRK.next_buf_xmit == BUF0) { \
       	      	    x = &(WRK.buf0);	\
		    WRK.buf0.state == TRANSMITTING; \
     } \
     else if (WRK.buf1.state == READY_TO_XMIT && WRK.next_buf_xmit == BUF1) { \
             x = &(WRK.buf1); \
	     WRK.buf1.state == TRANSMITTING; \
	   } \
	   else \
	     x = NULL;}

#define XMIT_BUF_TRANSMITTING ((WRK.buf0.state == TRANSMITTING) || \
		       (WRK.buf1.state == TRANSMITTING))

#define CUR_NUM_XMIT_BUFS ((WRK.buf0.state != EMPTY) + (WRK.buf1.state != EMPTY))

/* 
 * The component dump table will have the dd_ctl table,  and one dev_ctl 
 * for each adapter that is opened. Leave one extra entry always empty for
 * the table management.
 */

#define IEN_ISA_CDT_SIZE    (2 + IEN_ISA_MAX_ADAPTERS)
#define FILTER_TABLE_LEN    64    /* max 1-byte filter can set on adpt */

/* Delay mili-seconds */
#define DELAYMS(ms)         delay((int)(((ms)*HZ+999)/1000))

/* Set and start system timer for the mili-second specified */

#define STIMER_MS(ms) { \
    p_dds->systimer->timeout.it_value.tv_sec = ms / 1000; \
        p_dds->systimer->timeout.it_value.tv_nsec = (ms % 1000) * 1000000; \
    tstart(p_dds->systimer); \
}

/* Compare two ethernet network addresses */

#define SAME_NADR(a, b)    ( \
       *((ulong *)(a)) == *((ulong *)(b)) && \
       *((ushort *)((char *)(a) + 4)) == *((ushort *)((char *)(b) + 4)) \
            )
/* Copy a ethernet network address from a to b */

#define COPY_NADR(a, b)    { \
          *((ulong *)(b)) = *((ulong *)(a)); \
          *((ushort *)((char *)(b) + 4)) = *((ushort *)((char *)(a) + 4)); \
        }

/*
 * calculate the elapsed time by subtract the start tick counter (s) from
 * the current tick counter (lbolt) and convert it to seconds
 */

#define ELAPSED_TIME(s)     ((lbolt - s) / HZ)

#define WAITQ_FULL (WRK.xmits_queued >= DDI.xmt_que_size)

#define GET_FREE_ELEM(x) { WRK.xmits_queued++;                            \
                           x = WRK.tx_next_free;                          \
                           WRK.tx_next_free = WRK.tx_next_free->next;     \
                           x->next = NULL; }

#define ADD_2_FREE_Q(x) {                                            \
                       if (WRK.xmits_queued == DDI.xmt_que_size ) {  \
                         WRK.tx_next_free = x;                       \
                         WRK.tx_last_free = x;                       \
                       } else {                                      \
                           x->next = WRK.tx_next_free;               \
                           WRK.tx_next_free = x;                     \
                       }                                             \
                       WRK.xmits_queued--; }

#define ADD_2_WAIT_Q(x) { if (!WRK.nbr_in_wait_q) {                  \
                         WRK.tx_next_wait = x;                       \
                         WRK.tx_last_wait = x;                       \
                          } else {                                   \
                              WRK.tx_last_wait->next = x;            \
                              WRK.tx_last_wait = x;                  \
                            }                                        \
                        WRK.nbr_in_wait_q++;  			     \
			}

#define GET_WAIT_ELEM(x) { WRK.nbr_in_wait_q--;                      \
                           x = WRK.tx_next_wait;                     \
                           WRK.tx_next_wait = WRK.tx_next_wait->next;\
			   x->next = NULL;}


/* Updates the specified statistics counter. */

#define IEN_ISA_STAT_INCREMENT(lsw, msw)                \
   {                                                    \
     if (lsw == ULONG_MAX)                              \
       msw++;                                           \
     lsw++;                                             \
   }

#endif /* _H_IENT_MAC */
