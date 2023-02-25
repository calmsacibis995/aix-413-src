/* @(#)72	1.3  src/rspc/kernext/pcmcia/ssisa/pcmciassdd.h, pcmciaker, rspc41J, 9511A_all 3/13/95 20:46:05  */
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: PCMCIA socket services private headder file
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PCMCIASSDD
#define _H_PCMCIASSDD

#include <sys/pcmciass.h>
#include <sys/watchdog.h>
#include <sys/lockl.h>

/* PCMCIA I/O register index offsets */

#define PCM_ID_REVISION       0x00 /* Identification and Revision         */
#define PCM_IF_STAT           0x01 /* Interface Status                    */
#define PCM_PWR_RESETDRV      0x02 /* Power and Resetdrv Control          */
#define PCM_INTR_CTRL         0x03 /* Interrupt and General Control       */
#define PCM_STAT_CHG          0x04 /* Card Status Change                  */
#define PCM_STAT_CHG_INTR     0x05 /* Card Status Change Inerrupt Config. */
#define PCM_WINDOW_ENABLE     0x06 /* Address Window Enable               */
#define PCM_IO_CTRL           0x07 /* I/O Control                         */

#define PCM_IO0_START_LOW     0x08 /* I/O Address 0 Start Low  Byte */
#define PCM_IO0_START_HI      0x09 /* I/O Address 0 Start High Byte */
#define PCM_IO0_STOP_LOW      0x0A /* I/O Address 0 Stop  Low  Byte */
#define PCM_IO0_STOP_HI       0x0B /* I/O Address 0 Stop  High Byte */
#define PCM_IO1_START_LOW     0x0C /* I/O Address 0 Start Low  Byte */
#define PCM_IO1_START_HI      0x0D /* I/O Address 1 Start High Byte */
#define PCM_IO1_STOP_LOW      0x0E /* I/O Address 1 Stop  Low  Byte */
#define PCM_IO1_STOP_HI       0x0F /* I/O Address 1 Stop  High Byte */

                                   /* System Memory Address 3 Mapping */
#define PCM_MEM0_START_LOW    0x10 /*     Start Low  Byte             */
#define PCM_MEM0_START_HI     0x11 /*     Start High Byte             */
#define PCM_MEM0_STOP_LOW     0x12 /*     Stop  Low  Byte             */
#define PCM_MEM0_STOP_HI      0x13 /*     Stop  High Byte             */

                                   /* Card Memory Offset Address 3    */ 
#define PCM_OFFSET0_LOW       0x14 /*     Stop  Low  Byte             */
#define PCM_OFFSET0_HI        0x15 /*     Stop  High Byte             */


#define PCM_CARD_DETECT        0x16 /* Card Detect and General Control */
#define PCM_RESERVED0          0x17 /* Reserved                        */
									 
                                    /* System Memory Address 1 Mapping */
#define PCM_MEM1_START_LOW     0x18 /*     Start Low  Byte             */
#define PCM_MEM1_START_HI      0x19 /*     Start High Byte             */
#define PCM_MEM1_STOP_LOW      0x1A /*     Stop  Low  Byte             */
#define PCM_MEM1_STOP_HI       0x1B /*     Stop  High Byte             */
									                                      
                                    /* Card Memory Offset Address 1    */
#define PCM_OFFSET1_LOW        0x1C /*     Stop  Low  Byte             */
#define PCM_OFFSET1_HI         0x1D /*     Stop  High Byte             */

#define PCM_GLOBAL_CTRL         0x1E /* Global Control                  */
#define PCM_RESERVED1           0x1F /* Reserved                        */

                                     /* System Memory Address 2 Mapping */
#define PCM_MEM2_START_LOW      0x20 /*     Start Low  Byte             */
#define PCM_MEM2_START_HI       0x21 /*     Start High Byte             */
#define PCM_MEM2_STOP_LOW       0x22 /*     Stop  Low  Byte             */
#define PCM_MEM2_STOP_HI        0x23 /*     Stop  High Byte             */
									                                     
                                     /* Card Memory Offset Address 2    */
#define PCM_OFFSET2_LOW         0x24 /*     Stop  Low  Byte             */
#define PCM_OFFSET2_HI          0x25 /*     Stop  High Byte             */

                                     /* System Memory Address 3 Mapping */ 
#define PCM_MEM3_START_LOW      0x28 /*     Start Low  Byte             */
#define PCM_MEM3_START_HI       0x29 /*     Start High Byte             */
#define PCM_MEM3_STOP_LOW       0x2A /*     Stop  Low  Byte             */
#define PCM_MEM3_STOP_HI        0x2B /*     Stop  High Byte             */
									                                      
                                     /* Card Memory Offset Address 3    */
#define PCM_OFFSET3_LOW         0x2C /*     Stop  Low  Byte             */
#define PCM_OFFSET3_HI          0x2D /*     Stop  High Byte             */

#define PCM_RESERVED2           0x2E /* Reserved                        */
#define PCM_RESERVED3           0x2F /* Reserved                        */

                                     /* System Memory Address 3 Mapping */
#define PCM_MEM4_START_LOW      0x30 /*     Start Low  Byte             */
#define PCM_MEM4_START_HI       0x31 /*     Start High Byte             */
#define PCM_MEM4_STOP_LOW       0x32 /*     Stop  Low  Byte             */
#define PCM_MEM4_STOP_HI        0x33 /*     Stop  High Byte             */
									                                      
									 /* Card Memory Offset Address 3    */
#define PCM_OFFSET4_LOW         0x34 /*     Stop  Low  Byte             */
#define PCM_OFFSET4_HI          0x35 /*     Stop  High Byte             */

#define PCM_RESERVED4           0x36 /* Reserved                        */
#define PCM_RESERVED5           0x37 /* Reserved                        */
#define PCM_RESERVED6           0x38 /* Reserved                        */
#define PCM_RESERVED7           0x39 /* Reserved                        */
#define PCM_RESERVED8           0x3A /* Reserved                        */
#define PCM_RESERVED9           0x3B /* Reserved                        */
#define PCM_RESERVED10          0x3C /* Reserved                        */
#define PCM_RESERVED11          0x3D /* Reserved                        */
#define PCM_RESERVED12          0x3E /* Reserved                        */
#define PCM_ACCESS_INDLAMP      0x3F /* Access Inicator Lamp Register   */

#define IPL_MAX_SECS    10      /* timeout for card diagnostics */

#define BIT_0 0x01
#define BIT_1 0x02
#define BIT_2 0x04
#define BIT_3 0x08
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80

#define ioreg_index( X, Y ) ( (X) + (Y) * 0x40 )
#define pcmcia_word(A,B,C,D) ((A)<<24|(B)<<16|(C)<<8|(D))
#define pcmcia_wait_ms(X) delay(HZ * (X) / 1000 + ( HZ * (X) % 1000 ? 1 : 0))
#define pcmcia_xor( X, Y )  ( ~ ((X) & (Y)) & ((X) | (Y)) )
#define pcmcia_win2socket(W) ((W)/NUM_WINDOWS)


struct pcmcia_ddi {

    ulong       bus_id;             /* adapter I/O bus id value     */
    ushort      bus_type;           /* adapter I/O bus type         */
    uchar       slot;               /* I/O slot number              */
    int         io_addr;            /* I/O address                  */
    int         int_lvl;            /* interrupt level              */
    int         int_prior;          /* interrupt priority           */
    char        resource_name[16];  /* resource name for err log*/
    int		int_mode;	    /* interrupt mode : Edge: 'N' or Level: 'I' */
};

struct socket_def { 
	int SCIntMask;
	uchar status_change;
	uchar old_state;
	uchar CtlInd;
	uchar old_value[0x40];
};

struct window_def {
	SetWindow_S setwindow_s;
	SetPage_S   setpage_s;
};

struct adapter_def {
    dev_t devno;            /* adapter major/minor */
    struct adapter_def *next; /* pointer to next str. */
    lock_t ap_lock;         /* per adapter lock     */
	struct pcmcia_ddi ddi;  /* passed init data */
    uchar inited;             /* adapter initalized */
    uchar opened;             /* adapter first opened */
	uchar adapter_mode;     /* mode opened in:      */
						    /*  0 = normal mode     */
						    /*  1 = diagnostic mode */
#define NORMAL_MODE     0   /*  normal operation    */
#define DIAG_MODE       1   /*  diagnostic mode     */

	/* PCMCIA specific definitions */
	uchar num_sockets;               /* number of the sockets */
	uchar as_maintain;               /* maintenance mode */
	int sc_int_mask;                 /* enabled status change interrupt */
	struct socket_def socket[4];     /* PCMCIA socekt status */
	struct window_def window[4 * 7]; /* PCMCIA window setup */
};

#define MAX_SOCKETS_REAL 4                /* max number of sockts */
#define MAX_SOCKETS 2                /* max number of sockts */
#define PCM_EXCLUSIVE 0x01           /* exclusive mode open  */

#endif /* _H_PCMCIASSDD */
