/* @(#)34	1.1  src/tcpip/usr/include/inetodm.h, tcpinet, tcpip411, GOLD410 10/21/89 11:33:15 */
/* 
 * COMPONENT NAME: (TCPIP) SRC commands
 *
 * FUNCTIONS:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * inetodm - structures for ODM interface 
 */
#include <odmi.h>

#define	InetServ_PATH	"/etc/objrepos"

struct InetServ {
	long _id;
	long _reserved;
	long _scratch;
	long state;
	char servname[20];
	char socktype[20];
	char protocol[10];
	char waitstate[10];
	char user[20];
	char path[50];
	char command[50];
	long portnum;
	char alias[50];
	char description[40];
	char statusmethod[256];	/* method */
	};
#define InetServ_Descs 12

extern struct Class InetServ_CLASS[];
#define get_InetServ_list(a,b,c,d,e) (struct InetServ * )odm_get_list(a,b,c,d,e)
