/* @(#)88       1.2  src/rspc/kernext/pm/wakeup/ioport.h, pwrsysx, rspc41J, 9513A_all 3/24/95 23:56:54 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for I/O port access macros' definition.
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_IOPORT
#define	_H_IOPORT

/*
   macros for I/O access
*/

#define WritePortUchar(Data,Port)   \
    WriteOneByte(Data,Port);        \
    FlushIO()

#define ReadPortUchar(Port)         \
    ReadOneByte(Port);              \
    FlushIO()  

#endif
