/* @(#)13	1.1  src/rspc/usr/lib/boot/softros/lib/libstand/am_con.h, rspc_softros, rspc411, GOLD410 4/18/94 15:49:28  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: none
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

/* Defines for compatability with boca ROS routines	*/

#include <boca_compat.h>

/* Serial port routine definitions                                           */

int sio_init(int port, int baud, int words);
int sio_stat(int port);
char sio_getc(int port);
void sio_putc(int port,int value);
void display_putc(int port,int value);
