static char sccsid[] = "@(#)37  1.1  src/rspc/kernext/pm/pmmi/pmdump/pmdumptools.c, pwrsysx, rspc41J, 9510A 3/8/95 11:50:27";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: memcmp, memcpy, memset
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/adspace.h>
#include <sys/ioacc.h>

/*
 *  NOTE: all the subroutines in this file need to be pinned.
 */

/*
 * NAME:  memcmp
 *
 * FUNCTION:  compares data 
 *
 * DATA STRUCTURES:
 *      char *s1	data 1
 *      char *s2	data 2
 *      int l		length to be compared
 *
 * RETURN VALUE DESCRIPTION:
 *      0: two data are same.
 *      1: two data are different.
 */
int
memcmp(char *s1, char *s2, int l)
{
	for(; l > 0; l--){
		if(*s1++ != *s2++){
			return 1;
		}
	}
	return 0;
}


/*
 * NAME:  memcpy
 *
 * FUNCTION:  copies data
 *
 * DATA STRUCTURES:
 *      char *t		target address
 *      char *f		source address
 *      int l           length to be copied
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 */
void
memcpy(char *t, char *f, int l)
{
	for(; l > 0; l--){
		*t++ = *f++;
	}
}


/*
 * NAME:  memset
 *
 * FUNCTION:  initializes buffer
 *
 * DATA STRUCTURES:
 *      char *t		points to a buffer
 *      int i		value to be set
 *      int l           length to be set
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 */
void
memset(char *t, int i, int l)
{
	for(; l > 0; l--){
		*t++ = i;
	}
}
