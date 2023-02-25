/* @(#)36  1.1  src/bos/usr/samples/ndd/entsamp_hw.h, entsamp, bos411, 9428A410j 1/14/94 13:38:36 */
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_hw.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
 */

#ifndef _H_ENTSAMP_HW
#define _H_ENTSAMP_HW


/*****************************************************************************/
/*                                                                           */
/*   entsamp Ethernet Adapter Buffer Descriptor Definitions                  */
/*                                                                           */
/*****************************************************************************/
typedef struct {                                /* BUFFER DESCRIPTOR:        */
        unsigned char   status;                 /* Status Byte               */
        unsigned char   control;                /* Control Byte              */
        unsigned short  next;                   /* Link to next pkt buffer   */
        unsigned short  count;                  /* Byte count                */
        unsigned short  buflo;                  /* Low part of buffer addr   */
        unsigned short  bufhi;                  /* High part of buffer addr  */
} BUFDESC;

#define BUF_DES_SIZE	(sizeof(BUFDESC))

/* bit mask for the status field */
#define CMPLT_BIT_MASK (0x80) /* Transfer complete bit mask                  */
#define OK_BIT_MASK    (0x40) /* Transfer completed OK bit mask              */
#define BUSY_BIT_MASK  (0x20) /* Transfer busy bit mask                      */
#define BD_ERR_MASK    (0x0F) /* Buffer Descriptor Error Code Mask           */

/* bit mask for the control field */
#define EOP_BIT_MASK   (0x80) /* End of packet bit mask                      */
#define EL_BIT_MASK    (0x40) /* End of list bit mask                        */


/**************************************************************************/
/* Command Register (I/O Base + 0, Read/Write)                            */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* Overflow|execute | Receive Commands/Status  | Transmit Commands/Status */
/**************************************************************************/

#define COMMAND_REG   (0x0)  /* Command      Register I/O Base offset */
#define OFLOW_MSK     (0x80) /* Over Flow  mask for Command Register  */
#define EXECUTE_MSK   (0x40) /* Execute    mask for Command Register  */
#define SELF_TESTS_OK (0x00) /* Adapter Self Test Completed OK, mask  */

#define RECEIVE_MSK  (0x38) /* Receive    mask for Command Register   */
#define RX_NOP       (0x00) /* Receive - No Operation Mask            */
#define RX_START     (0x08) /* Receive - Start Packet Reception Mask  */
#define RX_P_RCVD    (0x10) /* Receive - Packet Received Mask         */
#define RX_ABORT     (0x20) /* Receive - Abort Reception Mask         */
#define RX_RESTART   (0x28) /* Receive - Restart Reception Mask       */
#define RX_ERROR     (0x30) /* Receive - Out of Receive Resources Mask*/

#define XMIT_MSK     (0x07) /* Transmit   mask for Command Register   */
#define TX_NOP       (0x00) /* Transmit - No Operation Mask           */
#define TX_START     (0x01) /* Transmit - Execute list Mask           */
#define TX_P_SENT    (0x02) /* Transmit - Single Packet xmitted Mask  */
#define TX_ABORT     (0x04) /* Transmit - Abort Transmission Mask     */
#define TX_RESTART   (0x05) /* Transmit - Restart Transmission Mask   */
#define TX_ERROR     (0x06) /* Transmit - Transmission Error Mask     */
#define TE_OK        (0x00) /* Transmit Error - No error              */
#define TE_COLLISION (0x01) /* Transmit Error - Max Collisions Mask   */
#define TE_UNDERRUN  (0x02) /* Transmit Error - FIFO Underrun  Mask   */
#define TE_CARRIER   (0x03) /* Transmit Error - Carrier Sense Lost    */
#define TE_CTS_LOST  (0x04) /* Transmit Error - Clear to Send Lost    */
#define TE_TIMEOUT   (0x05) /* Transmit Error - Transmit Timeout Mask */
#define TE_SIZE      (0x06) /* Transmit Error - Bad Packet Size Mask  */

/**************************************************************************/
/* Host Status Register (I/O Base + 2, RO except for parity)              */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +Parity |  +CRR  |  +CWR  |   *    |    *   |    *   |    *   |    *   */
/**************************************************************************/

#define STATUS_REG   (0x2)  /* Host Status   Register I/O Base offset */
#define PARITY_MSK   (0x80) /* Parity Bit mask for Status Register    */
#define CRR_MSK      (0x40) /* CMD Reg Ready mask for Status Register */
#define CWR_MSK      (0x20) /* CMD Byte Recd mask for Status Register */
#define INTR_MSK     (0xA0) /* Interrupt Recd mask for Status Register*/

/**************************************************************************/
/* Host Control Register (I/O Base + 6, Read/Write)                       */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +ATTN   | +RESET |    *   |   *    |    *   |+CMDINTE|    *   |    *   */
/**************************************************************************/
#define CONTROL_REG  (0x6)  /* Host Control  Register I/O Base offset */
#define HARD_RST_MSK (0xC0) /* Hard Reset   mask for Control Register */
#define EN_INTR_MSK  (0x04) /* Enable Interrupt mask for Control Reg  */
#define CONTROL_VALUE (0x00) /* Host Control Base Value               */

/**************************************************************************/
/* RAM Page Register (I/O Base + 8, Read/Write)                           */
/**************************************************************************/
#define RAM_PAGE_REG (0x8)  /* RAM Page      Register I/O Base offset */
#define RAM_PAGE_VALUE (0x00) /* RAM Page Register Default value      */

/**************************************************************************/
/* Parity Control Register (I/O Base + A, Read/Write) (Version 1 & 2)     */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +RAMERR |   *    |    *   |   *    |    *   |+DATAERR|+ADDRERR| +PAREN */
/**************************************************************************/
/**************************************************************************/
/* Parity Control Register (I/O Base + A, Read/Write) (Version 3 card)    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +RAMERR |   *    |    *   |   *    |+ MCHCK |+DATAERR|+FBRTERR| +PAREN */
/**************************************************************************/
#define PARITY_REG    (0x0A)   /* Parity Enable Register I/O Base offset  */
#define ANY_ERR_MSK   (0x86)   /* Any Parity error Mask for Parity Reg    */
#define RAM_ERR_MSK   (0x80)   /* RAM Parity error Mask for Parity Reg    */
#define DATA_ERR_MSK  (0x04)   /* Data Parity error Mask for Parity Reg   */
#define ADDR_ERR_MSK  (0x02)   /* Address Parity error Msk in Parity Reg  */
#define PAREN_MSK     (0x01)   /* Parity Enable Mask  in the  Parity Reg  */
#define PARDS_MSK     (0x00)   /* Parity Disable Mask in the  Parity Reg  */

/* Parity Control Register (I/O Base + A, Read/Write) (Version 3 card)    */
#define ANY_ERR3_MSK  (0x8E)   /* Any Parity error Mask for Parity Reg    */
#define MCHK_ERR3_MSK (0x08)   /* Channel Check Mask in the Parity Reg    */
#define FBRT_ERR3_MSK (0x02)   /* Feedback Return error Msk in Parity Reg */

#endif /* _H_ENTSAMP_HW */
