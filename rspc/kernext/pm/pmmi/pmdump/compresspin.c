static char sccsid[] = "@(#)29  1.1  src/rspc/kernext/pm/pmmi/pmdump/compresspin.c, pwrsysx, rspc41J, 9510A 3/8/95 11:50:15";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: compress
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
#include <sys/types.h>
#include <sys/malloc.h>
#include "pmdump.h"
#include "pmdumpdebug.h"

#define BITS	10
#define HSIZE	5003
#define HSHIFT	4

#define FIRST	257	/* first free entry */

int		*htab = NULL;
unsigned short	*codetab = NULL;

/*
 * NAME:  compress
 *
 * FUNCTION:  execute compression
 *
 * DATA STRUCTURES:
 *      char *s		Input data. Treaded as PHYSICAL ADDRESS.
 *      int  in		Input data length.
 *      char *t         Output buffer pointer.
 *      int *out	Returns output data length.
 *
 * RETURN VALUE DESCRIPTION:
 *      none.
 */
/*
 *  Compression algorithm
 *    The compression algorithm is same as that of the compress command.
 *    Data size and compressed code format was modified to optimize
 *    compression speed for small data (4KB).
 *
 *  Compressed Code Format
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
 *
 *  Compressed Code Header
 *    - First 2 bytes (short) is length (including the header).
 *    - Next 1 byte is flag (0x01 == compressed, 0x00 == not compressed)
 *    - Next 1 byte is check sum of original code.
 *
 *  Note:
 *    If output data becomes bigger that the input data, data is just
 *    copied without compression. This means 4KB data becomes 4K + 4(header)
 *    bytes at most.
 */
void
compress(char *s, int in, char *t, int *out)
{
	register int	fcode;
	register int	i;
	register int	c;
    	register int	ent;
	register int	disp;
	register int	free_ent = FIRST;
	register int	offset = 0;
	register char	head = 0;
	register char	*ss = s;
	register char	*ts = t;
	register char	*endp = s + in;
	char		shift[4] = {8, 6, 4, 2};
	register char	sum;

	/* clear hash table */
	for(i = 0; i < HSIZE; i++){
		htab[i] = -1;
	}

	ent = sum = *s++;			/* first byte */
	t += 5;					/* output space for header */

	while(s < endp){
		c = *s++;
		sum += c;
		fcode = ((c << BITS) + ent);
		i = ((c << HSHIFT) ^ ent);	/* xor hashing */

		if(htab[i] == fcode){
			ent = codetab[i];
			continue;

		}else if(htab[i] < 0)		/* empty slot */
			goto nomatch;

		/* otherwize */
		disp = HSIZE - i;	/* secondary hash (after G. Knott) */
		if(i == 0)
			disp = 1;
probe:
		if((i -= disp) < 0)
			i += HSIZE;
		if(htab[i] == fcode){
			ent = codetab[i];
			continue;
		}
		if(htab[i] > 0)
			goto probe;
nomatch:
		/* output */
		*t++ = ent;
		head |= ((ent & 0x0300) >> shift[offset++]);
		if(offset == 4){
			*(t - 5) = head;
			t++;
			head = 0;
			offset = 0;
		}
		ent = c;

		if(free_ent < (1 << BITS)){
			codetab[i] = free_ent++;	/* code -> hashtable */
			htab[i] = fcode;
		}
	} /* end of while(s < endp) */

	/*
	 * Put out the final code.
	 */
	*t++ = ent;
	head |= ((ent & 0x0300) >> shift[offset]);
	*(t - offset - 2) = head;
	c = t - ts;
	if(c < in + 4){
		*ts++ = (0xff00 & c) >> 8; 	/* length */
		*ts++ = (0x00ff & c);
		*ts++ = 0x01;			/* compressed */
		*ts++ = sum;			/* check sum */
	}else{
		c = in + 4;
		*ts++ = (0xff00 & c) >> 8; 	/* length */
		*ts++ = (0x00ff & c);
		*ts++ = 0x00;			/* not compressed */
		s = ts++;			/* byte for check sum */
		sum = 0;
		for(i = 0; i < in; i++){	/* copy original data */
			ent = *ss++;
			*ts++ = ent;
			sum += ent;
		}
		*s = sum;			/* check sum */
	}
#ifdef PM_HIBERNATION_DEBUG
	{	static int x = 0;
		printf("%02x ", sum);
		x++;
		if((x % 24) == 0){
			printf("\n");
		}
	}
#endif /* PM_HIBERNATION_DEBUG */
	*out = c;
	return;
}
