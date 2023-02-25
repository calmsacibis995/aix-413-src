/* @(#)10	1.4  src/tcpip/usr/sbin/snmpd/smux_g.h, snmp, tcpip411, GOLD410 9/3/93 16:10:25 */
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
 * FILE:	src/tcpip/usr/sbin/snmpd/smux_g.h
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* smux-g.h - SMUX group */

#ifndef _SMUX_G_H_
#define _SMUX_G_H_

struct smuxPeer {
    struct smuxPeer *pb_forw;		/* doubly-linked list */
    struct smuxPeer *pb_back;		/*   .. */

    char    pb_source[30];
    struct fdinfo	*pb_fi;

    int     pb_index;                   /* smuxPindex */
    OID	    pb_identity;		/* smuxPidentity */
    char   *pb_description;		/* smuxPdescription */

    int	    pb_priority;		/* minimum allowed priority */

    int     pb_newstatus;		/* for setting smuxPstatus */
    int     pb_invalid;
};

extern	struct smuxPeer *PHead;


struct smuxTree {
    struct smuxTree *tb_forw;		/* doubly-linked list */
    struct smuxTree *tb_back;		/*   .. */

#define	TB_SIZE	30			/* object instance */
    unsigned int    tb_instance[TB_SIZE + 1];
    int	    tb_insize;

    OT	    tb_subtree;			/* smuxTsubtree */
    int	    tb_priority;		/* smuxTpriority */
    struct smuxPeer *tb_peer;		/* smuxTindex */

    struct smuxTree *tb_next;		/* linked list for ot_smux */

    int     tb_newstatus;		/* for setting smuxTstatus */
    int     tb_invalid;
    int     tb_smux_init;               /* determines if only smux monitored */
#define SMUX_INITED     1
#define SMUX_NOTINITED  0
};

extern	struct smuxTree *THead;

extern void pb_free (), tb_free ();


struct smuxClient {
    struct smuxClient	*sc_forw,
			*sc_back;
    OID			sc_id;		/* smux client identifier */
    char		*sc_password;	/* smux attach passwd */
    struct sockaddr_in	sc_sin,   	/* ip address   */
    			sc_netmask;
};

extern	struct smuxClient *SHead;

#endif /* _SMUX_G_H_ */
