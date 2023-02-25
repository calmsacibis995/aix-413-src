/* @(#)45	1.4  src/tcpip/usr/include/isode/pepsy/SNMP_defs.h, isodelib7, tcpip411, GOLD410 4/5/93 13:23:33 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: decode_SNMP_GetNextRequest__PDU decode_SNMP_GetRequest__PDU 
 *    decode_SNMP_GetResponse__PDU decode_SNMP_Message decode_SNMP_PDU 
 *    decode_SNMP_PDUs decode_SNMP_SetRequest__PDU decode_SNMP_Trap__PDU 
 *    decode_SNMP_VarBind decode_SNMP_VarBindList 
 *    encode_SNMP_GetNextRequest__PDU encode_SNMP_GetRequest__PDU 
 *    encode_SNMP_GetResponse__PDU encode_SNMP_Message encode_SNMP_PDU 
 *    encode_SNMP_PDUs encode_SNMP_SetRequest__PDU encode_SNMP_Trap__PDU 
 *    encode_SNMP_VarBind encode_SNMP_VarBindList 
 *    print_SNMP_GetNextRequest__PDU print_SNMP_GetRequest__PDU 
 *    print_SNMP_GetResponse__PDU print_SNMP_Message print_SNMP_PDU 
 *    print_SNMP_PDUs print_SNMP_SetRequest__PDU print_SNMP_Trap__PDU 
 *    print_SNMP_VarBind print_SNMP_VarBindList
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/SNMP_defs.h
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#ifndef _SNMP_defs_
#define _SNMP_defs_

#include <isode/pepsy/SNMP_pre_defs.h>

#ifndef	lint
#define encode_SNMP_Message(pe, top, len, buffer, parm) \
    enc_f(_ZMessageSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_Message(pe, top, len, buffer, parm) \
    dec_f(_ZMessageSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_Message(pe, top, len, buffer, parm) \
    prnt_f(_ZMessageSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_Message_P    _ZMessageSNMP, &_ZSNMP_mod

#define encode_SNMP_PDUs(pe, top, len, buffer, parm) \
    enc_f(_ZPDUsSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_PDUs(pe, top, len, buffer, parm) \
    dec_f(_ZPDUsSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_PDUs(pe, top, len, buffer, parm) \
    prnt_f(_ZPDUsSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_PDUs_P    _ZPDUsSNMP, &_ZSNMP_mod

#define encode_SNMP_GetRequest__PDU(pe, top, len, buffer, parm) \
    enc_f(_ZGetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_GetRequest__PDU(pe, top, len, buffer, parm) \
    dec_f(_ZGetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_GetRequest__PDU(pe, top, len, buffer, parm) \
    prnt_f(_ZGetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_GetRequest__PDU_P    _ZGetRequest_PDUSNMP, &_ZSNMP_mod

#define encode_SNMP_GetNextRequest__PDU(pe, top, len, buffer, parm) \
    enc_f(_ZGetNextRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_GetNextRequest__PDU(pe, top, len, buffer, parm) \
    dec_f(_ZGetNextRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_GetNextRequest__PDU(pe, top, len, buffer, parm) \
    prnt_f(_ZGetNextRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_GetNextRequest__PDU_P    _ZGetNextRequest_PDUSNMP, &_ZSNMP_mod

#define encode_SNMP_GetResponse__PDU(pe, top, len, buffer, parm) \
    enc_f(_ZGetResponse_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_GetResponse__PDU(pe, top, len, buffer, parm) \
    dec_f(_ZGetResponse_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_GetResponse__PDU(pe, top, len, buffer, parm) \
    prnt_f(_ZGetResponse_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_GetResponse__PDU_P    _ZGetResponse_PDUSNMP, &_ZSNMP_mod

#define encode_SNMP_SetRequest__PDU(pe, top, len, buffer, parm) \
    enc_f(_ZSetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_SetRequest__PDU(pe, top, len, buffer, parm) \
    dec_f(_ZSetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_SetRequest__PDU(pe, top, len, buffer, parm) \
    prnt_f(_ZSetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_SetRequest__PDU_P    _ZSetRequest_PDUSNMP, &_ZSNMP_mod

#define encode_SNMP_PDU(pe, top, len, buffer, parm) \
    enc_f(_ZPDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_PDU(pe, top, len, buffer, parm) \
    dec_f(_ZPDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_PDU(pe, top, len, buffer, parm) \
    prnt_f(_ZPDUSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_PDU_P    _ZPDUSNMP, &_ZSNMP_mod

#define encode_SNMP_Trap__PDU(pe, top, len, buffer, parm) \
    enc_f(_ZTrap_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_Trap__PDU(pe, top, len, buffer, parm) \
    dec_f(_ZTrap_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_Trap__PDU(pe, top, len, buffer, parm) \
    prnt_f(_ZTrap_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_Trap__PDU_P    _ZTrap_PDUSNMP, &_ZSNMP_mod

#define encode_SNMP_VarBind(pe, top, len, buffer, parm) \
    enc_f(_ZVarBindSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_VarBind(pe, top, len, buffer, parm) \
    dec_f(_ZVarBindSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_VarBind(pe, top, len, buffer, parm) \
    prnt_f(_ZVarBindSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_VarBind_P    _ZVarBindSNMP, &_ZSNMP_mod

#define encode_SNMP_VarBindList(pe, top, len, buffer, parm) \
    enc_f(_ZVarBindListSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char *) parm)

#define decode_SNMP_VarBindList(pe, top, len, buffer, parm) \
    dec_f(_ZVarBindListSNMP, &_ZSNMP_mod, pe, top, len, buffer, (char **) parm)

#define print_SNMP_VarBindList(pe, top, len, buffer, parm) \
    prnt_f(_ZVarBindListSNMP, &_ZSNMP_mod, pe, top, len, buffer)
#define print_SNMP_VarBindList_P    _ZVarBindListSNMP, &_ZSNMP_mod


#endif   /* lint */

#endif /* _SNMP_defs_ */
