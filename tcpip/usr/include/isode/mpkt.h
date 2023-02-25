/* @(#)29	1.1.1.3  src/tcpip/usr/include/isode/mpkt.h, isodelib7, tcpip411, GOLD410 4/5/93 13:18:43 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/mpkt.h
 */

/* mpkt.h - defines the report format for management */

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/include/isode/mpkt.h,v 1.3 93/02/09 08:57:15 snmp Exp $
 *
 *
 * $Log:	mpkt.h,v $
 * Revision 1.3  93/02/09  08:57:15  snmp
 * Header change for c++
 * 
 * Revision 7.1  91/02/22  09:24:50  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:55:50  mrose
 * Release 6.0
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

#ifndef _MPKT_
#define _MPKT_

#include <isode/isoaddrs.h>


struct MReport {
    u_short type;
#define OPREQIN         1
#define OPREQOUT        2
#define USERDT          3
#define USERDR          4
#define DISCREQ         5
#define PROTERR         6
#define CONGEST         7
#define CONFIGBAD       8
#define OPREQINBAD      9
#define OPREQOUTBAD     10
#define SOURCEADDR      11
#define	STARTLISTEN	12
#define	ENDLISTEN	13

    long    id;		/* process id */
    u_short cid;        /* connection fd */

    union {
	struct {
	    int a, b, c, d, e, f;
	} gp;

	struct {
	    int	    tsel_len;
	    char    tsel[TSSIZE];
	    struct NSAPaddr nsap;
	} taddr;
    } u;
};

int	TManGen ();

#endif /* _MPKT_ */
