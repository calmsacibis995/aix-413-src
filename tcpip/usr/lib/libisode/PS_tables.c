static char sccsid[] = "@(#)69	1.3  src/tcpip/usr/lib/libisode/PS_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:34:53";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AOFFSET OFFSET
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/PS_tables.c
 */

#include <stdio.h>
#include "PS-types.h"


# line 32 "ps.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/PS_tables.c,v 1.2 93/02/05 17:02:15 snmp Exp $";
#endif

static char LCP_type_version_0[] = {
 0x80,};

static char LCPA_type_version_1[] = {
 0x80,};

static char LCPR_type_version_2[] = {
 0x80,};

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_CP_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_CP__type, mode), _ZMode_selectorPS, FL_CONTEXT },
	{ ANY, OFFSET(struct type_PS_CP__type, x410__mode), 1, FL_CONTEXT|FL_OPTIONAL },
	{ SEQ_START, OFFSET(struct type_PS_CP__type, normal__mode), 2, FL_CONTEXT|FL_OPTIONAL },
	{ DFLT_F,	1,	0,	0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_DEFAULT },
	{ OBJECT, OFFSET(struct element_PS_0, version), _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, calling), _ZCalling_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, called), _ZCalled_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 4, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, context__list), _ZDefinition_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 6, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, default__context), _ZContext_namePS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 8, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, presentation__fu), _ZPresentation_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 9, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, session__fu), _ZUser_session_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CPC_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZUser_dataPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CPA_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_CPA__type, mode), _ZMode_selectorPS, FL_CONTEXT },
	{ ANY, OFFSET(struct type_PS_CPA__type, x410__mode), 1, FL_CONTEXT|FL_OPTIONAL },
	{ SEQ_START, OFFSET(struct type_PS_CPA__type, normal__mode), 2, FL_CONTEXT|FL_OPTIONAL },
	{ DFLT_F,	1,	1,	0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_DEFAULT },
	{ OBJECT, OFFSET(struct element_PS_1, version), _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT },
	{ IMP_OBJ, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, responding), _ZResponding_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, context__list), _ZDefinition_result_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 8, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, presentation__fu), _ZPresentation_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 9, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, session__fu), _ZUser_session_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CPR_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_PS_CPR__type, offset), 0, 0 },
	{ ANY, OFFSET(struct type_PS_CPR__type, un.x410__mode), 17, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_CPR__type, un.normal__mode), 16, FL_UNIVERSAL },
	{ OPTL, OFFSET(struct element_PS_2, optionals), 0, 0 },
	{ DFLT_F,	1,	2,	0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_DEFAULT },
	{ OBJECT, OFFSET(struct element_PS_2, version), _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT },
	{ IMP_OBJ, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, responding), _ZResponding_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, context__list), _ZDefinition_result_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 7, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, default__context), _ZContext_resultPS, FL_CONTEXT|FL_OPTIONAL },
	{ INTEGER, OFFSET(struct element_PS_2, reason), 10, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Abort_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_PS_Abort__type, offset), 0, 0 },
	{ ANY, OFFSET(struct type_PS_Abort__type, un.x410__mode), 17, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_Abort__type, un.normal__mode), 0, FL_CONTEXT },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_3, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_3, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ OBJECT, OFFSET(struct type_PS_Abort__type, un.provider__abort), _ZARP_PPDUPS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ARU_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_PS_ARU__PPDU, offset), 0, 0 },
	{ ANY, OFFSET(struct type_PS_ARU__PPDU, un.x410__mode), 17, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_ARU__PPDU, un.normal__mode), 0, FL_CONTEXT },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_4, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_4, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ARP_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ARP__PPDU, provider__reason), _ZAbort_reasonPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ARP__PPDU, event), _ZEvent_identifierPS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Typed_data_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_PS_Typed__data__type, offset), 0, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.acPPDU), _ZAC_PPDUPS, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.acaPPDU), _ZACA_PPDUPS, FL_CONTEXT },
	{ IMP_OBJ, 0, 0, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.simple), _ZSimply_encoded_dataPS, FL_APPLICATION },
	{ IMP_OBJ, 0, 1, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.complex), _ZFully_encoded_dataPS, FL_APPLICATION },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AC_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_AC__PPDU, additions), _ZAddition_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_AC__PPDU, deletions), _ZDeletion_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_AC__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ACA_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ACA__PPDU, additions), _ZAddition_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ACA__PPDU, deletions), _ZDeletion_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ACA__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RS_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RS__PPDU, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RS__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RSA_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RSA__PPDU, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RSA__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Abort_reasonPS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_PS_Abort__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Abstract_syntax_namePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Called_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSelectorPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Calling_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSelectorPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Context_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_Context__list, element_PS_5), 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct element_PS_6, identifier), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct element_PS_6, abstract__syntax), _ZAbstract_syntax_namePS, 0 },
	{ SEQOF_START, OFFSET(struct element_PS_6, transfer__syntax__list), 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct element_PS_7, Transfer__syntax__name), _ZTransfer_syntax_namePS, 0 },
	{ PE_END, OFFSET(struct element_PS_7, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, OFFSET(struct type_PS_Context__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Context_namePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Context__name, abstract__syntax), _ZAbstract_syntax_namePS, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Context__name, transfer__syntax), _ZTransfer_syntax_namePS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Context_resultPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZResultPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Event_identifierPS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_PS_Event__identifier, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Mode_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_PS_Mode__selector, parm), 0, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Addition_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZContext_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Addition_result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZResult_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Definition_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZContext_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Definition_result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZResult_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Deletion_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_PS_Deletion__list, element_PS_8), 2, FL_UNIVERSAL },
	{ PE_END, OFFSET(struct type_PS_Deletion__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Deletion_result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_PS_Deletion__result__list, element_PS_9), 2, FL_UNIVERSAL },
	{ PE_END, OFFSET(struct type_PS_Deletion__result__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_IdentifierPS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_PS_Identifier, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Identifier_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_Identifier__list, element_PS_10), 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct element_PS_11, identifier), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct element_PS_11, transfer__syntax), _ZTransfer_syntax_namePS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, OFFSET(struct type_PS_Identifier__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Presentation_requirementsPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SBITSTRING, 0, 3, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SelectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Protocol_versionPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SBITSTRING, 0, 3, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Provider_reasonPS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_PS_Provider__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Responding_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSelectorPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ResultPS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_PS_Result, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_Result__list, element_PS_12), 16, FL_UNIVERSAL },
	{ OPTL, OFFSET(struct element_PS_13, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct element_PS_13, result), 0, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_13, transfer__syntax), _ZTransfer_syntax_namePS, FL_CONTEXT|FL_OPTIONAL },
	{ INTEGER, OFFSET(struct element_PS_13, provider__reason), 2, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, OFFSET(struct type_PS_Result__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Transfer_syntax_namePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_User_dataPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_PS_User__data, offset), 0, 0 },
	{ IMP_OBJ, 0, 0, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_User__data, un.simple), _ZSimply_encoded_dataPS, FL_APPLICATION },
	{ IMP_OBJ, 0, 1, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_User__data, un.complex), _ZFully_encoded_dataPS, FL_APPLICATION },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Simply_encoded_dataPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_Fully_encoded_dataPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_PS_Fully__encoded__data, PDV__list), _ZPDV_listPS, 0 },
	{ PE_END, OFFSET(struct type_PS_Fully__encoded__data, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PDV_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_PS_PDV__list, transfer__syntax), _ZTransfer_syntax_namePS, FL_OPTIONAL },
	{ INTEGER, OFFSET(struct type_PS_PDV__list, identifier), 2, FL_UNIVERSAL },
	{ CHOICE_START, OFFSET(struct type_PS_PDV__list, presentation__data__values), 0, 0 },
	{ SCTRL, OFFSET(struct choice_PS_0, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct choice_PS_0, un.single__ASN1__type), -1, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct choice_PS_0, un.octet__aligned), 1, FL_CONTEXT },
	{ BITSTRING, OFFSET(struct choice_PS_0, un.arbitrary), 2, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_User_session_requirementsPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SBITSTRING, 0, 3, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CP_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_CP__type), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_CP__type, mode), _ZMode_selectorPS, FL_CONTEXT },
	{ ANY, OFFSET(struct type_PS_CP__type, x410__mode), 1, FL_CONTEXT|FL_OPTIONAL },
	{ SEQ_START, OFFSET(struct type_PS_CP__type, normal__mode), 2, FL_CONTEXT|FL_OPTIONAL },
	{ MEMALLOC, 0, sizeof (struct element_PS_0), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_DEFAULT },
	{ OBJECT, OFFSET(struct element_PS_0, version), _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	1,	0,	0 },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, calling), _ZCalling_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, called), _ZCalled_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 4, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, context__list), _ZDefinition_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 6, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, default__context), _ZContext_namePS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 8, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, presentation__fu), _ZPresentation_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 9, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, session__fu), _ZUser_session_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_0, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CPC_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZUser_dataPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CPA_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_CPA__type), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_CPA__type, mode), _ZMode_selectorPS, FL_CONTEXT },
	{ ANY, OFFSET(struct type_PS_CPA__type, x410__mode), 1, FL_CONTEXT|FL_OPTIONAL },
	{ SEQ_START, OFFSET(struct type_PS_CPA__type, normal__mode), 2, FL_CONTEXT|FL_OPTIONAL },
	{ MEMALLOC, 0, sizeof (struct element_PS_1), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_DEFAULT },
	{ OBJECT, OFFSET(struct element_PS_1, version), _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	1,	1,	0 },
	{ IMP_OBJ, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, responding), _ZResponding_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, context__list), _ZDefinition_result_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 8, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, presentation__fu), _ZPresentation_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 9, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, session__fu), _ZUser_session_requirementsPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_1, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CPR_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_CPR__type), 0 },
	{ SCTRL, OFFSET(struct type_PS_CPR__type, offset), 0, 0 },
	{ ANY, OFFSET(struct type_PS_CPR__type, un.x410__mode), 17, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_CPR__type, un.normal__mode), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_PS_2), 0 },
	{ OPTL, OFFSET(struct element_PS_2, optionals), 0, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_DEFAULT },
	{ OBJECT, OFFSET(struct element_PS_2, version), _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	1,	2,	0 },
	{ IMP_OBJ, 0, 3, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, responding), _ZResponding_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 5, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, context__list), _ZDefinition_result_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 7, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, default__context), _ZContext_resultPS, FL_CONTEXT|FL_OPTIONAL },
	{ INTEGER, OFFSET(struct element_PS_2, reason), 10, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_2, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Abort_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_Abort__type), 0 },
	{ SCTRL, OFFSET(struct type_PS_Abort__type, offset), 0, 0 },
	{ ANY, OFFSET(struct type_PS_Abort__type, un.x410__mode), 17, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_Abort__type, un.normal__mode), 0, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct element_PS_3), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_3, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_3, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ OBJECT, OFFSET(struct type_PS_Abort__type, un.provider__abort), _ZARP_PPDUPS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ARU_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_ARU__PPDU), 0 },
	{ SCTRL, OFFSET(struct type_PS_ARU__PPDU, offset), 0, 0 },
	{ ANY, OFFSET(struct type_PS_ARU__PPDU, un.x410__mode), 17, FL_UNIVERSAL },
	{ SEQ_START, OFFSET(struct type_PS_ARU__PPDU, un.normal__mode), 0, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct element_PS_4), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_4, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_4, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ARP_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_ARP__PPDU), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ARP__PPDU, provider__reason), _ZAbort_reasonPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ARP__PPDU, event), _ZEvent_identifierPS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Typed_data_typePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_Typed__data__type), 0 },
	{ SCTRL, OFFSET(struct type_PS_Typed__data__type, offset), 0, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.acPPDU), _ZAC_PPDUPS, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.acaPPDU), _ZACA_PPDUPS, FL_CONTEXT },
	{ IMP_OBJ, 0, 0, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.simple), _ZSimply_encoded_dataPS, FL_APPLICATION },
	{ IMP_OBJ, 0, 1, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_Typed__data__type, un.complex), _ZFully_encoded_dataPS, FL_APPLICATION },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AC_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_AC__PPDU), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_AC__PPDU, additions), _ZAddition_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_AC__PPDU, deletions), _ZDeletion_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_AC__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ACA_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_ACA__PPDU), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ACA__PPDU, additions), _ZAddition_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ACA__PPDU, deletions), _ZDeletion_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_ACA__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RS_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_RS__PPDU), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RS__PPDU, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RS__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RSA_PPDUPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_RSA__PPDU), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RSA__PPDU, context__list), _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_PS_RSA__PPDU, user__data), _ZUser_dataPS, FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Abort_reasonPS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_Abort__reason), 0 },
	{ INTEGER, OFFSET(struct type_PS_Abort__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Abstract_syntax_namePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Called_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSelectorPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Calling_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSelectorPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Context_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Context__list), 0 },
	{ SEQ_START, OFFSET(struct type_PS_Context__list, element_PS_5), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_PS_6), 0 },
	{ INTEGER, OFFSET(struct element_PS_6, identifier), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct element_PS_6, abstract__syntax), _ZAbstract_syntax_namePS, 0 },
	{ SEQOF_START, OFFSET(struct element_PS_6, transfer__syntax__list), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_PS_7), 0 },
	{ OBJECT, OFFSET(struct element_PS_7, Transfer__syntax__name), _ZTransfer_syntax_namePS, 0 },
	{ PE_END, OFFSET(struct element_PS_7, next), 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, OFFSET(struct type_PS_Context__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Context_namePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Context__name), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Context__name, abstract__syntax), _ZAbstract_syntax_namePS, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_PS_Context__name, transfer__syntax), _ZTransfer_syntax_namePS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Context_resultPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZResultPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Event_identifierPS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_Event__identifier), 0 },
	{ INTEGER, OFFSET(struct type_PS_Event__identifier, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Mode_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Mode__selector), 0 },
	{ INTEGER, OFFSET(struct type_PS_Mode__selector, parm), 0, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Addition_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZContext_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Addition_result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZResult_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Definition_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZContext_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Definition_result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZResult_listPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Deletion_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Deletion__list), 0 },
	{ INTEGER, OFFSET(struct type_PS_Deletion__list, element_PS_8), 2, FL_UNIVERSAL },
	{ PE_END, OFFSET(struct type_PS_Deletion__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Deletion_result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Deletion__result__list), 0 },
	{ INTEGER, OFFSET(struct type_PS_Deletion__result__list, element_PS_9), 2, FL_UNIVERSAL },
	{ PE_END, OFFSET(struct type_PS_Deletion__result__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_IdentifierPS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_Identifier), 0 },
	{ INTEGER, OFFSET(struct type_PS_Identifier, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Identifier_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Identifier__list), 0 },
	{ SEQ_START, OFFSET(struct type_PS_Identifier__list, element_PS_10), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_PS_11), 0 },
	{ INTEGER, OFFSET(struct element_PS_11, identifier), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct element_PS_11, transfer__syntax), _ZTransfer_syntax_namePS, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, OFFSET(struct type_PS_Identifier__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Presentation_requirementsPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SBITSTRING, 0, 3, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SelectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Protocol_versionPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SBITSTRING, 0, 3, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Provider_reasonPS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_Provider__reason), 0 },
	{ INTEGER, OFFSET(struct type_PS_Provider__reason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Responding_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSelectorPS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ResultPS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_Result), 0 },
	{ INTEGER, OFFSET(struct type_PS_Result, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Result_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Result__list), 0 },
	{ SEQ_START, OFFSET(struct type_PS_Result__list, element_PS_12), 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct element_PS_13), 0 },
	{ OPTL, OFFSET(struct element_PS_13, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct element_PS_13, result), 0, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_PS_13, transfer__syntax), _ZTransfer_syntax_namePS, FL_CONTEXT|FL_OPTIONAL },
	{ INTEGER, OFFSET(struct element_PS_13, provider__reason), 2, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, OFFSET(struct type_PS_Result__list, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Transfer_syntax_namePS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJID, 0, 6, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_User_dataPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_PS_User__data), 0 },
	{ SCTRL, OFFSET(struct type_PS_User__data, offset), 0, 0 },
	{ IMP_OBJ, 0, 0, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_User__data, un.simple), _ZSimply_encoded_dataPS, FL_APPLICATION },
	{ IMP_OBJ, 0, 1, FL_APPLICATION },
	{ OBJECT, OFFSET(struct type_PS_User__data, un.complex), _ZFully_encoded_dataPS, FL_APPLICATION },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Simply_encoded_dataPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_Fully_encoded_dataPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_Fully__encoded__data), 0 },
	{ OBJECT, OFFSET(struct type_PS_Fully__encoded__data, PDV__list), _ZPDV_listPS, 0 },
	{ PE_END, OFFSET(struct type_PS_Fully__encoded__data, next), 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PDV_listPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_PS_PDV__list), 0 },
	{ OBJECT, OFFSET(struct type_PS_PDV__list, transfer__syntax), _ZTransfer_syntax_namePS, FL_OPTIONAL },
	{ INTEGER, OFFSET(struct type_PS_PDV__list, identifier), 2, FL_UNIVERSAL },
	{ CHOICE_START, OFFSET(struct type_PS_PDV__list, presentation__data__values), 0, 0 },
	{ MEMALLOC, 0, sizeof (struct choice_PS_0), 0 },
	{ SCTRL, OFFSET(struct choice_PS_0, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct choice_PS_0, un.single__ASN1__type), -1, FL_UNIVERSAL },
	{ OCTETSTRING, OFFSET(struct choice_PS_0, un.octet__aligned), 1, FL_CONTEXT },
	{ BITSTRING, OFFSET(struct choice_PS_0, un.arbitrary), 2, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_User_session_requirementsPS[] = {
	{ PE_START, 0, 0, 0 },
	{ SBITSTRING, 0, 3, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_CP_typePS[] = {
	{ PE_START, 0, 0, 0, "CP-type" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "mode" },
	{ OBJECT, 0, _ZMode_selectorPS, FL_CONTEXT, "mode" },
	{ ANY, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "x410-mode" },
	{ SEQ_START, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "normal-mode" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "version" },
	{ OBJECT, 0, _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "version" },
	{ DFLT_B,	1,	0,	0 },
	{ IMP_OBJ, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "calling" },
	{ OBJECT, 0, _ZCalling_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "calling" },
	{ IMP_OBJ, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "called" },
	{ OBJECT, 0, _ZCalled_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "called" },
	{ IMP_OBJ, -1, 4, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZDefinition_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ IMP_OBJ, -1, 6, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "default-context" },
	{ OBJECT, 0, _ZContext_namePS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "default-context" },
	{ IMP_OBJ, -1, 8, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "presentation-fu" },
	{ OBJECT, 0, _ZPresentation_requirementsPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "presentation-fu" },
	{ IMP_OBJ, -1, 9, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "session-fu" },
	{ OBJECT, 0, _ZUser_session_requirementsPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "session-fu" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CPC_typePS[] = {
	{ PE_START, 0, 0, 0, "CPC-type" },
	{ SOBJECT, 0, _ZUser_dataPS, 0, "User-data" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CPA_typePS[] = {
	{ PE_START, 0, 0, 0, "CPA-type" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "mode" },
	{ OBJECT, 0, _ZMode_selectorPS, FL_CONTEXT, "mode" },
	{ ANY, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "x410-mode" },
	{ SEQ_START, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "normal-mode" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "version" },
	{ OBJECT, 0, _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "version" },
	{ DFLT_B,	1,	1,	0 },
	{ IMP_OBJ, -1, 3, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding" },
	{ OBJECT, 0, _ZResponding_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding" },
	{ IMP_OBJ, -1, 5, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZDefinition_result_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ IMP_OBJ, -1, 8, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "presentation-fu" },
	{ OBJECT, 0, _ZPresentation_requirementsPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "presentation-fu" },
	{ IMP_OBJ, -1, 9, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "session-fu" },
	{ OBJECT, 0, _ZUser_session_requirementsPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "session-fu" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CPR_typePS[] = {
	{ PE_START, 0, 0, 0, "CPR-type" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ ANY, -1, 17, FL_UNIVERSAL, "x410-mode" },
	{ SEQ_START, -1, 16, FL_UNIVERSAL, "normal-mode" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "version" },
	{ OBJECT, 0, _ZProtocol_versionPS, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "version" },
	{ DFLT_B,	1,	2,	0 },
	{ IMP_OBJ, -1, 3, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding" },
	{ OBJECT, 0, _ZResponding_presentation_selectorPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "responding" },
	{ IMP_OBJ, -1, 5, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZDefinition_result_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ IMP_OBJ, -1, 7, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "default-context" },
	{ OBJECT, 0, _ZContext_resultPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "default-context" },
	{ INTEGER, -1, 10, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "reason" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Abort_typePS[] = {
	{ PE_START, 0, 0, 0, "Abort-type" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ ANY, -1, 17, FL_UNIVERSAL, "x410-mode" },
	{ SEQ_START, -1, 0, FL_CONTEXT, "normal-mode" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ OBJECT, 0, _ZARP_PPDUPS, 0, "provider-abort" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ARU_PPDUPS[] = {
	{ PE_START, 0, 0, 0, "ARU-PPDU" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ ANY, -1, 17, FL_UNIVERSAL, "x410-mode" },
	{ SEQ_START, -1, 0, FL_CONTEXT, "normal-mode" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ARP_PPDUPS[] = {
	{ PE_START, 0, 0, 0, "ARP-PPDU" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "provider-reason" },
	{ OBJECT, 0, _ZAbort_reasonPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "provider-reason" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "event" },
	{ OBJECT, 0, _ZEvent_identifierPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "event" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Typed_data_typePS[] = {
	{ PE_START, 0, 0, 0, "Typed-data-type" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "acPPDU" },
	{ OBJECT, 0, _ZAC_PPDUPS, FL_CONTEXT, "acPPDU" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT, "acaPPDU" },
	{ OBJECT, 0, _ZACA_PPDUPS, FL_CONTEXT, "acaPPDU" },
	{ IMP_OBJ, -1, 0, FL_APPLICATION, "simple" },
	{ OBJECT, 0, _ZSimply_encoded_dataPS, FL_APPLICATION, "simple" },
	{ IMP_OBJ, -1, 1, FL_APPLICATION, "complex" },
	{ OBJECT, 0, _ZFully_encoded_dataPS, FL_APPLICATION, "complex" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AC_PPDUPS[] = {
	{ PE_START, 0, 0, 0, "AC-PPDU" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "additions" },
	{ OBJECT, 0, _ZAddition_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "additions" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "deletions" },
	{ OBJECT, 0, _ZDeletion_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "deletions" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ACA_PPDUPS[] = {
	{ PE_START, 0, 0, 0, "ACA-PPDU" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "additions" },
	{ OBJECT, 0, _ZAddition_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "additions" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "deletions" },
	{ OBJECT, 0, _ZDeletion_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "deletions" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RS_PPDUPS[] = {
	{ PE_START, 0, 0, 0, "RS-PPDU" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RSA_PPDUPS[] = {
	{ PE_START, 0, 0, 0, "RSA-PPDU" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZIdentifier_listPS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "context-list" },
	{ OBJECT, 0, _ZUser_dataPS, FL_OPTIONAL, "user-data" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Abort_reasonPS[] = {
	{ PE_START, 0, 0, 0, "Abort-reason" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Abstract_syntax_namePS[] = {
	{ PE_START, 0, 0, 0, "Abstract-syntax-name" },
	{ SOBJID, -1, 6, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Called_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0, "Called-presentation-selector" },
	{ SOBJECT, 0, _ZSelectorPS, 0, "Selector" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Calling_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0, "Calling-presentation-selector" },
	{ SOBJECT, 0, _ZSelectorPS, 0, "Selector" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Context_listPS[] = {
	{ PE_START, 0, 0, 0, "Context-list" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ SEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "identifier" },
	{ OBJECT, 0, _ZAbstract_syntax_namePS, 0, "abstract-syntax" },
	{ SEQOF_START, -1, 16, FL_UNIVERSAL, "transfer-syntax-list" },
	{ OBJECT, 0, _ZTransfer_syntax_namePS, 0, "Transfer-syntax-name" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Context_namePS[] = {
	{ PE_START, 0, 0, 0, "Context-name" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "abstract-syntax" },
	{ OBJECT, 0, _ZAbstract_syntax_namePS, FL_CONTEXT, "abstract-syntax" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT, "transfer-syntax" },
	{ OBJECT, 0, _ZTransfer_syntax_namePS, FL_CONTEXT, "transfer-syntax" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Context_resultPS[] = {
	{ PE_START, 0, 0, 0, "Context-result" },
	{ SOBJECT, 0, _ZResultPS, 0, "Result" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Event_identifierPS[] = {
	{ PE_START, 0, 0, 0, "Event-identifier" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Mode_selectorPS[] = {
	{ PE_START, 0, 0, 0, "Mode-selector" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Addition_listPS[] = {
	{ PE_START, 0, 0, 0, "Addition-list" },
	{ SOBJECT, 0, _ZContext_listPS, 0, "Context-list" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Addition_result_listPS[] = {
	{ PE_START, 0, 0, 0, "Addition-result-list" },
	{ SOBJECT, 0, _ZResult_listPS, 0, "Result-list" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Definition_listPS[] = {
	{ PE_START, 0, 0, 0, "Definition-list" },
	{ SOBJECT, 0, _ZContext_listPS, 0, "Context-list" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Definition_result_listPS[] = {
	{ PE_START, 0, 0, 0, "Definition-result-list" },
	{ SOBJECT, 0, _ZResult_listPS, 0, "Result-list" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Deletion_listPS[] = {
	{ PE_START, 0, 0, 0, "Deletion-list" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Deletion_result_listPS[] = {
	{ PE_START, 0, 0, 0, "Deletion-result-list" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_IdentifierPS[] = {
	{ PE_START, 0, 0, 0, "Identifier" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Identifier_listPS[] = {
	{ PE_START, 0, 0, 0, "Identifier-list" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ SEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "identifier" },
	{ OBJECT, 0, _ZTransfer_syntax_namePS, 0, "transfer-syntax" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Presentation_requirementsPS[] = {
	{ PE_START, 0, 0, 0, "Presentation-requirements" },
	{ SBITSTRING, 3, 3, FL_UNIVERSAL, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SelectorPS[] = {
	{ PE_START, 0, 0, 0, "Selector" },
	{ SOCTETSTRING, -1, 4, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Protocol_versionPS[] = {
	{ PE_START, 0, 0, 0, "Protocol-version" },
	{ SBITSTRING, 4, 3, FL_UNIVERSAL, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Provider_reasonPS[] = {
	{ PE_START, 0, 0, 0, "Provider-reason" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Responding_presentation_selectorPS[] = {
	{ PE_START, 0, 0, 0, "Responding-presentation-selector" },
	{ SOBJECT, 0, _ZSelectorPS, 0, "Selector" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ResultPS[] = {
	{ PE_START, 0, 0, 0, "Result" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Result_listPS[] = {
	{ PE_START, 0, 0, 0, "Result-list" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ SEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT, "result" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "transfer-syntax" },
	{ OBJECT, 0, _ZTransfer_syntax_namePS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "transfer-syntax" },
	{ INTEGER, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "provider-reason" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Transfer_syntax_namePS[] = {
	{ PE_START, 0, 0, 0, "Transfer-syntax-name" },
	{ SOBJID, -1, 6, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_User_dataPS[] = {
	{ PE_START, 0, 0, 0, "User-data" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ IMP_OBJ, -1, 0, FL_APPLICATION, "simple" },
	{ OBJECT, 0, _ZSimply_encoded_dataPS, FL_APPLICATION, "simple" },
	{ IMP_OBJ, -1, 1, FL_APPLICATION, "complex" },
	{ OBJECT, 0, _ZFully_encoded_dataPS, FL_APPLICATION, "complex" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Simply_encoded_dataPS[] = {
	{ PE_START, 0, 0, 0, "Simply-encoded-data" },
	{ SOCTETSTRING, -1, 4, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_Fully_encoded_dataPS[] = {
	{ PE_START, 0, 0, 0, "Fully-encoded-data" },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZPDV_listPS, 0, "PDV-list" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PDV_listPS[] = {
	{ PE_START, 0, 0, 0, "PDV-list" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZTransfer_syntax_namePS, FL_OPTIONAL, "transfer-syntax" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, "identifier" },
	{ CHOICE_START, -1, 0, 0, "presentation-data-values" },
	{ SCTRL, 0, 0, 0, "presentation-data-values" },
	{ ETAG, -1, 0, FL_CONTEXT, "single-ASN1-type" },
	{ ANY, -1, -1, FL_UNIVERSAL, NULL},
	{ OCTETSTRING, -1, 1, FL_CONTEXT, "octet-aligned" },
	{ BITSTRING, -1, 2, FL_CONTEXT, "arbitrary" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_User_session_requirementsPS[] = {
	{ PE_START, 0, 0, 0, "User-session-requirements" },
	{ SBITSTRING, 5, 3, FL_UNIVERSAL, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_CP_typePS,
	et_CPC_typePS,
	et_CPA_typePS,
	et_CPR_typePS,
	et_Abort_typePS,
	et_ARU_PPDUPS,
	et_ARP_PPDUPS,
	et_Typed_data_typePS,
	et_AC_PPDUPS,
	et_ACA_PPDUPS,
	et_RS_PPDUPS,
	et_RSA_PPDUPS,
	et_Abort_reasonPS,
	et_Abstract_syntax_namePS,
	et_Called_presentation_selectorPS,
	et_Calling_presentation_selectorPS,
	et_Context_listPS,
	et_Context_namePS,
	et_Context_resultPS,
	et_Event_identifierPS,
	et_Mode_selectorPS,
	et_Addition_listPS,
	et_Addition_result_listPS,
	et_Definition_listPS,
	et_Definition_result_listPS,
	et_Deletion_listPS,
	et_Deletion_result_listPS,
	et_IdentifierPS,
	et_Identifier_listPS,
	et_Presentation_requirementsPS,
	et_SelectorPS,
	et_Protocol_versionPS,
	et_Provider_reasonPS,
	et_Responding_presentation_selectorPS,
	et_ResultPS,
	et_Result_listPS,
	et_Transfer_syntax_namePS,
	et_User_dataPS,
	et_Simply_encoded_dataPS,
	et_Fully_encoded_dataPS,
	et_PDV_listPS,
	et_User_session_requirementsPS,
	};

static tpe *dtabl[] = {
	dt_CP_typePS,
	dt_CPC_typePS,
	dt_CPA_typePS,
	dt_CPR_typePS,
	dt_Abort_typePS,
	dt_ARU_PPDUPS,
	dt_ARP_PPDUPS,
	dt_Typed_data_typePS,
	dt_AC_PPDUPS,
	dt_ACA_PPDUPS,
	dt_RS_PPDUPS,
	dt_RSA_PPDUPS,
	dt_Abort_reasonPS,
	dt_Abstract_syntax_namePS,
	dt_Called_presentation_selectorPS,
	dt_Calling_presentation_selectorPS,
	dt_Context_listPS,
	dt_Context_namePS,
	dt_Context_resultPS,
	dt_Event_identifierPS,
	dt_Mode_selectorPS,
	dt_Addition_listPS,
	dt_Addition_result_listPS,
	dt_Definition_listPS,
	dt_Definition_result_listPS,
	dt_Deletion_listPS,
	dt_Deletion_result_listPS,
	dt_IdentifierPS,
	dt_Identifier_listPS,
	dt_Presentation_requirementsPS,
	dt_SelectorPS,
	dt_Protocol_versionPS,
	dt_Provider_reasonPS,
	dt_Responding_presentation_selectorPS,
	dt_ResultPS,
	dt_Result_listPS,
	dt_Transfer_syntax_namePS,
	dt_User_dataPS,
	dt_Simply_encoded_dataPS,
	dt_Fully_encoded_dataPS,
	dt_PDV_listPS,
	dt_User_session_requirementsPS,
	};

static ptpe *ptabl[] = {
	pt_CP_typePS,
	pt_CPC_typePS,
	pt_CPA_typePS,
	pt_CPR_typePS,
	pt_Abort_typePS,
	pt_ARU_PPDUPS,
	pt_ARP_PPDUPS,
	pt_Typed_data_typePS,
	pt_AC_PPDUPS,
	pt_ACA_PPDUPS,
	pt_RS_PPDUPS,
	pt_RSA_PPDUPS,
	pt_Abort_reasonPS,
	pt_Abstract_syntax_namePS,
	pt_Called_presentation_selectorPS,
	pt_Calling_presentation_selectorPS,
	pt_Context_listPS,
	pt_Context_namePS,
	pt_Context_resultPS,
	pt_Event_identifierPS,
	pt_Mode_selectorPS,
	pt_Addition_listPS,
	pt_Addition_result_listPS,
	pt_Definition_listPS,
	pt_Definition_result_listPS,
	pt_Deletion_listPS,
	pt_Deletion_result_listPS,
	pt_IdentifierPS,
	pt_Identifier_listPS,
	pt_Presentation_requirementsPS,
	pt_SelectorPS,
	pt_Protocol_versionPS,
	pt_Provider_reasonPS,
	pt_Responding_presentation_selectorPS,
	pt_ResultPS,
	pt_Result_listPS,
	pt_Transfer_syntax_namePS,
	pt_User_dataPS,
	pt_Simply_encoded_dataPS,
	pt_Fully_encoded_dataPS,
	pt_PDV_listPS,
	pt_User_session_requirementsPS,
	};


/* Pointer table 6 entries */
static caddr_t _ZptrtabPS[] = {
    (caddr_t ) LCP_type_version_0,
    (caddr_t ) LCPA_type_version_1,
    (caddr_t ) LCPR_type_version_2,
    (caddr_t ) bits_PS_Presentation__requirements,
    (caddr_t ) bits_PS_Protocol__version,
    (caddr_t ) bits_PS_User__session__requirements,
};

modtyp _ZPS_mod = {
	"PS",
	42,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabPS,
	};


#ifdef	lint

#undef encode_PS_CP__type
int	encode_PS_CP__type(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_CP__type *parm;
{
  return (enc_f(_ZCP_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_CP__type
int	decode_PS_CP__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CP__type **parm;
{
  return (dec_f(_ZCP_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_CP__type
/* ARGSUSED */
int	print_PS_CP__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CP__type *parm;
{
  return (prnt_f(_ZCP_typePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_CPC__type
int	encode_PS_CPC__type(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_CPC__type *parm;
{
  return (enc_f(_ZCPC_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_CPC__type
int	decode_PS_CPC__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CPC__type **parm;
{
  return (dec_f(_ZCPC_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_CPC__type
/* ARGSUSED */
int	print_PS_CPC__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CPC__type *parm;
{
  return (prnt_f(_ZCPC_typePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_CPA__type
int	encode_PS_CPA__type(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_CPA__type *parm;
{
  return (enc_f(_ZCPA_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_CPA__type
int	decode_PS_CPA__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CPA__type **parm;
{
  return (dec_f(_ZCPA_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_CPA__type
/* ARGSUSED */
int	print_PS_CPA__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CPA__type *parm;
{
  return (prnt_f(_ZCPA_typePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_CPR__type
int	encode_PS_CPR__type(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_CPR__type *parm;
{
  return (enc_f(_ZCPR_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_CPR__type
int	decode_PS_CPR__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CPR__type **parm;
{
  return (dec_f(_ZCPR_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_CPR__type
/* ARGSUSED */
int	print_PS_CPR__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_CPR__type *parm;
{
  return (prnt_f(_ZCPR_typePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Abort__type
int	encode_PS_Abort__type(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Abort__type *parm;
{
  return (enc_f(_ZAbort_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Abort__type
int	decode_PS_Abort__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Abort__type **parm;
{
  return (dec_f(_ZAbort_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Abort__type
/* ARGSUSED */
int	print_PS_Abort__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Abort__type *parm;
{
  return (prnt_f(_ZAbort_typePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_ARU__PPDU
int	encode_PS_ARU__PPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_ARU__PPDU *parm;
{
  return (enc_f(_ZARU_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_ARU__PPDU
int	decode_PS_ARU__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_ARU__PPDU **parm;
{
  return (dec_f(_ZARU_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_ARU__PPDU
/* ARGSUSED */
int	print_PS_ARU__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_ARU__PPDU *parm;
{
  return (prnt_f(_ZARU_PPDUPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_ARP__PPDU
int	encode_PS_ARP__PPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_ARP__PPDU *parm;
{
  return (enc_f(_ZARP_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_ARP__PPDU
int	decode_PS_ARP__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_ARP__PPDU **parm;
{
  return (dec_f(_ZARP_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_ARP__PPDU
/* ARGSUSED */
int	print_PS_ARP__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_ARP__PPDU *parm;
{
  return (prnt_f(_ZARP_PPDUPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Typed__data__type
int	encode_PS_Typed__data__type(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Typed__data__type *parm;
{
  return (enc_f(_ZTyped_data_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Typed__data__type
int	decode_PS_Typed__data__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Typed__data__type **parm;
{
  return (dec_f(_ZTyped_data_typePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Typed__data__type
/* ARGSUSED */
int	print_PS_Typed__data__type(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Typed__data__type *parm;
{
  return (prnt_f(_ZTyped_data_typePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_AC__PPDU
int	encode_PS_AC__PPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_AC__PPDU *parm;
{
  return (enc_f(_ZAC_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_AC__PPDU
int	decode_PS_AC__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_AC__PPDU **parm;
{
  return (dec_f(_ZAC_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_AC__PPDU
/* ARGSUSED */
int	print_PS_AC__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_AC__PPDU *parm;
{
  return (prnt_f(_ZAC_PPDUPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_ACA__PPDU
int	encode_PS_ACA__PPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_ACA__PPDU *parm;
{
  return (enc_f(_ZACA_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_ACA__PPDU
int	decode_PS_ACA__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_ACA__PPDU **parm;
{
  return (dec_f(_ZACA_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_ACA__PPDU
/* ARGSUSED */
int	print_PS_ACA__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_ACA__PPDU *parm;
{
  return (prnt_f(_ZACA_PPDUPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_RS__PPDU
int	encode_PS_RS__PPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_RS__PPDU *parm;
{
  return (enc_f(_ZRS_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_RS__PPDU
int	decode_PS_RS__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_RS__PPDU **parm;
{
  return (dec_f(_ZRS_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_RS__PPDU
/* ARGSUSED */
int	print_PS_RS__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_RS__PPDU *parm;
{
  return (prnt_f(_ZRS_PPDUPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_RSA__PPDU
int	encode_PS_RSA__PPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_RSA__PPDU *parm;
{
  return (enc_f(_ZRSA_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_RSA__PPDU
int	decode_PS_RSA__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_RSA__PPDU **parm;
{
  return (dec_f(_ZRSA_PPDUPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_RSA__PPDU
/* ARGSUSED */
int	print_PS_RSA__PPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_RSA__PPDU *parm;
{
  return (prnt_f(_ZRSA_PPDUPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Abort__reason
int	encode_PS_Abort__reason(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Abort__reason *parm;
{
  return (enc_f(_ZAbort_reasonPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Abort__reason
int	decode_PS_Abort__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Abort__reason **parm;
{
  return (dec_f(_ZAbort_reasonPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Abort__reason
/* ARGSUSED */
int	print_PS_Abort__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Abort__reason *parm;
{
  return (prnt_f(_ZAbort_reasonPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Abstract__syntax__name
int	encode_PS_Abstract__syntax__name(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Abstract__syntax__name *parm;
{
  return (enc_f(_ZAbstract_syntax_namePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Abstract__syntax__name
int	decode_PS_Abstract__syntax__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Abstract__syntax__name **parm;
{
  return (dec_f(_ZAbstract_syntax_namePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Abstract__syntax__name
/* ARGSUSED */
int	print_PS_Abstract__syntax__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Abstract__syntax__name *parm;
{
  return (prnt_f(_ZAbstract_syntax_namePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Called__presentation__selector
int	encode_PS_Called__presentation__selector(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Called__presentation__selector *parm;
{
  return (enc_f(_ZCalled_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Called__presentation__selector
int	decode_PS_Called__presentation__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Called__presentation__selector **parm;
{
  return (dec_f(_ZCalled_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Called__presentation__selector
/* ARGSUSED */
int	print_PS_Called__presentation__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Called__presentation__selector *parm;
{
  return (prnt_f(_ZCalled_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Calling__presentation__selector
int	encode_PS_Calling__presentation__selector(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Calling__presentation__selector *parm;
{
  return (enc_f(_ZCalling_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Calling__presentation__selector
int	decode_PS_Calling__presentation__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Calling__presentation__selector **parm;
{
  return (dec_f(_ZCalling_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Calling__presentation__selector
/* ARGSUSED */
int	print_PS_Calling__presentation__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Calling__presentation__selector *parm;
{
  return (prnt_f(_ZCalling_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Context__list
int	encode_PS_Context__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Context__list *parm;
{
  return (enc_f(_ZContext_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Context__list
int	decode_PS_Context__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Context__list **parm;
{
  return (dec_f(_ZContext_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Context__list
/* ARGSUSED */
int	print_PS_Context__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Context__list *parm;
{
  return (prnt_f(_ZContext_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Context__name
int	encode_PS_Context__name(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Context__name *parm;
{
  return (enc_f(_ZContext_namePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Context__name
int	decode_PS_Context__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Context__name **parm;
{
  return (dec_f(_ZContext_namePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Context__name
/* ARGSUSED */
int	print_PS_Context__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Context__name *parm;
{
  return (prnt_f(_ZContext_namePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Context__result
int	encode_PS_Context__result(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Context__result *parm;
{
  return (enc_f(_ZContext_resultPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Context__result
int	decode_PS_Context__result(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Context__result **parm;
{
  return (dec_f(_ZContext_resultPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Context__result
/* ARGSUSED */
int	print_PS_Context__result(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Context__result *parm;
{
  return (prnt_f(_ZContext_resultPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Event__identifier
int	encode_PS_Event__identifier(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Event__identifier *parm;
{
  return (enc_f(_ZEvent_identifierPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Event__identifier
int	decode_PS_Event__identifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Event__identifier **parm;
{
  return (dec_f(_ZEvent_identifierPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Event__identifier
/* ARGSUSED */
int	print_PS_Event__identifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Event__identifier *parm;
{
  return (prnt_f(_ZEvent_identifierPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Mode__selector
int	encode_PS_Mode__selector(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Mode__selector *parm;
{
  return (enc_f(_ZMode_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Mode__selector
int	decode_PS_Mode__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Mode__selector **parm;
{
  return (dec_f(_ZMode_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Mode__selector
/* ARGSUSED */
int	print_PS_Mode__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Mode__selector *parm;
{
  return (prnt_f(_ZMode_selectorPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Addition__list
int	encode_PS_Addition__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Addition__list *parm;
{
  return (enc_f(_ZAddition_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Addition__list
int	decode_PS_Addition__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Addition__list **parm;
{
  return (dec_f(_ZAddition_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Addition__list
/* ARGSUSED */
int	print_PS_Addition__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Addition__list *parm;
{
  return (prnt_f(_ZAddition_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Addition__result__list
int	encode_PS_Addition__result__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Addition__result__list *parm;
{
  return (enc_f(_ZAddition_result_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Addition__result__list
int	decode_PS_Addition__result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Addition__result__list **parm;
{
  return (dec_f(_ZAddition_result_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Addition__result__list
/* ARGSUSED */
int	print_PS_Addition__result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Addition__result__list *parm;
{
  return (prnt_f(_ZAddition_result_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Definition__list
int	encode_PS_Definition__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Definition__list *parm;
{
  return (enc_f(_ZDefinition_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Definition__list
int	decode_PS_Definition__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Definition__list **parm;
{
  return (dec_f(_ZDefinition_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Definition__list
/* ARGSUSED */
int	print_PS_Definition__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Definition__list *parm;
{
  return (prnt_f(_ZDefinition_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Definition__result__list
int	encode_PS_Definition__result__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Definition__result__list *parm;
{
  return (enc_f(_ZDefinition_result_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Definition__result__list
int	decode_PS_Definition__result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Definition__result__list **parm;
{
  return (dec_f(_ZDefinition_result_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Definition__result__list
/* ARGSUSED */
int	print_PS_Definition__result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Definition__result__list *parm;
{
  return (prnt_f(_ZDefinition_result_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Deletion__list
int	encode_PS_Deletion__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Deletion__list *parm;
{
  return (enc_f(_ZDeletion_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Deletion__list
int	decode_PS_Deletion__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Deletion__list **parm;
{
  return (dec_f(_ZDeletion_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Deletion__list
/* ARGSUSED */
int	print_PS_Deletion__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Deletion__list *parm;
{
  return (prnt_f(_ZDeletion_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Deletion__result__list
int	encode_PS_Deletion__result__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Deletion__result__list *parm;
{
  return (enc_f(_ZDeletion_result_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Deletion__result__list
int	decode_PS_Deletion__result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Deletion__result__list **parm;
{
  return (dec_f(_ZDeletion_result_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Deletion__result__list
/* ARGSUSED */
int	print_PS_Deletion__result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Deletion__result__list *parm;
{
  return (prnt_f(_ZDeletion_result_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Identifier
int	encode_PS_Identifier(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Identifier *parm;
{
  return (enc_f(_ZIdentifierPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Identifier
int	decode_PS_Identifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Identifier **parm;
{
  return (dec_f(_ZIdentifierPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Identifier
/* ARGSUSED */
int	print_PS_Identifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Identifier *parm;
{
  return (prnt_f(_ZIdentifierPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Identifier__list
int	encode_PS_Identifier__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Identifier__list *parm;
{
  return (enc_f(_ZIdentifier_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Identifier__list
int	decode_PS_Identifier__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Identifier__list **parm;
{
  return (dec_f(_ZIdentifier_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Identifier__list
/* ARGSUSED */
int	print_PS_Identifier__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Identifier__list *parm;
{
  return (prnt_f(_ZIdentifier_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Presentation__requirements
int	encode_PS_Presentation__requirements(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Presentation__requirements *parm;
{
  return (enc_f(_ZPresentation_requirementsPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Presentation__requirements
int	decode_PS_Presentation__requirements(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Presentation__requirements **parm;
{
  return (dec_f(_ZPresentation_requirementsPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Presentation__requirements
/* ARGSUSED */
int	print_PS_Presentation__requirements(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Presentation__requirements *parm;
{
  return (prnt_f(_ZPresentation_requirementsPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Selector
int	encode_PS_Selector(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Selector *parm;
{
  return (enc_f(_ZSelectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Selector
int	decode_PS_Selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Selector **parm;
{
  return (dec_f(_ZSelectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Selector
/* ARGSUSED */
int	print_PS_Selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Selector *parm;
{
  return (prnt_f(_ZSelectorPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Protocol__version
int	encode_PS_Protocol__version(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Protocol__version *parm;
{
  return (enc_f(_ZProtocol_versionPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Protocol__version
int	decode_PS_Protocol__version(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Protocol__version **parm;
{
  return (dec_f(_ZProtocol_versionPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Protocol__version
/* ARGSUSED */
int	print_PS_Protocol__version(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Protocol__version *parm;
{
  return (prnt_f(_ZProtocol_versionPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Provider__reason
int	encode_PS_Provider__reason(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Provider__reason *parm;
{
  return (enc_f(_ZProvider_reasonPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Provider__reason
int	decode_PS_Provider__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Provider__reason **parm;
{
  return (dec_f(_ZProvider_reasonPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Provider__reason
/* ARGSUSED */
int	print_PS_Provider__reason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Provider__reason *parm;
{
  return (prnt_f(_ZProvider_reasonPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Responding__presentation__selector
int	encode_PS_Responding__presentation__selector(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Responding__presentation__selector *parm;
{
  return (enc_f(_ZResponding_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Responding__presentation__selector
int	decode_PS_Responding__presentation__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Responding__presentation__selector **parm;
{
  return (dec_f(_ZResponding_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Responding__presentation__selector
/* ARGSUSED */
int	print_PS_Responding__presentation__selector(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Responding__presentation__selector *parm;
{
  return (prnt_f(_ZResponding_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Result
int	encode_PS_Result(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Result *parm;
{
  return (enc_f(_ZResultPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Result
int	decode_PS_Result(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Result **parm;
{
  return (dec_f(_ZResultPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Result
/* ARGSUSED */
int	print_PS_Result(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Result *parm;
{
  return (prnt_f(_ZResultPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Result__list
int	encode_PS_Result__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Result__list *parm;
{
  return (enc_f(_ZResult_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Result__list
int	decode_PS_Result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Result__list **parm;
{
  return (dec_f(_ZResult_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Result__list
/* ARGSUSED */
int	print_PS_Result__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Result__list *parm;
{
  return (prnt_f(_ZResult_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Transfer__syntax__name
int	encode_PS_Transfer__syntax__name(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Transfer__syntax__name *parm;
{
  return (enc_f(_ZTransfer_syntax_namePS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Transfer__syntax__name
int	decode_PS_Transfer__syntax__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Transfer__syntax__name **parm;
{
  return (dec_f(_ZTransfer_syntax_namePS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Transfer__syntax__name
/* ARGSUSED */
int	print_PS_Transfer__syntax__name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Transfer__syntax__name *parm;
{
  return (prnt_f(_ZTransfer_syntax_namePS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_User__data
int	encode_PS_User__data(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_User__data *parm;
{
  return (enc_f(_ZUser_dataPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_User__data
int	decode_PS_User__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_User__data **parm;
{
  return (dec_f(_ZUser_dataPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_User__data
/* ARGSUSED */
int	print_PS_User__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_User__data *parm;
{
  return (prnt_f(_ZUser_dataPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Simply__encoded__data
int	encode_PS_Simply__encoded__data(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Simply__encoded__data *parm;
{
  return (enc_f(_ZSimply_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Simply__encoded__data
int	decode_PS_Simply__encoded__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Simply__encoded__data **parm;
{
  return (dec_f(_ZSimply_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Simply__encoded__data
/* ARGSUSED */
int	print_PS_Simply__encoded__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Simply__encoded__data *parm;
{
  return (prnt_f(_ZSimply_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_Fully__encoded__data
int	encode_PS_Fully__encoded__data(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_Fully__encoded__data *parm;
{
  return (enc_f(_ZFully_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_Fully__encoded__data
int	decode_PS_Fully__encoded__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Fully__encoded__data **parm;
{
  return (dec_f(_ZFully_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_Fully__encoded__data
/* ARGSUSED */
int	print_PS_Fully__encoded__data(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_Fully__encoded__data *parm;
{
  return (prnt_f(_ZFully_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_PDV__list
int	encode_PS_PDV__list(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_PDV__list *parm;
{
  return (enc_f(_ZPDV_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_PDV__list
int	decode_PS_PDV__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_PDV__list **parm;
{
  return (dec_f(_ZPDV_listPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_PDV__list
/* ARGSUSED */
int	print_PS_PDV__list(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_PDV__list *parm;
{
  return (prnt_f(_ZPDV_listPS, &_ZPS_mod, pe, top, len, buffer));
}

#undef encode_PS_User__session__requirements
int	encode_PS_User__session__requirements(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_PS_User__session__requirements *parm;
{
  return (enc_f(_ZUser_session_requirementsPS, &_ZPS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_PS_User__session__requirements
int	decode_PS_User__session__requirements(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_User__session__requirements **parm;
{
  return (dec_f(_ZUser_session_requirementsPS, &_ZPS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_PS_User__session__requirements
/* ARGSUSED */
int	print_PS_User__session__requirements(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_PS_User__session__requirements *parm;
{
  return (prnt_f(_ZUser_session_requirementsPS, &_ZPS_mod, pe, top, len, buffer));
}

#endif	/* lint */
