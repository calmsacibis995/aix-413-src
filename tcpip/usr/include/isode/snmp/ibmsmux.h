/* @(#)06	1.3  src/tcpip/usr/include/isode/snmp/ibmsmux.h, snmp, tcpip411, GOLD410 3/3/93 11:39:02 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE: src/tcpip/usr/include/isode/snmp/ibmsmux.h
 */

#ifndef _IBMSMUX_
#define _IBMSMUX_

/*
 * generic isode SMUX to IBM SMUX mapping
 */

/* SMI */
#define type_SNMP_ObjectName	type_SMI_ObjectName
#define free_SNMP_ObjectName	free_SMI_ObjectName
#define encode_SNMP_ObjectName	encode_SMI_ObjectName
#define decode_SNMP_ObjectName	decode_SMI_ObjectName
#define print_SNMP_ObjectName	print_SMI_ObjectName

#define type_SNMP_NetworkAddress	type_SMI_NetworkAddress
#define free_SNMP_NetworkAddress	free_SMI_NetworkAddress
#define encode_SNMP_NetworkAddress	encode_SMI_NetworkAddress
#define decode_SNMP_NetworkAddress	decode_SMI_NetworkAddress
#define print_SNMP_NetworkAddress	print_SMI_NetworkAddress

#define type_SNMP_IpAddress	type_SMI_IpAddress
#define free_SNMP_IpAddress	free_SMI_IpAddress
#define encode_SNMP_IpAddress	encode_SMI_IpAddress
#define decode_SNMP_IpAddress	decode_SMI_IpAddress
#define print_SNMP_IpAddress	print_SMI_IpAddress

#define type_SNMP_DisplayString	type_SMI_DisplayString
#define free_SNMP_DisplayString	free_SMI_DisplayString
#define encode_SNMP_DisplayString	encode_SMI_DisplayString
#define decode_SNMP_DisplayString	decode_SMI_DisplayString
#define print_SNMP_DisplayString	print_SMI_DisplayString

#define type_SNMP_TimeTicks	type_SMI_TimeTicks
#define free_SNMP_TimeTicks	free_SMI_TimeTicks
#define encode_SNMP_TimeTicks	encode_SMI_TimeTicks
#define decode_SNMP_TimeTicks	decode_SMI_TimeTicks
#define print_SNMP_TimeTicks	print_SMI_TimeTicks

#define type_SNMP_ObjectSyntax	type_SMI_ObjectSyntax
#define	free_SNMP_ObjectSyntax	free_SMI_ObjectSyntax
#define	encode_SNMP_ObjectSyntax	encode_SMI_ObjectSyntax
#define	decode_SNMP_ObjectSyntax	decode_SMI_ObjectSyntax
#define	print_SNMP_ObjectSyntax	print_SMI_ObjectSyntax

/* SMUX */

/* constants */
#define type_SNMP_SMUX__PDUs_simple	type_SMUX_PDUs_simple
#define type_SNMP_SMUX__PDUs_close	type_SMUX_PDUs_close
#define type_SNMP_SMUX__PDUs_registerRequest	type_SMUX_PDUs_registerRequest
#define type_SNMP_SMUX__PDUs_registerResponse	type_SMUX_PDUs_registerResponse
#define type_SNMP_SMUX__PDUs_get__request	type_SMUX_PDUs_get__request
#define type_SNMP_SMUX__PDUs_get__next__request	type_SMUX_PDUs_get__next__request
#define type_SNMP_SMUX__PDUs_get__response	type_SMUX_PDUs_get__response
#define type_SNMP_SMUX__PDUs_set__request	type_SMUX_PDUs_set__request
#define type_SNMP_SMUX__PDUs_trap	type_SMUX_PDUs_trap
#define type_SNMP_SMUX__PDUs_commitOrRollback	type_SMUX_PDUs_commitOrRollback

#define int_SNMP_ClosePDU_goingDown	int_SMUX_ClosePDU_goingDown
#define int_SNMP_ClosePDU_unsupportedVersion	int_SMUX_ClosePDU_unsupportedVersion
#define int_SNMP_ClosePDU_packetFormat	int_SMUX_ClosePDU_packetFormat
#define int_SNMP_ClosePDU_protocolError	int_SMUX_ClosePDU_protocolError
#define int_SNMP_ClosePDU_internalError	int_SMUX_ClosePDU_internalError
#define int_SNMP_ClosePDU_authenticationFailure	int_SMUX_ClosePDU_authenticationFailure
#define int_SNMP_operation_delete	int_SMUX_operation_delete
#define int_SNMP_operation_readOnly	int_SMUX_operation_readOnly
#define int_SNMP_operation_readWrite	int_SMUX_operation_readWrite
#define int_SNMP_RRspPDU_failure	int_SMUX_RRspPDU_failure
#define int_SNMP_SOutPDU_commit	int_SMUX_SOutPDU_commit
#define int_SNMP_SOutPDU_rollback	int_SMUX_SOutPDU_rollback

/* subroutines/functions */
#define type_SNMP_OpenPDU	type_SMUX_OpenPDU
#define free_SNMP_OpenPDU	free_SMUX_OpenPDU
#define encode_SNMP_OpenPDU	encode_SMUX_OpenPDU
#define decode_SNMP_OpenPDU	decode_SMUX_OpenPDU
#define print_SNMP_OpenPDU	print_SMUX_OpenPDU

#define type_SNMP_SMUX__PDUs	type_SMUX_PDUs
#define free_SNMP_SMUX__PDUs	free_SMUX_PDUs
#define encode_SNMP_SMUX__PDUs	encode_SMUX_PDUs
#define decode_SNMP_SMUX__PDUs	decode_SMUX_PDUs
#define print_SNMP_SMUX__PDUs	print_SMUX_PDUs

#define type_SNMP_SimpleOpen	type_SMUX_SimpleOpen
#define free_SNMP_SimpleOpen	free_SMUX_SimpleOpen
#define encode_SNMP_SimpleOpen	encode_SMUX_SimpleOpen
#define decode_SNMP_SimpleOpen	decode_SMUX_SimpleOpen
#define print_SNMP_SimpleOpen	print_SMUX_SimpleOpen

#define type_SNMP_ClosePDU	type_SMUX_ClosePDU
#define free_SNMP_ClosePDU	free_SMUX_ClosePDU
#define encode_SNMP_ClosePDU	encode_SMUX_ClosePDU
#define decode_SNMP_ClosePDU	decode_SMUX_ClosePDU
#define print_SNMP_ClosePDU	print_SMUX_ClosePDU

#define type_SNMP_RReqPDU	type_SMUX_RReqPDU
#define free_SNMP_RReqPDU	free_SMUX_RReqPDU
#define encode_SNMP_RReqPDU	encode_SMUX_RReqPDU
#define decode_SNMP_RReqPDU	decode_SMUX_RReqPDU
#define print_SNMP_RReqPDU	print_SMUX_RReqPDU

#define type_SNMP_RRspPDU	type_SMUX_RRspPDU
#define free_SNMP_RRspPDU	free_SMUX_RRspPDU
#define encode_SNMP_RRspPDU	encode_SMUX_RRspPDU
#define decode_SNMP_RRspPDU	decode_SMUX_RRspPDU
#define print_SNMP_RRspPDU	print_SMUX_RRspPDU

#define type_SNMP_SOutPDU	type_SMUX_SOutPDU
#define free_SNMP_SOutPDU	free_SMUX_SOutPDU
#define encode_SNMP_SOutPDU	encode_SMUX_SOutPDU
#define decode_SNMP_SOutPDU	decode_SMUX_SOutPDU
#define print_SNMP_SOutPDU	print_SMUX_SOutPDU

#endif /* _IBMSMUX_ */
