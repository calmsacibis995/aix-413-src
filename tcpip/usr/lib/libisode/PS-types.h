/* @(#)65	1.3  src/tcpip/usr/lib/libisode/PS-types.h, isodelib7, tcpip411, GOLD410 4/5/93 13:34:27 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: free_PS_ACA__PPDU free_PS_AC__PPDU free_PS_ARP__PPDU 
 *    free_PS_ARU__PPDU free_PS_Abort__reason free_PS_Abort__type 
 *    free_PS_CPA__type free_PS_CPR__type free_PS_CP__type 
 *    free_PS_Context__list free_PS_Context__name free_PS_Deletion__list 
 *    free_PS_Deletion__result__list free_PS_Event__identifier 
 *    free_PS_Fully__encoded__data free_PS_Identifier free_PS_Identifier__list 
 *    free_PS_Mode__selector free_PS_PDV__list free_PS_Provider__reason 
 *    free_PS_RSA__PPDU free_PS_RS__PPDU free_PS_Result free_PS_Result__list 
 *    free_PS_Typed__data__type free_PS_User__data
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/PS-types.h
 */

/* automatically generated by pepsy 6.0 #104 (vikings.austin.ibm.com), do not edit! */

#ifndef	_module_PS_defined_
#define	_module_PS_defined_

#ifndef	PEPSY_VERSION
#define	PEPSY_VERSION		2
#endif

#include <isode/psap.h>
#include <isode/pepsy.h>
#include <isode/pepsy/UNIV-types.h>

#include <isode/pepsy/PS_defs.h>

#define	type_PS_CPC__type	type_PS_User__data
#define	free_PS_CPC__type	free_PS_User__data

#define	type_PS_Abstract__syntax__name	OIDentifier
#define	free_PS_Abstract__syntax__name	oid_free

#define	type_PS_Called__presentation__selector	type_PS_Selector
#define	free_PS_Called__presentation__selector	free_PS_Selector

#define	type_PS_Calling__presentation__selector	type_PS_Selector
#define	free_PS_Calling__presentation__selector	free_PS_Selector

#define	type_PS_Context__result	type_PS_Result
#define	free_PS_Context__result	free_PS_Result

#define	type_PS_Addition__list	type_PS_Context__list
#define	free_PS_Addition__list	free_PS_Context__list

#define	type_PS_Addition__result__list	type_PS_Result__list
#define	free_PS_Addition__result__list	free_PS_Result__list

#define	type_PS_Definition__list	type_PS_Context__list
#define	free_PS_Definition__list	free_PS_Context__list

#define	type_PS_Definition__result__list	type_PS_Result__list
#define	free_PS_Definition__result__list	free_PS_Result__list

#define	type_PS_Presentation__requirements	PElement
#define	bits_PS_Presentation__requirements	"\020\01context-management\02restoration"
#define	bit_PS_Presentation__requirements_context__management	0
#define	bit_PS_Presentation__requirements_restoration	1
#define	free_PS_Presentation__requirements	pe_free

#define	type_PS_Selector	qbuf
#define	free_PS_Selector	qb_free

#define	type_PS_Protocol__version	PElement
#define	bits_PS_Protocol__version	"\020\01version-1"
#define	bit_PS_Protocol__version_version__1	0
#define	free_PS_Protocol__version	pe_free

#define	type_PS_Responding__presentation__selector	type_PS_Selector
#define	free_PS_Responding__presentation__selector	free_PS_Selector

#define	type_PS_Transfer__syntax__name	OIDentifier
#define	free_PS_Transfer__syntax__name	oid_free

#define	type_PS_Simply__encoded__data	qbuf
#define	free_PS_Simply__encoded__data	qb_free

#define	type_PS_User__session__requirements	PElement
#define	bits_PS_User__session__requirements	"\020\01half-duplex\02duplex\03expedited-data\04minor-synchronize\05major-synchronize\06resynchronize\07activity-management\010negotiated-release\011capability-data\012exceptions\013typed-data"
#define	bit_PS_User__session__requirements_half__duplex	0
#define	bit_PS_User__session__requirements_duplex	1
#define	bit_PS_User__session__requirements_expedited__data	2
#define	bit_PS_User__session__requirements_minor__synchronize	3
#define	bit_PS_User__session__requirements_major__synchronize	4
#define	bit_PS_User__session__requirements_resynchronize	5
#define	bit_PS_User__session__requirements_activity__management	6
#define	bit_PS_User__session__requirements_negotiated__release	7
#define	bit_PS_User__session__requirements_capability__data	8
#define	bit_PS_User__session__requirements_exceptions	9
#define	bit_PS_User__session__requirements_typed__data	10
#define	free_PS_User__session__requirements	pe_free

struct type_PS_CP__type {
    struct type_PS_Mode__selector *mode;

    PE      x410__mode;

    struct element_PS_0 {
        struct type_PS_Protocol__version *version;

        struct type_PS_Calling__presentation__selector *calling;

        struct type_PS_Called__presentation__selector *called;

        struct type_PS_Definition__list *context__list;

        struct type_PS_Context__name *default__context;

        struct type_PS_Presentation__requirements *presentation__fu;

        struct type_PS_User__session__requirements *session__fu;

        struct type_PS_User__data *user__data;
    } *normal__mode;
};
#define	free_PS_CP__type(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZCP_typePS], &_ZPS_mod, 1)

struct type_PS_CPA__type {
    struct type_PS_Mode__selector *mode;

    PE      x410__mode;

    struct element_PS_1 {
        struct type_PS_Protocol__version *version;

        struct type_PS_Responding__presentation__selector *responding;

        struct type_PS_Definition__result__list *context__list;

        struct type_PS_Presentation__requirements *presentation__fu;

        struct type_PS_User__session__requirements *session__fu;

        struct type_PS_User__data *user__data;
    } *normal__mode;
};
#define	free_PS_CPA__type(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZCPA_typePS], &_ZPS_mod, 1)

struct type_PS_CPR__type {
    int         offset;
#define	type_PS_CPR__type_x410__mode	1
#define	type_PS_CPR__type_normal__mode	2

    union {
        PE      x410__mode;

        struct element_PS_2 {
            integer     optionals;
#define	opt_PS_element_PS_2_reason (000000001)

            struct type_PS_Protocol__version *version;

            struct type_PS_Responding__presentation__selector *responding;

            struct type_PS_Definition__result__list *context__list;

            struct type_PS_Context__result *default__context;

            integer     reason;
#define	int_PS_reason_reason__not__specified	0
#define	int_PS_reason_temporary__congestion	1
#define	int_PS_reason_local__limit__exceeded	2
#define	int_PS_reason_called__presentation__address__unknown	3
#define	int_PS_reason_protocol__version__not__supported	4
#define	int_PS_reason_default__context__not__supported	5
#define	int_PS_reason_user__data__not__readable	6
#define	int_PS_reason_no__PSAP__available	7

            struct type_PS_User__data *user__data;
        } *normal__mode;
    }       un;
};
#define	free_PS_CPR__type(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZCPR_typePS], &_ZPS_mod, 1)

struct type_PS_Abort__type {
    int         offset;
#define	type_PS_Abort__type_x410__mode	1
#define	type_PS_Abort__type_normal__mode	2
#define	type_PS_Abort__type_provider__abort	3

    union {
        PE      x410__mode;

        struct element_PS_3 {
            struct type_PS_Identifier__list *context__list;

            struct type_PS_User__data *user__data;
        } *normal__mode;

        struct type_PS_ARP__PPDU *provider__abort;
    }       un;
};
#define	free_PS_Abort__type(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZAbort_typePS], &_ZPS_mod, 1)

struct type_PS_ARU__PPDU {
    int         offset;
#define	type_PS_ARU__PPDU_x410__mode	1
#define	type_PS_ARU__PPDU_normal__mode	2

    union {
        PE      x410__mode;

        struct element_PS_4 {
            struct type_PS_Identifier__list *context__list;

            struct type_PS_User__data *user__data;
        } *normal__mode;
    }       un;
};
#define	free_PS_ARU__PPDU(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZARU_PPDUPS], &_ZPS_mod, 1)

struct type_PS_ARP__PPDU {
    struct type_PS_Abort__reason *provider__reason;

    struct type_PS_Event__identifier *event;
};
#define	free_PS_ARP__PPDU(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZARP_PPDUPS], &_ZPS_mod, 1)

struct type_PS_Typed__data__type {
    int         offset;
#define	type_PS_Typed__data__type_acPPDU	1
#define	type_PS_Typed__data__type_acaPPDU	2
#define	type_PS_Typed__data__type_simple	3
#define	type_PS_Typed__data__type_complex	4

    union {
        struct type_PS_AC__PPDU *acPPDU;

        struct type_PS_ACA__PPDU *acaPPDU;

        struct type_PS_Simply__encoded__data *simple;

        struct type_PS_Fully__encoded__data *complex;
    }       un;
};
#define	free_PS_Typed__data__type(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZTyped_data_typePS], &_ZPS_mod, 1)

struct type_PS_AC__PPDU {
    struct type_PS_Addition__list *additions;

    struct type_PS_Deletion__list *deletions;

    struct type_PS_User__data *user__data;
};
#define	free_PS_AC__PPDU(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZAC_PPDUPS], &_ZPS_mod, 1)

struct type_PS_ACA__PPDU {
    struct type_PS_Addition__list *additions;

    struct type_PS_Deletion__list *deletions;

    struct type_PS_User__data *user__data;
};
#define	free_PS_ACA__PPDU(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZACA_PPDUPS], &_ZPS_mod, 1)

struct type_PS_RS__PPDU {
    struct type_PS_Identifier__list *context__list;

    struct type_PS_User__data *user__data;
};
#define	free_PS_RS__PPDU(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZRS_PPDUPS], &_ZPS_mod, 1)

struct type_PS_RSA__PPDU {
    struct type_PS_Identifier__list *context__list;

    struct type_PS_User__data *user__data;
};
#define	free_PS_RSA__PPDU(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZRSA_PPDUPS], &_ZPS_mod, 1)

struct type_PS_Abort__reason {
    integer     parm;
#define	int_PS_Abort__reason_reason__not__specified	0
#define	int_PS_Abort__reason_unrecognized__ppdu	1
#define	int_PS_Abort__reason_unexpected__ppdu	2
#define	int_PS_Abort__reason_unexpected__session__service__primitive	3
#define	int_PS_Abort__reason_unrecognized__ppdu__parameter	4
#define	int_PS_Abort__reason_unexpected__ppdu__parameter	5
#define	int_PS_Abort__reason_invalid__ppdu__parameter	6
};
#define	free_PS_Abort__reason(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZAbort_reasonPS], &_ZPS_mod, 1)

struct type_PS_Context__list {
        struct element_PS_6 {
            integer     identifier;

            struct type_PS_Abstract__syntax__name *abstract__syntax;

            struct element_PS_7 {
                struct type_PS_Transfer__syntax__name *Transfer__syntax__name;

                struct element_PS_7 *next;
            } *transfer__syntax__list;
        } *element_PS_5;

        struct type_PS_Context__list *next;
};
#define	free_PS_Context__list(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZContext_listPS], &_ZPS_mod, 1)

struct type_PS_Context__name {
    struct type_PS_Abstract__syntax__name *abstract__syntax;

    struct type_PS_Transfer__syntax__name *transfer__syntax;
};
#define	free_PS_Context__name(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZContext_namePS], &_ZPS_mod, 1)

struct type_PS_Event__identifier {
    integer     parm;
#define	int_PS_Event__identifier_cp__PPDU	0
#define	int_PS_Event__identifier_cpa__PPDU	1
#define	int_PS_Event__identifier_cpr__PPDU	2
#define	int_PS_Event__identifier_aru__PPDU	3
#define	int_PS_Event__identifier_arp__PPDU	4
#define	int_PS_Event__identifier_ac__PPDU	5
#define	int_PS_Event__identifier_aca__PPDU	6
#define	int_PS_Event__identifier_td__PPDU	7
#define	int_PS_Event__identifier_ttd__PPDU	8
#define	int_PS_Event__identifier_te__PPDU	9
#define	int_PS_Event__identifier_tc__PPDU	10
#define	int_PS_Event__identifier_tcc__PPDU	11
#define	int_PS_Event__identifier_rs__PPDU	12
#define	int_PS_Event__identifier_rsa__PPDU	13
#define	int_PS_Event__identifier_s__release__indication	14
#define	int_PS_Event__identifier_s__release__confirm	15
#define	int_PS_Event__identifier_s__token__give__indication	16
#define	int_PS_Event__identifier_s__token__please__indication	17
#define	int_PS_Event__identifier_s__control__give__indication	18
#define	int_PS_Event__identifier_s__sync__minor__indication	19
#define	int_PS_Event__identifier_s__sync__minor__confirm	20
#define	int_PS_Event__identifier_s__sync__major__indication	21
#define	int_PS_Event__identifier_s__sync__major__confirm	22
#define	int_PS_Event__identifier_s__p__exception__report__indication	23
#define	int_PS_Event__identifier_s__u__exception__report__indication	24
#define	int_PS_Event__identifier_s__activity__start__indication	25
#define	int_PS_Event__identifier_s__activity__resume__indication	26
#define	int_PS_Event__identifier_s__activity__interrupt__indication	27
#define	int_PS_Event__identifier_s__activity__start__confirm	28
#define	int_PS_Event__identifier_s__activity__discard__indication	29
#define	int_PS_Event__identifier_s__activity__discard__confirm	30
#define	int_PS_Event__identifier_s__activity__end__indication	31
#define	int_PS_Event__identifier_s__activity__end__confirm	32
};
#define	free_PS_Event__identifier(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZEvent_identifierPS], &_ZPS_mod, 1)

struct type_PS_Mode__selector {
    integer     parm;
#define	int_PS_Mode__selector_x410__1984__mode	0
#define	int_PS_Mode__selector_normal__mode	1
};
#define	free_PS_Mode__selector(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZMode_selectorPS], &_ZPS_mod, 1)

struct type_PS_Deletion__list {
        integer     element_PS_8;

        struct type_PS_Deletion__list *next;
};
#define	free_PS_Deletion__list(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZDeletion_listPS], &_ZPS_mod, 1)

struct type_PS_Deletion__result__list {
        integer     element_PS_9;
#define	int_PS_element_PS_9_acceptance	0
#define	int_PS_element_PS_9_user__rejection	1

        struct type_PS_Deletion__result__list *next;
};
#define	free_PS_Deletion__result__list(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZDeletion_result_listPS], &_ZPS_mod, 1)

struct type_PS_Identifier {
    integer     parm;
};
#define	free_PS_Identifier(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZIdentifierPS], &_ZPS_mod, 1)

struct type_PS_Identifier__list {
        struct element_PS_11 {
            integer     identifier;

            struct type_PS_Transfer__syntax__name *transfer__syntax;
        } *element_PS_10;

        struct type_PS_Identifier__list *next;
};
#define	free_PS_Identifier__list(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZIdentifier_listPS], &_ZPS_mod, 1)

struct type_PS_Provider__reason {
    integer     parm;
#define	int_PS_Provider__reason_reason__not__specified	0
#define	int_PS_Provider__reason_temporary__congestion	1
#define	int_PS_Provider__reason_local__limit__exceeded	2
#define	int_PS_Provider__reason_called__presentation__address__unknown	3
#define	int_PS_Provider__reason_protocol__version__not__supported	4
#define	int_PS_Provider__reason_default__context__not__supported	5
#define	int_PS_Provider__reason_user__data__not__readable	6
#define	int_PS_Provider__reason_no__PSAP__available	7
};
#define	free_PS_Provider__reason(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZProvider_reasonPS], &_ZPS_mod, 1)

struct type_PS_Result {
    integer     parm;
#define	int_PS_Result_acceptance	0
#define	int_PS_Result_user__rejection	1
#define	int_PS_Result_provider__rejection	2
};
#define	free_PS_Result(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZResultPS], &_ZPS_mod, 1)

struct type_PS_Result__list {
        struct element_PS_13 {
            integer     optionals;
#define	opt_PS_element_PS_13_provider__reason (000000001)

            integer     result;
#define	int_PS_result_acceptance	0
#define	int_PS_result_user__rejection	1
#define	int_PS_result_provider__rejection	2

            struct type_PS_Transfer__syntax__name *transfer__syntax;

            integer     provider__reason;
#define	int_PS_provider__reason_reason__not__specified	0
#define	int_PS_provider__reason_abstract__syntax__not__supported	1
#define	int_PS_provider__reason_proposed__transfer__syntaxes__not__supported	2
#define	int_PS_provider__reason_local__limit__on__DCS__exceeded	3
        } *element_PS_12;

        struct type_PS_Result__list *next;
};
#define	free_PS_Result__list(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZResult_listPS], &_ZPS_mod, 1)

struct type_PS_User__data {
    int         offset;
#define	type_PS_User__data_simple	1
#define	type_PS_User__data_complex	2

    union {
        struct type_PS_Simply__encoded__data *simple;

        struct type_PS_Fully__encoded__data *complex;
    }       un;
};
#define	free_PS_User__data(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZUser_dataPS], &_ZPS_mod, 1)

struct type_PS_Fully__encoded__data {
        struct type_PS_PDV__list *PDV__list;

        struct type_PS_Fully__encoded__data *next;
};
#define	free_PS_Fully__encoded__data(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZFully_encoded_dataPS], &_ZPS_mod, 1)

struct type_PS_PDV__list {
    struct type_PS_Transfer__syntax__name *transfer__syntax;

    integer     identifier;

    struct choice_PS_0 {
        int         offset;
#define	choice_PS_0_single__ASN1__type	1
#define	choice_PS_0_octet__aligned	2
#define	choice_PS_0_arbitrary	3

        union {
            PE      single__ASN1__type;

            struct qbuf *octet__aligned;

            PE      arbitrary;
        }       un;
    } *presentation__data__values;
};
#define	free_PS_PDV__list(parm)\
	(void) fre_obj((char *) parm, _ZPS_mod.md_dtab[_ZPDV_listPS], &_ZPS_mod, 1)
#endif
