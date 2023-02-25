static char sccsid[] = "@(#)84  1.1  src/rspc/kernext/pm/wakeup/decompress.c, pwrsysx, rspc41J, 9510A 3/8/95 06:44:10";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: decompress 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   ABSTRACT:
 *
 *   This code is a basic function for data decompressing. 
 */

#define BITS	10
#define HSIZE	1024

#define FIRST	257	/* first free entry */


/*
 *  Compressed Code Format.
 *    - All output code is 10 bits.
 *    - Every 5 bytes has four codes.
 *    - The first byte contains the most significant 2 bits of all codes.
 *
 *      1st byte       2nd byte      3rd byte      4th byte      5th byte
 *  +-+-+-+-+-+-+-+-+-------------+-------------+-------------+-------------+
 *  |   |   |   |   |             |             |             |             |
 *  | 4 | 3 | 2 | 1 |      1      |      2      |      3      |      4      |
 *  |   |   |   |   |             |             |             |             |
 *  +-+-+-+-+-+-+-+-+-------------+-------------+-------------+-------------+
 *  |  2 bits each  |   8 bits    |   8 bits    |   8 bits    |   8 bits    |
 */
void
decompress(char *s, int in, char *t, int *out)
{
        char		htab[HSIZE];
        unsigned short	codetab[HSIZE];
	register char	finchar;
	register int	code, oldcode, incode;
	register int	free_ent;
	register int	offset;
	register int	head;
	char		shift[4] = {8, 6, 4, 2};
	register char	*ts = t;
	register char	*endp = s + in;
	char		stack[128];
	register char	*stackp = stack;

	/*
	 * initialize the first 256 entries in the table.
	 */
	for(code = 255; code >= 0; code--){
		codetab[code] = 0;
		htab[code] = (char)code;
	}
	free_ent = FIRST;

	head = *s++;
	*t++ = finchar = oldcode = *s++;
	offset = 1;

	while(s < endp){
		if(offset == 4){
			head = *s++;
			offset = 0;
		}
		code = (*s++ | ((head << shift[offset++]) & 0x0300));
		incode = code;
		/*
		 * Special case for KwKwK string.
		 */
		if(code >= free_ent){
			*stackp++ = finchar;
			code = oldcode;
		}

		/*
		 * Generate output characters in reverse order
		 */
		while(code >= 256){
			*stackp++ = htab[code];
			code = codetab[code];
		}
		*t++ = finchar = htab[code];

		/*
		 * And put them out in forward order
		 */
		while(stackp > stack){
			*t++ = *--stackp;
		}

		/*
		 * Generate the new entry.
		 */
		if(free_ent < (1 << BITS)){
			codetab[free_ent] = (unsigned short)oldcode;
			htab[free_ent] = finchar;
			free_ent++;
		} 
		/*
		 * Remember previous code.
		 */
		oldcode = incode;
	}

	*out = t - ts;
	return;
}
