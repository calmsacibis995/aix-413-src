static char sccsid[] = "@(#)59	1.3  src/tcpip/usr/lib/libisode/DASE_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:32:54";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/DASE_tables.c
 */

#include <stdio.h>
#include "DASE-types.h"


# line 33 "dase.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/DASE_tables.c,v 1.2 93/02/05 17:01:55 snmp Exp $";
#endif

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_Query_REQDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 0, FL_CONTEXT },
	{ SEQOF_START, OFFSET(struct type_DASE_Query__REQ, name), 16, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct element_DASE_0, IA5String), 22, 0 },
	{ PE_END, OFFSET(struct element_DASE_0, next), 0, 0 },
	{ BOOLEAN, OFFSET(struct type_DASE_Query__REQ, interactive), 1, FL_UNIVERSAL },
	{ SEQOF_START, OFFSET(struct type_DASE_Query__REQ, envlist), 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct element_DASE_1, Environment), _ZEnvironmentDASE, 0 },
	{ PE_END, OFFSET(struct element_DASE_1, next), 0, 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__REQ, context), 22, 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__REQ, userdn), 22, FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__REQ, passwd), 22, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_EnvironmentDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_DASE_Environment, upper), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_DASE_Environment, lower), 2, FL_UNIVERSAL },
	{ SEQOF_START, OFFSET(struct type_DASE_Environment, path), 16, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct element_DASE_2, IA5String), 22, 0 },
	{ PE_END, OFFSET(struct element_DASE_2, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Callback_REQDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 1, FL_CONTEXT },
	{ OCTETSTRING, OFFSET(struct type_DASE_Callback__REQ, string), 22, 0 },
	{ SEQOF_START, OFFSET(struct type_DASE_Callback__REQ, choices), 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct element_DASE_3, Pair), _ZPairDASE, 0 },
	{ PE_END, OFFSET(struct element_DASE_3, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PairDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct type_DASE_Pair, friendly), 22, 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Pair, complete), 22, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Callback_RSPDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 2, FL_CONTEXT },
	{ OCTETSTRING, OFFSET(struct type_DASE_Callback__RSP, IA5String), 22, 0 },
	{ PE_END, OFFSET(struct type_DASE_Callback__RSP, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Query_RSPDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 3, FL_CONTEXT },
	{ ETAG, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__RSP, friendly), 22, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_DASE_Query__RSP, name), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_DASE_Query__RSP, value), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__RSP, diagnostic), 22, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_MessageDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_DASE_Message, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.query__request), _ZQuery_REQDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.callback__request), _ZCallback_REQDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.callback__response), _ZCallback_RSPDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.query__response), _ZQuery_RSPDASE, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Provider_RSPDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_DASE_Provider__RSP, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Provider__RSP, un.callback), _ZCallback_REQDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Provider__RSP, un.answer), _ZQuery_RSPDASE, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Query_REQDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 0, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Query__REQ), 0 },
	{ SEQOF_START, OFFSET(struct type_DASE_Query__REQ, name), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_DASE_0), 0 },
	{ OCTETSTRING, OFFSET(struct element_DASE_0, IA5String), 22, 0 },
	{ PE_END, OFFSET(struct element_DASE_0, next), 0, 0 },
	{ BOOLEAN, OFFSET(struct type_DASE_Query__REQ, interactive), 1, FL_UNIVERSAL },
	{ SEQOF_START, OFFSET(struct type_DASE_Query__REQ, envlist), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_DASE_1), 0 },
	{ OBJECT, OFFSET(struct element_DASE_1, Environment), _ZEnvironmentDASE, 0 },
	{ PE_END, OFFSET(struct element_DASE_1, next), 0, 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__REQ, context), 22, 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__REQ, userdn), 22, FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__REQ, passwd), 22, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_EnvironmentDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Environment), 0 },
	{ INTEGER, OFFSET(struct type_DASE_Environment, upper), 2, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_DASE_Environment, lower), 2, FL_UNIVERSAL },
	{ SEQOF_START, OFFSET(struct type_DASE_Environment, path), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_DASE_2), 0 },
	{ OCTETSTRING, OFFSET(struct element_DASE_2, IA5String), 22, 0 },
	{ PE_END, OFFSET(struct element_DASE_2, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Callback_REQDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 1, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Callback__REQ), 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Callback__REQ, string), 22, 0 },
	{ SEQOF_START, OFFSET(struct type_DASE_Callback__REQ, choices), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_DASE_3), 0 },
	{ OBJECT, OFFSET(struct element_DASE_3, Pair), _ZPairDASE, 0 },
	{ PE_END, OFFSET(struct element_DASE_3, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PairDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Pair), 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Pair, friendly), 22, 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Pair, complete), 22, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Callback_RSPDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 2, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Callback__RSP), 0 },
	{ OCTETSTRING, OFFSET(struct type_DASE_Callback__RSP, IA5String), 22, 0 },
	{ PE_END, OFFSET(struct type_DASE_Callback__RSP, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Query_RSPDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 3, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Query__RSP), 0 },
	{ ETAG, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__RSP, friendly), 22, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_DASE_Query__RSP, name), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_DASE_Query__RSP, value), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DASE_Query__RSP, diagnostic), 22, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_MessageDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Message), 0 },
	{ SCTRL, OFFSET(struct type_DASE_Message, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.query__request), _ZQuery_REQDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.callback__request), _ZCallback_REQDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.callback__response), _ZCallback_RSPDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Message, un.query__response), _ZQuery_RSPDASE, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Provider_RSPDASE[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_DASE_Provider__RSP), 0 },
	{ SCTRL, OFFSET(struct type_DASE_Provider__RSP, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Provider__RSP, un.callback), _ZCallback_REQDASE, 0 },
	{ OBJECT, OFFSET(struct type_DASE_Provider__RSP, un.answer), _ZQuery_RSPDASE, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_Query_REQDASE[] = {
	{ PE_START, 0, 0, 0, "Query-REQ" },
	{ SSEQ_START, -1, 0, FL_CONTEXT, NULL},
	{ SEQOF_START, -1, 16, FL_UNIVERSAL, "name" },
	{ OCTETSTRING, 0, 22, 0, "IA5String" },
	{ PE_END, 0, 0, 0, NULL },
	{ BOOLEAN, -1, 1, FL_UNIVERSAL, "interactive" },
	{ SEQOF_START, -1, 16, FL_UNIVERSAL, "envlist" },
	{ OBJECT, 0, _ZEnvironmentDASE, 0, "Environment" },
	{ PE_END, 0, 0, 0, NULL },
	{ OCTETSTRING, 0, 22, 0, "context" },
	{ OCTETSTRING, 0, 22, FL_OPTIONAL, "userdn" },
	{ OCTETSTRING, 0, 22, FL_OPTIONAL, "passwd" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_EnvironmentDASE[] = {
	{ PE_START, 0, 0, 0, "Environment" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "upper" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "lower" },
	{ SEQOF_START, -1, 16, FL_UNIVERSAL, "path" },
	{ OCTETSTRING, 0, 22, 0, "IA5String" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Callback_REQDASE[] = {
	{ PE_START, 0, 0, 0, "Callback-REQ" },
	{ SSEQ_START, -1, 1, FL_CONTEXT, NULL},
	{ OCTETSTRING, 0, 22, 0, "string" },
	{ SEQOF_START, -1, 16, FL_UNIVERSAL, "choices" },
	{ OBJECT, 0, _ZPairDASE, 0, "Pair" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PairDASE[] = {
	{ PE_START, 0, 0, 0, "Pair" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OCTETSTRING, 0, 22, 0, "friendly" },
	{ OCTETSTRING, 0, 22, 0, "complete" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Callback_RSPDASE[] = {
	{ PE_START, 0, 0, 0, "Callback-RSP" },
	{ SSEQOF_START, -1, 2, FL_CONTEXT, NULL},
	{ OCTETSTRING, 0, 22, 0, "IA5String" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Query_RSPDASE[] = {
	{ PE_START, 0, 0, 0, "Query-RSP" },
	{ SSEQ_START, -1, 3, FL_CONTEXT, NULL},
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "friendly" },
	{ OCTETSTRING, 0, 22, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "name" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "value" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 3, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "diagnostic" },
	{ OCTETSTRING, 0, 22, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_MessageDASE[] = {
	{ PE_START, 0, 0, 0, "Message" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZQuery_REQDASE, 0, "query-request" },
	{ OBJECT, 0, _ZCallback_REQDASE, 0, "callback-request" },
	{ OBJECT, 0, _ZCallback_RSPDASE, 0, "callback-response" },
	{ OBJECT, 0, _ZQuery_RSPDASE, 0, "query-response" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Provider_RSPDASE[] = {
	{ PE_START, 0, 0, 0, "Provider-RSP" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZCallback_REQDASE, 0, "callback" },
	{ OBJECT, 0, _ZQuery_RSPDASE, 0, "answer" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_Query_REQDASE,
	et_EnvironmentDASE,
	et_Callback_REQDASE,
	et_PairDASE,
	et_Callback_RSPDASE,
	et_Query_RSPDASE,
	et_MessageDASE,
	et_Provider_RSPDASE,
	};

static tpe *dtabl[] = {
	dt_Query_REQDASE,
	dt_EnvironmentDASE,
	dt_Callback_REQDASE,
	dt_PairDASE,
	dt_Callback_RSPDASE,
	dt_Query_RSPDASE,
	dt_MessageDASE,
	dt_Provider_RSPDASE,
	};

static ptpe *ptabl[] = {
	pt_Query_REQDASE,
	pt_EnvironmentDASE,
	pt_Callback_REQDASE,
	pt_PairDASE,
	pt_Callback_RSPDASE,
	pt_Query_RSPDASE,
	pt_MessageDASE,
	pt_Provider_RSPDASE,
	};


/* Pointer table 0 entries */
static caddr_t _ZptrtabDASE[] = {
    (caddr_t ) 0,
};

modtyp _ZDASE_mod = {
	"DASE",
	8,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabDASE,
	};


#ifdef	lint

#undef encode_DASE_Query__REQ
int	encode_DASE_Query__REQ(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Query__REQ *parm;
{
  return (enc_f(_ZQuery_REQDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Query__REQ
int	decode_DASE_Query__REQ(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Query__REQ **parm;
{
  return (dec_f(_ZQuery_REQDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Query__REQ
/* ARGSUSED */
int	print_DASE_Query__REQ(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Query__REQ *parm;
{
  return (prnt_f(_ZQuery_REQDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#undef encode_DASE_Environment
int	encode_DASE_Environment(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Environment *parm;
{
  return (enc_f(_ZEnvironmentDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Environment
int	decode_DASE_Environment(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Environment **parm;
{
  return (dec_f(_ZEnvironmentDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Environment
/* ARGSUSED */
int	print_DASE_Environment(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Environment *parm;
{
  return (prnt_f(_ZEnvironmentDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#undef encode_DASE_Callback__REQ
int	encode_DASE_Callback__REQ(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Callback__REQ *parm;
{
  return (enc_f(_ZCallback_REQDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Callback__REQ
int	decode_DASE_Callback__REQ(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Callback__REQ **parm;
{
  return (dec_f(_ZCallback_REQDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Callback__REQ
/* ARGSUSED */
int	print_DASE_Callback__REQ(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Callback__REQ *parm;
{
  return (prnt_f(_ZCallback_REQDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#undef encode_DASE_Pair
int	encode_DASE_Pair(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Pair *parm;
{
  return (enc_f(_ZPairDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Pair
int	decode_DASE_Pair(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Pair **parm;
{
  return (dec_f(_ZPairDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Pair
/* ARGSUSED */
int	print_DASE_Pair(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Pair *parm;
{
  return (prnt_f(_ZPairDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#undef encode_DASE_Callback__RSP
int	encode_DASE_Callback__RSP(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Callback__RSP *parm;
{
  return (enc_f(_ZCallback_RSPDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Callback__RSP
int	decode_DASE_Callback__RSP(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Callback__RSP **parm;
{
  return (dec_f(_ZCallback_RSPDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Callback__RSP
/* ARGSUSED */
int	print_DASE_Callback__RSP(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Callback__RSP *parm;
{
  return (prnt_f(_ZCallback_RSPDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#undef encode_DASE_Query__RSP
int	encode_DASE_Query__RSP(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Query__RSP *parm;
{
  return (enc_f(_ZQuery_RSPDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Query__RSP
int	decode_DASE_Query__RSP(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Query__RSP **parm;
{
  return (dec_f(_ZQuery_RSPDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Query__RSP
/* ARGSUSED */
int	print_DASE_Query__RSP(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Query__RSP *parm;
{
  return (prnt_f(_ZQuery_RSPDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#undef encode_DASE_Message
int	encode_DASE_Message(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Message *parm;
{
  return (enc_f(_ZMessageDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Message
int	decode_DASE_Message(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Message **parm;
{
  return (dec_f(_ZMessageDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Message
/* ARGSUSED */
int	print_DASE_Message(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Message *parm;
{
  return (prnt_f(_ZMessageDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#undef encode_DASE_Provider__RSP
int	encode_DASE_Provider__RSP(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DASE_Provider__RSP *parm;
{
  return (enc_f(_ZProvider_RSPDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DASE_Provider__RSP
int	decode_DASE_Provider__RSP(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Provider__RSP **parm;
{
  return (dec_f(_ZProvider_RSPDASE, &_ZDASE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DASE_Provider__RSP
/* ARGSUSED */
int	print_DASE_Provider__RSP(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DASE_Provider__RSP *parm;
{
  return (prnt_f(_ZProvider_RSPDASE, &_ZDASE_mod, pe, top, len, buffer));
}

#endif	/* lint */
