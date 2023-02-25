/* @(#)21	1.1  src/rspc/usr/lib/boot/softros/include/condev.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:11  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: BIT
 *		IIR_INT_ID
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Registers */

# define THR            0x800003F8      /* Transmitter Holding Register */
# define RDR            0x800003F8      /* Receiver Data Register */
# define MSR            0x800003FE      /* Modem Status Register */
# define BDRL           0x800003F8      /* Divisor Latch, Low Byte */
# define BDRM           0x800003F9      /* Divisor Latch, High Byte */
# define LCR            0x800003FB      /* Line Control Register */
# define LSR            0x800003FD      /* Line Status Register */
# define MCR            0x800003FC      /* Modem Control Register */
# define PI_CTRL        0x80000200
# define IER            0x800003F9      /* Interrupt Enable Register */
# define IMR            0x80000021
# define IIR            0x800003FA      /* Interrupt Ident Register */
# define EOI            0x20

# define BIT(x)         (1 << (x))

/* Baud Rate divisors */

# define DB0                       0
# define DB50                   2304
# define DB75                   1536
# define DB110                  1047
# define DB134                   857
# define DB150                   768
# define DB300                   384
# define DB600                   192
# define DB1200                   96
# define DB1800                   64
# define DB2400                   48
# define DB4800                   24
# define DB9600                   12
# define DB19200                   6
# define DB38400                   3

/* IER Definitions */

# define IER_MS_INT             BIT(3)
# define IER_RLS_INT            BIT(2)
# define IER_THRE_INT           BIT(1)
# define IER_RD_INT             BIT(0)
# define IER_TO_INT             BIT(0)

/* IIR Definitions */

# define IIR_TYPE2_CTRL         BIT(7) | BIT(6)
# define IIR_TYPE1_CTRL         BIT(7)
# define IIR_NO_INT             BIT(0)

# define IIR_INT_ID( x )        ((( x ) & ( BIT(3)|BIT(2)|BIT(1) )) >> 1 )

/* LCR Definitions */

# define LCR_DL_ACC             BIT(7)
# define LCR_SET_BREAK          BIT(6)
# define LCR_STICK_PAR          BIT(5)
# define LCR_EVEN_PAR           BIT(4)
# define LCR_PAR_ENABLE         BIT(3)
# define LCR_2_STOPBIT          BIT(2)
# define LCR_1_STOPBIT          0
# define LCR_5BIT               0
# define LCR_6BIT               BIT(0)
# define LCR_7BIT               BIT(1)
# define LCR_8BIT               BIT(0) | BIT(1)

/* MCR Definitions */

# define MCR_LOOP               BIT(4)
# define MCR_OUT2               BIT(3)
# define MCR_OUT1               BIT(2)
# define MCR_RTS                BIT(1)
# define MCR_DTR                BIT(0)

/* LSR Definitions */

# define LSR_FIFO_ERR           BIT(7)
# define LSR_TSR_EMPTY          BIT(6)
# define LSR_THR_EMPTY          BIT(5)
# define LSR_BRK_INT            BIT(4)
# define LSR_FRAME_ERR          BIT(3)
# define LSR_PAR_ERR            BIT(2)
# define LSR_OVERRUN_ERR        BIT(1)
# define LSR_DATA_RDY           BIT(0)

/* MSR Definitions */

# define MSR_DCD                BIT(7)
# define MSR_RI                 BIT(6)
# define MSR_DSR                BIT(5)
# define MSR_CTS                BIT(4)
# define MSR_DDCD               BIT(3)
# define MSR_TERI               BIT(2)
# define MSR_DDSR               BIT(1)
# define MSR_DCTS               BIT(0)

extern void  outbyte( uchar *addr, uchar value );
extern unsigned char inbyte( uchar *addr );
