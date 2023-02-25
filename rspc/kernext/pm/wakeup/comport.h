/* @(#)79       1.1  src/rspc/kernext/pm/wakeup/comport.h, pwrsysx, rspc41J, 9510A 3/8/95 06:44:03 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for COM port definition. 
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_COMPORT
#define _H_COMPORT

/*****************************************************************************/
/**  Constant Definition                                                    **/
/*****************************************************************************/

#define  MSB_1200        0x00            /*  1200 baud MSB */
#define  LSB_1200        0x60            /*  1200 baud LSB */
#define  MSB_2400        0x00            /*  2400 baud MSB */
#define  LSB_2400        0x30            /*  2400 baud LSB */
#define  MSB_4800        0x00            /*  4800 baud MSB */
#define  LSB_4800        0x18            /*  4800 baud LSB */
#define  MSB_9600        0x00            /*  9600 baud MSB */
#define  LSB_9600        0x0c            /*  9600 baud LSB */
#define  MSB_19200       0x00            /* 19200 baud MSB */
#define  LSB_19200       0x06            /* 19200 baud LSB */
#define  MSB_38400       0x00            /* 38400 baud MSB */
#define  LSB_38400       0x03            /* 38400 baud LSB */

#define  TXB             0               /* Transmit buffer           */
#define  RXB             0               /* Receive buffer            */
#define  LATLSB          0               /* Divisor latch, LSB        */
#define  LATMSB          1               /* Divisor latch, MSB        */
#define  IER             1               /* Interrupt enable register */
#define  IIR             2               /* Interrupt identification  */
#define  FCR             2               /* FIFO control register     */
#define  LCR             3               /* Line control register     */
#define  MCR             4               /* Modem control register    */
#define  LSR             5               /* Line status register      */
#define  MSR             6               /* Modem status register     */

/*
   Interrupt enable masks
*/

#define  IE_RX          0x01  /* 00000001b  read data available       */
#define  IE_TX          0x02  /* 00000010b  transmit buffer empty     */
#define  IE_LX          0x04  /* 00000100b  line status change        */
#define  IE_MX          0x08  /* 00001000b  modem status change       */

/*
   FIFO control
*/

#define  FC_ENB         0x01  /* 00000001b  FIFO enable               */
#define  FC_RFRS        0x02  /* 00000010b  RX FIFO reset             */
#define  FC_TFRS        0x04  /* 00000100b  TX FIFO reset             */
#define  FC_RSVD        0x38  /* 00111000b  reserved                  */
#define  FC_RTL0        0x40  /* 01000000b  FIFO RX trigger level 0   */ 
#define  FC_RTL1        0x80  /* 10000000b  FIFO RX trigger level 1   */ 

/*
   Line control masks
*/

#define  LC_BMASK        0x03 /* 00000011b  data bits mask            */
#define  LC_BITS5        0x00 /* 00000000b  5 data bits               */
#define  LC_BITS6        0x01 /* 00000001b  6 data bits               */
#define  LC_BITS7        0x02 /* 00000010b  7 data bits               */
#define  LC_BITS8        0x03 /* 00000011b  8 data bits               */

#define  LC_SMASK        0x04 /* 00000100b  stop bits mask            */
#define  LC_STOP1        0x00 /* 00000000b  1 stop bit                */
#define  LC_STOP2        0x04 /* 00000100b  2 stop bits(1.5 if 5bits) */

#define  LC_PMASK        0x38 /* 00111000b  parity mask               */
#define  LC_PNONE        0x00 /* 00000000b  none parity               */
#define  LC_PODD         0x08 /* 00001000b  odd parity                */
#define  LC_PEVEN        0x18 /* 00011000b  even parity               */
#define  LC_PMARK        0x28 /* 00101000b  mark parity               */
#define  LC_PSPACE       0x38 /* 00111000b  space parity              */

#define  LC_BREAK        0x40 /* 01000000b  transmit break            */
#define  LC_DLAB         0x80 /* 10000000b  divisor latch access bit  */
#define  LC_MASK         0x7f /* 01111111b  line ctrl register bits   */

/*
   Modem control register masks
*/

#define  MC_DTR          0x01 /* 00000001b  data terminal ready       */
#define  MC_RTS          0x02 /* 00000010b  request to send           */
#define  MC_OUT1         0x04 /* 00000100b  output 1                  */
#define  MC_OUT2         0x08 /* 00001000b  output 2                  */
#define  MC_LOOP         0x10 /* 00010000b  loopback mode             */

/*
   Line status register masks
*/

#define  LS_DR           0x01 /* 00000001b  data ready                */
#define  LS_OERR         0x02 /* 00000010b  overrun error             */
#define  LS_PERR         0x04 /* 00000100b  parity error              */
#define  LS_FERR         0x08 /* 00001000b  framing error             */
#define  LS_BI           0x10 /* 00010000b  break interrupt           */
#define  LS_THRE         0x20 /* 00100000b  TX holding register empty */
#define  LS_TSRE         0x40 /* 01000000b  TX shift register empty   */

/*
   Modem status register definitions:
*/

#define  MS_DCTS        0x01 /* 00000001b  delta clear to send        */
#define  MS_DDSR        0x02 /* 00000010b  delta data set ready       */
#define  MS_TERI        0x04 /* 00000100b  trailing edge of RI        */
#define  MS_DDCD        0x08 /* 00001000b  delta receiver line detect */
#define  MS_CTS         0x10 /* 00010000b  clear to send              */
#define  MS_DSR         0x20 /* 00100000b  data set ready             */
#define  MS_RI          0x40 /* 01000000b  ring indicator             */
#define  MS_DCD         0x80 /* 10000000b  receiver line signal detect*/

/*
   Define COM port interrupt level
*/

#define  COM1_INTERRUPT_LEVEL 4
#define  COM2_INTERRUPT_LEVEL 3
#define  COM3_INTERRUPT_LEVEL 4
#define  COM4_INTERRUPT_LEVEL 3

/*
   Define COM port base address
*/

#define  COM1_BASE   0x3f8
#define  COM2_BASE   0x2f8
#define  COM3_BASE   0x3e8
#define  COM4_BASE   0x2e8

/* 
   Return codes with data  
*/

#define  RX_DATAREADY   0x0ff00
#define  TX_SUCCESS     0x0ff00

/*
   Other special controls
*/

#ifdef   CRLF_OUTBOUND                 /* LF -> CR/LF CONVERSION        */

#define  ASCI_LF        0x0a
#define  ASCI_CR        0x0d

#endif

#endif
