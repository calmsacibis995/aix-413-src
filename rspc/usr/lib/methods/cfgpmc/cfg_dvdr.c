static char sccsid[] = "@(#)70  1.1  src/rspc/usr/lib/methods/cfgpmc/cfg_dvdr.c, pwrsysx, rspc41J, 9510A 3/8/95 04:18:36";
/*
 * COMPONENT_NAME: (PWRSYSX)
 *
 * FUNCTIONS: configure_device, err_exit, cfgpmc_err_exit, unload_pmmd,
 *            get_bus_id, get_pmmd_data
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

/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

/* Local header files */
#include "cfgcommon.h"
#include "cfgdebug.h"
#include "pm_6030.h"

/* definition for PM controller */
#define PMC_BASE	0x4100
#define XIOC_BASE	0x15e8
#define INTR_LEVEL	11
#define INTR_PRIORITY	4
#define PCI_BUS		"bus/sys/pci"
#define ISA_BUS		"bus/pci/isa"

/* path name of PM machine dependent (PMMD) part */
#define PMMD_PATH	"/usr/lib/drivers/pmmd_6030"

extern int	errno;

static int	started_pmmi = 0;	/* whether PMMI is started or not */
static int	loaded_pmmd = 0;	/* whether PMMD is loaded or not */


/*
 * NAME:  configure_device
 *
 * FUNCTION: Loads and initializes PM machine independent (PMMI) part and
 *           PM machine dependent (PMMD) part.
 *
 * DATA STRUCTURES:
 *        cfg_kmod - Structure for calling a kernel externsion's module
 *        cfg_load - Structure for loading a kernel extension
 *                   entry point
 *
 * RETURN VALUE DESCRIPTION:	
 *        E_OK - Successfully loaded and configured.
 *        other than E_OK - error.
 */
int
configure_device()
{
	struct cfg_load			loadpmmd;
	struct cfg_kmod			kmodpmmd;
	struct _pm_ODM_basic_data	pmmd_data;
	int				rc;	/* return codes go here */

	/* load and initialize PM machine independent part */
	DEBUG_0("cfgpmc: Start PMMI\n")
	if(odm_run_method("/usr/lib/methods/startpm", NULL, NULL, NULL) != 0){
		DEBUG_1("cfgpmc: startpm failed %d\n", errno)
		err_exit(E_ODMRUNMETHOD);
	}
	started_pmmi = 1;

	/* get ODM data */
	DEBUG_0("cfgpmc: Get PM controller ODM data\n")
	if(get_pmmd_data(&pmmd_data) != 0){
		DEBUG_1("cfgpmc: get_odm_data() failed %d\n", errno)
		err_exit(E_ODMGET);
	}

	/* load PM machine dependent part */
	loadpmmd.path = PMMD_PATH;
	loadpmmd.libpath = NULL;
	loadpmmd.kmid = 0;
	DEBUG_1("cfgpmc: sysconfig(SYS_KLOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_KLOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpmc: sysconfig(SYS_KLOAD) failed %d\n", errno);
		err_exit(E_LOADEXT);
	}
	loaded_pmmd = 1;

	/* initialize PM machine dependent part */
	kmodpmmd.kmid = loadpmmd.kmid;
	kmodpmmd.cmd = PM_CONFIG;
	kmodpmmd.mdiptr = (caddr_t)&pmmd_data;
	kmodpmmd.mdilen = sizeof(struct _pm_device_id);
	DEBUG_1("cfgpmc: sysconfig(SYS_CFGKMOD) kmid = 0x%x\n", kmodpmmd.kmid);
	if(sysconfig(SYS_CFGKMOD, &kmodpmmd, sizeof(kmodpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpmc: sysconfig(SYS_CFGKMOD) failed %d\n", errno);
		err_exit(E_CFGINIT);
	}

	return(E_OK);
}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */
err_exit(char exitcode)
{
	/* Terminate the ODM */
	odm_terminate();

	exit(exitcode);
}


/*
 * NAME: cfgpmc_err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * NOTES:
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */
static int
cfgpmc_err_exit(char exitcode)
{
	/* unconfigure PM machine independent part */
	if(started_pmmi != 0){
		odm_run_method("/usr/lib/methods/startpm", "-u", NULL, NULL);
	}

	/* unload PM machine independent part */
	if(loaded_pmmd != 0){
		unload_pmmd();
	}

	err_exit(exitcode);
}


/*
 * NAME:  unload_pmmd
 *
 * FUNCTION: Load and configure PM machine independent part kernel extension.
 *
 * DATA STRUCTURES:
 *        cfg_load - Structure for loading a kernel extension
 *        cfg_kmod - Structure for calling a kernel externsion's module
 *                   entry point
 *
 * RETURN VALUE DESCRIPTION:	
 *        E_OK - Successfully loaded and configured.
 *        other than E_OK - error.
 */
static int
unload_pmmd()
{
	struct cfg_load	loadpmmd;

	/* get kmid of PMMD */
	loadpmmd.path = PMMD_PATH;
	loadpmmd.libpath = NULL;
	loadpmmd.kmid = 0;
	DEBUG_1("cfgpmc: sysconfig(SYS_QUERYLOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_QUERYLOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpmc: sysconfig(SYS_QUERYLOAD) failed %d\n", errno);
		return E_LOADEXT;
	}

	if(loadpmmd.kmid == 0){
		DEBUG_0("startpm: sysconfig: loadpmmd.kmid == 0\n");
		return E_LOADEXT;
	}

	/* unload PMMD */
	DEBUG_1("cfgpmc: sysconfig(SYS_KULOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_KULOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpmc: sysconfig(SYS_KULOAD) failed %d\n", errno);
		return E_LOADEXT;
	}

	return E_OK;
}


/*
 * NAME:  get_bus_id
 *
 * FUNCTION: Gets bus_id value from ODM.
 *
 * DATA STRUCTURES:
 *        CuAt object class
 *        CuDv object class
 *
 * RETURN VALUE DESCRIPTION:
 *        non-negative integer: attribute value.
 *        -1:                   data not found.
 */
static int
get_bus_id(char *pddvln)
{
	struct CuDv		cudv;
	struct attr_list	*alist;
	int			bc, val, how_many;
	char			criteria[256];

	sprintf(criteria, "PdDvLn = %s", pddvln);
	if(odm_get_obj(CuDv_CLASS, criteria, &cudv, ODM_FIRST) == NULL){
		DEBUG_1("cfgpmc: odm_get_obj(%s) failed.\n", criteria);
		return -1;
	}

	/* read the attributes from the customized & predefined classes */
	alist = (struct attr_list *)get_attr_list(cudv.name, pddvln,
								&how_many, 10);
	if(alist == NULL){
		DEBUG_1("cfgpmc: get_attr_list(%s) failed.\n", cudv.name);
		return -1;
	}

	if(getatt(alist, "bus_id", &val, 'i', &bc) != 0){
		DEBUG_0("cfgpmc: getatt(bus_id) failed.\n");
		return -1;
	}

	return val;
}


/*
 * NAME:  get_pmmd_data
 *
 * FUNCTION:  Gets default values from ODM and set them to a PM structure.
 *
 * DATA STRUCTURES:
 *        _pm_ODM_basic_data - PM machine dependent part structure
 *                             passing data to kernel extension.
 *
 * RETURN VALUE DESCRIPTION:
 *        0 - Successfully processed.
 *        -1 - Failed to call ODM functions or set appropriate values.
 */
static int
get_pmmd_data(struct _pm_ODM_basic_data *pmmd_data)
{
        /* I/O values */
        pmmd_data->pmc_base = PMC_BASE;
        pmmd_data->xioc_base = XIOC_BASE;
        pmmd_data->intr_level = INTR_LEVEL;
        pmmd_data->intr_priority = INTR_PRIORITY;

	/* ODM data */
        if((pmmd_data->pmc_bus_id = get_bus_id(PCI_BUS)) == -1) return -1;
        DEBUG_1("cfgpmc: pmc_bus_id = 0x%x\n", pmmd_data->pmc_bus_id);

        if((pmmd_data->xioc_bus_id = get_bus_id(ISA_BUS)) == -1) return -1;
        DEBUG_1("cfgpmc: xioc_bus_id = 0x%x\n", pmmd_data->xioc_bus_id);

	return 0;
}
