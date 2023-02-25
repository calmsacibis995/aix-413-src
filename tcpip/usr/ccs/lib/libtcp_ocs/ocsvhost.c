static char sccsid[] = "@(#)69	1.2.1.1  src/tcpip/usr/ccs/lib/libtcp_ocs/ocsvhost.c, tcpip, tcpip411, GOLD410 3/18/94 13:18:21";
/*
 * COMPONENT_NAME: TCPIP ocsvhost.c
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Virtual Host Database
 * Structures for ODM Interface for OCSvhost Object Class
 * Output .c file from odmcreate()
 */

#include "ocsvhost.h"

static struct ClassElem OCSvhost_ClassElem[] =  {
 { "vh_inetaddr",ODM_CHAR, 12,256, NULL,NULL,0,NULL ,-1,0},
 { "protoname",ODM_CHAR, 268,10, NULL,NULL,0,NULL ,-1,0},
 { "hostname",ODM_CHAR, 278,64, NULL,NULL,0,NULL ,-1,0},
 { "alt_hostname_one",ODM_CHAR, 342,64, NULL,NULL,0,NULL ,-1,0},
 { "alt_hostname_two",ODM_CHAR, 406,64, NULL,NULL,0,NULL ,-1,0},
 };
struct Class OCSvhost_CLASS[] = {
 ODMI_MAGIC, "OCSvhost", sizeof(struct OCSvhost), OCSvhost_Descs, OCSvhost_ClassElem, NULL,FALSE,NULL,NULL,0,0,NULL,0,"", 0,-ODMI_MAGIC
 };
