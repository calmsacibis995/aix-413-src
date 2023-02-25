/* @(#)22	1.3  src/tcpip/usr/sbin/snmpd/view.h, snmp, tcpip411, GOLD410 5/26/93 14:03:49 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: inSubTree
 *
 * ORIGINS: 27 60
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/view.h
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* view-g.h - VIEW group */

#ifndef _VIEW_G_H_
#define _VIEW_G_H_

/*    VIEWS */

#define	inSubtree(tree,object) \
    	((tree) -> oid_nelem <= (object) -> oid_nelem \
	     && bcmp ((char *) (tree) -> oid_elements, \
		      (char *) (object) -> oid_elements, \
		      (tree) -> oid_nelem \
		          * sizeof ((tree) -> oid_elements[0])) == 0)

struct subtree {
    struct subtree *s_forw;	/* doubly-linked list */
    struct subtree *s_back;	/* doubly-linked list */

    OID	    s_subtree;		/* subtree */
    char   *s_text;		/* subtree name */
};

struct view {
    struct view *v_forw;	/* doubly-linked list */
    struct view *v_back;	/*   .. */

    OID	    v_name;		/* view name */
    u_long  v_mask;		/* view mask */

    struct subtree v_subtree;	/* list of subtrees */

    struct qbuf *v_community;	/* for proxy, traps... */
    struct sockaddr v_sa;

    unsigned int *v_instance;	/* object instance */
    int	    v_insize;		/*   .. */
};

extern struct view *VHead;

/*    COMMUNITIES */

struct community {
    struct community *c_forw;	/* doubly-linked list */
    struct community *c_back;	/*   .. */

    char   *c_name;		/* community name */
    struct sockaddr_in c_sin;   /* ip address   */
    struct sockaddr_in c_netmask;   /* netmask */

    int	    c_permission;	/* same as ot_access */
#define	OT_YYY	0x08

    OID	    c_vu;		/* associated view */
    struct view *c_view;	/*   .. */

    unsigned int *c_instance;	/* object instance */
    int	    c_insize;		/*   .. */
    struct community *c_next;	/* next in lexi-order */
};

extern struct community *CHead;  /* defined in view.c */

/*    TRAPS */

struct trap {
    struct trap *t_forw;	/* doubly-linked list */
    struct trap *t_back;	/*   .. */

    char   *t_name;		/* community name */

    struct view  t_vu;		/* associated view */
    struct view *t_view;	/*   .. */

    u_long  t_generics;		/* generic traps enabled */

    unsigned int *t_instance;	/* object instance */
    int	    t_insize;		/*   .. */
};

extern struct trap *UHead;

#endif /* _VIEW_G_H_ */
