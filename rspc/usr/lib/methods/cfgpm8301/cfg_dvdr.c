static char sccsid[] = "@(#)77	1.3  src/rspc/usr/lib/methods/cfgpm8301/cfg_dvdr.c, pwrsysx, rspc41J, 9515B_all 4/14/95 10:54:37";
/*
 * COMPONENT_NAME: (PWRSYSX)
 *
 * FUNCTIONS: configure_device, err_exit, cfgpm8301_err_exit, unload_pmmd,
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
#include <sys/intr.h>
#include <sys/pm.h>

/* Local header files */
#include "cfgcommon.h"
#include "cfgdebug.h"
#include "pmdi.h"

/* definition for PM controller */
#define PMC_PORT1		0x82a
#define PMC_PORT2		0x82b
#define PMI_REQ_PORT		0x838
#define PMI_INTR_LEVEL		13
#define PMI_INTR_PRIORITY	INTCLASS2

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
	char				pmmd_path[256];
	int				rc;	/* return codes go here */

	/* load and initialize PM machine independent part */
	DEBUG_0("cfgpm8301: Start PMMI\n")
	if(odm_run_method("/usr/lib/methods/startpm", NULL, NULL, NULL) != 0){
		DEBUG_1("cfgpm8301: startpm failed %d\n", errno)
		err_exit(E_ODMRUNMETHOD);
	}
	started_pmmi = 1;

	/* get ODM data */
	DEBUG_0("cfgpm8301: Get PM controller ODM data\n")
	if(get_pmmd_data(&pmmd_data) != 0){
		DEBUG_1("cfgpm8301: get_odm_data() failed %d\n", errno)
		err_exit(E_ODMGET);
	}

	/* load PM machine dependent part */
	sprintf(pmmd_path, "/etc/drivers/%s", pddv.DvDr);
	loadpmmd.path = pmmd_path;
	loadpmmd.libpath = NULL;
	loadpmmd.kmid = 0;
	DEBUG_1("cfgpm8301: sysconfig(SYS_KLOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_KLOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpm8301: sysconfig(SYS_KLOAD) failed %d\n", errno);
		err_exit(E_LOADEXT);
	}
	loaded_pmmd = 1;

	/* initialize PM machine dependent part */
	kmodpmmd.kmid = loadpmmd.kmid;
	kmodpmmd.cmd = PM_CONFIG;
	kmodpmmd.mdiptr = (caddr_t)&pmmd_data;
	kmodpmmd.mdilen = sizeof(struct _pm_ODM_basic_data);
	DEBUG_1("cfgpm8301: sysconfig(SYS_CFGKMOD) kmid = 0x%x\n",
	    kmodpmmd.kmid);
	if(sysconfig(SYS_CFGKMOD, &kmodpmmd, sizeof(kmodpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpm8301: sysconfig(SYS_CFGKMOD) failed %d\n", errno);
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
 * NAME: cfgpm8301_err_exit
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
cfgpm8301_err_exit(char exitcode)
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
	char		pmmd_path[256];

	/* get kmid of PMMD */
	sprintf(pmmd_path, "/etc/drivers/%s", pddv.DvDr);
	loadpmmd.path = pmmd_path;
	loadpmmd.libpath = NULL;
	loadpmmd.kmid = 0;
	DEBUG_1("cfgpm8301: sysconfig(SYS_QUERYLOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_QUERYLOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpm8301: sysconfig(SYS_QUERYLOAD) failed %d\n",
		    errno);
		return E_LOADEXT;
	}

	if(loadpmmd.kmid == 0){
		DEBUG_0("startpm: sysconfig: loadpmmd.kmid == 0\n");
		return E_LOADEXT;
	}

	/* unload PMMD */
	DEBUG_1("cfgpm8301: sysconfig(SYS_KULOAD) %s\n", loadpmmd.path);
	if(sysconfig(SYS_KULOAD, &loadpmmd, sizeof(loadpmmd)) == CONF_FAIL){
		DEBUG_1("cfgpm8301: sysconfig(SYS_KULOAD) failed %d\n", errno);
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
		DEBUG_1("cfgpm8301: odm_get_obj(%s) failed.\n", criteria);
		return -1;
	}

	/* read the attributes from the customized & predefined classes */
	alist = (struct attr_list *)get_attr_list(cudv.name, pddvln,
								&how_many, 10);
	if(alist == NULL){
		DEBUG_1("cfgpm8301: get_attr_list(%s) failed.\n", cudv.name);
		return -1;
	}

	if(getatt(alist, "bus_id", &val, 'i', &bc) != 0){
		DEBUG_0("cfgpm8301: getatt(bus_id) failed.\n");
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
	struct CuDv	cust_obj, bus_obj;
	char		srchstr[256];
	int		rc;

        /* I/O values */
        pmmd_data->pmc_port1 = PMC_PORT1;
        pmmd_data->pmc_port2 = PMC_PORT2;
        pmmd_data->pmi_req = PMI_REQ_PORT;
        pmmd_data->intr_level = PMI_INTR_LEVEL;
        pmmd_data->intr_priority = PMI_INTR_PRIORITY;

	/* ODM data */
	/* get unique type from customized OC */
	sprintf(srchstr,"name = '%s'",cudv.name);
	if ((rc=(int)odm_get_first(CuDv_CLASS,srchstr,&cust_obj)) == 0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;

	rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj);
	if (rc) {
		if (rc == E_PARENT)
			rc = E_NOCuDvPARENT;
		return(rc);
	}

        if((pmmd_data->isa_bus_id = get_bus_id(bus_obj.PdDvLn_Lvalue)) == -1)
		return E_ODMGET;
        DEBUG_1("cfgpm8301: isa_bus_id = 0x%x\n", pmmd_data->isa_bus_id);

	return 0;
}
