static char sccsid[] = "@(#)86	1.3  src/tcpip/usr/lib/libisode/RONOT_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:35:31";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AOFFSET OFFSET
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/RONOT_tables.c
 */

#include <stdio.h>
#include "RONOT-types.h"


#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_BindArgumentValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 16, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_BindResultValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 17, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_BindErrorValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 18, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_UnBindArgumentValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 19, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_UnBindResultValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 20, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_UnBindErrorValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 21, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_BindArgumentValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 16, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_BindResultValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 17, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_BindErrorValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 18, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_UnBindArgumentValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 19, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_UnBindResultValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 20, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_UnBindErrorValueRONOT[] = {
	{ PE_START, 0, 0, 0 },
	{ ETAG, 0, 21, FL_CONTEXT },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_BindArgumentValueRONOT[] = {
	{ PE_START, 0, 0, 0, "BindArgumentValue" },
	{ ETAG, -1, 16, FL_CONTEXT, NULL },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_BindResultValueRONOT[] = {
	{ PE_START, 0, 0, 0, "BindResultValue" },
	{ ETAG, -1, 17, FL_CONTEXT, NULL },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_BindErrorValueRONOT[] = {
	{ PE_START, 0, 0, 0, "BindErrorValue" },
	{ ETAG, -1, 18, FL_CONTEXT, NULL },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_UnBindArgumentValueRONOT[] = {
	{ PE_START, 0, 0, 0, "UnBindArgumentValue" },
	{ ETAG, -1, 19, FL_CONTEXT, NULL },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_UnBindResultValueRONOT[] = {
	{ PE_START, 0, 0, 0, "UnBindResultValue" },
	{ ETAG, -1, 20, FL_CONTEXT, NULL },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_UnBindErrorValueRONOT[] = {
	{ PE_START, 0, 0, 0, "UnBindErrorValue" },
	{ ETAG, -1, 21, FL_CONTEXT, NULL },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_BindArgumentValueRONOT,
	et_BindResultValueRONOT,
	et_BindErrorValueRONOT,
	et_UnBindArgumentValueRONOT,
	et_UnBindResultValueRONOT,
	et_UnBindErrorValueRONOT,
	};

static tpe *dtabl[] = {
	dt_BindArgumentValueRONOT,
	dt_BindResultValueRONOT,
	dt_BindErrorValueRONOT,
	dt_UnBindArgumentValueRONOT,
	dt_UnBindResultValueRONOT,
	dt_UnBindErrorValueRONOT,
	};

static ptpe *ptabl[] = {
	pt_BindArgumentValueRONOT,
	pt_BindResultValueRONOT,
	pt_BindErrorValueRONOT,
	pt_UnBindArgumentValueRONOT,
	pt_UnBindResultValueRONOT,
	pt_UnBindErrorValueRONOT,
	};


/* Pointer table 0 entries */
static caddr_t _ZptrtabRONOT[] = {
    (caddr_t ) 0,
};

modtyp _ZRONOT_mod = {
	"RONOT",
	6,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabRONOT,
	};


#ifdef	lint

#undef encode_RONOT_BindArgumentValue
int	encode_RONOT_BindArgumentValue(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RONOT_BindArgumentValue *parm;
{
  return (enc_f(_ZBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RONOT_BindArgumentValue
int	decode_RONOT_BindArgumentValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_BindArgumentValue **parm;
{
  return (dec_f(_ZBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RONOT_BindArgumentValue
/* ARGSUSED */
int	print_RONOT_BindArgumentValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_BindArgumentValue *parm;
{
  return (prnt_f(_ZBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer));
}

#undef encode_RONOT_BindResultValue
int	encode_RONOT_BindResultValue(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RONOT_BindResultValue *parm;
{
  return (enc_f(_ZBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RONOT_BindResultValue
int	decode_RONOT_BindResultValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_BindResultValue **parm;
{
  return (dec_f(_ZBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RONOT_BindResultValue
/* ARGSUSED */
int	print_RONOT_BindResultValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_BindResultValue *parm;
{
  return (prnt_f(_ZBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer));
}

#undef encode_RONOT_BindErrorValue
int	encode_RONOT_BindErrorValue(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RONOT_BindErrorValue *parm;
{
  return (enc_f(_ZBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RONOT_BindErrorValue
int	decode_RONOT_BindErrorValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_BindErrorValue **parm;
{
  return (dec_f(_ZBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RONOT_BindErrorValue
/* ARGSUSED */
int	print_RONOT_BindErrorValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_BindErrorValue *parm;
{
  return (prnt_f(_ZBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer));
}

#undef encode_RONOT_UnBindArgumentValue
int	encode_RONOT_UnBindArgumentValue(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RONOT_UnBindArgumentValue *parm;
{
  return (enc_f(_ZUnBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RONOT_UnBindArgumentValue
int	decode_RONOT_UnBindArgumentValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_UnBindArgumentValue **parm;
{
  return (dec_f(_ZUnBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RONOT_UnBindArgumentValue
/* ARGSUSED */
int	print_RONOT_UnBindArgumentValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_UnBindArgumentValue *parm;
{
  return (prnt_f(_ZUnBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer));
}

#undef encode_RONOT_UnBindResultValue
int	encode_RONOT_UnBindResultValue(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RONOT_UnBindResultValue *parm;
{
  return (enc_f(_ZUnBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RONOT_UnBindResultValue
int	decode_RONOT_UnBindResultValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_UnBindResultValue **parm;
{
  return (dec_f(_ZUnBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RONOT_UnBindResultValue
/* ARGSUSED */
int	print_RONOT_UnBindResultValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_UnBindResultValue *parm;
{
  return (prnt_f(_ZUnBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer));
}

#undef encode_RONOT_UnBindErrorValue
int	encode_RONOT_UnBindErrorValue(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RONOT_UnBindErrorValue *parm;
{
  return (enc_f(_ZUnBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RONOT_UnBindErrorValue
int	decode_RONOT_UnBindErrorValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_UnBindErrorValue **parm;
{
  return (dec_f(_ZUnBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RONOT_UnBindErrorValue
/* ARGSUSED */
int	print_RONOT_UnBindErrorValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RONOT_UnBindErrorValue *parm;
{
  return (prnt_f(_ZUnBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer));
}

#endif	/* lint */
