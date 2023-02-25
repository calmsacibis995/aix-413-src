static char sccsid[] = "@(#)24	1.1  src/tcpip/usr/sbin/compat/inetodm.c, tcpip, tcpip411, GOLD410 3/30/94 17:27:31";
/* 
 * COMPONENT_NAME: (TCPIP) SRC commands
 *
 * FUNCTIONS:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * inetodm - structures for ODM interface 
 */
#include "inetodm.h"

static struct ClassElem InetServ_ClassElem[] =  
	{
	{ "state",ODM_LONG, 12, 4, NULL,NULL,0,NULL ,-1,0 },
	{ "servname",ODM_CHAR, 16,20, NULL,NULL,0,NULL ,-1,0 },
	{ "socktype",ODM_CHAR, 36,20, NULL,NULL,0,NULL ,-1,0 },
	{ "protocol",ODM_CHAR, 56,10, NULL,NULL,0,NULL ,-1,0 },
	{ "waitstate",ODM_CHAR, 66,10, NULL,NULL,0,NULL ,-1,0 },
	{ "user",ODM_CHAR, 76,20, NULL,NULL,0,NULL ,-1,0 },
	{ "path",ODM_CHAR, 96,50, NULL,NULL,0,NULL ,-1,0 },
	{ "command",ODM_CHAR, 146,50, NULL,NULL,0,NULL ,-1,0 },
	{ "portnum",ODM_LONG, 196, 4, NULL,NULL,0,NULL ,-1,0 },
	{ "alias",ODM_CHAR, 200,50, NULL,NULL,0,NULL ,-1,0 },
	{ "description",ODM_CHAR, 250,40, NULL,NULL,0,NULL ,-1,0 },
	{ "statusmethod",ODM_METHOD, 290,256, NULL,NULL,0,NULL ,-1,0 },
	};

struct Class InetServ_CLASS[] = 
	{
	ODMI_MAGIC, 
	"InetServ", 
	sizeof(struct InetServ), 
	InetServ_Descs, 
	InetServ_ClassElem, 
	NULL,
	FALSE,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	0,
	"", 
	0,
	-ODMI_MAGIC
	};
