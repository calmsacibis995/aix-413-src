static char sccsid[] = "@(#)38  1.1  src/rspc/kernext/pm/pmmi/pmdump/pmhiber.c, pwrsysx, rspc41J, 9510A 3/8/95 11:50:28";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: copy_pmhibernation
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
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/pm.h>
#include "hibernation.h"
#include "pmdumpdebug.h"

/*
 * NAME:  copy_pmhibernation
 *
 * FUNCTION:  makes a private copy of the pm_hibernation_t structure
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      pm_hibernation_t *hiber		input pm_hibernation structure
 *      pm_hibernation_t **pmhiber	returns newly allocated structure
 *
 * RETURN VALUE DESCRIPTION:
 *      0    : success
 *      not 0: error
 */
int
copy_pmhibernation(pm_hibernation_t *hiber, pm_hibernation_t **pmhiber)
{
	register int		n;
	pm_hibernation_t	*pmh;

	DEBUG_1("Enter copy_pmhibernation(0x%x)\n", hiber);

	/* allocate word aligned area for pm_hibernation_t */
	n = sizeof(pm_hibernation_t);
	if((pmh = (pm_hibernation_t *)xmalloc(n, 2, pinned_heap)) == NULL){
		DEBUG_0("Exit copy_pmhibernation(): xmalloc(pmh) error\n");
		return -1;
	}

	/* allocate word aligned area for pm_sector_t */
	n = sizeof(pm_sector_t) * (hiber->snum + 1);
	if((pmh->slist = (pm_sector_t *)xmalloc(n, 2, pinned_heap)) == NULL){
		DEBUG_0("Exit copy_pmhibernation(): xmalloc(slist) error\n");
		return -2;
	}

	/* copy pm_hibernation_t data */
	pmh->devno = hiber->devno;
	pmh->ppnum = hiber->ppnum;
	pmh->ppsize = hiber->ppsize;
	pmh->snum = hiber->snum;
	for(n = 0; n < pmh->snum; n++){
		if(hiber->slist[n].size <= 0){
			DEBUG_0("Exit copy_pmhibernation(): ");
			DEBUG_1("hiber->slist[%d].size <= 0\n", n);
			return -3;
		}
		pmh->slist[n].start = hiber->slist[n].start;
		pmh->slist[n].size = hiber->slist[n].size;
	}
	pmh->slist[n].start = 0;
	pmh->slist[n].size = 0;

	*pmhiber = pmh;
	DEBUG_0("Exit copy_pmhibernation() successfully\n");
	return 0;
}
