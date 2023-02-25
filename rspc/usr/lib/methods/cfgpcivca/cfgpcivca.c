static char sccsid[] = "@(#)41  1.7 src/rspc/usr/lib/methods/cfgpcivca/cfgpcivca.c, pcivca, rspc41J, 9512A_all 3/16/95 19:45:59";
/*
 *
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Adapter Device Driver
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files,
 *            define_children, get_ODM_info
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

/*
 * Include files needed for this module follow
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <sys/ioacc.h>

/*
 * odmi interface include files
 */
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"

/*
 * Local include for vca dds structure
 */
#include <vcacmd.h>
#include <vcadds.h>

#ifdef PM_SUPPORT
#include <sys/pm.h>
#include <sys/pmdev.h>
#endif /* PM_SUPPORT */

extern char *malloc();
extern int *genminor();

struct vca_dds      *dds;      /* pointer to dds structure */


#define CONN_MAX_LEN           16
#define BUSN_MAX_LEN           16
#define FAIL -1
#define MKNOD_MODE (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)


get_ODM_info (logical_name, where, bus_name)
        char    *logical_name;
	char	*where;
	char	*bus_name;
{
        struct Class    *p_Class_CuDv;
        struct Class    *p_Class_PdDv;
        struct PdDv     s_PdDv_object;
        struct CuDv     cusobj ;        /* Customized object for device */
        struct CuDv     parobj ;        /* Customized object for parent */
        int             rc;
        extern int      odmerrno;
        char            search_string[256];

        DEBUG_1 ("cfg_vca: logical_name = %s\n", logical_name);

        /*-----------------------------------------------------------------
        | Now, open the customized object class and get the customized obj
        |-----------------------------------------------------------------*/

        DEBUG_0 ("cfg_vca: calling odm_open_class CuDv\n")

        if ((int) (p_Class_CuDv = odm_open_class (CuDv_CLASS)) == FAIL)
        {
                DEBUG_0 ("cfg_vca: open class CuDv failed\n")
                return (E_ODMOPEN);
        }

        /*-----------------------------------------------------------------
        |  next, open the predefined device object class and get the
        |  predefined device object for our device and parent
        |-----------------------------------------------------------------*/

        DEBUG_0 ("cfg_vca: calling odm_open_class PdDv\n")

        if (((int) (p_Class_PdDv = odm_open_class (PdDv_CLASS))) == FAIL)
        {
                DEBUG_1 ("cfg_vca: open class PdDv failed rc=%d.\n", rc)
                odm_close_class (p_Class_CuDv);
                return(E_ODMOPEN);
        }

        DEBUG_1 ("cfg_vca: calling odm_get_first() for device %s.\n",
                 logical_name )

        sprintf (search_string, "name = '%s'", logical_name);

        rc = (int) odm_get_first (     p_Class_CuDv,
                                       search_string,
                                       &cusobj   );

        if ((rc == FAIL) || (rc == NULL))
        {
                DEBUG_2 ("cfg_vca: Get CuDv failed rc=%d 0x%x.\n", rc, rc )
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_NOCuDv);
        }

        /* Read Customized object of parent */
        sprintf (search_string, "name = '%s'", cusobj.parent );
        rc = (int) odm_get_first (     p_Class_CuDv,
                                       search_string,
                                       &parobj   );

        if ((rc == FAIL) || (rc == NULL))
        {
                DEBUG_2 ("cfg_vca: Get CuDv failed rc=%d 0x%x.\n", rc, rc )
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_PARENT);
        }

        DEBUG_1 ("cfg_vca: searching PdDv for  %s\n",
                  cusobj.PdDvLn_Lvalue )

        sprintf (    search_string,
                     "uniquetype = '%s'",
                     cusobj.PdDvLn_Lvalue );

        rc = (int) odm_get_first( p_Class_PdDv,
                                  search_string,
                                  &s_PdDv_object  );

        if ((rc == FAIL) || (rc == NULL))
        {
                DEBUG_1 ("cfg_vca: failed to find PdDv object rc=%d.\n", rc)
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_ODMGET);
        }

        strncpy(where, cusobj.connwhere, CONN_MAX_LEN);
        DEBUG_1("cfg_vca: connwhere = %s\n", where);

        strncpy(bus_name, cusobj.parent, BUSN_MAX_LEN);
        DEBUG_1("cfg_vca: bus_name = %s\n", bus_name);

        /* Close down the files now */
        odm_close_class( p_Class_PdDv );
        odm_close_class( p_Class_CuDv );

        return(0);
}

/*
 * NAME: build_dds
 *
 * FUNCTION:
 *   build_dds will allocate memory for the dds structure, reporting any
 *   errors, then open the Customized Attribute Class to get the attribute
 *   objects needed for filling the dds structure.
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   The return vales are
 *     0 - for success
 *     positive return code for failure
 */
build_dds(lname, dds_out, size)
char *lname;                     /* logical name of device */
char **dds_out;                  /* pointer to dds structure for return */
int  *size;                      /* pointer to dds structure size */
{
int             rc;             /* return status */
int             i;              /* loop control variable*/
int             lock_id;        /* odm_lock id returned by odm_lock */
char            crit[80];       /* search criteria string */
ulong           value;          /* temp variable to get value into */
char            temp[256];      /* temporary string variables */
struct CuAt     *get_attrval(); /* routine to get attribute value */
char            where[CONN_MAX_LEN];
char            bus_name[BUSN_MAX_LEN];

        /*
         * Obtain size of device specific dds structure
         */
        *size = sizeof(struct vca_dds);

        /*
         * allocate the space required for the dds structure
         */
        if( (dds = (struct vca_dds *) malloc(*size)) == NULL )
        {
            DEBUG_0("cfg_vca: malloc failed for dds structure");
            return(E_MALLOC);
        }

        /* Initialize the ODM */
        if (odm_initialize () == FAIL)
        {
            /* initialization failed */
            DEBUG_0 ("cfg_vca: odm_initialize() failed\n")
            return (E_ODMINIT);
        }

        if (((lock_id = odm_lock ("/etc/objrepos/config_lock", 0)) == FAIL))
        {
            DEBUG_0 ("cfg_vca: odm_lock() failed\n")
            return (E_ODMLOCK);
        }

        DEBUG_0 ("cfg_vca: ODM initialized and locked\n")

        /*-----------------------------------------------------------------
        | ODM is initialized and locked.
        |-----------------------------------------------------------------*/

        rc = get_ODM_info (lname,  where, bus_name);

        if(rc == 0)
        {

        /*
         * Search the connection string for = (i.e. slot=1).  If an =
         * was found, extract the connection from the remaining string,
         * otherwards extract the connection from the original string.
         */

            dds->slot_number = strtoul(where,NULL,10);

            DEBUG_1("cfg_vca: dds->slot_number = %d\n", dds->slot_number);

            DEBUG_1("cfg_vca: bus_name = %s\n", bus_name);

        }
        else
	{
            DEBUG_1 ("cfg_vca: get_ODM_info rc=0x%x\n",rc);
	}

        /*
         * Get the proper dds attribute values from the Customized
         * Attributes class
         */
        if(rc == 0)
        {
            if( get_attrval(bus_name, "bus_id", (char *)NULL, &value, &rc)
                    != NULL)
            {
                dds->bus_number = BID_ALTREG( value, PCI_IOMEM );
                DEBUG_1("cfg_vca: bid = 0x%8.8x\n", dds->bus_number);
            }
            else 
            {   
                DEBUG_0 ("cfg_vca: ODM attr: bus_id missing\n");
            }
        }
        DEBUG_1("cfg_vca: lname = %s\n", lname);
        strcpy(dds->logical_name, lname);
        if(rc == 0)
        {
            if( get_attrval(lname, "bus_io_addr", (char *)NULL, &value, &rc)
                    != NULL)
            {
                dds->bus_io_addr        = value;
                DEBUG_1("cfg_vca: bus_io_addr = 0x%8.8x\n", dds->bus_io_addr);
            }
            else 
	    {
		DEBUG_0 ("cfg_vca: ODM attr: bus_io_addr missing\n");
	    }
        }

        if(rc == 0)
        {
            if( get_attrval(lname, "intr_level", (char *)NULL, &value, 
			&rc) != NULL )
            {
	    	dds->device_intr_level = value;
            	DEBUG_1 ("cfg_vca: device_intr_level = %d\n", 
					dds->device_intr_level);
            }
            else 
	    {
                DEBUG_0 ("cfg_vca: ODM attr: intr_level missing\n");
	    }
	}
        if(rc == 0)
        {
            if( get_attrval(lname, "intr_priority", (char *)NULL, &value,
				 &rc) != NULL )
            {
		dds->device_intr_priority = value;
            	DEBUG_1 ("cfg_vca: device_intr_priority = %d\n", 
					dds->device_intr_priority);
	    }
            else 
	    {
                DEBUG_0 ("cfg_vca: ODM attr: intr_priority missing\n");
            }
	}
#ifdef	PM_SUPPORT
        if(rc == 0)
        {
            if( get_attrval(lname, "pm_dev_itime", (char *)NULL, 
					&value, &rc) != NULL )
            {
                dds->device_idle_time = value;
                DEBUG_1 ("cfg_vca: device_idle_time = 0x%8.8x\n", 
					dds->device_idle_time);
            }
            else 
	    {
		DEBUG_0 ("cfg_vca: ODM attr: device_idle_time missing\n");
	    }
	}
        if(rc == 0)
        {
            if( get_attrval(lname, "pm_dev_stime", (char *)NULL, 
					&value, &rc) != NULL )
            {
                dds->device_stanby = value;
                DEBUG_1 ("cfg_vca: device_stanby = 0x%8.8x\n", 
					dds->device_stanby);
            }
            else 
	    {
		DEBUG_0 ("cfg_vca: ODM attr: device_stanby missing\n");
	    }
        }

        /* PMDEV_CAMERA value */
	dds->pm_ccd_id = PMDEV_CAMERA;

        DEBUG_1("cfg_vca: pm_ccd_id = 0x%8.8x\n", dds->pm_ccd_id);
        DEBUG_1("cfg_vca: rc = %d\n", rc);

#endif	/* PM_SUPPORT */

            /*
              Copy logical device name to dds for error logging by device.
            */

        if(rc == 0)
        {
            *dds_out = (char *)dds;
	}

        odm_unlock(lock_id);

        odm_terminate( );

        return(rc);
}


/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependent routine for generating the device minor number
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from the device independent configuration
 *  method.
 *
 * RETURNS:
 *   minor number success
 *   positive return code on failure
 */
int
generate_minor(lname, majno, minorno)
char  *lname;     /* logical device name */
long   majno;      /* device major number */
long    *minorno;  /* device minor number */
{
        long *minorptr;

        minorptr = genminor(lname, majno, -1, 1, 1, 1);

        if( minorptr == (long *)NULL )
                 return(E_MINORNO);

        *minorno = *minorptr;
        return(E_OK);
}


/*
 * NAME: make_special_files
 *
 * FUNCTION: Device dependant routine creating the devices special files
 *   in /dev
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   0 - success
 */
make_special_files(lname, devno)
char  *lname;     /* logical device name */
dev_t devno;      /* major/minor number */
{

	DEBUG_0(" creating special file for vca\n")
        return(mk_sp_file(devno,lname,MKNOD_MODE));

}




/*
 * NAME: download_microcode
 *
 * FUNCTION: Device dependant routine for downloading microcode
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS:
 *   0 - success
 */
int
download_microcode(lname)
char  *lname;     /* logical device name */
{
        return(E_OK);
}



/*
 * NAME: query_vpd
 *
 * FUNCTION: Device dependent routine for obtaining VPD data for a device
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   0 - success
 *   positive return code for failure
 */
int
query_vpd(cusobj, kmid, devno, vpd)
struct CuDv     *cusobj;    /* customized device object */
mid_t           kmid;       /* driver module id */
dev_t           devno;      /* major/minor number */
char            *vpd;           /* vpd passed back to generic piece */
{
char            vpd_data[256];  /* storage for vpd data */
struct cfg_dd   cfgdata;        /* sysconfig call structure */
int             rc;             /* return code */
int             i;              /* loop variable */

        return(E_OK);
}

/*
 * NAME     : device_specific
 *
 * FUNCTION : This function allows for graphics specific code to be
 *            executed.
 *
 * NOTES :
 *      This adapter does not have a special task to do so this routine does
 *      nothing
 *
 * RETURNS : 0
 */

int device_specific()
{
        return 0;
}

/*
 * NAME: define_children
 *
 * FUNCTION: Routine for detecting and managing children of a logical device
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS:
 *   0 - success
 */
int
define_children(lname, phase)
char  *lname;     /* logical device name */
int   phase;      /* phase of ipl : 0=RUN, 1=PHASE_1, 2=PHASE_2 */
{
       	char    adap_dev[32];           /* adapter device /dev name */
	int	adapter;		/* file descriptor for adapter */
	int	rc;
	struct	ccd_cfg	*ccd_cfg_ptr;	/* for VCA_IOC_CCD_CFG ioctl */
	struct	PdDv	pddvport;
	struct	CuDv	cudvport;
	char		*out_p;
	char		string[512];
	
DEBUG_0("    Enter define_children() for vca\n");

	if (strcmp( lname, "vca0" ) != 0)
		return (E_OK);

        /*-------- Open adapter device ----------*/
        sprintf( adap_dev,"/dev/%s",lname );
        if(( adapter = openx( adap_dev,O_RDWR,0,VCA_CCD_CFG )) < 0){
            DEBUG_1("define_children:can not open %s \n",adap_dev);
            return( E_DEVACCESS );
        }

        /*---  Get configured ccd_status by using VCA_IOC_CCD_CFG ioctl ----*/
DEBUG_0("    Pre ioctl(VCA_IOC_CCD_CFG)\n");
        if( ioctlx( adapter, VCA_IOC_CCD_CFG, ccd_cfg_ptr, VCA_CCD_CFG ) < 0 ) {
		DEBUG_1("    Could not get ccd_status for %s\n",lname);
		close(adapter);
                return( E_DEVACCESS );
        }
DEBUG_1("    Post ioctl(VCA_IOC_CCD_CFG)  ccd = %d\n",ccd_cfg_ptr->ccd_status);
	close(adapter);

        /*------- Check the ccd status -------*/
	if ( ccd_cfg_ptr->ccd_status == CCD_OFF ){
		sprintf(string,"PdDvLn = 'adapter/vca/ccd'");
		if (( rc = odm_get_obj( CuDv_CLASS, string,
				&cudvport, ODM_FIRST )) != 0 ) {
			if (odm_rm_obj(CuDv_CLASS,string) == -1) {
				DEBUG_0("ODM error while restoring data\n");
				return (E_ODMDELETE);
			}
		}
		return (E_OK);
	}

	/*---- Read predef object for ccd port ----*/
	if (( rc = odm_get_obj( CuDv_CLASS,"PdDvLn = 'adapter/vca/ccd'",
				&cudvport, ODM_FIRST )) == NULL ) {
DEBUG_0("    CuDv not defined, so now defined. \n");
	        if (( rc = odm_get_obj( PdDv_CLASS,
				"uniquetype = 'adapter/vca/ccd'",
				&pddvport, ODM_FIRST )) == NULL ) {
DEBUG_2("    Get failed lname=%s rc%d\n", lname, rc);
				return E_NOPdDv;
		}

		sprintf( string,
			"-c adapter -s vca -t %s -p %s -w %d",
			"ccd", lname, 1 );
DEBUG_2("    Calling %s %s\n", pddvport.Define, string );
		if ( odm_run_method(pddvport.Define, string, &out_p, NULL )){
			DEBUG_1("cfgvca: can't run %s\n", pddvport.Define);
			return E_ODMRUNMETHOD;
		}
		if (( rc = odm_get_obj( CuDv_CLASS,
				"PdDvLn = 'adapter/vca/ccd'",
				&cudvport, ODM_FIRST )) == NULL ) {
				return E_NOCuDv;
		}
		if ( cudvport.status == DEFINED ){
			cudvport.status = AVAILABLE;
			if(( rc = odm_change_obj(CuDv_CLASS, &cudvport)) < 0 ){
				DEBUG_0("    Change failed\n");
				return E_ODMUPDATE;
			}
		}
	} else if (rc > 1) {
DEBUG_0("    No mach in CuDv for ccd \n");
		cudvport.status = AVAILABLE;
		/*
		 * Change Customized Device Object Class
                 */
		if (( rc = odm_change_obj(CuDv_CLASS, &cudvport)) < 0 ) {
			DEBUG_0("    Change failed\n");
			return E_ODMUPDATE;
		}
	}
DEBUG_0("    Return to main configure routine\n");
        return(E_OK);
}
