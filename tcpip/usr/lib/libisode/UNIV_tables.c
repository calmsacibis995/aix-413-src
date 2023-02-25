static char sccsid[] = "@(#)45	1.4  src/tcpip/usr/lib/libisode/UNIV_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:36:54";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AOFFSET OFFSET
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/UNIV_tables.c
 */

#include <stdio.h>
#include <isode/pepsy/UNIV-types.h>


# line 33 "UNIV.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/UNIV_tables.c,v 1.2 93/02/05 17:02:41 snmp Exp $";
#endif

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_IA5StringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 22, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_NumericStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 18, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PrintableStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 19, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_T61StringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_TeletexStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_VideotexStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 21, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GeneralizedTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 24, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GeneralisedTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 24, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_UTCTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_UniversalTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GraphicStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 25, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_VisibleStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 26, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ISO646StringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 26, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GeneralStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 27, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CharacterStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 28, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_EXTERNALUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 8, FL_UNIVERSAL },
	{ OBJID, OFFSET(struct type_UNIV_EXTERNAL, direct__reference), 6, FL_UNIVERSAL|FL_OPTIONAL },
	{ DFLT_F,	0,	0,	0 },
	{ INTEGER, OFFSET(struct type_UNIV_EXTERNAL, indirect__reference), 2, FL_UNIVERSAL|FL_DEFAULT },
	{ OBJECT, OFFSET(struct type_UNIV_EXTERNAL, data__value__descriptor), _ZObjectDescriptorUNIV, FL_OPTIONAL },
	{ CHOICE_START, OFFSET(struct type_UNIV_EXTERNAL, encoding), 0, 0 },
	{ SCTRL, OFFSET(struct choice_UNIV_0, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct choice_UNIV_0, un.single__ASN1__type), -1, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct choice_UNIV_0, un.octet__aligned), 1, FL_CONTEXT },
	{ BITSTRING, OFFSET(struct choice_UNIV_0, un.arbitrary), 2, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ObjectDescriptorUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 7, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_IA5StringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 22, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_NumericStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 18, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PrintableStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 19, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_T61StringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_TeletexStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_VideotexStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 21, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GeneralizedTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 24, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GeneralisedTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 24, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_UTCTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_UniversalTimeUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GraphicStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 25, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_VisibleStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 26, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ISO646StringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 26, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GeneralStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 27, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CharacterStringUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 28, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_EXTERNALUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 8, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_UNIV_EXTERNAL), 0 },
	{ OBJID, OFFSET(struct type_UNIV_EXTERNAL, direct__reference), 6, FL_UNIVERSAL|FL_OPTIONAL },
	{ INTEGER, OFFSET(struct type_UNIV_EXTERNAL, indirect__reference), 2, FL_UNIVERSAL|FL_DEFAULT },
	{ DFLT_B,	0,	0,	0 },
	{ OBJECT, OFFSET(struct type_UNIV_EXTERNAL, data__value__descriptor), _ZObjectDescriptorUNIV, FL_OPTIONAL },
	{ CHOICE_START, OFFSET(struct type_UNIV_EXTERNAL, encoding), 0, 0 },
	{ MEMALLOC, 0, sizeof (struct choice_UNIV_0), 0 },
	{ SCTRL, OFFSET(struct choice_UNIV_0, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct choice_UNIV_0, un.single__ASN1__type), -1, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct choice_UNIV_0, un.octet__aligned), 1, FL_CONTEXT },
	{ BITSTRING, OFFSET(struct choice_UNIV_0, un.arbitrary), 2, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ObjectDescriptorUNIV[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 7, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_IA5StringUNIV[] = {
	{ PE_START, 0, 0, 0, "IA5String" },
	{ SOCTETSTRING, -1, 22, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_NumericStringUNIV[] = {
	{ PE_START, 0, 0, 0, "NumericString" },
	{ SOCTETSTRING, 0, 18, FL_UNIVERSAL, "IA5String" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PrintableStringUNIV[] = {
	{ PE_START, 0, 0, 0, "PrintableString" },
	{ SOCTETSTRING, 0, 19, FL_UNIVERSAL, "IA5String" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_T61StringUNIV[] = {
	{ PE_START, 0, 0, 0, "T61String" },
	{ SOCTETSTRING, -1, 20, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_TeletexStringUNIV[] = {
	{ PE_START, 0, 0, 0, "TeletexString" },
	{ SOCTETSTRING, 0, 20, 0, "T61String" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_VideotexStringUNIV[] = {
	{ PE_START, 0, 0, 0, "VideotexString" },
	{ SOCTETSTRING, -1, 21, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GeneralizedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, "GeneralizedTime" },
	{ SOCTETSTRING, 0, 24, FL_UNIVERSAL, "VisibleString" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GeneralisedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, "GeneralisedTime" },
	{ SOCTETSTRING, 0, 24, 0, "GeneralizedTime" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_UTCTimeUNIV[] = {
	{ PE_START, 0, 0, 0, "UTCTime" },
	{ SOCTETSTRING, 0, 23, FL_UNIVERSAL, "VisibleString" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_UniversalTimeUNIV[] = {
	{ PE_START, 0, 0, 0, "UniversalTime" },
	{ SOCTETSTRING, 0, 23, 0, "UTCTime" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GraphicStringUNIV[] = {
	{ PE_START, 0, 0, 0, "GraphicString" },
	{ SOCTETSTRING, -1, 25, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_VisibleStringUNIV[] = {
	{ PE_START, 0, 0, 0, "VisibleString" },
	{ SOCTETSTRING, -1, 26, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ISO646StringUNIV[] = {
	{ PE_START, 0, 0, 0, "ISO646String" },
	{ SOCTETSTRING, 0, 26, 0, "VisibleString" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GeneralStringUNIV[] = {
	{ PE_START, 0, 0, 0, "GeneralString" },
	{ SOCTETSTRING, -1, 27, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CharacterStringUNIV[] = {
	{ PE_START, 0, 0, 0, "CharacterString" },
	{ SOCTETSTRING, -1, 28, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_EXTERNALUNIV[] = {
	{ PE_START, 0, 0, 0, "EXTERNAL" },
	{ SSEQ_START, -1, 8, FL_UNIVERSAL, NULL},
	{ OBJID, -1, 6, FL_UNIVERSAL|FL_OPTIONAL, "direct-reference" },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_DEFAULT, "indirect-reference" },
	{ DFLT_B,	0,	0,	0 },
	{ OBJECT, 0, _ZObjectDescriptorUNIV, FL_OPTIONAL, "data-value-descriptor" },
	{ CHOICE_START, -1, 0, 0, "encoding" },
	{ SCTRL, 0, 0, 0, "encoding" },
	{ ETAG, -1, 0, FL_CONTEXT, "single-ASN1-type" },
	{ ANY, -1, -1, FL_UNIVERSAL, NULL},
	{ OCTETSTRING, -1, 1, FL_CONTEXT, "octet-aligned" },
	{ BITSTRING, -1, 2, FL_CONTEXT, "arbitrary" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ObjectDescriptorUNIV[] = {
	{ PE_START, 0, 0, 0, "ObjectDescriptor" },
	{ SOCTETSTRING, 0, 7, FL_UNIVERSAL, "GraphicString" },
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_IA5StringUNIV,
	et_NumericStringUNIV,
	et_PrintableStringUNIV,
	et_T61StringUNIV,
	et_TeletexStringUNIV,
	et_VideotexStringUNIV,
	et_GeneralizedTimeUNIV,
	et_GeneralisedTimeUNIV,
	et_UTCTimeUNIV,
	et_UniversalTimeUNIV,
	et_GraphicStringUNIV,
	et_VisibleStringUNIV,
	et_ISO646StringUNIV,
	et_GeneralStringUNIV,
	et_CharacterStringUNIV,
	et_EXTERNALUNIV,
	et_ObjectDescriptorUNIV,
	};

static tpe *dtabl[] = {
	dt_IA5StringUNIV,
	dt_NumericStringUNIV,
	dt_PrintableStringUNIV,
	dt_T61StringUNIV,
	dt_TeletexStringUNIV,
	dt_VideotexStringUNIV,
	dt_GeneralizedTimeUNIV,
	dt_GeneralisedTimeUNIV,
	dt_UTCTimeUNIV,
	dt_UniversalTimeUNIV,
	dt_GraphicStringUNIV,
	dt_VisibleStringUNIV,
	dt_ISO646StringUNIV,
	dt_GeneralStringUNIV,
	dt_CharacterStringUNIV,
	dt_EXTERNALUNIV,
	dt_ObjectDescriptorUNIV,
	};

static ptpe *ptabl[] = {
	pt_IA5StringUNIV,
	pt_NumericStringUNIV,
	pt_PrintableStringUNIV,
	pt_T61StringUNIV,
	pt_TeletexStringUNIV,
	pt_VideotexStringUNIV,
	pt_GeneralizedTimeUNIV,
	pt_GeneralisedTimeUNIV,
	pt_UTCTimeUNIV,
	pt_UniversalTimeUNIV,
	pt_GraphicStringUNIV,
	pt_VisibleStringUNIV,
	pt_ISO646StringUNIV,
	pt_GeneralStringUNIV,
	pt_CharacterStringUNIV,
	pt_EXTERNALUNIV,
	pt_ObjectDescriptorUNIV,
	};


/* Pointer table 0 entries */
static caddr_t _ZptrtabUNIV[] = {
    (caddr_t ) 0,
};

modtyp _ZUNIV_mod = {
	"UNIV",
	17,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabUNIV,
	};


# line 145 "UNIV.py"


#ifndef PEPSY_VERSION

PEPYPARM NullParm = (PEPYPARM) 0;

#endif


#ifdef	lint

#undef encode_UNIV_IA5String
int	encode_UNIV_IA5String(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_IA5String *parm;
{
  return (enc_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_IA5String
int	decode_UNIV_IA5String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_IA5String **parm;
{
  return (dec_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_IA5String
/* ARGSUSED */
int	print_UNIV_IA5String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_IA5String *parm;
{
  return (prnt_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_NumericString
int	encode_UNIV_NumericString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_NumericString *parm;
{
  return (enc_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_NumericString
int	decode_UNIV_NumericString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_NumericString **parm;
{
  return (dec_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_NumericString
/* ARGSUSED */
int	print_UNIV_NumericString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_NumericString *parm;
{
  return (prnt_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_PrintableString
int	encode_UNIV_PrintableString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_PrintableString *parm;
{
  return (enc_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_PrintableString
int	decode_UNIV_PrintableString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_PrintableString **parm;
{
  return (dec_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_PrintableString
/* ARGSUSED */
int	print_UNIV_PrintableString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_PrintableString *parm;
{
  return (prnt_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_T61String
int	encode_UNIV_T61String(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_T61String *parm;
{
  return (enc_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_T61String
int	decode_UNIV_T61String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_T61String **parm;
{
  return (dec_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_T61String
/* ARGSUSED */
int	print_UNIV_T61String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_T61String *parm;
{
  return (prnt_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_TeletexString
int	encode_UNIV_TeletexString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_TeletexString *parm;
{
  return (enc_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_TeletexString
int	decode_UNIV_TeletexString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_TeletexString **parm;
{
  return (dec_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_TeletexString
/* ARGSUSED */
int	print_UNIV_TeletexString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_TeletexString *parm;
{
  return (prnt_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_VideotexString
int	encode_UNIV_VideotexString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_VideotexString *parm;
{
  return (enc_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_VideotexString
int	decode_UNIV_VideotexString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_VideotexString **parm;
{
  return (dec_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_VideotexString
/* ARGSUSED */
int	print_UNIV_VideotexString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_VideotexString *parm;
{
  return (prnt_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_GeneralizedTime
int	encode_UNIV_GeneralizedTime(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_GeneralizedTime *parm;
{
  return (enc_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_GeneralizedTime
int	decode_UNIV_GeneralizedTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralizedTime **parm;
{
  return (dec_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_GeneralizedTime
/* ARGSUSED */
int	print_UNIV_GeneralizedTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralizedTime *parm;
{
  return (prnt_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_GeneralisedTime
int	encode_UNIV_GeneralisedTime(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_GeneralisedTime *parm;
{
  return (enc_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_GeneralisedTime
int	decode_UNIV_GeneralisedTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralisedTime **parm;
{
  return (dec_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_GeneralisedTime
/* ARGSUSED */
int	print_UNIV_GeneralisedTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralisedTime *parm;
{
  return (prnt_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_UTCTime
int	encode_UNIV_UTCTime(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_UTCTime *parm;
{
  return (enc_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_UTCTime
int	decode_UNIV_UTCTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_UTCTime **parm;
{
  return (dec_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_UTCTime
/* ARGSUSED */
int	print_UNIV_UTCTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_UTCTime *parm;
{
  return (prnt_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_UniversalTime
int	encode_UNIV_UniversalTime(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_UniversalTime *parm;
{
  return (enc_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_UniversalTime
int	decode_UNIV_UniversalTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_UniversalTime **parm;
{
  return (dec_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_UniversalTime
/* ARGSUSED */
int	print_UNIV_UniversalTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_UniversalTime *parm;
{
  return (prnt_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_GraphicString
int	encode_UNIV_GraphicString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_GraphicString *parm;
{
  return (enc_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_GraphicString
int	decode_UNIV_GraphicString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GraphicString **parm;
{
  return (dec_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_GraphicString
/* ARGSUSED */
int	print_UNIV_GraphicString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GraphicString *parm;
{
  return (prnt_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_VisibleString
int	encode_UNIV_VisibleString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_VisibleString *parm;
{
  return (enc_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_VisibleString
int	decode_UNIV_VisibleString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_VisibleString **parm;
{
  return (dec_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_VisibleString
/* ARGSUSED */
int	print_UNIV_VisibleString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_VisibleString *parm;
{
  return (prnt_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_ISO646String
int	encode_UNIV_ISO646String(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_ISO646String *parm;
{
  return (enc_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_ISO646String
int	decode_UNIV_ISO646String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_ISO646String **parm;
{
  return (dec_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_ISO646String
/* ARGSUSED */
int	print_UNIV_ISO646String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_ISO646String *parm;
{
  return (prnt_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_GeneralString
int	encode_UNIV_GeneralString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_GeneralString *parm;
{
  return (enc_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_GeneralString
int	decode_UNIV_GeneralString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralString **parm;
{
  return (dec_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_GeneralString
/* ARGSUSED */
int	print_UNIV_GeneralString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralString *parm;
{
  return (prnt_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_CharacterString
int	encode_UNIV_CharacterString(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_CharacterString *parm;
{
  return (enc_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_CharacterString
int	decode_UNIV_CharacterString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_CharacterString **parm;
{
  return (dec_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_CharacterString
/* ARGSUSED */
int	print_UNIV_CharacterString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_CharacterString *parm;
{
  return (prnt_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_EXTERNAL
int	encode_UNIV_EXTERNAL(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_EXTERNAL *parm;
{
  return (enc_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_EXTERNAL
int	decode_UNIV_EXTERNAL(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_EXTERNAL **parm;
{
  return (dec_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_EXTERNAL
/* ARGSUSED */
int	print_UNIV_EXTERNAL(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_EXTERNAL *parm;
{
  return (prnt_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#undef encode_UNIV_ObjectDescriptor
int	encode_UNIV_ObjectDescriptor(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_UNIV_ObjectDescriptor *parm;
{
  return (enc_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_UNIV_ObjectDescriptor
int	decode_UNIV_ObjectDescriptor(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_ObjectDescriptor **parm;
{
  return (dec_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_UNIV_ObjectDescriptor
/* ARGSUSED */
int	print_UNIV_ObjectDescriptor(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_ObjectDescriptor *parm;
{
  return (prnt_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer));
}

#endif	/* lint */
