static char sccsid[] = "@(#)39  1.1  src/rspc/kernext/pm/pmmi/pmdump/pmhlv.c, pwrsysx, rspc41J, 9510A 3/8/95 11:50:30";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: check_pmhlv
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
#include <sys/device.h>
#include <sys/dump.h>
#include <sys/adspace.h>
#include <sys/pm.h>
#include "cmdlvm.h"
#include "pmdump.h"
#include "pmdumpdebug.h"

/*
 * NAME:  check_pmhlv
 *
 * FUNCTION:  checks whether PM hibernation volume is valid or not
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      LVCB (Logcal Volume Control Block) of the pm hibernation
 *      logical volume is checked.
 *
 * DATA STRUCTURES:
 *      pm_hibernation_t *pmhp	PM hibernation logical volume information
 *
 * RETURN VALUE DESCRIPTION:
 *      0    : success
 *      not 0: error
 */
int
check_pmhlv(pm_hibernation_t *pmhp)
{
	struct file	*fp;
	char		buf[SECTOR_SIZE];
	struct LVCB	*lvcb = (struct LVCB *)buf;
	int		i;

	DEBUG_1("Enter check_pmhlv(0x%x)\n", pmhp);

	if(pmhp->snum <= 0){	/* no sector */
		DEBUG_0("Exit check_pmhlv(): pmhp->snum <= 0\n");
		return -1;
	}

	/* open the device and get the LVCB (Logical Volume Control Block) */
	if(fp_opendev(pmhp->devno, DREAD, 0, 0, &fp) != 0){
		DEBUG_0("Exit check_pmhlv(): fp_opendev error\n");
		return -2;
	}
	if(fp_lseek(fp, (pmhp->slist[0].start * SECTOR_SIZE), SEEK_SET) != 0){
		DEBUG_0("Exit check_pmhlv(): fp_lseek error\n");
		fp_close(fp);
		return -3;
	}
	if(fp_read(fp, buf, SECTOR_SIZE, 0, SYS_ADSPACE, &i) != 0){
		DEBUG_0("Exit check_pmhlv(): fp_read error\n");
		fp_close(fp);
		return -4;
	}
	fp_close(fp);

	/* Check LVCB data */
	if(memcmp(lvcb->validity_tag, "AIX LVCB", sizeof("AIX LVCB")) != 0 ||
	   memcmp(lvcb->type, "pmhibernation", sizeof("pmhibernation")) != 0 ||
	   memcmp(lvcb->lvname, "pmhibernation", sizeof("pmhibernation"))!= 0 ||
	   lvcb->num_lps != pmhp->ppnum){
		DEBUG_0("Exit check_pmhlv(): LVCB data check error\n");
		return -5;
	}

	DEBUG_0("Exit check_pmhlv() successfully\n");
	return 0;
}
