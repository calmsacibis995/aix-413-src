static char sccsid[] = "@(#)63	1.3  src/tcpip/usr/lib/libisode/DSE_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:33:32";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/DES_tables.c
 */

#include <stdio.h>
#include "DSE-types.h"


# line 36 "dse.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/DSE_tables.c,v 1.2 93/02/05 17:01:59 snmp Exp $";
#endif

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_PSAPaddrDSE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ ETAG, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DSE_PSAPaddr, pSelector), 4, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DSE_PSAPaddr, sSelector), 4, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DSE_PSAPaddr, tSelector), 4, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ SETOF_START, OFFSET(struct type_DSE_PSAPaddr, nAddress), 17, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct member_DSE_0, member_DSE_1), 4, FL_UNIVERSAL },
	{ PE_END, OFFSET(struct member_DSE_0, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PSAPaddrDSE[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_DSE_PSAPaddr), 0 },
	{ ETAG, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DSE_PSAPaddr, pSelector), 4, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DSE_PSAPaddr, sSelector), 4, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ OCTETSTRING, OFFSET(struct type_DSE_PSAPaddr, tSelector), 4, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ SETOF_START, OFFSET(struct type_DSE_PSAPaddr, nAddress), 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct member_DSE_0), 0 },
	{ OCTETSTRING, OFFSET(struct member_DSE_0, member_DSE_1), 4, FL_UNIVERSAL },
	{ PE_END, OFFSET(struct member_DSE_0, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_PSAPaddrDSE[] = {
	{ PE_START, 0, 0, 0, "PSAPaddr" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "pSelector" },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "sSelector" },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "tSelector" },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 3, FL_CONTEXT, "nAddress" },
	{ SETOF_START, -1, 17, FL_UNIVERSAL, NULL},
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL, NULL},
	{ PE_END, OFFSET(struct member_DSE_0, next), 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_PSAPaddrDSE,
	};

static tpe *dtabl[] = {
	dt_PSAPaddrDSE,
	};

static ptpe *ptabl[] = {
	pt_PSAPaddrDSE,
	};


/* Pointer table 0 entries */
static caddr_t _ZptrtabDSE[] = {
    (caddr_t ) 0,
};

modtyp _ZDSE_mod = {
	"DSE",
	1,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabDSE,
	};


#ifdef	lint

#undef encode_DSE_PSAPaddr
int	encode_DSE_PSAPaddr(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_DSE_PSAPaddr *parm;
{
  return (enc_f(_ZPSAPaddrDSE, &_ZDSE_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_DSE_PSAPaddr
int	decode_DSE_PSAPaddr(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DSE_PSAPaddr **parm;
{
  return (dec_f(_ZPSAPaddrDSE, &_ZDSE_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_DSE_PSAPaddr
/* ARGSUSED */
int	print_DSE_PSAPaddr(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_DSE_PSAPaddr *parm;
{
  return (prnt_f(_ZPSAPaddrDSE, &_ZDSE_mod, pe, top, len, buffer));
}

#endif	/* lint */
