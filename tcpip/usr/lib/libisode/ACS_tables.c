static char sccsid[] = "@(#)55	1.3  src/tcpip/usr/lib/libisode/ACS_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:32:20";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AOFFSET OFFSET
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ACS_tables.c
 */

#include <stdio.h>
#include "ACS-types.h"


# line 31 "acs.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ACS_tables.c,v 1.2 93/02/06 00:53:09 snmp Exp $";
#endif

static char LAARQ_apdu_protocol_version_0[] = "€";

static char LAARE_apdu_protocol_version_1[] = "€";

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_ACSE_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_ACS_ACSE__apdu, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.aarq), _ZAARQ_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.aare), _ZAARE_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.rlrq), _ZRLRQ_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.rlre), _ZRLRE_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.abrt), _ZABRT_apduACS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AARQ_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 0, FL_APPLICATION },
	{ DFLT_F,	1,	0,	0 },
	{ BITSTRING, OFFSET(struct type_ACS_AARQ__apdu, protocol__version), 0, FL_CONTEXT|FL_DEFAULT },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, application__context__name), _ZApplication_context_nameACS, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AP__title), _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AE__qualifier), _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 4, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AP__invocation__id), _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AE__invocation__id), _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 6, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AP__title), _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 7, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AE__qualifier), _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 8, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AP__invocation__id), _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 9, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AE__invocation__id), _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ IMP_OBJ, 0, 29, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, implementation__information), _ZImplementation_dataACS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AARE_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 1, FL_APPLICATION },
	{ DFLT_F,	1,	1,	0 },
	{ BITSTRING, OFFSET(struct type_ACS_AARE__apdu, protocol__version), 0, FL_CONTEXT|FL_DEFAULT },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, application__context__name), _ZApplication_context_nameACS, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ INTEGER, OFFSET(struct type_ACS_AARE__apdu, result), 2, FL_UNIVERSAL },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, result__source__diagnostic), _ZAssociate_source_diagnosticACS, FL_UNIVERSAL },
	{ ETAG, 0, 4, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AP__title), _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AE__qualifier), _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 6, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AP__invocation__id), _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 7, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AE__invocation__id), _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ IMP_OBJ, 0, 29, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, implementation__information), _ZImplementation_dataACS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RLRQ_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 2, FL_APPLICATION },
	{ OPTL, OFFSET(struct type_ACS_RLRQ__apdu, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_RLRQ__apdu, reason), 0, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_RLRQ__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RLRE_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 3, FL_APPLICATION },
	{ OPTL, OFFSET(struct type_ACS_RLRE__apdu, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_RLRE__apdu, reason), 0, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_RLRE__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ABRT_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 4, FL_APPLICATION },
	{ INTEGER, OFFSET(struct type_ACS_ABRT__apdu, abort__source), 0, FL_CONTEXT },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_ABRT__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ABRT_sourceACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_ABRT__source, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Application_context_nameACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AP_titleACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AE_qualifierACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AE_titleACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ACS_AE__title, title), _ZAP_titleACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_AE__title, qualifier), _ZAE_qualifierACS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AE_invocation_idACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_AE__invocation__id, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AP_invocation_idACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_AP__invocation__id, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Associate_resultACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_Associate__result, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Associate_source_diagnosticACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_ACS_Associate__source__diagnostic, offset), 0, 0 },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ INTEGER, OFFSET(struct type_ACS_Associate__source__diagnostic, un.acse__service__user), 2, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ INTEGER, OFFSET(struct type_ACS_Associate__source__diagnostic, un.acse__service__provider), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Association_informationACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ EXTOBJ, OFFSET(struct type_ACS_Association__information, EXTERNAL), _ZEXTERNALUNIV, 0 },
	{ EXTMOD, 2, 0, 0 },
	{ PE_END, OFFSET(struct type_ACS_Association__information, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Implementation_dataACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 25, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Release_request_reasonACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_Release__request__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Release_response_reasonACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_Release__response__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ACSE_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_ACSE__apdu), 0 },
	{ SCTRL, OFFSET(struct type_ACS_ACSE__apdu, offset), 0, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.aarq), _ZAARQ_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.aare), _ZAARE_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.rlrq), _ZRLRQ_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.rlre), _ZRLRE_apduACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_ACSE__apdu, un.abrt), _ZABRT_apduACS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AARQ_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 0, FL_APPLICATION },
	{ MEMALLOC, 0, sizeof (struct type_ACS_AARQ__apdu), 0 },
	{ BITSTRING, OFFSET(struct type_ACS_AARQ__apdu, protocol__version), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	1,	0,	0 },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, application__context__name), _ZApplication_context_nameACS, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AP__title), _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AE__qualifier), _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 4, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AP__invocation__id), _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, called__AE__invocation__id), _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 6, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AP__title), _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 7, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AE__qualifier), _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 8, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AP__invocation__id), _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 9, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, calling__AE__invocation__id), _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ IMP_OBJ, 0, 29, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, implementation__information), _ZImplementation_dataACS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARQ__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AARE_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 1, FL_APPLICATION },
	{ MEMALLOC, 0, sizeof (struct type_ACS_AARE__apdu), 0 },
	{ BITSTRING, OFFSET(struct type_ACS_AARE__apdu, protocol__version), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	1,	1,	0 },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, application__context__name), _ZApplication_context_nameACS, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ INTEGER, OFFSET(struct type_ACS_AARE__apdu, result), 2, FL_UNIVERSAL },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, result__source__diagnostic), _ZAssociate_source_diagnosticACS, FL_UNIVERSAL },
	{ ETAG, 0, 4, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AP__title), _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AE__qualifier), _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 6, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AP__invocation__id), _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ ETAG, 0, 7, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, responding__AE__invocation__id), _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL },
	{ IMP_OBJ, 0, 29, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, implementation__information), _ZImplementation_dataACS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_AARE__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RLRQ_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 2, FL_APPLICATION },
	{ MEMALLOC, 0, sizeof (struct type_ACS_RLRQ__apdu), 0 },
	{ OPTL, OFFSET(struct type_ACS_RLRQ__apdu, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_RLRQ__apdu, reason), 0, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_RLRQ__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RLRE_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 3, FL_APPLICATION },
	{ MEMALLOC, 0, sizeof (struct type_ACS_RLRE__apdu), 0 },
	{ OPTL, OFFSET(struct type_ACS_RLRE__apdu, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct type_ACS_RLRE__apdu, reason), 0, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_RLRE__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ABRT_apduACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 4, FL_APPLICATION },
	{ MEMALLOC, 0, sizeof (struct type_ACS_ABRT__apdu), 0 },
	{ INTEGER, OFFSET(struct type_ACS_ABRT__apdu, abort__source), 0, FL_CONTEXT },
	{ IMP_OBJ, 0, 30, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ACS_ABRT__apdu, user__information), _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ABRT_sourceACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_ABRT__source), 0 },
	{ INTEGER, OFFSET(struct type_ACS_ABRT__source, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Application_context_nameACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AP_titleACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AE_qualifierACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SANY, 0, -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AE_titleACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ACS_AE__title), 0 },
	{ OBJECT, OFFSET(struct type_ACS_AE__title, title), _ZAP_titleACS, 0 },
	{ OBJECT, OFFSET(struct type_ACS_AE__title, qualifier), _ZAE_qualifierACS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AE_invocation_idACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_AE__invocation__id), 0 },
	{ INTEGER, OFFSET(struct type_ACS_AE__invocation__id, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AP_invocation_idACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_AP__invocation__id), 0 },
	{ INTEGER, OFFSET(struct type_ACS_AP__invocation__id, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Associate_resultACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_Associate__result), 0 },
	{ INTEGER, OFFSET(struct type_ACS_Associate__result, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Associate_source_diagnosticACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_Associate__source__diagnostic), 0 },
	{ SCTRL, OFFSET(struct type_ACS_Associate__source__diagnostic, offset), 0, 0 },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ INTEGER, OFFSET(struct type_ACS_Associate__source__diagnostic, un.acse__service__user), 2, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ INTEGER, OFFSET(struct type_ACS_Associate__source__diagnostic, un.acse__service__provider), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Association_informationACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ACS_Association__information), 0 },
	{ EXTOBJ, OFFSET(struct type_ACS_Association__information, EXTERNAL), _ZEXTERNALUNIV, 0 },
	{ EXTMOD, 2, 0, 0 },
	{ PE_END, OFFSET(struct type_ACS_Association__information, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Implementation_dataACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 25, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Release_request_reasonACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_Release__request__reason), 0 },
	{ INTEGER, OFFSET(struct type_ACS_Release__request__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Release_response_reasonACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ACS_Release__response__reason), 0 },
	{ INTEGER, OFFSET(struct type_ACS_Release__response__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_ACSE_apduACS[] = {
	{ PE_START, 0, 0, 0, "ACSE-apdu" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZAARQ_apduACS, 0, "aarq" },
	{ OBJECT, 0, _ZAARE_apduACS, 0, "aare" },
	{ OBJECT, 0, _ZRLRQ_apduACS, 0, "rlrq" },
	{ OBJECT, 0, _ZRLRE_apduACS, 0, "rlre" },
	{ OBJECT, 0, _ZABRT_apduACS, 0, "abrt" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AARQ_apduACS[] = {
	{ PE_START, 0, 0, 0, "AARQ-apdu" },
	{ SSEQ_START, -1, 0, FL_APPLICATION, NULL},
	{ BITSTRING, 3, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "protocol-version" },
	{ DFLT_B,	1,	0,	0 },
	{ ETAG, -1, 1, FL_CONTEXT, "application-context-name" },
	{ OBJECT, 0, _ZApplication_context_nameACS, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "called-AP-title" },
	{ OBJECT, 0, _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 3, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "called-AE-qualifier" },
	{ OBJECT, 0, _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 4, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "called-AP-invocation-id" },
	{ OBJECT, 0, _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 5, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "called-AE-invocation-id" },
	{ OBJECT, 0, _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 6, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "calling-AP-title" },
	{ OBJECT, 0, _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 7, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "calling-AE-qualifier" },
	{ OBJECT, 0, _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 8, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "calling-AP-invocation-id" },
	{ OBJECT, 0, _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 9, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "calling-AE-invocation-id" },
	{ OBJECT, 0, _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ IMP_OBJ, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "implementation-information" },
	{ OBJECT, 0, _ZImplementation_dataACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "implementation-information" },
	{ IMP_OBJ, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ OBJECT, 0, _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AARE_apduACS[] = {
	{ PE_START, 0, 0, 0, "AARE-apdu" },
	{ SSEQ_START, -1, 1, FL_APPLICATION, NULL},
	{ BITSTRING, 3, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "protocol-version" },
	{ DFLT_B,	1,	1,	0 },
	{ ETAG, -1, 1, FL_CONTEXT, "application-context-name" },
	{ OBJECT, 0, _ZApplication_context_nameACS, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 2, FL_CONTEXT, "result" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 3, FL_CONTEXT, "result-source-diagnostic" },
	{ OBJECT, 0, _ZAssociate_source_diagnosticACS, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 4, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding-AP-title" },
	{ OBJECT, 0, _ZAP_titleACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 5, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding-AE-qualifier" },
	{ OBJECT, 0, _ZAE_qualifierACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 6, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding-AP-invocation-id" },
	{ OBJECT, 0, _ZAP_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ ETAG, -1, 7, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding-AE-invocation-id" },
	{ OBJECT, 0, _ZAE_invocation_idACS, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ IMP_OBJ, -1, 29, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "implementation-information" },
	{ OBJECT, 0, _ZImplementation_dataACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "implementation-information" },
	{ IMP_OBJ, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ OBJECT, 0, _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RLRQ_apduACS[] = {
	{ PE_START, 0, 0, 0, "RLRQ-apdu" },
	{ SSEQ_START, -1, 2, FL_APPLICATION, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "reason" },
	{ IMP_OBJ, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ OBJECT, 0, _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RLRE_apduACS[] = {
	{ PE_START, 0, 0, 0, "RLRE-apdu" },
	{ SSEQ_START, -1, 3, FL_APPLICATION, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "reason" },
	{ IMP_OBJ, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ OBJECT, 0, _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ABRT_apduACS[] = {
	{ PE_START, 0, 0, 0, "ABRT-apdu" },
	{ SSEQ_START, -1, 4, FL_APPLICATION, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT, "abort-source" },
	{ IMP_OBJ, -1, 30, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ OBJECT, 0, _ZAssociation_informationACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "user-information" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ABRT_sourceACS[] = {
	{ PE_START, 0, 0, 0, "ABRT-source" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Application_context_nameACS[] = {
	{ PE_START, 0, 0, 0, "Application-context-name" },
	{ SOBJID, -1, 6, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AP_titleACS[] = {
	{ PE_START, 0, 0, 0, "AP-title" },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AE_qualifierACS[] = {
	{ PE_START, 0, 0, 0, "AE-qualifier" },
	{ SANY, -1, -1, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AE_titleACS[] = {
	{ PE_START, 0, 0, 0, "AE-title" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZAP_titleACS, 0, "title" },
	{ OBJECT, 0, _ZAE_qualifierACS, 0, "qualifier" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AE_invocation_idACS[] = {
	{ PE_START, 0, 0, 0, "AE-invocation-id" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AP_invocation_idACS[] = {
	{ PE_START, 0, 0, 0, "AP-invocation-id" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Associate_resultACS[] = {
	{ PE_START, 0, 0, 0, "Associate-result" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Associate_source_diagnosticACS[] = {
	{ PE_START, 0, 0, 0, "Associate-source-diagnostic" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ ETAG, -1, 1, FL_CONTEXT, "acse-service-user" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 2, FL_CONTEXT, "acse-service-provider" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Association_informationACS[] = {
	{ PE_START, 0, 0, 0, "Association-information" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ EXTOBJ, 0, _ZEXTERNALUNIV, 0, "EXTERNAL" },
	{ EXTMOD, 2, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Implementation_dataACS[] = {
	{ PE_START, 0, 0, 0, "Implementation-data" },
	{ SOCTETSTRING, 0, 25, 0, "GraphicString" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Release_request_reasonACS[] = {
	{ PE_START, 0, 0, 0, "Release-request-reason" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Release_response_reasonACS[] = {
	{ PE_START, 0, 0, 0, "Release-response-reason" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_ACSE_apduACS,
	et_AARQ_apduACS,
	et_AARE_apduACS,
	et_RLRQ_apduACS,
	et_RLRE_apduACS,
	et_ABRT_apduACS,
	et_ABRT_sourceACS,
	et_Application_context_nameACS,
	et_AP_titleACS,
	et_AE_qualifierACS,
	et_AE_titleACS,
	et_AE_invocation_idACS,
	et_AP_invocation_idACS,
	et_Associate_resultACS,
	et_Associate_source_diagnosticACS,
	et_Association_informationACS,
	et_Implementation_dataACS,
	et_Release_request_reasonACS,
	et_Release_response_reasonACS,
	};

static tpe *dtabl[] = {
	dt_ACSE_apduACS,
	dt_AARQ_apduACS,
	dt_AARE_apduACS,
	dt_RLRQ_apduACS,
	dt_RLRE_apduACS,
	dt_ABRT_apduACS,
	dt_ABRT_sourceACS,
	dt_Application_context_nameACS,
	dt_AP_titleACS,
	dt_AE_qualifierACS,
	dt_AE_titleACS,
	dt_AE_invocation_idACS,
	dt_AP_invocation_idACS,
	dt_Associate_resultACS,
	dt_Associate_source_diagnosticACS,
	dt_Association_informationACS,
	dt_Implementation_dataACS,
	dt_Release_request_reasonACS,
	dt_Release_response_reasonACS,
	};

static ptpe *ptabl[] = {
	pt_ACSE_apduACS,
	pt_AARQ_apduACS,
	pt_AARE_apduACS,
	pt_RLRQ_apduACS,
	pt_RLRE_apduACS,
	pt_ABRT_apduACS,
	pt_ABRT_sourceACS,
	pt_Application_context_nameACS,
	pt_AP_titleACS,
	pt_AE_qualifierACS,
	pt_AE_titleACS,
	pt_AE_invocation_idACS,
	pt_AP_invocation_idACS,
	pt_Associate_resultACS,
	pt_Associate_source_diagnosticACS,
	pt_Association_informationACS,
	pt_Implementation_dataACS,
	pt_Release_request_reasonACS,
	pt_Release_response_reasonACS,
	};


/* Pointer table 4 entries */
static caddr_t _ZptrtabACS[] = {
    (caddr_t ) LAARQ_apdu_protocol_version_0,
    (caddr_t ) LAARE_apdu_protocol_version_1,
    (caddr_t ) &_ZUNIV_mod,
    (caddr_t ) bits_ACS_protocol__version,
};

modtyp _ZACS_mod = {
	"ACS",
	19,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabACS,
	};


#ifdef	lint

#undef encode_ACS_ACSE__apdu
int	encode_ACS_ACSE__apdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_ACSE__apdu *parm;
{
  return (enc_f(_ZACSE_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_ACSE__apdu
int	decode_ACS_ACSE__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_ACSE__apdu **parm;
{
  return (dec_f(_ZACSE_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_ACSE__apdu
/* ARGSUSED */
int	print_ACS_ACSE__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_ACSE__apdu *parm;
{
  return (prnt_f(_ZACSE_apduACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_AARQ__apdu
int	encode_ACS_AARQ__apdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_AARQ__apdu *parm;
{
  return (enc_f(_ZAARQ_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_AARQ__apdu
int	decode_ACS_AARQ__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AARQ__apdu **parm;
{
  return (dec_f(_ZAARQ_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_AARQ__apdu
/* ARGSUSED */
int	print_ACS_AARQ__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AARQ__apdu *parm;
{
  return (prnt_f(_ZAARQ_apduACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_AARE__apdu
int	encode_ACS_AARE__apdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_AARE__apdu *parm;
{
  return (enc_f(_ZAARE_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_AARE__apdu
int	decode_ACS_AARE__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AARE__apdu **parm;
{
  return (dec_f(_ZAARE_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_AARE__apdu
/* ARGSUSED */
int	print_ACS_AARE__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AARE__apdu *parm;
{
  return (prnt_f(_ZAARE_apduACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_RLRQ__apdu
int	encode_ACS_RLRQ__apdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_RLRQ__apdu *parm;
{
  return (enc_f(_ZRLRQ_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_RLRQ__apdu
int	decode_ACS_RLRQ__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_RLRQ__apdu **parm;
{
  return (dec_f(_ZRLRQ_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_RLRQ__apdu
/* ARGSUSED */
int	print_ACS_RLRQ__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_RLRQ__apdu *parm;
{
  return (prnt_f(_ZRLRQ_apduACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_RLRE__apdu
int	encode_ACS_RLRE__apdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_RLRE__apdu *parm;
{
  return (enc_f(_ZRLRE_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_RLRE__apdu
int	decode_ACS_RLRE__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_RLRE__apdu **parm;
{
  return (dec_f(_ZRLRE_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_RLRE__apdu
/* ARGSUSED */
int	print_ACS_RLRE__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_RLRE__apdu *parm;
{
  return (prnt_f(_ZRLRE_apduACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_ABRT__apdu
int	encode_ACS_ABRT__apdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_ABRT__apdu *parm;
{
  return (enc_f(_ZABRT_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_ABRT__apdu
int	decode_ACS_ABRT__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_ABRT__apdu **parm;
{
  return (dec_f(_ZABRT_apduACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_ABRT__apdu
/* ARGSUSED */
int	print_ACS_ABRT__apdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_ABRT__apdu *parm;
{
  return (prnt_f(_ZABRT_apduACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_ABRT__source
int	encode_ACS_ABRT__source(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_ABRT__source *parm;
{
  return (enc_f(_ZABRT_sourceACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_ABRT__source
int	decode_ACS_ABRT__source(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_ABRT__source **parm;
{
  return (dec_f(_ZABRT_sourceACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_ABRT__source
/* ARGSUSED */
int	print_ACS_ABRT__source(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_ABRT__source *parm;
{
  return (prnt_f(_ZABRT_sourceACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_Application__context__name
int	encode_ACS_Application__context__name(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_Application__context__name *parm;
{
  return (enc_f(_ZApplication_context_nameACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_Application__context__name
int	decode_ACS_Application__context__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Application__context__name **parm;
{
  return (dec_f(_ZApplication_context_nameACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_Application__context__name
/* ARGSUSED */
int	print_ACS_Application__context__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Application__context__name *parm;
{
  return (prnt_f(_ZApplication_context_nameACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_AP__title
int	encode_ACS_AP__title(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_AP__title *parm;
{
  return (enc_f(_ZAP_titleACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_AP__title
int	decode_ACS_AP__title(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AP__title **parm;
{
  return (dec_f(_ZAP_titleACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_AP__title
/* ARGSUSED */
int	print_ACS_AP__title(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AP__title *parm;
{
  return (prnt_f(_ZAP_titleACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_AE__qualifier
int	encode_ACS_AE__qualifier(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_AE__qualifier *parm;
{
  return (enc_f(_ZAE_qualifierACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_AE__qualifier
int	decode_ACS_AE__qualifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AE__qualifier **parm;
{
  return (dec_f(_ZAE_qualifierACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_AE__qualifier
/* ARGSUSED */
int	print_ACS_AE__qualifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AE__qualifier *parm;
{
  return (prnt_f(_ZAE_qualifierACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_AE__title
int	encode_ACS_AE__title(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_AE__title *parm;
{
  return (enc_f(_ZAE_titleACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_AE__title
int	decode_ACS_AE__title(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AE__title **parm;
{
  return (dec_f(_ZAE_titleACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_AE__title
/* ARGSUSED */
int	print_ACS_AE__title(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AE__title *parm;
{
  return (prnt_f(_ZAE_titleACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_AE__invocation__id
int	encode_ACS_AE__invocation__id(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_AE__invocation__id *parm;
{
  return (enc_f(_ZAE_invocation_idACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_AE__invocation__id
int	decode_ACS_AE__invocation__id(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AE__invocation__id **parm;
{
  return (dec_f(_ZAE_invocation_idACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_AE__invocation__id
/* ARGSUSED */
int	print_ACS_AE__invocation__id(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AE__invocation__id *parm;
{
  return (prnt_f(_ZAE_invocation_idACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_AP__invocation__id
int	encode_ACS_AP__invocation__id(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_AP__invocation__id *parm;
{
  return (enc_f(_ZAP_invocation_idACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_AP__invocation__id
int	decode_ACS_AP__invocation__id(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AP__invocation__id **parm;
{
  return (dec_f(_ZAP_invocation_idACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_AP__invocation__id
/* ARGSUSED */
int	print_ACS_AP__invocation__id(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_AP__invocation__id *parm;
{
  return (prnt_f(_ZAP_invocation_idACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_Associate__result
int	encode_ACS_Associate__result(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_Associate__result *parm;
{
  return (enc_f(_ZAssociate_resultACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_Associate__result
int	decode_ACS_Associate__result(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Associate__result **parm;
{
  return (dec_f(_ZAssociate_resultACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_Associate__result
/* ARGSUSED */
int	print_ACS_Associate__result(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Associate__result *parm;
{
  return (prnt_f(_ZAssociate_resultACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_Associate__source__diagnostic
int	encode_ACS_Associate__source__diagnostic(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_Associate__source__diagnostic *parm;
{
  return (enc_f(_ZAssociate_source_diagnosticACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_Associate__source__diagnostic
int	decode_ACS_Associate__source__diagnostic(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Associate__source__diagnostic **parm;
{
  return (dec_f(_ZAssociate_source_diagnosticACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_Associate__source__diagnostic
/* ARGSUSED */
int	print_ACS_Associate__source__diagnostic(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Associate__source__diagnostic *parm;
{
  return (prnt_f(_ZAssociate_source_diagnosticACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_Association__information
int	encode_ACS_Association__information(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_Association__information *parm;
{
  return (enc_f(_ZAssociation_informationACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_Association__information
int	decode_ACS_Association__information(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Association__information **parm;
{
  return (dec_f(_ZAssociation_informationACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_Association__information
/* ARGSUSED */
int	print_ACS_Association__information(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Association__information *parm;
{
  return (prnt_f(_ZAssociation_informationACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_Implementation__data
int	encode_ACS_Implementation__data(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_Implementation__data *parm;
{
  return (enc_f(_ZImplementation_dataACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_Implementation__data
int	decode_ACS_Implementation__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Implementation__data **parm;
{
  return (dec_f(_ZImplementation_dataACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_Implementation__data
/* ARGSUSED */
int	print_ACS_Implementation__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Implementation__data *parm;
{
  return (prnt_f(_ZImplementation_dataACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_Release__request__reason
int	encode_ACS_Release__request__reason(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_Release__request__reason *parm;
{
  return (enc_f(_ZRelease_request_reasonACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_Release__request__reason
int	decode_ACS_Release__request__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Release__request__reason **parm;
{
  return (dec_f(_ZRelease_request_reasonACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_Release__request__reason
/* ARGSUSED */
int	print_ACS_Release__request__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Release__request__reason *parm;
{
  return (prnt_f(_ZRelease_request_reasonACS, &_ZACS_mod, pe, top, len, buffer));
}

#undef encode_ACS_Release__response__reason
int	encode_ACS_Release__response__reason(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ACS_Release__response__reason *parm;
{
  return (enc_f(_ZRelease_response_reasonACS, &_ZACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ACS_Release__response__reason
int	decode_ACS_Release__response__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Release__response__reason **parm;
{
  return (dec_f(_ZRelease_response_reasonACS, &_ZACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ACS_Release__response__reason
/* ARGSUSED */
int	print_ACS_Release__response__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ACS_Release__response__reason *parm;
{
  return (prnt_f(_ZRelease_response_reasonACS, &_ZACS_mod, pe, top, len, buffer));
}

#endif	/* lint */
