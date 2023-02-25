/* @(#)89	1.7  src/tcpip/usr/include/isode/snmp/smux.h, snmp, tcpip411, GOLD410 9/3/93 16:27:22 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE: src/tcpip/usr/include/isode/snmp/smux.h
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* smux.h - SMUX include file */

#ifndef _SMUX_
#define _SMUX_

#include <isode/pepsy/SMUX-types.h>
#include <isode/snmp/ibmsmux.h>

/*  */

#define	readOnly	int_SMUX_operation_readOnly
#define	readWrite	int_SMUX_operation_readWrite
#define	delete		int_SMUX_operation_delete


#define	goingDown		int_SMUX_ClosePDU_goingDown
#define	unsupportedVersion	int_SMUX_ClosePDU_unsupportedVersion
#define	packetFormat		int_SMUX_ClosePDU_packetFormat
#define	protocolError		int_SMUX_ClosePDU_protocolError
#define	internalError		int_SMUX_ClosePDU_internalError
#define	authenticationFailure	int_SMUX_ClosePDU_authenticationFailure

#define	invalidOperation	(-1)
#define	parameterMissing	(-2)
#define	systemError		(-3)
#define	youLoseBig		(-4)
#define	congestion		(-5)
#define	inProgress		(-6)

extern	integer	smux_errno;
extern	char	smux_info[];

/*  */

int	smux_init ();				/* INIT */
int	smux_simple_open ();			/* (simple) OPEN */
int	smux_close ();				/* CLOSE */
int	smux_register ();			/* REGISTER */
int	smux_response ();			/* RESPONSE */
int	smux_wait ();				/* WAIT */
int	smux_trap ();				/* TRAP */

char   *smux_error ();				/* TEXTUAL ERROR */

/*  */

struct smuxEntry {
    char   *se_name;

    OIDentifier se_identity;
    char   *se_password;

    int	    se_priority;
};

int	setsmuxEntry (), endsmuxEntry ();

struct smuxEntry *getsmuxEntry ();

struct smuxEntry *getsmuxEntrybyname ();
struct smuxEntry *getsmuxEntrybyidentity ();

void smux_free_tree();

#endif /* _SMUX_ */
