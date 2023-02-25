/* @(#)19	1.3  src/tcpip/usr/include/isode/config.h, isodelib7, tcpip411, GOLD410 3/2/93 09:46:54 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/config.h
 */

/* bsd44.h - site configuration file for 4.4BSD UNIX */

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/include/isode/config.h,v 1.2 93/02/05 16:34:53 snmp Exp $
 *
 *
 * $Log:	config.h,v $
 * Revision 1.2  93/02/05  16:34:53  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/01/10  16:13:03  mrose
 * x25
 * 
 * Revision 7.1  90/07/09  14:32:41  mrose
 * sync
 * 
 * Revision 7.0  89/11/23  21:26:00  mrose
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


#ifndef	_CONFIG_
#define	_CONFIG_

#define	WRITEV			/*   real Berkeley UNIX */
#define	VSPRINTF		/* has vprintf(3s) routines */

#ifndef RT
#  define	BSD42			/* Berkeley UNIX */
#  define	BSD43			/*   4.3BSD or later */
#  define	BSD44			/*   4.4BSD to be precise! */
#  ifdef AOS
#    undef BSD44
#    undef VSPRINTF		/* has vprintf(3s) routines */
#  endif /* AOS */
#  ifdef PS2
#    undef BSD44
#  endif /* PS2 */
#endif /* RT */

#define	TCP			/* has TCP/IP (of course) */
#define	SOCKETS			/*   provided by sockets */

#define	GETDENTS		/* has getdirent(2) call */

#endif
