static char sccsid[] = "@(#)47	1.4  src/tcpip/usr/lib/libsnmp/SNMP_tables.c, snmp, tcpip411, GOLD410 4/5/93 17:28:22";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/SNMP_tables.c
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#include <stdio.h>
#include <isode/pepsy/SNMP-types.h>

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_MessageSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_Message, version), 2, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct type_SNMP_Message, community), 4, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_SNMP_Message, data), _ZPDUsSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PDUsSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_SNMP_PDUs, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.get__request), _ZGetRequest_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.get__next__request), _ZGetNextRequest_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.get__response), _ZGetResponse_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.set__request), _ZSetRequest_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.trap), _ZTrap_PDUSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GetRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GetNextRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GetResponse_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 2, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SetRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 3, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_PDU, request__id), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_PDU, error__status), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_PDU, error__index), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_SNMP_PDU, variable__bindings), _ZVarBindListSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Trap_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 4, FL_CONTEXT },
	{ OBJID, OFFSET(struct type_SNMP_Trap__PDU, enterprise), 6, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_SNMP_Trap__PDU, agent__addr), _ZNetworkAddressSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SNMP_Trap__PDU, generic__trap), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_Trap__PDU, specific__trap), 2, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_SNMP_Trap__PDU, time__stamp), _ZTimeTicksSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_Trap__PDU, variable__bindings), _ZVarBindListSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_VarBindSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_SNMP_VarBind, name), _ZObjectNameSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SNMP_VarBind, value), _ZObjectSyntaxSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_VarBindListSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_SNMP_VarBindList, VarBind), _ZVarBindSNMP, 0 },
	{ PE_END, OFFSET(struct type_SNMP_VarBindList, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_MessageSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_SNMP_Message), 0 },
	{ INTEGER, OFFSET(struct type_SNMP_Message, version), 2, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct type_SNMP_Message, community), 4, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_SNMP_Message, data), _ZPDUsSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PDUsSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SNMP_PDUs), 0 },
	{ SCTRL, OFFSET(struct type_SNMP_PDUs, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.get__request), _ZGetRequest_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.get__next__request), _ZGetNextRequest_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.get__response), _ZGetResponse_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.set__request), _ZSetRequest_PDUSNMP, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_PDUs, un.trap), _ZTrap_PDUSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GetRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GetNextRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GetResponse_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 2, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SetRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ IMP_OBJ, 0, 3, FL_CONTEXT },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_SNMP_PDU), 0 },
	{ INTEGER, OFFSET(struct type_SNMP_PDU, request__id), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_PDU, error__status), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_PDU, error__index), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_SNMP_PDU, variable__bindings), _ZVarBindListSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Trap_PDUSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 4, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct type_SNMP_Trap__PDU), 0 },
	{ OBJID, OFFSET(struct type_SNMP_Trap__PDU, enterprise), 6, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_SNMP_Trap__PDU, agent__addr), _ZNetworkAddressSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SNMP_Trap__PDU, generic__trap), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SNMP_Trap__PDU, specific__trap), 2, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_SNMP_Trap__PDU, time__stamp), _ZTimeTicksSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ OBJECT, OFFSET(struct type_SNMP_Trap__PDU, variable__bindings), _ZVarBindListSNMP, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_VarBindSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_SNMP_VarBind), 0 },
	{ EXTOBJ, OFFSET(struct type_SNMP_VarBind, name), _ZObjectNameSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SNMP_VarBind, value), _ZObjectSyntaxSMI, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_VarBindListSNMP[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_SNMP_VarBindList), 0 },
	{ OBJECT, OFFSET(struct type_SNMP_VarBindList, VarBind), _ZVarBindSNMP, 0 },
	{ PE_END, OFFSET(struct type_SNMP_VarBindList, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_MessageSNMP[] = {
	{ PE_START, 0, 0, 0, "Message" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "version" },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL, "community" },
	{ OBJECT, 0, _ZPDUsSNMP, 0, "data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PDUsSNMP[] = {
	{ PE_START, 0, 0, 0, "PDUs" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZGetRequest_PDUSNMP, 0, "get-request" },
	{ OBJECT, 0, _ZGetNextRequest_PDUSNMP, 0, "get-next-request" },
	{ OBJECT, 0, _ZGetResponse_PDUSNMP, 0, "get-response" },
	{ OBJECT, 0, _ZSetRequest_PDUSNMP, 0, "set-request" },
	{ OBJECT, 0, _ZTrap_PDUSNMP, 0, "trap" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GetRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0, "GetRequest-PDU" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "PDU" },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT, "PDU" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GetNextRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0, "GetNextRequest-PDU" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT, "PDU" },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT, "PDU" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GetResponse_PDUSNMP[] = {
	{ PE_START, 0, 0, 0, "GetResponse-PDU" },
	{ IMP_OBJ, -1, 2, FL_CONTEXT, "PDU" },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT, "PDU" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SetRequest_PDUSNMP[] = {
	{ PE_START, 0, 0, 0, "SetRequest-PDU" },
	{ IMP_OBJ, -1, 3, FL_CONTEXT, "PDU" },
	{ SOBJECT, 0, _ZPDUSNMP, FL_CONTEXT, "PDU" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PDUSNMP[] = {
	{ PE_START, 0, 0, 0, "PDU" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "request-id" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "error-status" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "error-index" },
	{ OBJECT, 0, _ZVarBindListSNMP, 0, "variable-bindings" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Trap_PDUSNMP[] = {
	{ PE_START, 0, 0, 0, "Trap-PDU" },
	{ SSEQ_START, -1, 4, FL_CONTEXT, NULL},
	{ OBJID, -1, 6, FL_UNIVERSAL, "enterprise" },
	{ EXTOBJ, 0, _ZNetworkAddressSMI, 0, "agent-addr" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "generic-trap" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "specific-trap" },
	{ EXTOBJ, 0, _ZTimeTicksSMI, 0, "time-stamp" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZVarBindListSNMP, 0, "variable-bindings" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_VarBindSNMP[] = {
	{ PE_START, 0, 0, 0, "VarBind" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ EXTOBJ, 0, _ZObjectNameSMI, 0, "name" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ EXTOBJ, 0, _ZObjectSyntaxSMI, 0, "value" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_VarBindListSNMP[] = {
	{ PE_START, 0, 0, 0, "VarBindList" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZVarBindSNMP, 0, "VarBind" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_MessageSNMP,
	et_PDUsSNMP,
	et_GetRequest_PDUSNMP,
	et_GetNextRequest_PDUSNMP,
	et_GetResponse_PDUSNMP,
	et_SetRequest_PDUSNMP,
	et_PDUSNMP,
	et_Trap_PDUSNMP,
	et_VarBindSNMP,
	et_VarBindListSNMP,
	};

static tpe *dtabl[] = {
	dt_MessageSNMP,
	dt_PDUsSNMP,
	dt_GetRequest_PDUSNMP,
	dt_GetNextRequest_PDUSNMP,
	dt_GetResponse_PDUSNMP,
	dt_SetRequest_PDUSNMP,
	dt_PDUSNMP,
	dt_Trap_PDUSNMP,
	dt_VarBindSNMP,
	dt_VarBindListSNMP,
	};

static ptpe *ptabl[] = {
	pt_MessageSNMP,
	pt_PDUsSNMP,
	pt_GetRequest_PDUSNMP,
	pt_GetNextRequest_PDUSNMP,
	pt_GetResponse_PDUSNMP,
	pt_SetRequest_PDUSNMP,
	pt_PDUSNMP,
	pt_Trap_PDUSNMP,
	pt_VarBindSNMP,
	pt_VarBindListSNMP,
	};


/* Pointer table 1 entries */
static caddr_t _ZptrtabSNMP[] = {
    (caddr_t ) &_ZSMI_mod,
};

modtyp _ZSNMP_mod = {
	"SNMP",
	10,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabSNMP,
	};


#ifdef	lint

#undef encode_SNMP_Message
int	encode_SNMP_Message(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_Message *parm;
{
  return (enc_f(_ZMessageSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_Message
int	decode_SNMP_Message(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_Message **parm;
{
  return (dec_f(_ZMessageSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_Message
/* ARGSUSED */
int	print_SNMP_Message(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_Message *parm;
{
  return (prnt_f(_ZMessageSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_PDUs
int	encode_SNMP_PDUs(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_PDUs *parm;
{
  return (enc_f(_ZPDUsSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_PDUs
int	decode_SNMP_PDUs(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_PDUs **parm;
{
  return (dec_f(_ZPDUsSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_PDUs
/* ARGSUSED */
int	print_SNMP_PDUs(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_PDUs *parm;
{
  return (prnt_f(_ZPDUsSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_GetRequest__PDU
int	encode_SNMP_GetRequest__PDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_GetRequest__PDU *parm;
{
  return (enc_f(_ZGetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_GetRequest__PDU
int	decode_SNMP_GetRequest__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_GetRequest__PDU **parm;
{
  return (dec_f(_ZGetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_GetRequest__PDU
/* ARGSUSED */
int	print_SNMP_GetRequest__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_GetRequest__PDU *parm;
{
  return (prnt_f(_ZGetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_GetNextRequest__PDU
int	encode_SNMP_GetNextRequest__PDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_GetNextRequest__PDU *parm;
{
  return (enc_f(_ZGetNextRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_GetNextRequest__PDU
int	decode_SNMP_GetNextRequest__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_GetNextRequest__PDU **parm;
{
  return (dec_f(_ZGetNextRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_GetNextRequest__PDU
/* ARGSUSED */
int	print_SNMP_GetNextRequest__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_GetNextRequest__PDU *parm;
{
  return (prnt_f(_ZGetNextRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_GetResponse__PDU
int	encode_SNMP_GetResponse__PDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_GetResponse__PDU *parm;
{
  return (enc_f(_ZGetResponse_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_GetResponse__PDU
int	decode_SNMP_GetResponse__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_GetResponse__PDU **parm;
{
  return (dec_f(_ZGetResponse_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_GetResponse__PDU
/* ARGSUSED */
int	print_SNMP_GetResponse__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_GetResponse__PDU *parm;
{
  return (prnt_f(_ZGetResponse_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_SetRequest__PDU
int	encode_SNMP_SetRequest__PDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_SetRequest__PDU *parm;
{
  return (enc_f(_ZSetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_SetRequest__PDU
int	decode_SNMP_SetRequest__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_SetRequest__PDU **parm;
{
  return (dec_f(_ZSetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_SetRequest__PDU
/* ARGSUSED */
int	print_SNMP_SetRequest__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_SetRequest__PDU *parm;
{
  return (prnt_f(_ZSetRequest_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_PDU
int	encode_SNMP_PDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_PDU *parm;
{
  return (enc_f(_ZPDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_PDU
int	decode_SNMP_PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_PDU **parm;
{
  return (dec_f(_ZPDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_PDU
/* ARGSUSED */
int	print_SNMP_PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_PDU *parm;
{
  return (prnt_f(_ZPDUSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_Trap__PDU
int	encode_SNMP_Trap__PDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_Trap__PDU *parm;
{
  return (enc_f(_ZTrap_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_Trap__PDU
int	decode_SNMP_Trap__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_Trap__PDU **parm;
{
  return (dec_f(_ZTrap_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_Trap__PDU
/* ARGSUSED */
int	print_SNMP_Trap__PDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_Trap__PDU *parm;
{
  return (prnt_f(_ZTrap_PDUSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_VarBind
int	encode_SNMP_VarBind(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_VarBind *parm;
{
  return (enc_f(_ZVarBindSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_VarBind
int	decode_SNMP_VarBind(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_VarBind **parm;
{
  return (dec_f(_ZVarBindSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_VarBind
/* ARGSUSED */
int	print_SNMP_VarBind(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_VarBind *parm;
{
  return (prnt_f(_ZVarBindSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#undef encode_SNMP_VarBindList
int	encode_SNMP_VarBindList(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SNMP_VarBindList *parm;
{
  return (enc_f(_ZVarBindListSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SNMP_VarBindList
int	decode_SNMP_VarBindList(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_VarBindList **parm;
{
  return (dec_f(_ZVarBindListSNMP, &_ZSNMP_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SNMP_VarBindList
/* ARGSUSED */
int	print_SNMP_VarBindList(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SNMP_VarBindList *parm;
{
  return (prnt_f(_ZVarBindListSNMP, &_ZSNMP_mod, pe, top, len, buffer));
}

#endif	/* lint */
