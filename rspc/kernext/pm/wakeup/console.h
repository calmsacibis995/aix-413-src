/* @(#)81       1.1  src/rspc/kernext/pm/wakeup/console.h, pwrsysx, rspc41J, 9510A 3/8/95 06:44:06 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for console characters.
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_CONSOLE
#define _H_CONSOLE

/*
   Define special character values.
*/

#define ASCI_NUL   0x00
#define ASCI_BEL   0x07
#define ASCI_BS    0x08
#define ASCI_HT    0x09
#define ASCI_LF    0x0A
#define ASCI_VT    0x0B
#define ASCI_FF    0x0C
#define ASCI_CR    0x0D
#define ASCI_CSI   0x9B
#define ASCI_ESC   0x1B
#define ASCI_SYSRQ 0x80
#define ASCI_XTEN1 0x00
#define ASCI_XTEN2 0xe0

#endif
