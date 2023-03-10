/* @(#)35	1.4  src/tcpip/usr/include/isode/ronot.h, isodelib7, tcpip411, GOLD410 4/5/93 13:26:55 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: copyRoNOTdata
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/ronot.h
 */

/* ronot.h - Additions to properly support ABSTRACT-BIND */

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/include/isode/ronot.h,v 1.2 93/02/05 16:36:03 snmp Exp $
 *
 *
 * $Log:	ronot.h,v $
 * Revision 1.2  93/02/05  16:36:03  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:24:58  mrose
 * Interim 6.8
 * 
 * Revision 7.0  90/07/26  14:26:02  mrose
 * *** empty log message ***
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */

#ifndef	_RoNot_
#define	_RoNot_

#ifndef	_MANIFEST_
#include <isode/manifest.h>
#endif
#ifndef	_GENERAL_
#include <isode/general.h>
#endif

#ifndef	_AcSAP_
#include <isode/acsap.h>	/* definitions for AcS-USERs */
#endif

#ifndef	_RoSAP_
#include <isode/rosap.h>	/* definitions for RoS-USERs */
#endif

#define BIND_RESULT	1	/* indicates a bind result occured */
#define BIND_ERROR	2	/* indicates a bind error occured */

struct RoNOTindication {
    int	    rni_reason;		/* reason for failure */
#define RBI_ACSE		1	/* ACSE provider failed */
#define RBI_SET_ROSE_PRES	2	/* Failed to set ROS-USER */
#define RBI_ENC_BIND_ARG	3	/* Failed encoding bind argument */
#define RBI_ENC_BIND_RES	4	/* Failed encoding bind result */
#define RBI_ENC_BIND_ERR	5	/* Failed encoding bind error */
#define RBI_ENC_UNBIND_ARG	6	/* Failed encoding unbind argument */
#define RBI_ENC_UNBIND_RES	7	/* Failed encoding unbind result */
#define RBI_ENC_UNBIND_ERR	8	/* Failed encoding unbind error */
#define RBI_DEC_BIND_ARG	9	/* Failed decoding bind argument */
#define RBI_DEC_BIND_RES	10	/* Failed decoding bind result */
#define RBI_DEC_BIND_ERR	11	/* Failed decoding bind error */
#define RBI_DEC_UNBIND_ARG	12	/* Failed decoding unbind argument */
#define RBI_DEC_UNBIND_RES	13	/* Failed decoding unbind result */
#define RBI_DEC_UNBIND_ERR	14	/* Failed decoding unbind error */
#define RBI_DEC_NINFO		15	/* Erroneous number of user infos */

				/* diagnostics from provider */
#define	RB_SIZE	512
    int	    rni_cc;		/*   length */
    char    rni_data[RB_SIZE];	/*   data */
};

#ifndef	lint
#if	!defined(_POWER)
#define	copyRoNOTdata(base,len,d) \
{ \
    register int i = len; \
    if ((d -> d/* */_cc = min (i, sizeof d -> d/* */_data)) > 0) \
	bcopy (base, d -> d/* */_data, d -> d/* */_cc); \
}
#else
#define	copyRoNOTdata(base,len,d) \
{ \
    register int i = len; \
    if ((d -> d##_cc = min (i, sizeof d -> d##_data)) > 0) \
	bcopy (base, d -> d##_data, d -> d##_cc); \
}
#endif
#else
#define	copyRoNOTdata(base,len,d)	bcopy (base, (char *) d, len)
#endif

#endif
