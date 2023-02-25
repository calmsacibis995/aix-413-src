/* @(#)47	1.7  src/bos/usr/sbin/install/messages/instlmsg.h, cmdinstl, bos411, 9428A410j 1/15/94 18:36:07 */

#ifndef _H_INSTLMSG
#define _H_INSTLMSG
/*
 * COMPONENT_NAME: (CMDINSTL) command install
 *
 * FUNCTIONS:  This is the header for the set definitions for the messages
 *	for CMDINSTL
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "cmdinstl_e_msg.h"
#include <nl_types.h>

#define MSGSTR(Set,Num,Str) catgets(catd,Set,Num,Str)
#define CATOPEN() catd = catopen(MF_CMDINSTL_E, NL_CAT_LOCALE)
#define CATCLOSE() catclose(catd)


/* Defines to be used to pass in command name to common messages */
#define INU_BFFCREATE_CMD     "bffcreate"
#define INU_CKPREREQ_CMD      "ckprereq"
#define INU_INSTALLP_CMD      "installp"
#define INU_INSTLCLIENT_CMD   "instlclient"
#define INU_INUCONVERT_CMD    "inuconvert"
#define INU_INUCP_CMD         "inucp"
#define INU_INURECV_CMD       "inurecv"
#define INU_INUREST_CMD       "inurest"
#define INU_INUSAVE_CMD       "inusave"
#define INU_INUTOC_CMD        "inutoc"
#define INU_INUUMSG_CMD       "inuumsg"
#define INU_MAINTCLIENT_CMD   "maintclient"


/* These are the set numbers for all the sets included in cmdinstl.msg
 *     MS_BFF
 *     MS_CKPREREQ
 *     MS_INSTALLP
 *     MS_INSTLCLIENT
 *     MS_INUCONVERT
 *     MS_INUCP
 *     MS_INUCOMMON
 *     MS_INURECV
 *     MS_INUREST
 *     MS_INUSAVE
 *     MS_INUTOC
 *     MS_SYSCK
 *     MS_MAINTCLIENT
 *     MS_AUTOINST
 */


/* example of use */
/*	#include "../messages/instlmsg.h"
	nl_catd catd;

	main(int argc, char **argv)
	{
		(void) setlocale("LC_ALL","");
		CATOPEN();
			
		fprintf(stderr,MSGSTR(SYSCKSET,EXAM1,"Hello from sysck\n"));
			
		CATCLOSE();
	}
 */	

#endif /* _H_INSTLMSG */
