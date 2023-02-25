static char sccsid[] = "@(#)23	1.11  src/tcpip/usr/sbin/snmpd/vpd.c, snmp, tcpip411, GOLD410 5/25/94 15:38:20";
/*
 * COMPONENT_NAME: (SNMPD) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: parse_VPD()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/vpd.c
 */

#ifdef _POWER

#include "snmpd.h"
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

/* ^L */
/*
 * NAME: strnchr
 *
 * FUNCTION: this function is the same as the strchr library function except
 *              a length variable is used to indicate maximum length of scan.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: pointer to the location of ch if found.
 *      a null pointer if ch is not found within len bytes
 */
char *strnchr(char *sptr, char ch, int len)
{
  int cnt;

  for(cnt=0; cnt < len; cnt++,sptr++) {
    if(*sptr == ch) return(sptr);
  }
  return(((char *) '\0'));
}

/*  */
/*  NAME: parse_VPD
 *
 *  FUNCTION:  parse VPD info
 */
int
parse_VPD (inbuf_ptr, outbuf_ptr, vpd_keys, nkeys)
char    *inbuf_ptr,
	*outbuf_ptr;
struct  key *vpd_keys;
int     nkeys;
{
    int i;
    int length;
    int (*ptr) ();	/* pointer to function */
    char *vpdvalue, 
	 *vpdptr, 
	 *text;
    char keyStr[2];
    char *na = NA_MSG;

    /* intended for value, keyword[1]valuekeyword[1]value ...*/
    vpdvalue = (char *)malloc (VPDSIZE);

    /* reset value field */
    for (i = 0; i < nkeys; i++) 
	vpd_keys[i].value = NULL;
    
    while (inbuf_ptr = (char *) strnchr (inbuf_ptr, '*', VPDSIZE)) {
	keyStr[0] = toupper (*++inbuf_ptr);
	keyStr[1] = toupper (*++inbuf_ptr);
	for (i = 0; i < nkeys; i++) {
	    if (!strncmp (keyStr, vpd_keys[i].keyword, 2) || 
		    keyStr[0] == 'Z') {
		if (keyStr[0] == 'Z') {
		    i = nkeys -1;
		    continue;
		}
		/* get length of data and copy it to vpdvalue */
		if ((length = ((*++inbuf_ptr *2) - 4)) <= 0)
		    continue;
		vpdptr = vpdvalue;
		if ((ptr = vpd_keys[i].func) != NULL)
		    (*ptr) (length, ++inbuf_ptr, vpdptr);
		else {
		    if (*++inbuf_ptr == ' ') {
			length -= 1;
			++inbuf_ptr;
		    }
		    strncpy (vpdptr, inbuf_ptr, length);
		    vpdvalue[length] = '\0';
		}
		length = strlen (vpdvalue);
		text = (char *)malloc (length * 2);
		sprintf (text, "%s", vpdvalue);
		vpd_keys[i].value = text;    /* save our value for later */
		break;
	    }
	}
    }

    /* build up our ifDescr/sysDescr string */
    *outbuf_ptr = '\0';			/* null out our output string */
    for (i = 0; i < nkeys; i++) {
	strcat (outbuf_ptr, vpd_keys[i].mesg);
	strcat (outbuf_ptr, " ");
	strcat (outbuf_ptr, 
		(vpd_keys[i].value == NULL) ? na : vpd_keys[i].value);
	strcat (outbuf_ptr, " ");
	if (vpd_keys[i].value != NULL)	/* free it if we malloc'ed it */
	    free (vpd_keys[i].value);
    }
    free (vpdvalue);

    return (0);
}

#endif /* _POWER */
