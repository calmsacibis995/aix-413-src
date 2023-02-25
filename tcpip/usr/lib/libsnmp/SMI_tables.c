static char sccsid[] = "@(#)37	1.4  src/tcpip/usr/lib/libsnmp/SMI_tables.c, snmp, tcpip411, GOLD410 4/5/93 17:27:28";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: AOFFSET OFFSET
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/SMI_tables.c
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#include <isode/pepsy/SMI-types.h>

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_ObjectNameSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ObjectSyntaxSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_SMI_ObjectSyntax, offset), 0, 0 },
	{ INTEGER, OFFSET(struct type_SMI_ObjectSyntax, un.number), 2, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct type_SMI_ObjectSyntax, un.string), 4, FL_UNIVERSAL },
	{ OBJID, OFFSET(struct type_SMI_ObjectSyntax, un.object), 6, FL_UNIVERSAL },
	{ T_NULL, 0, 5, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.internet), _ZIpAddressSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.counter), _ZCounterSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.gauge), _ZGaugeSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.ticks), _ZTimeTicksSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.arbitrary), _ZOpaqueSMI, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SimpleSyntaxSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_SMI_SimpleSyntax, offset), 0, 0 },
	{ INTEGER, OFFSET(struct type_SMI_SimpleSyntax, un.number), 2, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct type_SMI_SimpleSyntax, un.string), 4, FL_UNIVERSAL },
	{ OBJID, OFFSET(struct type_SMI_SimpleSyntax, un.object), 6, FL_UNIVERSAL },
	{ T_NULL, 0, 5, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ApplicationSyntaxSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_SMI_ApplicationSyntax, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.internet), _ZIpAddressSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.counter), _ZCounterSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.gauge), _ZGaugeSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.ticks), _ZTimeTicksSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.arbitrary), _ZOpaqueSMI, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_NetworkAddressSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZIpAddressSMI, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_IpAddressSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 0, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CounterSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMI_Counter, parm), 1, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GaugeSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMI_Gauge, parm), 2, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_TimeTicksSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMI_TimeTicks, parm), 3, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_OpaqueSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_DisplayStringSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ObjectNameSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ObjectSyntaxSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMI_ObjectSyntax), 0 },
	{ SCTRL, OFFSET(struct type_SMI_ObjectSyntax, offset), 0, 0 },
	{ INTEGER, OFFSET(struct type_SMI_ObjectSyntax, un.number), 2, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct type_SMI_ObjectSyntax, un.string), 4, FL_UNIVERSAL },
	{ OBJID, OFFSET(struct type_SMI_ObjectSyntax, un.object), 6, FL_UNIVERSAL },
	{ T_NULL, 0, 5, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.internet), _ZIpAddressSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.counter), _ZCounterSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.gauge), _ZGaugeSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.ticks), _ZTimeTicksSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ObjectSyntax, un.arbitrary), _ZOpaqueSMI, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SimpleSyntaxSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMI_SimpleSyntax), 0 },
	{ SCTRL, OFFSET(struct type_SMI_SimpleSyntax, offset), 0, 0 },
	{ INTEGER, OFFSET(struct type_SMI_SimpleSyntax, un.number), 2, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct type_SMI_SimpleSyntax, un.string), 4, FL_UNIVERSAL },
	{ OBJID, OFFSET(struct type_SMI_SimpleSyntax, un.object), 6, FL_UNIVERSAL },
	{ T_NULL, 0, 5, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ApplicationSyntaxSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMI_ApplicationSyntax), 0 },
	{ SCTRL, OFFSET(struct type_SMI_ApplicationSyntax, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.internet), _ZIpAddressSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.counter), _ZCounterSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.gauge), _ZGaugeSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.ticks), _ZTimeTicksSMI, 0 },
	{ OBJECT, OFFSET(struct type_SMI_ApplicationSyntax, un.arbitrary), _ZOpaqueSMI, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_NetworkAddressSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZIpAddressSMI, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_IpAddressSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 0, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CounterSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMI_Counter), 0 },
	{ INTEGER, OFFSET(struct type_SMI_Counter, parm), 1, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GaugeSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMI_Gauge), 0 },
	{ INTEGER, OFFSET(struct type_SMI_Gauge, parm), 2, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_TimeTicksSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMI_TimeTicks), 0 },
	{ INTEGER, OFFSET(struct type_SMI_TimeTicks, parm), 3, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_OpaqueSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_DisplayStringSMI[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_ObjectNameSMI[] = {
	{ PE_START, 0, 0, 0, "ObjectName" },
	{ SOBJID, -1, 6, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ObjectSyntaxSMI[] = {
	{ PE_START, 0, 0, 0, "ObjectSyntax" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "number" },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL, "string" },
	{ OBJID, -1, 6, FL_UNIVERSAL, "object" },
	{ T_NULL, -1, 5, FL_UNIVERSAL, "empty" },
	{ OBJECT, 0, _ZIpAddressSMI, 0, "internet" },
	{ OBJECT, 0, _ZCounterSMI, 0, "counter" },
	{ OBJECT, 0, _ZGaugeSMI, 0, "gauge" },
	{ OBJECT, 0, _ZTimeTicksSMI, 0, "ticks" },
	{ OBJECT, 0, _ZOpaqueSMI, 0, "arbitrary" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SimpleSyntaxSMI[] = {
	{ PE_START, 0, 0, 0, "SimpleSyntax" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "number" },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL, "string" },
	{ OBJID, -1, 6, FL_UNIVERSAL, "object" },
	{ T_NULL, -1, 5, FL_UNIVERSAL, "empty" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ApplicationSyntaxSMI[] = {
	{ PE_START, 0, 0, 0, "ApplicationSyntax" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZIpAddressSMI, 0, "internet" },
	{ OBJECT, 0, _ZCounterSMI, 0, "counter" },
	{ OBJECT, 0, _ZGaugeSMI, 0, "gauge" },
	{ OBJECT, 0, _ZTimeTicksSMI, 0, "ticks" },
	{ OBJECT, 0, _ZOpaqueSMI, 0, "arbitrary" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_NetworkAddressSMI[] = {
	{ PE_START, 0, 0, 0, "NetworkAddress" },
	{ SOBJECT, 0, _ZIpAddressSMI, 0, "internet" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_IpAddressSMI[] = {
	{ PE_START, 0, 0, 0, "IpAddress" },
	{ SOCTETSTRING, -1, 0, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CounterSMI[] = {
	{ PE_START, 0, 0, 0, "Counter" },
	{ INTEGER, -1, 1, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GaugeSMI[] = {
	{ PE_START, 0, 0, 0, "Gauge" },
	{ INTEGER, -1, 2, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_TimeTicksSMI[] = {
	{ PE_START, 0, 0, 0, "TimeTicks" },
	{ INTEGER, -1, 3, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_OpaqueSMI[] = {
	{ PE_START, 0, 0, 0, "Opaque" },
	{ SOCTETSTRING, -1, 4, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_DisplayStringSMI[] = {
	{ PE_START, 0, 0, 0, "DisplayString" },
	{ SOCTETSTRING, -1, 4, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_ObjectNameSMI,
	et_ObjectSyntaxSMI,
	et_SimpleSyntaxSMI,
	et_ApplicationSyntaxSMI,
	et_NetworkAddressSMI,
	et_IpAddressSMI,
	et_CounterSMI,
	et_GaugeSMI,
	et_TimeTicksSMI,
	et_OpaqueSMI,
	et_DisplayStringSMI,
	};

static tpe *dtabl[] = {
	dt_ObjectNameSMI,
	dt_ObjectSyntaxSMI,
	dt_SimpleSyntaxSMI,
	dt_ApplicationSyntaxSMI,
	dt_NetworkAddressSMI,
	dt_IpAddressSMI,
	dt_CounterSMI,
	dt_GaugeSMI,
	dt_TimeTicksSMI,
	dt_OpaqueSMI,
	dt_DisplayStringSMI,
	};

static ptpe *ptabl[] = {
	pt_ObjectNameSMI,
	pt_ObjectSyntaxSMI,
	pt_SimpleSyntaxSMI,
	pt_ApplicationSyntaxSMI,
	pt_NetworkAddressSMI,
	pt_IpAddressSMI,
	pt_CounterSMI,
	pt_GaugeSMI,
	pt_TimeTicksSMI,
	pt_OpaqueSMI,
	pt_DisplayStringSMI,
	};


/* Pointer table 0 entries */
static caddr_t _ZptrtabSMI[] = {
    (caddr_t ) 0,
};

modtyp _ZSMI_mod = {
	"SMI",
	11,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabSMI,
	};


#ifdef	lint

#undef encode_SMI_ObjectName
int	encode_SMI_ObjectName(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_ObjectName *parm;
{
  return (enc_f(_ZObjectNameSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_ObjectName
int	decode_SMI_ObjectName(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_ObjectName **parm;
{
  return (dec_f(_ZObjectNameSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_ObjectName
/* ARGSUSED */
int	print_SMI_ObjectName(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_ObjectName *parm;
{
  return (prnt_f(_ZObjectNameSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_ObjectSyntax
int	encode_SMI_ObjectSyntax(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_ObjectSyntax *parm;
{
  return (enc_f(_ZObjectSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_ObjectSyntax
int	decode_SMI_ObjectSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_ObjectSyntax **parm;
{
  return (dec_f(_ZObjectSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_ObjectSyntax
/* ARGSUSED */
int	print_SMI_ObjectSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_ObjectSyntax *parm;
{
  return (prnt_f(_ZObjectSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_SimpleSyntax
int	encode_SMI_SimpleSyntax(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_SimpleSyntax *parm;
{
  return (enc_f(_ZSimpleSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_SimpleSyntax
int	decode_SMI_SimpleSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_SimpleSyntax **parm;
{
  return (dec_f(_ZSimpleSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_SimpleSyntax
/* ARGSUSED */
int	print_SMI_SimpleSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_SimpleSyntax *parm;
{
  return (prnt_f(_ZSimpleSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_ApplicationSyntax
int	encode_SMI_ApplicationSyntax(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_ApplicationSyntax *parm;
{
  return (enc_f(_ZApplicationSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_ApplicationSyntax
int	decode_SMI_ApplicationSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_ApplicationSyntax **parm;
{
  return (dec_f(_ZApplicationSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_ApplicationSyntax
/* ARGSUSED */
int	print_SMI_ApplicationSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_ApplicationSyntax *parm;
{
  return (prnt_f(_ZApplicationSyntaxSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_NetworkAddress
int	encode_SMI_NetworkAddress(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_NetworkAddress *parm;
{
  return (enc_f(_ZNetworkAddressSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_NetworkAddress
int	decode_SMI_NetworkAddress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_NetworkAddress **parm;
{
  return (dec_f(_ZNetworkAddressSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_NetworkAddress
/* ARGSUSED */
int	print_SMI_NetworkAddress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_NetworkAddress *parm;
{
  return (prnt_f(_ZNetworkAddressSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_IpAddress
int	encode_SMI_IpAddress(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_IpAddress *parm;
{
  return (enc_f(_ZIpAddressSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_IpAddress
int	decode_SMI_IpAddress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_IpAddress **parm;
{
  return (dec_f(_ZIpAddressSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_IpAddress
/* ARGSUSED */
int	print_SMI_IpAddress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_IpAddress *parm;
{
  return (prnt_f(_ZIpAddressSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_Counter
int	encode_SMI_Counter(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_Counter *parm;
{
  return (enc_f(_ZCounterSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_Counter
int	decode_SMI_Counter(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_Counter **parm;
{
  return (dec_f(_ZCounterSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_Counter
/* ARGSUSED */
int	print_SMI_Counter(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_Counter *parm;
{
  return (prnt_f(_ZCounterSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_Gauge
int	encode_SMI_Gauge(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_Gauge *parm;
{
  return (enc_f(_ZGaugeSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_Gauge
int	decode_SMI_Gauge(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_Gauge **parm;
{
  return (dec_f(_ZGaugeSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_Gauge
/* ARGSUSED */
int	print_SMI_Gauge(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_Gauge *parm;
{
  return (prnt_f(_ZGaugeSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_TimeTicks
int	encode_SMI_TimeTicks(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_TimeTicks *parm;
{
  return (enc_f(_ZTimeTicksSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_TimeTicks
int	decode_SMI_TimeTicks(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_TimeTicks **parm;
{
  return (dec_f(_ZTimeTicksSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_TimeTicks
/* ARGSUSED */
int	print_SMI_TimeTicks(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_TimeTicks *parm;
{
  return (prnt_f(_ZTimeTicksSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_Opaque
int	encode_SMI_Opaque(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_Opaque *parm;
{
  return (enc_f(_ZOpaqueSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_Opaque
int	decode_SMI_Opaque(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_Opaque **parm;
{
  return (dec_f(_ZOpaqueSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_Opaque
/* ARGSUSED */
int	print_SMI_Opaque(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_Opaque *parm;
{
  return (prnt_f(_ZOpaqueSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#undef encode_SMI_DisplayString
int	encode_SMI_DisplayString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMI_DisplayString *parm;
{
  return (enc_f(_ZDisplayStringSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMI_DisplayString
int	decode_SMI_DisplayString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_DisplayString **parm;
{
  return (dec_f(_ZDisplayStringSMI, &_ZSMI_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMI_DisplayString
/* ARGSUSED */
int	print_SMI_DisplayString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMI_DisplayString *parm;
{
  return (prnt_f(_ZDisplayStringSMI, &_ZSMI_mod, pe, top, len, buffer));
}

#endif	/* lint */
