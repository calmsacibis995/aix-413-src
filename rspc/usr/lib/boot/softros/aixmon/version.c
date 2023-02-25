static char sccsid[] = "@(#)04	1.2  src/rspc/usr/lib/boot/softros/aixmon/version.c, rspc_softros, rspc411, GOLD410 6/13/94 15:40:47";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: version
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

#include <printf.h>
#include <console.h>
#include <colors.h>

extern char rostime[10];

char *product = "AIX System Firmware II";

char *versn = "Version 0.2, ";
char *revdate = "built on " __DATE__ " at " __TIME__ "\n";
char *copyright = "(C)Copyright IBM Corporation, 1994. All rights reserved.";
/*char *ros_base = "Based on 1/19/94 Dakota System ROS)      \n";*/
char *ros_base = ")                                            \n";

extern SCR_INFO screen;

void version(void){
   int x, y, i;
   unsigned int bus_info;

				/* Get rostime from build time file	*/
#include	"version.h"

   set_attribute(CLR_WHITE,CLR_DARKBLUE);
   open_window(0,0,screen.scr_max_x,2);
   clear_window();

   printf(product);
   i = get_PVR() >> 16;
   printf(" (6%2.2d",i);
   printf(ros_base);
   printf(versn);
   printf(revdate);
   printf(copyright);

   set_attribute(CLR_BLACK,CLR_WHITE);
   open_window(0,3, screen.scr_max_x,screen.scr_max_y);

   return;
}
