static char sccsid[] = "@(#)42	1.4  src/tcpip/usr/lib/libsnmp/SMUX_tables.c, snmp, tcpip411, GOLD410 4/5/93 17:27:57";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/SMUX_tables.c
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
#include <isode/pepsy/SMUX-types.h>

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_PDUsSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_SMUX_PDUs, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.simple), _ZSimpleOpenSMUX, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.close), _ZClosePDUSMUX, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.registerRequest), _ZRReqPDUSMUX, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.registerResponse), _ZRRspPDUSMUX, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.get__request), _ZGetRequest_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.get__next__request), _ZGetNextRequest_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.get__response), _ZGetResponse_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.set__request), _ZSetRequest_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.trap), _ZTrap_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.commitOrRollback), _ZSOutPDUSMUX, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_OpenPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSimpleOpenSMUX, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SimpleOpenSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 0, FL_APPLICATION },
	{ INTEGER, OFFSET(struct type_SMUX_SimpleOpen, version), 2, FL_UNIVERSAL },
	{ OBJID, OFFSET(struct type_SMUX_SimpleOpen, identity), 6, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_SMUX_SimpleOpen, description), _ZDisplayStringSMI, 0 },
	{ EXTMOD, 1, 0, 0 },
	{ OCTETSTRING, OFFSET(struct type_SMUX_SimpleOpen, password), 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ClosePDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMUX_ClosePDU, parm), 1, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RReqPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 2, FL_APPLICATION },
	{ EXTOBJ, OFFSET(struct type_SMUX_RReqPDU, subtree), _ZObjectNameSMI, 0 },
	{ EXTMOD, 1, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMUX_RReqPDU, priority), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SMUX_RReqPDU, operation), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RRspPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMUX_RRspPDU, parm), 3, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SOutPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMUX_SOutPDU, parm), 4, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PDUsSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMUX_PDUs), 0 },
	{ SCTRL, OFFSET(struct type_SMUX_PDUs, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.simple), _ZSimpleOpenSMUX, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.close), _ZClosePDUSMUX, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.registerRequest), _ZRReqPDUSMUX, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.registerResponse), _ZRRspPDUSMUX, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.get__request), _ZGetRequest_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.get__next__request), _ZGetNextRequest_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.get__response), _ZGetResponse_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.set__request), _ZSetRequest_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_PDUs, un.trap), _ZTrap_PDUSNMP, 0 },
	{ EXTMOD, 0, 0, 0 },
	{ OBJECT, OFFSET(struct type_SMUX_PDUs, un.commitOrRollback), _ZSOutPDUSMUX, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_OpenPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSimpleOpenSMUX, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SimpleOpenSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 0, FL_APPLICATION },
	{ MEMALLOC, 0, sizeof (struct type_SMUX_SimpleOpen), 0 },
	{ INTEGER, OFFSET(struct type_SMUX_SimpleOpen, version), 2, FL_UNIVERSAL },
	{ OBJID, OFFSET(struct type_SMUX_SimpleOpen, identity), 6, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_SMUX_SimpleOpen, description), _ZDisplayStringSMI, 0 },
	{ EXTMOD, 1, 0, 0 },
	{ OCTETSTRING, OFFSET(struct type_SMUX_SimpleOpen, password), 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ClosePDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMUX_ClosePDU), 0 },
	{ INTEGER, OFFSET(struct type_SMUX_ClosePDU, parm), 1, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RReqPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 2, FL_APPLICATION },
	{ MEMALLOC, 0, sizeof (struct type_SMUX_RReqPDU), 0 },
	{ EXTOBJ, OFFSET(struct type_SMUX_RReqPDU, subtree), _ZObjectNameSMI, 0 },
	{ EXTMOD, 1, 0, 0 },
	{ INTEGER, OFFSET(struct type_SMUX_RReqPDU, priority), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_SMUX_RReqPDU, operation), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RRspPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMUX_RRspPDU), 0 },
	{ INTEGER, OFFSET(struct type_SMUX_RRspPDU, parm), 3, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SOutPDUSMUX[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_SMUX_SOutPDU), 0 },
	{ INTEGER, OFFSET(struct type_SMUX_SOutPDU, parm), 4, FL_APPLICATION },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_PDUsSMUX[] = {
	{ PE_START, 0, 0, 0, "PDUs" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZSimpleOpenSMUX, 0, "simple" },
	{ OBJECT, 0, _ZClosePDUSMUX, 0, "close" },
	{ OBJECT, 0, _ZRReqPDUSMUX, 0, "registerRequest" },
	{ OBJECT, 0, _ZRRspPDUSMUX, 0, "registerResponse" },
	{ EXTOBJ, 0, _ZGetRequest_PDUSNMP, 0, "get-request" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ EXTOBJ, 0, _ZGetNextRequest_PDUSNMP, 0, "get-next-request" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ EXTOBJ, 0, _ZGetResponse_PDUSNMP, 0, "get-response" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ EXTOBJ, 0, _ZSetRequest_PDUSNMP, 0, "set-request" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ EXTOBJ, 0, _ZTrap_PDUSNMP, 0, "trap" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZSOutPDUSMUX, 0, "commitOrRollback" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_OpenPDUSMUX[] = {
	{ PE_START, 0, 0, 0, "OpenPDU" },
	{ SOBJECT, 0, _ZSimpleOpenSMUX, 0, "simple" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SimpleOpenSMUX[] = {
	{ PE_START, 0, 0, 0, "SimpleOpen" },
	{ SSEQ_START, -1, 0, FL_APPLICATION, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "version" },
	{ OBJID, -1, 6, FL_UNIVERSAL, "identity" },
	{ EXTOBJ, 0, _ZDisplayStringSMI, 0, "description" },
	{ EXTMOD, 1, 0, 0, NULL },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL, "password" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ClosePDUSMUX[] = {
	{ PE_START, 0, 0, 0, "ClosePDU" },
	{ INTEGER, -1, 1, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RReqPDUSMUX[] = {
	{ PE_START, 0, 0, 0, "RReqPDU" },
	{ SSEQ_START, -1, 2, FL_APPLICATION, NULL},
	{ EXTOBJ, 0, _ZObjectNameSMI, 0, "subtree" },
	{ EXTMOD, 1, 0, 0, NULL },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "priority" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "operation" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RRspPDUSMUX[] = {
	{ PE_START, 0, 0, 0, "RRspPDU" },
	{ INTEGER, -1, 3, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SOutPDUSMUX[] = {
	{ PE_START, 0, 0, 0, "SOutPDU" },
	{ INTEGER, -1, 4, FL_APPLICATION, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_PDUsSMUX,
	et_OpenPDUSMUX,
	et_SimpleOpenSMUX,
	et_ClosePDUSMUX,
	et_RReqPDUSMUX,
	et_RRspPDUSMUX,
	et_SOutPDUSMUX,
	};

static tpe *dtabl[] = {
	dt_PDUsSMUX,
	dt_OpenPDUSMUX,
	dt_SimpleOpenSMUX,
	dt_ClosePDUSMUX,
	dt_RReqPDUSMUX,
	dt_RRspPDUSMUX,
	dt_SOutPDUSMUX,
	};

static ptpe *ptabl[] = {
	pt_PDUsSMUX,
	pt_OpenPDUSMUX,
	pt_SimpleOpenSMUX,
	pt_ClosePDUSMUX,
	pt_RReqPDUSMUX,
	pt_RRspPDUSMUX,
	pt_SOutPDUSMUX,
	};


/* Pointer table 2 entries */
static caddr_t _ZptrtabSMUX[] = {
    (caddr_t ) &_ZSNMP_mod,
    (caddr_t ) &_ZSMI_mod,
};

modtyp _ZSMUX_mod = {
	"SMUX",
	7,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabSMUX,
	};


#ifdef	lint

#undef encode_SMUX_PDUs
int	encode_SMUX_PDUs(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMUX_PDUs *parm;
{
  return (enc_f(_ZPDUsSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMUX_PDUs
int	decode_SMUX_PDUs(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_PDUs **parm;
{
  return (dec_f(_ZPDUsSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMUX_PDUs
/* ARGSUSED */
int	print_SMUX_PDUs(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_PDUs *parm;
{
  return (prnt_f(_ZPDUsSMUX, &_ZSMUX_mod, pe, top, len, buffer));
}

#undef encode_SMUX_OpenPDU
int	encode_SMUX_OpenPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMUX_OpenPDU *parm;
{
  return (enc_f(_ZOpenPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMUX_OpenPDU
int	decode_SMUX_OpenPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_OpenPDU **parm;
{
  return (dec_f(_ZOpenPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMUX_OpenPDU
/* ARGSUSED */
int	print_SMUX_OpenPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_OpenPDU *parm;
{
  return (prnt_f(_ZOpenPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer));
}

#undef encode_SMUX_SimpleOpen
int	encode_SMUX_SimpleOpen(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMUX_SimpleOpen *parm;
{
  return (enc_f(_ZSimpleOpenSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMUX_SimpleOpen
int	decode_SMUX_SimpleOpen(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_SimpleOpen **parm;
{
  return (dec_f(_ZSimpleOpenSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMUX_SimpleOpen
/* ARGSUSED */
int	print_SMUX_SimpleOpen(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_SimpleOpen *parm;
{
  return (prnt_f(_ZSimpleOpenSMUX, &_ZSMUX_mod, pe, top, len, buffer));
}

#undef encode_SMUX_ClosePDU
int	encode_SMUX_ClosePDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMUX_ClosePDU *parm;
{
  return (enc_f(_ZClosePDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMUX_ClosePDU
int	decode_SMUX_ClosePDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_ClosePDU **parm;
{
  return (dec_f(_ZClosePDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMUX_ClosePDU
/* ARGSUSED */
int	print_SMUX_ClosePDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_ClosePDU *parm;
{
  return (prnt_f(_ZClosePDUSMUX, &_ZSMUX_mod, pe, top, len, buffer));
}

#undef encode_SMUX_RReqPDU
int	encode_SMUX_RReqPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMUX_RReqPDU *parm;
{
  return (enc_f(_ZRReqPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMUX_RReqPDU
int	decode_SMUX_RReqPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_RReqPDU **parm;
{
  return (dec_f(_ZRReqPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMUX_RReqPDU
/* ARGSUSED */
int	print_SMUX_RReqPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_RReqPDU *parm;
{
  return (prnt_f(_ZRReqPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer));
}

#undef encode_SMUX_RRspPDU
int	encode_SMUX_RRspPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMUX_RRspPDU *parm;
{
  return (enc_f(_ZRRspPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMUX_RRspPDU
int	decode_SMUX_RRspPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_RRspPDU **parm;
{
  return (dec_f(_ZRRspPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMUX_RRspPDU
/* ARGSUSED */
int	print_SMUX_RRspPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_RRspPDU *parm;
{
  return (prnt_f(_ZRRspPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer));
}

#undef encode_SMUX_SOutPDU
int	encode_SMUX_SOutPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_SMUX_SOutPDU *parm;
{
  return (enc_f(_ZSOutPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_SMUX_SOutPDU
int	decode_SMUX_SOutPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_SOutPDU **parm;
{
  return (dec_f(_ZSOutPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_SMUX_SOutPDU
/* ARGSUSED */
int	print_SMUX_SOutPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_SMUX_SOutPDU *parm;
{
  return (prnt_f(_ZSOutPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer));
}

#endif	/* lint */
