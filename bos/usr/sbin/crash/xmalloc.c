static char sccsid[] = "@(#)44	1.6  src/bos/usr/sbin/crash/xmalloc.c, cmdcrash, bos411, 9428A410j 5/1/91 14:48:06";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: xmalloc,memuse,updatestats
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */


/* 
 * uxmalloc() 
 * displays xmalloc() usage in kernel.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/xmalloc.h>
#include <nlist.h>
#include "crash.h"

struct stats {
	uint	next;	/* next on hash chain */
	uint	from;	/* address of requestor */
	uint	num;	/* number of requests */
	uint	bytes;  /* total space in bytes */
};

#define NHASH 512
#define NSTAT 100000
int   hstat[NHASH];
int   freestat = 1;
struct stats mstat[NSTAT];

xmalloc()
{
	struct heap * kernheap, * pinheap;

	/* get address of kernel_heap and of pinned_heap
	 */
	kernheap = (struct heap *) SYM_VALUE(Kernel_heap);
	pinheap = (struct heap *) SYM_VALUE(Pinned_heap);

	/* summary usage of kernel heap
	 */
	printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_1, "Kernel heap usage \n") );
	memuse(kernheap, 1);

	/* summary usage of pinned_heap 
	 */
	printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_2, "Pinned heap usage \n") );
	memuse(pinheap, 1);

	/* usage by requestor if available
	 */
	printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_3, "Kernel and pinned heap usage \n") );
	memuse(kernheap, 0);

}

memuse(heapptr,summary)
struct heap ** heapptr;
int summary;	/* true for summary , false for detailed */
{
	struct heap *hptr;
	struct record **hash;
	struct record *rp;
	struct record rec;
	struct heap hp;
	int rc,len, k, max, n;

	/* get the pointer to the heap structure
	 */
	if(readmem (&hptr, heapptr, 4, CURPROC) != 4){
		printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_4, "0452-412: Cannot read pointer to heap at address 0x%8x\n") ,heapptr);
		return -1;
	}

	/* read in the heap structure.
	 */
	if(readmem( &hp, hptr, sizeof (struct heap), CURPROC) 
	   != sizeof (struct heap)){
		printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_5, "0452-413: Cannot read heap from address 0x%8x\n") ,hptr);
		return -1;
	}

	/* print out summary information
	 */
	if (summary)
		printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_6, "heap size = %8d amount used = %8d  \n") ,
			hp.numpages<<PGSHIFT, hp.amount);

	/* return if summary only or no detailed records available.
	 */
	if ((hash = hp.rhash) == 0 || summary ) 
		return;

	/* init static data structures
	 */
	freestat = 1;
	for (k = 0; k < NHASH; k++)
		hstat[k] = 0;

	len = sizeof (struct record);
	for (k = 0; k < 256; k++, hash++)
	{
		if(readmem(&rp, hash, 4, CURPROC) != 4){
			printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_7, "0452-415: Cannot read hash table at address 0x%8x\n") ,hash);
			return -1;
		}
		while (rp)
		{
			if (len != readmem(&rec, rp, len, CURPROC))
			{
				printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_8, "0452-416: Cannot read record at address 0x%8x.\n") ,rp);
				return -1;	
			}
			if (rc = updatestats(&rec)) 
				return rc;
			rp = rec.next;
		}
	}

	/* output data sorted by bytes.
	 */
	for (k = 1; k < freestat; k ++)
	{
		max = k;
		for (n = k + 1; n < freestat; n++)
		{
			if (mstat[max].bytes >= mstat[n].bytes)
				continue;
			max = n;
		}
		printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_9, "from = %8x bytes = %8d number = %8d \n") ,
			mstat[max].from,mstat[max].bytes, mstat[max].num);

		mstat[max] = mstat[k];
	}

}

/* 
 * updatestats(&rec)
 * process a xmalloc record.
 */

int
updatestats(rp)
struct record *rp;
{
	uint hc,k;
	
	/* is there already a mstat structure for "from".
	 */
	hc = (NHASH - 1) & (rp->from);
	for (k = hstat[hc]; k; k = mstat[k].next)
	{
		if (rp->from == mstat[k].from)
			break;
	}

	if (k == 0)
	{
		/* allocate a stats structure
		 */
		if (freestat == NSTAT)
		{
			printf( catgets(scmc_catd, MS_crsh, XMALLOC_MSG_10, "Not enough mstat structures.\n") );
			return -1;
		}
		k = freestat;
		freestat += 1;

		/* insert k on hash chain and init it.
		 */
		mstat[k].next = hstat[hc];
		hstat[hc] = k;
		mstat[k].from = rp->from;
		mstat[k].num = mstat[k].bytes = 0;
	}

	/* update stats in stat[k].
	 */
	mstat[k].num += 1;
	mstat[k].bytes += rp->req_size;
	return 0;
}

