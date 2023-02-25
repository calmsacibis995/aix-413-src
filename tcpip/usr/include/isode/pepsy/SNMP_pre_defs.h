/* @(#)46	1.3  src/tcpip/usr/include/isode/pepsy/SNMP_pre_defs.h, isodelib7, tcpip411, GOLD410 3/3/93 11:41:11 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/SNMP_pre_defs.h
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#ifndef _SNMP_pre_defs_
#define _SNMP_pre_defs_

extern modtyp	_ZSNMP_mod;
#define _ZGetRequest_PDUSNMP	2
#define _ZGetNextRequest_PDUSNMP	3
#define _ZPDUsSNMP	1
#define _ZPDUSNMP	6
#define _ZTrap_PDUSNMP	7
#define _ZSetRequest_PDUSNMP	5
#define _ZVarBindSNMP	8
#define _ZVarBindListSNMP	9
#define _ZMessageSNMP	0
#define _ZGetResponse_PDUSNMP	4

#endif /* _SNMP_pre_defs_ */
