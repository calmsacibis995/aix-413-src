/* @(#)18	1.2  src/tcpip/usr/include/isode/cmd_srch.h, isodelib7, tcpip411, GOLD410 3/2/93 09:46:41 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/cmd_srch.h
 */

/* cmd_srch.h - command search structure */

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/include/isode/cmd_srch.h,v 1.2 93/02/05 16:34:47 snmp Exp $
 *
 *
 * $Log:	cmd_srch.h,v $
 * Revision 1.2  93/02/05  16:34:47  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:24:36  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:55:40  mrose
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


#ifndef _CMD_SRCH_
#define _CMD_SRCH_

typedef struct  cmd_table {
	char    *cmd_key;
	int     cmd_value;
} CMD_TABLE;


struct  cm_args {
	char    *cm_key;
	char    *cm_value;
};

int     cmd_srch ();
char   *rcmd_srch ();

#endif
