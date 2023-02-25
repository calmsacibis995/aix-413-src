static char sccsid[] = "@(#)77  1.4  src/rspc/usr/lib/methods/startpm/startpm.c, pwrsysx, rspc41J, 9510A 3/8/95 04:23:18";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: main, unconfigure_pmmi
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/pm.h>
#include <cf.h>
#include "cfgdebug.h"

/* path name of PM machine independent (PMMI) part */
#define PMMI_PATH       "/usr/lib/drivers/pmmi"

/*
 * NAME:  main
 *
 * FUNCTION: configure/start PM (Power Management)
 *
 * NOTE: -u flag is to unconfigure/unload PM.
 *
 * RETURN VALUE DESCRIPTION:	
 *        E_OK - Succeeded to configure/start PM.
 *        otherwize it exits with an error.
 */
main(int argc, char *argv[])
{
	struct cfg_load loadpmmi;
	struct cfg_kmod	kmodpmmi;

	/* if -u flag is specified, unconfigure PMMI and exit */
	if(strcmp(argv[1], "-u") == 0){
		exit(unconfigure_pmmi(1));
	}

	/* load PM machine independent part */
	loadpmmi.path = PMMI_PATH;
	loadpmmi.libpath = NULL;
	loadpmmi.kmid = 0;
	DEBUG_1("startpm: sysconfig(SYS_KLOAD) %s\n", loadpmmi.path);
	if(sysconfig(SYS_KLOAD, &loadpmmi, sizeof(loadpmmi)) == CONF_FAIL){
		DEBUG_1("startpm: sysconfig(SYS_KLOAD) failed %d\n", errno);
		exit(E_LOADEXT);
	}

	/* initialize PM machine independent part */
	kmodpmmi.kmid = loadpmmi.kmid;
	kmodpmmi.cmd = PM_CONFIG;
	kmodpmmi.mdiptr = NULL;
	kmodpmmi.mdilen = 0;
	DEBUG_1("startpm: sysconfig(SYS_CFGKMOD) kmid = 0x%x\n", kmodpmmi.kmid);
	if(sysconfig(SYS_CFGKMOD, &kmodpmmi, sizeof(kmodpmmi)) == CONF_FAIL){
		DEBUG_1("startpm: sysconfig(SYS_CFGKMOD) failed %d\n", errno);
		unconfigure_pmmi(0);
		exit(E_CFGINIT);
	}

	exit(E_OK);
}


/*
 * NAME:  unconfigure_pmmi
 *
 * FUNCTION:  unconfigure and unload PM machine independent part
 *            
 * DATA STRUCTURES:
 *        int unconfig - flag to specify whethere unconfig is done or not.
 *        cfg_load - Structure to load a kernel extension.
 *        cfg_kmod - Structure to call a kernel externsion.
 *
 * RETURN VALUE DESCRIPTION:	
 *        E_OK - Successfully processed.
 *        other than E_OK - error.
 */
static int
unconfigure_pmmi(int unconfig)
{
	struct cfg_load loadpmmi;
	struct cfg_kmod	kmodpmmi;

	/* get kmid of PMMI */
	loadpmmi.path = PMMI_PATH;
	loadpmmi.libpath = NULL;
	loadpmmi.kmid = 0;
	DEBUG_1("startpm: sysconfig(SYS_QUERYLOAD) %s\n", loadpmmi.path);
	if(sysconfig(SYS_QUERYLOAD, &loadpmmi, sizeof(loadpmmi)) == CONF_FAIL){
		DEBUG_1("startpm: sysconfig(SYS_QUERYLOAD) failed %d\n", errno);
		return E_LOADEXT;
	}

	if(loadpmmi.kmid == 0){
		DEBUG_0("startpm: sysconfig: loadpmmi.kmid == 0\n");
		return E_LOADEXT;
	}
		
	/* unconfig PMMI */
	if(unconfig != 0){
		kmodpmmi.kmid = loadpmmi.kmid;
		kmodpmmi.cmd = PM_UNCONFIG;
		kmodpmmi.mdiptr = NULL;
		kmodpmmi.mdilen = 0;
		DEBUG_1("startpm: sysconfig(SYS_CFGKMOD) kmid = 0x%x\n",
								kmodpmmi.kmid);
		if(sysconfig(SYS_CFGKMOD, &kmodpmmi, sizeof(kmodpmmi)) ==
								CONF_FAIL){
			DEBUG_1("startpm: sysconfig(SYS_CFGKMOD) failed %d\n",
									errno);
			return E_LOADEXT;
		}
	}

	/* unload PMMI */
	DEBUG_1("startpm: sysconfig(SYS_KULOAD) %s\n", loadpmmi.path);
	if(sysconfig(SYS_KULOAD, &loadpmmi, sizeof(loadpmmi)) == CONF_FAIL){
		DEBUG_1("startpm: sysconfig(SYS_KULOAD) failed %d\n", errno);
		return E_LOADEXT;
	}

	return E_OK;
}
