static char sccsid[] = "@(#)79	1.5  src/rspc/usr/lib/methods/ucfgpm8301/ucfg_dvdr.c, pwrsysx, rspc41J, 9523A_all 6/1/95 18:36:18";
/*
 * COMPONENT_NAME: (PWRSYSX)
 *
 * FUNCTIONS: unconfigure_device, unconfigure_pmmd
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/pm.h>

#include "cfgdebug.h"

extern	int errno;		/* System Error Number */

/*
 * NAME:  unconfigure_device
 *
 * FUNCTION: Unconfigures and unloads PM machine independent (PMMI) part and
 *           PM machine dependent (PMMD) part.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *        E_OK - Successfully loaded and configured.
 *        other than E_OK - error.
 */
int
unconfigure_device(struct PdDv *pddv, struct CuDv *cudv)
{
	pm_system_state_t	pm_system_state;

	/* Must be in FULL_ON state before we unconfigure */
	pm_system_state.state = PM_SYSTEM_FULL_ON;
	pm_control_state(PM_CTRL_START_SYSTEM_STATE, &pm_system_state);

	/* both PMMD and PMMI must be unconfigured. */

	/* unconfigure PM machine dependent */
	DEBUG_0("ucfgpmc: Unconfigure PMMD\n")
	unconfigure_pmmd(pddv);

	/* load and initialize PM machine independent part */
	DEBUG_0("ucfgpmc: Unconfigure PMMI\n")
	if(odm_run_method("/usr/lib/methods/startpm", "-u", NULL, NULL) != 0){
		DEBUG_1("ucfgpmc: startpm failed %d\n", errno)
	}

	return E_OK;
}


/*
 * NAME:  unconfigure_pmmd
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
unconfigure_pmmd(struct PdDv *pddv)
{
	struct cfg_load loadpmmd;
	struct cfg_kmod kmodpmmd;
	char		pmmd_path[256];
	int		rc = E_OK;

	/* get kmid of PMMD */
	sprintf(pmmd_path, "/etc/drivers/%s", pddv->DvDr);
	loadpmmd.path = pmmd_path;
	loadpmmd.libpath = NULL;
	loadpmmd.kmid = 0;
	DEBUG_1("ucfgpmc: sysconfig(SYS_QUERYLOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_QUERYLOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("ucfgpmc: sysconfig(SYS_QUERYLOAD) failed %d\n", errno);
		return E_LOADEXT;
	}

	if(loadpmmd.kmid == 0){
		DEBUG_0("ucfgpmc: sysconfig: loadpmmd.kmid == 0\n");
		return E_LOADEXT;
	}

	/* unconfigure PM machine dependent part */
	kmodpmmd.kmid = loadpmmd.kmid;
	kmodpmmd.cmd = PM_UNCONFIG;
	kmodpmmd.mdiptr = NULL;
	kmodpmmd.mdilen = 0;
	DEBUG_1("ucfgpmc: sysconfig(SYS_CFGKMOD) kmid = 0x%x\n", kmodpmmd.kmid);
	if(sysconfig(SYS_CFGKMOD, &kmodpmmd, sizeof(kmodpmmd)) == CONF_FAIL){
		DEBUG_1("ucfgpmc: sysconfig(SYS_CFGKMOD) failed %d\n", errno);
		/* Both PMMD MUST be unloaded. Do not return. */
		rc = E_LOADEXT;
	}

	/* unload PMMD */
	DEBUG_1("ucfgpmc: sysconfig(SYS_KULOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_KULOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("ucfgpmc: sysconfig(SYS_KULOAD) failed %d\n", errno);
		rc = E_LOADEXT;
	}

        return rc;
}
