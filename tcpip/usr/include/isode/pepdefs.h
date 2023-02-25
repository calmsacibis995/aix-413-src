/* @(#)30	1.2  src/tcpip/usr/include/isode/pepdefs.h, isodelib7, tcpip411, GOLD410 3/2/93 09:49:16 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepdefs.h
 */

/* pepdefs.h */

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/include/isode/pepdefs.h,v 1.2 93/02/05 16:35:43 snmp Exp $
 *
 *
 * $Log:	pepdefs.h,v $
 * Revision 1.2  93/02/05  16:35:43  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:24:51  mrose
 * Interim 6.8
 * 
 * Revision 7.0  90/07/01  19:52:37  mrose
 * *** empty log message ***
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


#ifndef PEPDEF_DEFINITIONS
#define PEPDEF_DEFINITIONS
/*
 * Globally known pep definitions
 */

typedef	struct	{
	char	*md_name;	/* Name of this module */
	int	md_nentries;	/* Number of entries */
	tpe	**md_etab;	/* Pointer to encoding tables */
	tpe	**md_dtab;	/* Pointer to decoding tables */
	ptpe **md_ptab;	/* Pointer to Printing tables */
	PE	(*md_eucode)();	/* User code for encoding */
	PE	(*md_ducode)();	/* User code for decoding */
	PE	(*md_pucode)();	/* User code for printing */

	}	modtyp;

#ifndef NULL
#define NULL	(char *)0
#endif

#endif
