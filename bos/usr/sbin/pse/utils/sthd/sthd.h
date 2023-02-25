/* @(#)78	1.1  src/bos/usr/sbin/pse/utils/sthd/sthd.h, cmdpse, bos411, 9428A410j 5/7/91 13:53:11 */
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/** Copyright (c) 1990  Mentat Inc.
 ** sthd.h 2.2, last change 4/9/91
 **/

#ifndef _STHD_
#define	_STHD_

#include <sys/stropts.h>
#include <errno.h>

#ifndef	BOGUS_ADDRESS
#define	BOGUS_ADDRESS ((char *)0xFFFFFFFF)
#endif

#define	ERROR		0x1	/* error mode */
#define	ONDELAY		0x2	/* nonblocking mode */
#define	HANGUP		0x4	/* hangup mode */

#define	DEFAULT_BAND	5	/* band to use for testing */

#define	PAUSE_HERE()	if (pause_between_tests) pause_now()
extern	int	pause_between_tests;

extern	int	fatal(   char * fmt, ...   );
extern	int	printe(   boolean print_errno, ...   );
extern	int	printok(   char * fmt, ...   );
extern	int	usage(   char * fmt, ...   );

extern	char	* echo_name;
extern	char	* errm_name;
extern	char	* nuls_name;
extern	char	* pass_name;
extern	char	* sad_name;
extern	char	* spass_name;

#endif
