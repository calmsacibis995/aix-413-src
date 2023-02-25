static char sccsid[] = "@(#)50	1.2  src/tcpip/usr/lib/libisode/pepsy_strings.c, isodelib7, tcpip411, GOLD410 3/3/93 17:18:22";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pepsy_strings.c
 */

/* pepy_strings.c - constant strings used in pepy */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pepsy_strings.c,v 1.2 93/02/05 17:06:29 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pepsy_strings.c,v 1.2 93/02/05 17:06:29 snmp Exp $
 *
 *
 * $Log:	pepsy_strings.c,v $
 * Revision 1.2  93/02/05  17:06:29  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:49:50  mrose
 * Interim 6.8
 * 
 * Revision 7.0  90/07/01  19:54:28  mrose
 * *** empty log message ***
 * 
 * Revision 7.0  89/11/23  22:11:54  mrose
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

char	*pepy_strings[] = {
	"bad ",				/* PEPY_ERR_BAD */
	"bad bitstring: ",		/* PEPY_ERR_BAD_BITS */
	"bad boolean: ",		/* PEPY_ERR_BAD_BOOLEAN */
	"bad class/id: ",		/* PEPY_ERR_BAD_CLASS */
	"bad class/form/id: ",		/* PEPY_ERR_BAD_CLASS_FORM_ID */
	"bad form ",			/* PEPY_ERR_BAD_FORM */
	"bad integer: ",		/* PEPY_ERR_BAD_INTEGER */
	"bad object identifier: ",	/* PEPY_ERR_BAD_OID */
	"bad octetstring: ",		/* PEPY_ERR_BAD_OCTET */
	"bad real: ",			/* PEPY_ERR_BAD_REAL */
	"bad sequence: ",		/* PEPY_ERR_BAD_SEQ */
	"bad set: ",			/* PEPY_ERR_BAD_SET */
	"has too many bits",		/* PEPY_ERR_TOO_MANY_BITS */
	"has too many elements",	/* PEPY_ERR_TOO_MANY_ELEMENTS */
	"has unknown choice: ",		/* PEPY_ERR_UNKNOWN_CHOICE */
	"has unknown component: ",	/* PEPY_ERR_UNK_COMP */
	"initialization fails",		/* PEPY_ERR_INIT_FAILED */
	"invalid choice selected: ",	/* PEPY_ERR_INVALID_CHOICE */
	"missing ",			/* PEPY_ERR_MISSING */
	"out of memory",		/* PEPY_ERR_NOMEM  */
	"too many elements for tagged ", /* PEPY_ERR_TOO_MANY_TAGGED */
	"warning: extra or duplicate members present in SET",
					/* PEPY_ERR_EXTRA_MEMBERS */
	(char *)0
};
	
	
