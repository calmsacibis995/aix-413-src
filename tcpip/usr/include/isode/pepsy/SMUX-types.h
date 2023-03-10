/* @(#)38	1.3  src/tcpip/usr/include/isode/pepsy/SMUX-types.h, isodelib7, tcpip411, GOLD410 4/5/93 13:22:30 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: free_SMUX_ClosePDU free_SMUX_PDUs free_SMUX_RReqPDU 
 *    free_SMUX_RRspPDU free_SMUX_SOutPDU free_SMUX_SimpleOpen
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/SMUX-types.h
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* automatically generated by pepsy 6.0 #104 (vikings.austin.ibm.com), do not edit! */

#ifndef	_module_SMUX_defined_
#define	_module_SMUX_defined_

#ifndef	PEPSY_VERSION
#define	PEPSY_VERSION		2
#endif

#include <isode/psap.h>
#include <isode/pepsy.h>
#include <isode/pepsy/UNIV-types.h>

#include <isode/pepsy/SMUX_defs.h>

#include <isode/pepsy/SNMP-types.h>
#include <isode/pepsy/SMI-types.h>

#define	type_SMUX_OpenPDU	type_SMUX_SimpleOpen
#define	free_SMUX_OpenPDU	free_SMUX_SimpleOpen

struct type_SMUX_PDUs {
    int         offset;
#define	type_SMUX_PDUs_simple	1
#define	type_SMUX_PDUs_close	2
#define	type_SMUX_PDUs_registerRequest	3
#define	type_SMUX_PDUs_registerResponse	4
#define	type_SMUX_PDUs_get__request	5
#define	type_SMUX_PDUs_get__next__request	6
#define	type_SMUX_PDUs_get__response	7
#define	type_SMUX_PDUs_set__request	8
#define	type_SMUX_PDUs_trap	9
#define	type_SMUX_PDUs_commitOrRollback	10

    union {
        struct type_SMUX_SimpleOpen *simple;

        struct type_SMUX_ClosePDU *close;

        struct type_SMUX_RReqPDU *registerRequest;

        struct type_SMUX_RRspPDU *registerResponse;

        struct type_SNMP_GetRequest__PDU *get__request;

        struct type_SNMP_GetNextRequest__PDU *get__next__request;

        struct type_SNMP_GetResponse__PDU *get__response;

        struct type_SNMP_SetRequest__PDU *set__request;

        struct type_SNMP_Trap__PDU *trap;

        struct type_SMUX_SOutPDU *commitOrRollback;
    }       un;
};
#define	free_SMUX_PDUs(parm)\
	(void) fre_obj((char *) parm, _ZSMUX_mod.md_dtab[_ZPDUsSMUX], &_ZSMUX_mod, 1)

struct type_SMUX_SimpleOpen {
    integer     version;
#define	int_SMUX_version_version__1	0

    OID     identity;

    struct type_SMI_DisplayString *description;

    struct qbuf *password;
};
#define	free_SMUX_SimpleOpen(parm)\
	(void) fre_obj((char *) parm, _ZSMUX_mod.md_dtab[_ZSimpleOpenSMUX], &_ZSMUX_mod, 1)

struct type_SMUX_ClosePDU {
    integer     parm;
#define	int_SMUX_ClosePDU_goingDown	0
#define	int_SMUX_ClosePDU_unsupportedVersion	1
#define	int_SMUX_ClosePDU_packetFormat	2
#define	int_SMUX_ClosePDU_protocolError	3
#define	int_SMUX_ClosePDU_internalError	4
#define	int_SMUX_ClosePDU_authenticationFailure	5
};
#define	free_SMUX_ClosePDU(parm)\
	(void) fre_obj((char *) parm, _ZSMUX_mod.md_dtab[_ZClosePDUSMUX], &_ZSMUX_mod, 1)

struct type_SMUX_RReqPDU {
    struct type_SMI_ObjectName *subtree;

    integer     priority;

    integer     operation;
#define	int_SMUX_operation_delete	0
#define	int_SMUX_operation_readOnly	1
#define	int_SMUX_operation_readWrite	2
};
#define	free_SMUX_RReqPDU(parm)\
	(void) fre_obj((char *) parm, _ZSMUX_mod.md_dtab[_ZRReqPDUSMUX], &_ZSMUX_mod, 1)

struct type_SMUX_RRspPDU {
    integer     parm;
#define	int_SMUX_RRspPDU_failure	-1
};
#define	free_SMUX_RRspPDU(parm)\
	(void) fre_obj((char *) parm, _ZSMUX_mod.md_dtab[_ZRRspPDUSMUX], &_ZSMUX_mod, 1)

struct type_SMUX_SOutPDU {
    integer     parm;
#define	int_SMUX_SOutPDU_commit	0
#define	int_SMUX_SOutPDU_rollback	1
};
#define	free_SMUX_SOutPDU(parm)\
	(void) fre_obj((char *) parm, _ZSMUX_mod.md_dtab[_ZSOutPDUSMUX], &_ZSMUX_mod, 1)
#endif
