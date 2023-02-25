static char sccsid[] = "@(#)19	1.10  src/rspc/usr/lib/methods/cfgasync_rspc/cfgasync_rspc.c, rspccfg, rspc41J, 9523C_all 6/2/95 16:20:04";
/*
 *   COMPONENT_NAME: rspccfg
 *
 *   FUNCTIONS: generate_minor
 *		build_dds
 *		define_children
 *		download_microcode
 *		make_special_files
 *		query_vpd
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
  This file contains the device dependent code pieces for RSPC native
  serial port config method.

  required entry points:

  generate_minor(char *logical_name,dev_t major)
  make_special_files(char *logical_name,dev_t devno)
  download_microcode(char *logical_name)
  query_vpd(char *newobj,mid_t cfg.kmid,dev_t devno, char *vpd)
  define_children(char *logical_name,int phase)
*/

#include <sys/types.h>
#include <cf.h>
#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/device.h>
#include <sys/str_tty.h>
#include <sys/termiox.h>
#include <sys/sysconfig.h>

#include "sf.h"		/* RSPC serial device DDS def'n	*/
#include "cfgcommon.h"
#include "cfgdebug.h"


extern int Get_Parent_Bus();    /* returns parent bus */

#define	TTYDBG_MODULE	"ttydbg"


#define GETATT( DEST, TYPE, ATTR, CUDV ) {				\
		int rc;							\
		rc = getatt( DEST, TYPE, CuAt_CLASS, PdAt_CLASS,	\
			CUDV.name, CUDV.PdDvLn_Lvalue, ATTR,		\
			(struct attr *)NULL );				\
		if (rc)							\
			return(rc);					\
	}

/*
 * -----------------------------------------------------------------------------
 *                       LOAD_CFGTTYDBG
 * -----------------------------------------------------------------------------
 *
 * loads the module ttydbg for dump and debug.
 *
 * Return code: 0 on succcess, E_LOADEXT or E_SYSCONFIG otherwise
 * -----------------------------------------------------------------------------
 */

load_ttydbg()
{
    struct  cfg_kmod cfg;
    int	error = 0;

    /* initialization */
    cfg.kmid = loadext(TTYDBG_MODULE,FALSE,TRUE);
    if (cfg.kmid == (mid_t)NULL) {			/* not yet loaded */
        if ((cfg.kmid = loadext(TTYDBG_MODULE,TRUE,TRUE)) == (mid_t)NULL) {
            DEBUG_2("load_ttydbg: loadext of %s failed with errno %d\n",
                     TTYDBG_MODULE, errno);
            error = E_LOADEXT;
        }
        else {
            /*****  SYS_CFGKMOD *****/

            cfg.mdiptr = (caddr_t)NULL;
            cfg.mdilen = 0;
            cfg.cmd = CFG_INIT;

            /* call sysconfig() to config the driver */
            if (sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg)) == CONF_FAIL) {
                DEBUG_2("load_ttydbg: sysconfig of %s failed with errno %d\n",
                         TTYDBG_MODULE, errno);
		    if (loadext(TTYDBG_MODULE, FALSE, FALSE) == (mid_t)NULL) {
			DEBUG_0("cfgasync: error unloading driver\n");
		    }
                error = E_SYSCONFIG;
            }
        }
    }

    return(error);

} /* End of load_ttydbg() function */

/*
 * NAME: generate_minor
 *
 * FUNCTION: This function generates device minor number for the specific
 *           device.
 *
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method
 *      to generate device minor number for the specific device.  For this
 *      device, the minor number returned is 0 for s1 and 1 for s2.
 *
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

long
generate_minor(lname, majorno, minorno)
char lname;
long majorno;
long *minorno;

{
	extern long *genminor();    /* generate minor number service */
	long	minor_num;
	long  * minor_ptr;

	DEBUG_0 ("generating minor\n")
	minor_num = (cudv.connwhere[7] - '0' - 1) | 0x8000;
	if ((minor_ptr=genminor(cudv.name, majorno, minor_num, 1, 1, 1)) == NULL) {
        	/* Failed to set minor number requested in CuDvDr Class */
        	DEBUG_1("gen_minor: genminor failed minor=%ld\n",minor_num);
        	return(E_MINORNO);
	}
        *minorno = *minor_ptr;
	return(0);
}

/*
 * NAME: build_dds
 *
 * FUNCTION: This function builds the DDS(Defined Data Structure) for the
 *           RSPC native serial port adapters
 *
 * EXECUTION ENVIRONMENT:
 *
 * This function operates as a device dependent subroutine called by the
 * generic configure method for all devices. It is used to build the dds
 * which describes the characteristics of an adapter to the device driver.
 *
 * NOTES: A pointer to the DDS built and its size are returned to the generic
 *        configure method.
 *
 * RETURNS: Returns 0 for SUCCESS.
 */


build_dds(logical_name,ddsptr,ddslen)
	char *logical_name;
	char **ddsptr;
	int *ddslen;
{

	static struct sf_dds dds;	/* DDS */
	struct CuDv	bus_cudv;	/* CuDv for parent bus */
	int	rc;


	/*
	 * this is a stupid place to put this, but a minor stupidity
	 * in the midst of this will probably go unnoticed.
	 */
	rc = load_ttydbg();
	if (rc)
		DEBUG_1("load_ttydbg failed with %d\n", rc);

	strncpy(dds.rc_name, cudv.name, sizeof (dds.rc_name) - 1);
	dds.rc_type = 0;		/* type 0 only type supported	*/
	dds.rc_flags = SFDDS_DEVICE;	/* device specific portion valid*/

	/* Get adapter attributes */
	GETATT( &dds.rc_conf.port, 'l', "bus_io_addr", cudv )
	GETATT( &dds.rc_conf.irq, 'h', "bus_intr_lvl", cudv )
	GETATT( &dds.rc_conf.xtal, 'l', "frequency", cudv )
	dds.rc_conf.rtrig = 0x03;
	dds.rc_conf.tbc = 0x10;
#ifdef PM_SUPPORT
	/* get power management attributes */
	GETATT( &dds.pm_dds.pm_dev_att,   'i', "pm_dev_att", cudv )
	GETATT( &dds.pm_dds.pm_dev_itime, 'i', "pm_dev_itime", cudv )
	GETATT( &dds.pm_dds.pm_dev_stime, 'i', "pm_dev_stime", cudv )
#endif

	/* Get parent bus device */
	rc = Get_Parent_Bus(CuDv_CLASS, cudv.parent, &bus_cudv);
	if (rc)
		return(rc);

	/* Get bus_id attribute of parent bus */
	GETATT( &dds.rc_conf.bid, 'l', "bus_id", bus_cudv )

	*ddsptr = (void *) &dds;
	*ddslen = sizeof (dds);
	return(0);
}



make_special_files(logical_name,devno)
	char	*logical_name;
	dev_t	devno;
{
	return(E_OK);
}
download_microcode(logical_name)
	char	*logical_name;

{
	return(E_OK);
}
query_vpd(newobj,kmid,devno,vpd)
	char	*newobj;
	mid_t	kmid;
	dev_t	devno;
	char	*vpd;
{
	return(E_OK);
}

/*
 * -----------------------------------------------------------------------------
 *                       DEFINE_CHILDREN
 * -----------------------------------------------------------------------------
 * This fucntion detects and manages children of the asynchronous
 * adapters.
 *
 * Children that need to be configured are detected by an
 * autoconfig attribute value of "available".
 * All children to be configured are returned via stdout to the
 * configuration manager which in turn will eventually invoke the
 * configure method for these child devices.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int define_children(logical_name,phase)
char	*logical_name;		/* logical name of parent device */
int	phase;			/* IPL or runtime flag */
{
	int rc;
	int odmflg;			/* flag for used in getting objects */
	struct CuDv child_cudv;		/* CuDv for child device */
	char auto_config[ATTRVALSIZE];	/* auto config value */
	char sstr[256];			/* search criteria */


	/* Search for customized object with parent = device */
	sprintf(sstr, "parent=%s", logical_name);

	/* Loop until no more children are found */
	odmflg = ODM_FIRST;
	while ((rc =
		(int)odm_get_obj(CuDv_CLASS, sstr, &child_cudv, odmflg)) != 0) {

		if (rc == -1)
			return(E_ODMGET);

		DEBUG_1("define_children: found child %s\n", child_cudv.name);

		odmflg = ODM_NEXT;

		/* If child is to be 'autoconfigured' */
		auto_config[0] = '\0';

		GETATT( &auto_config, 's', "autoconfig", child_cudv )
		if (!strcmp(auto_config, "available")) {
			fprintf(stdout, "%s\n", child_cudv.name);
		};
	} /* End while ((rc = (int)odm_get_obj(...))) */

	return(E_OK);
} /* End int define_children(...) */


/*
 * NAME     : device_specific
 *
 * FUNCTION : This function allows for device specific code to be
 *            executed.
 *
 * NOTES :
 *      This adapter does not have a special task to do, so this routine does
 *      nothing
 *
 * RETURNS : 0
 */

int device_specific()
{
        return(0);
}

