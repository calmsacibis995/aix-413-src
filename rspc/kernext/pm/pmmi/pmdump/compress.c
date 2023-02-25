static char sccsid[] = "@(#)28  1.1  src/rspc/kernext/pm/pmmi/pmdump/compress.c, pwrsysx, rspc41J, 9510A 3/8/95 11:50:13";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: init_compress, term_compress
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

#define HSIZE	5003
#define HSHIFT	4

extern int		*htab;
extern unsigned short	*codetab;

/*
 * NAME:  init_compress
 *
 * FUNCTION:  allocates work area for compression
 *
 * DATA STRUCTURES:
 *      none.
 *
 * RETURN VALUE DESCRIPTION:
 *      0:  success
 *      -1: cannat allocate memory
 */
int
init_compress()
{
	DEBUG_0("Enter init_compress()\n");

	if((htab = xmalloc(HSIZE*sizeof(int), 2, pinned_heap)) == NULL){
		DEBUG_0("Exit init_compress(): xmalloc(htab) error\n");
		return -1;
	}
	if((codetab = xmalloc(HSIZE*sizeof(unsigned short), 2, pinned_heap))
								== NULL){
		DEBUG_0("Exit init_compress(): xmalloc(codetab) error\n");
		term_compress();
		return -1;
	}

	DEBUG_0("Exit init_compress() successfully\n");
	return 0;
}


/*
 * NAME:  term_compress
 *
 * FUNCTION:  frees work area for compression
 *
 * DATA STRUCTURES:
 *      none.
 *
 * RETURN VALUE DESCRIPTION:
 *      0:  success
 *      -1: cannat allocate memory
 */
int
term_compress()
{
	DEBUG_0("Enter term_compress()\n");

	FREE_MEMORY(htab);
	FREE_MEMORY(codetab);

	DEBUG_0("Exit term_compress() successfully\n");
	return 0;
}
