/* @(#)82	1.2  src/rspc/kernext/inc/pcmciacsAix.h, pcmciaker, rspc41J, 9516A_all 4/18/95 00:36:29  */
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: Card Services AIX Specific Function Structure
 *              Definition
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#if !defined( H_PCMCIACSAIX )
# define H_PCMCIACSAIX

typedef struct {
  int       Socket;                     /* I    Logical Socket Number */
  int       funccode;                   /* I    Function Code */
  int       subcode;
  int       rsvd;
  int       opcode;
  int       rsvd2;
  char      subpkt[1];
} CSaixVdrSpcPkt;

/**** Function code definitions ****/

/* Direct Socket Service Call */
#define CSaixSocketServices   0x01
   /* subcode is used for the function code of socket services */
   /* subpkt is used for the packet passed to/from socket services */

/* The mechanism to lock the software access to a PCMCIA socket */
#define CSaixLockSocket       0x21
          /* subcodes */
#define   _CSaixHoldLock        0x01
#define   _CSaixReleaseLock     0x02

/* The interface to control LED ( PCMCIA activity lamp ) */
#define CSaixControlLED       0x22
          /* subcodes */
#define   _CSaixTurnOnLED       0x00
#define   _CSaixTurnOffLED      0x01

#endif /* H_PCMCIACSAIX */
