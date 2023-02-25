/* @(#)26	1.2  src/tcpip/usr/include/isode/isoservent.h, isodelib7, tcpip411, GOLD410 3/2/93 09:48:28 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/isoservent.h
 */

/* isoservent.h - ISODE services database access routines */

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/include/isode/isoservent.h,v 1.2 93/02/05 16:35:28 snmp Exp $
 *
 *
 * $Log:	isoservent.h,v $
 * Revision 1.2  93/02/05  16:35:28  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:24:45  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:55:47  mrose
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


#ifndef	_ISOSERVENT_
#define	_ISOSERVENT_


struct isoservent {
    char         *is_entity;	/* name of entity */
    char         *is_provider;	/* name of service provider */

#define	ISSIZE	64		/* xSAP selector/ID */
    int		  is_selectlen;
    union {
	char		is_un_selector[ISSIZE];
	unsigned short  is_un_port;
    }		un_is;
#define	is_selector	un_is.is_un_selector
#define	is_port		un_is.is_un_port

    char        **is_vec;	/* exec vector */
    char        **is_tail;	/* next free slot in vector */
};


int	setisoservent (), endisoservent ();

struct isoservent *getisoservent ();

struct isoservent *getisoserventbyname ();
struct isoservent *getisoserventbyselector ();
struct isoservent *getisoserventbyport ();

#endif
