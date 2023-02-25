/* @(#)90       1.1  src/rspc/kernext/pm/wakeup/spiosup.h, pwrsysx, rspc41J, 9510A 3/8/95 06:44:18 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for Super I/O port bit definition.
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_SPIOSUP
#define _H_SPIOSUP

/*++
   Define super i/o index/data address
--*/

#define  SpIO_INDEX   0x398 
#define  SpIO_DATA    0x399
#define  SpIO_INDEX2  0x26E
#define  SpIO_DATA2   0x26F

/*++
   Define super i/o index value 
*/

#define  FER_INDEX   0x00

#define  FER_IDE2ND  0x80
#define  FER_IDEENB  0x40
#define  FER_FDC2ND  0x20
#define  FER_FDCENC  0x10
#define  FER_FDCENB  0x08
#define  FER_UR2ENB  0x04
#define  FER_UR1ENB  0x02
#define  FER_PRTENB  0x01

#define  FAR_INDEX   0x01

#define  FAR_SLCOM1  0x80
#define  FAR_SLCOM0  0x40
#define  FAR_UR2AD1  0x20
#define  FAR_UR2AD0  0x10
#define  FAR_UR1AD1  0x08
#define  FAR_UR1AD0  0x04
#define  FAR_PRTAD1  0x02
#define  FAR_PRTAD0  0x01   
#define  FAR_DEFAULT (FAR_UR2AD0+FAR_PRTAD0)

#define  PTR_INDEX   0x02

#define  FCR_INDEX   0x03

#define  PCR_INDEX   0x04 

#endif
