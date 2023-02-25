/* @(#)35	1.4  src/tcpip/usr/include/isode/pepsy/SMI_defs.h, isodelib7, tcpip411, GOLD410 4/5/93 13:22:11 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: decode_SMI_ApplicationSyntax decode_SMI_Counter 
 *    decode_SMI_DisplayString decode_SMI_Gauge decode_SMI_IpAddress 
 *    decode_SMI_NetworkAddress decode_SMI_ObjectName decode_SMI_ObjectSyntax 
 *    decode_SMI_Opaque decode_SMI_SimpleSyntax decode_SMI_TimeTicks 
 *    encode_SMI_ApplicationSyntax encode_SMI_Counter encode_SMI_DisplayString 
 *    encode_SMI_Gauge encode_SMI_IpAddress encode_SMI_NetworkAddress 
 *    encode_SMI_ObjectName encode_SMI_ObjectSyntax encode_SMI_Opaque 
 *    encode_SMI_SimpleSyntax encode_SMI_TimeTicks print_SMI_ApplicationSyntax 
 *    print_SMI_Counter print_SMI_DisplayString print_SMI_Gauge 
 *    print_SMI_IpAddress print_SMI_NetworkAddress print_SMI_ObjectName 
 *    print_SMI_ObjectSyntax print_SMI_Opaque print_SMI_SimpleSyntax 
 *    print_SMI_TimeTicks
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/SMI_defs.h
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#ifndef _SMI_defs_
#define _SMI_defs_

#include <isode/pepsy/SMI_pre_defs.h>

#ifndef	lint
#define encode_SMI_ObjectName(pe, top, len, buffer, parm) \
    enc_f(_ZObjectNameSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_ObjectName(pe, top, len, buffer, parm) \
    dec_f(_ZObjectNameSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_ObjectName(pe, top, len, buffer, parm) \
    prnt_f(_ZObjectNameSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_ObjectName_P    _ZObjectNameSMI, &_ZSMI_mod

#define encode_SMI_ObjectSyntax(pe, top, len, buffer, parm) \
    enc_f(_ZObjectSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_ObjectSyntax(pe, top, len, buffer, parm) \
    dec_f(_ZObjectSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_ObjectSyntax(pe, top, len, buffer, parm) \
    prnt_f(_ZObjectSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_ObjectSyntax_P    _ZObjectSyntaxSMI, &_ZSMI_mod

#define encode_SMI_SimpleSyntax(pe, top, len, buffer, parm) \
    enc_f(_ZSimpleSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_SimpleSyntax(pe, top, len, buffer, parm) \
    dec_f(_ZSimpleSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_SimpleSyntax(pe, top, len, buffer, parm) \
    prnt_f(_ZSimpleSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_SimpleSyntax_P    _ZSimpleSyntaxSMI, &_ZSMI_mod

#define encode_SMI_ApplicationSyntax(pe, top, len, buffer, parm) \
    enc_f(_ZApplicationSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_ApplicationSyntax(pe, top, len, buffer, parm) \
    dec_f(_ZApplicationSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_ApplicationSyntax(pe, top, len, buffer, parm) \
    prnt_f(_ZApplicationSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_ApplicationSyntax_P    _ZApplicationSyntaxSMI, &_ZSMI_mod

#define encode_SMI_NetworkAddress(pe, top, len, buffer, parm) \
    enc_f(_ZNetworkAddressSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_NetworkAddress(pe, top, len, buffer, parm) \
    dec_f(_ZNetworkAddressSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_NetworkAddress(pe, top, len, buffer, parm) \
    prnt_f(_ZNetworkAddressSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_NetworkAddress_P    _ZNetworkAddressSMI, &_ZSMI_mod

#define encode_SMI_IpAddress(pe, top, len, buffer, parm) \
    enc_f(_ZIpAddressSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_IpAddress(pe, top, len, buffer, parm) \
    dec_f(_ZIpAddressSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_IpAddress(pe, top, len, buffer, parm) \
    prnt_f(_ZIpAddressSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_IpAddress_P    _ZIpAddressSMI, &_ZSMI_mod

#define encode_SMI_Counter(pe, top, len, buffer, parm) \
    enc_f(_ZCounterSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_Counter(pe, top, len, buffer, parm) \
    dec_f(_ZCounterSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_Counter(pe, top, len, buffer, parm) \
    prnt_f(_ZCounterSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_Counter_P    _ZCounterSMI, &_ZSMI_mod

#define encode_SMI_Gauge(pe, top, len, buffer, parm) \
    enc_f(_ZGaugeSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_Gauge(pe, top, len, buffer, parm) \
    dec_f(_ZGaugeSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_Gauge(pe, top, len, buffer, parm) \
    prnt_f(_ZGaugeSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_Gauge_P    _ZGaugeSMI, &_ZSMI_mod

#define encode_SMI_TimeTicks(pe, top, len, buffer, parm) \
    enc_f(_ZTimeTicksSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_TimeTicks(pe, top, len, buffer, parm) \
    dec_f(_ZTimeTicksSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_TimeTicks(pe, top, len, buffer, parm) \
    prnt_f(_ZTimeTicksSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_TimeTicks_P    _ZTimeTicksSMI, &_ZSMI_mod

#define encode_SMI_Opaque(pe, top, len, buffer, parm) \
    enc_f(_ZOpaqueSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_Opaque(pe, top, len, buffer, parm) \
    dec_f(_ZOpaqueSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_Opaque(pe, top, len, buffer, parm) \
    prnt_f(_ZOpaqueSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_Opaque_P    _ZOpaqueSMI, &_ZSMI_mod

#define encode_SMI_DisplayString(pe, top, len, buffer, parm) \
    enc_f(_ZDisplayStringSMI, &_ZSMI_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMI_DisplayString(pe, top, len, buffer, parm) \
    dec_f(_ZDisplayStringSMI, &_ZSMI_mod, pe, top, len, buffer, (char **) parm)

#define print_SMI_DisplayString(pe, top, len, buffer, parm) \
    prnt_f(_ZDisplayStringSMI, &_ZSMI_mod, pe, top, len, buffer)
#define print_SMI_DisplayString_P    _ZDisplayStringSMI, &_ZSMI_mod


#endif   /* lint */

#endif /* _SMI_defs_ */
