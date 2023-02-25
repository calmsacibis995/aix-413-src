/* @(#)51	1.8  src/bos/usr/sbin/nimclient/nimclient.h, cmdnim, bos411, 9428A410j  12/8/93  16:54:16 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/sbin/nimclient/nimclient.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_NIMCLIENT
#define _H_NIMCLIENT
/* --------------------- Defines and Typedefs for NIMCLIENT ---------------- */

#define ON	1
#define OFF 	0

#define ALLOWED_PARMS	"o:a:dS:Lt:s:qR:rpPZ:"

typedef struct {
	char	*valid_chars;
	void 	(*func)();
}parms;

/* --------------------- EXTERNS for GLOBAL MEMORY ------------------------ */

extern	parms	parm_list[];

extern	char	*optarg;


extern	do_yank();
extern	do_nim();
extern	set_date();
extern	state();
extern	resByType();
extern	resList();
extern	resAlloc();
extern	rootSinc();
extern	enable();
extern	disable();

#endif /* _H_NIMCLIENT */
