/* @(#)59	1.15  src/rspc/kernext/isa/tty/sf.h, isatty, rspc41J, 9523A_all 6/6/95 05:17:07 */
/*
 *   COMPONENT_NAME: isatty
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
#ifndef _SF_H
#define _SF_H

#ifdef PM_SUPPORT
#include <sys/pmdev.h>
#include <sys/lock_def.h>
#endif

#ifndef NSFCOM
#ifdef PCMCIA
#define NSFCOM  4 /* PCMCIA has 4 sockets */
#else
#define NSFCOM	2
#endif
#endif

#define SF_FIFOCNT 16			/* FIFOs are 16 bytes deep	*/

/*
 * Currently, I steal the "rs" driver hook word for sandalfoot.
 */
#define TRC_SF(w)   ((HKWD_STTY_RS)|w)

/*
 * sandalfoot serial device config struct.
 */
struct sf_config {
	ulong	bid;			/* BUS id			*/
	ulong	xtal;			/* clock freq			*/
	long	port;			/* I/O port offset		*/
	ushort	irq;			/* IRQ level			*/
	uchar	flags;			/* misc flags			*/
	uchar	rtrig;			/* receive trigger		*/
	uchar	tbc;			/* transmit buffer count	*/
};

#ifdef PCMCIA
struct pcmcia_dds {		/* PCMCIA data structure	*/
#ifdef PM_SUPPORT
	int bus_number;		/* bus# for pm_planar_control	*/
	int physical_socket;	/* physical socket		*/
#endif
	int logical_socket;	/* logical socket		*/
};
#endif

#ifdef PM_SUPPORT
struct pm_dds {			/* power management data structrue	*/
	int pm_dev_att;		/* pm device attribute			*/
	int pm_dev_itime;	/* pm time for idol			*/
	int pm_dev_stime;	/* pm time for stanby			*/
};
#endif

/*
 * device config struct
 */
struct sf_dds {
	char rc_name[16];		/* adapter name			*/
	uchar	rc_type;		/* adapter type			*/
	uchar	rc_flags;		/* config flags			*/
	struct sf_config rc_conf;	/* device config		*/
	struct termios rc_tios;		/* termios stuff		*/
	struct termiox rc_tiox;		/* termiox stuff		*/

#ifdef PM_SUPPORT
	struct pm_dds pm_dds;		/* power management data	*/
#endif

#ifdef PCMCIA
	struct pcmcia_dds pcmcia_dds;	/* PCMCIA data			*/
#endif
};

#define SFDDS_LINE	0x01		/* line specific parms valid	*/
#define SFDDS_DEVICE	0x02		/* device specifiv parms valid	*/

#ifdef _KERNEL

#define io_att(sf)	((volatile uchar*)\
			 iomem_att(&(sf)->sf_iom) + (sf)->sf_conf.port)
#define io_det(io)	iomem_det((ulong*)io)
#define SFIO_BID(num)	BID_VAL(IO_ISA, ISA_IOMEM, (num))

#endif
#if defined(_KERNEL) || defined(FOR_CRASH)

#ifdef PCMCIA
struct pcmcia		/* additional data of PCMCIA to sfcom structure */
{
	int	clientID;		/* Client ID			*/
	int	event_word;		/* For a e_sleep/e_wakeup event	*/
	int	configured;		/* flag for the devices who have */
					/* request resources to Card	*/
					/* services			*/
	int	logical_socket;		/* logical socket number	*/
#ifdef PM_SUPPORT
	int	physical_socket;	/* physical socket number	*/
#endif
	int	return_callback;	/* return code of callback	*/
	int	cb_flag;		/* -1:unconfig,1:callback,0:none  */
	int	no_card;		/* pc card insertion/removal flag */
	int	reg_comp;		/* registration complete flag	*/
	int	reset_comp;		/* reset complete flag		*/
	uchar	config_index;		/* config index		        */
};
#endif /* PCMCIA */

/*
 * one `sfcom' struct per device information structure.
 */
struct sfcom {
	struct intr sf_slih;		/* interrupt struct	*/
	struct sty_tty sf_tty;		/* tty structure	*/
	struct io_map sf_iom;		/* PIO I/O structure	*/
	struct sf_config sf_conf;	/* device config	*/
	char *sf_xdata;			/* ^ to nxt xmit char	*/
	uint sf_xcnt;			/* xmit count		*/
	unsigned sf_rcnt :8;		/* receive byte count	*/
	unsigned sf_cconf :1;		/* ctrl configured?	*/
	unsigned sf_uconf :1;		/* unit configured?	*/
	unsigned sf_open  :1;		/* device open		*/
	unsigned sf_pad	  :21;		/* pad to next word	*/
	/*
	 * access to the following struct elements should be done
	 * while disabled (particularly for write accesses)
	 */
	unsigned sf_msr	 :8;		/* last msr		*/
	unsigned sf_delta:8;		/* changed msr		*/
	unsigned sf_ethrei:1;		/* xmit int enabled	*/
	unsigned sf_xmit  :1;		/* xmit empty intr	*/
	unsigned sf_ioff  :1;		/* ioff, hoser		*/
	unsigned sf_flox  :1;		/* low level xoff	*/
	unsigned sf_watch :1;		/* watched		*/
	unsigned sf_dbg	  :1;		/* !0 => enter dbg	*/

#ifdef PM_SUPPORT
	unsigned sf_lcr  :8;		/* line control reg	*/       
	unsigned sf_mcr  :8;		/* machine ctonrol reg	*/
	unsigned sf_dll  :8;		/* divisor latch LSB reg */
	unsigned sf_dlm  :8;		/* divisor latch MSB reg */
	
	struct pm_handle pm_handle;	/* power management handle	*/
        struct {
		int intr;		/* 1:interrupt, 0:none		*/
		int ring_resume;	/* 1:enable, 0:disable		*/
		int freeze;		/* 1:PM freeze, 0:unfreeze	*/
		Simple_lock f_lock;     /* simple lock for freeze flag  */
	} sf_status;			/* device driver status for PM	*/
	int	pm_event_word;		/* used by suspend/hibernation	*/
	int	pm_device_id;		/* bus# required for PM		*/
	char	device_name[16];	/* name to be registered to PM	*/
	Simple_lock pm_lock;            /* simple lock for pm function  */
#endif
#ifdef PCMCIA
	struct pcmcia pcmcia;       /* additional info for PCMCIA */
#endif
} sfcom[NSFCOM];

#define sf_bid	 sf_conf.bid		/* BUS id		*/
#define sf_xtal	 sf_conf.xtal		/* xtal on board	*/
#define sf_port	 sf_conf.port		/* I/O port addr	*/
#define sf_irq	 sf_conf.irq		/* IRQ level		*/
#define sf_flags sf_conf.flags		/* misc flags		*/
#define sf_rtrig sf_conf.rtrig		/* receive trigger	*/
#define sf_tbc	 sf_conf.tbc		/* xmit byte count	*/

/*
 * h/w port layout.  These used to be nicely defined in a struct.  They got
 * changed back to this.  I give up.
 */
#define THR(x)  ((x)+0)            /* Transmit hold register                 */
#define RBR(x)  ((x)+0)            /* Receive buffer register                */
#define THR(x)  ((x)+0)            /* Transmitter holding register           */
#define DLL(x)  ((x)+0)            /* Divisor latch least significant bit    */

#define IER(x)  ((x)+1)            /* Interrupt Enabler Register             */
#define DLM(x)  ((x)+1)            /* Divisor latch most significant bit     */
#define ERBDAI 0x01
#define ETHREI 0x02
#define ELSI   0x04
#define EMSSI  0x08

#define IIR(x)  ((x)+2)            /* Interrupt identification register      */
#define CON_WRITE   0x01           /* gang writes */
#define BAUDOUT     0x02           /* BAUDOUT select */
#define RXRD_SEL    0x04           /* RXRD select */
#define LOOP_NOCARE 0x06           /* loop back don't care */
#define DISAB_DIV13 0x10

#define FCR(x)  ((x)+2)            /* Fifo control register (16550A only)    */
#define FIFO_ENABLE 0x01           /* enable fifo's */
#define RFIFO_RESET 0x02           /* reset receive fifo */
#define XFIFO_RESET 0x04           /* reset xmit fifo */
#define DMA_MODE    0x08           /* use mode 1 */
#define TRIG_MASK   0xC0
#define TRIG_01     0x00
#define TRIG_04     0x40
#define TRIG_08     0x80
#define TRIG_14     0xC0

#define LCR(x)  ((x)+3)            /* Line control register                  */
#define WLS0  0x01
#define WLS1  0x02
#define STB   0x04
#define PEN   0x08
#define EPS   0x10
#define STICK 0x20
#define BREAK 0x40
#define DLAB  0x80

#define MCR(x)  ((x)+4)            /* Modem control register                 */
#define DTR   0x01
#define RTS   0x02
#define OUT1  0x04
#define OUT2  0x08
#define LOOP  0x10

#define LSR(x)  ((x)+5)            /* Line status register                   */
#define DR    0x01
#define OE    0x02
#define PE    0x04
#define FE    0x08
#define BI    0x10
#define THRE  0x20
#define TEMT  0x40
#define EFIFO 0x80
#define LSR_MASK (OE|PE|FE|BI)

#define MSR(x)  ((x)+6)            /* Modem status register                  */
#define DCTS  0x01
#define DDSR  0x02
#define TERI  0x04
#define DDCD  0x08
#define CTS   0x10
#define DSR   0x20
#define RI    0x40
#define DCD   0x80

#define SCR(x)  ((x)+7)            /* Modem temporary byte (scratch pad)     */

#define set_dlab(port) (*LCR(port) |= DLAB, eieio())
#define clr_dlab(port) (*LCR(port) &= ~DLAB, eieio())

extern ulong d_offset;                  /* #### temp for debugger hook */
extern int dbg_pinned;                  /* must be true to access d_offset */

extern char *i_show[];

#endif /* _KERNEL || FOR_CRASH */
#endif /* _SF_H		*/
