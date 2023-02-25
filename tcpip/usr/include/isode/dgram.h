/* @(#)20	1.2  src/tcpip/usr/include/isode/dgram.h, isodelib7, tcpip411, GOLD410 3/2/93 09:47:08 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/dgram.h
 */

/* dgram.h - datagram (CL-mode TS) abstractions */

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/include/isode/dgram.h,v 1.2 93/02/05 16:34:58 snmp Exp $
 *
 *
 * $Log:	dgram.h,v $
 * Revision 1.2  93/02/05  16:34:58  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:24:36  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/01/07  12:39:23  mrose
 * update
 * 
 * Revision 7.0  89/12/19  15:13:52  mrose
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


#ifndef	_DGRAM_
#define	_DGRAM_

#define	MAXDGRAM	8192


int	join_dgram_aux ();
int	read_dgram_socket ();
int	write_dgram_socket ();
int	close_dgram_socket ();
int	select_dgram_socket ();
int	check_dgram_socket ();

#endif
