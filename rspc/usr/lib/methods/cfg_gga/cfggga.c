static char sccsid[] = "@(#)23	1.8  src/rspc/usr/lib/methods/cfg_gga/cfggga.c, pcigga, rspc41J, 9516B_all 4/18/95 10:32:37";
/*
 *
 * COMPONENT_NAME: (pcigga) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files,
 *            define_children, get_ODM_info, device_specific
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
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
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"

/*
 * Local include for gga dds structure
 */
#include <ggadds.h>
#include <gga_reg.h>
#include <gga_funct.h>

extern char *malloc();
extern int *genminor();
extern int assign_dsp();

struct gbdds      *dds;      /* pointer to dds structure */

#define DEVID_MAX_LEN          16
#define CONN_MAX_LEN           16
#define BUSN_MAX_LEN           16
#define FAIL -1

get_ODM_info (logical_name, dev_id, where, bus_name)
        char    *logical_name;
        char    *dev_id;
        char    *where;
        char    *bus_name;
{
        struct Class    *p_Class_CuDv;
        struct Class    *p_Class_PdDv;
        struct PdDv     s_PdDv_object;
        struct CuDv     cusobj ;        /* Customized object for device */
        struct CuDv     parobj ;        /* Customized object for parent */
        int             rc;
        extern int      odmerrno;
        char            search_string[256];

        DEBUG_1 ("cfg_gga: logical_name = %s\n", logical_name);

        /*-----------------------------------------------------------------
        | Now, open the customized object class and get the customized obj
        |-----------------------------------------------------------------*/

        DEBUG_0 ("cfg_gga: calling odm_open_class CuDv\n")

        if ((int) (p_Class_CuDv = odm_open_class (CuDv_CLASS)) == FAIL)
        {
                DEBUG_0 ("cfg_gga: open class CuDv failed\n")
                return (E_ODMOPEN);
        }

        /*-----------------------------------------------------------------
        |  next, open the predefined device object class and get the
        |  predefined device object for our device and parent
        |-----------------------------------------------------------------*/

        DEBUG_0 ("cfg_gga: calling odm_open_class PdDv\n")

        if (((int) (p_Class_PdDv = odm_open_class (PdDv_CLASS))) == FAIL)
        {
                DEBUG_1 ("cfg_gga: open class PdDv failed rc=%d.\n", rc)
                odm_close_class (p_Class_CuDv);
                return(E_ODMOPEN);
        }

        DEBUG_1 ("cfg_gga: calling odm_get_first() for device %s.\n",
                 logical_name )

        sprintf (search_string, "name = '%s'", logical_name);

        rc = (int) odm_get_first (     p_Class_CuDv,
                                       search_string,
                                       &cusobj   );

        if ((rc == FAIL) || (rc == NULL))
        {
                DEBUG_2 ("cfg_gga: Get CuDv failed rc=%d 0x%x.\n", rc, rc )
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
                DEBUG_2 ("cfg_gga: Get CuDv failed rc=%d 0x%x.\n", rc, rc )
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_PARENT);
        }

        DEBUG_1 ("cfg_gga: searching PdDv for  %s\n",
                  cusobj.PdDvLn_Lvalue )

        sprintf (    search_string,
                     "uniquetype = '%s'",
                     cusobj.PdDvLn_Lvalue );

        rc = (int) odm_get_first( p_Class_PdDv,
                                  search_string,
                                  &s_PdDv_object  );

        if ((rc == FAIL) || (rc == NULL))
        {
                DEBUG_1 ("cfg_gga: failed to find PdDv object rc=%d.\n", rc)
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_ODMGET);
        }

        /* Copy the information we need from the ODM now */
        strncpy(dev_id, s_PdDv_object.devid, DEVID_MAX_LEN);
        DEBUG_1("cfg_gga: dev_id = %s\n", dev_id);

        strncpy(where, cusobj.connwhere, CONN_MAX_LEN);
        DEBUG_1("cfg_gga: connwhere = %s\n", where);

        strncpy(bus_name, cusobj.parent, BUSN_MAX_LEN);
        DEBUG_1("cfg_gga: bus_name = %s\n", bus_name);

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
struct CuAt     *cusatt;        /* customized attribute ptr and object */
char            dev_id[DEVID_MAX_LEN];
char            where[CONN_MAX_LEN];
char            bus_name[BUSN_MAX_LEN];
ulong           default_color_table [16] = {
                    0x00000000, 0x00FF0000, 0x0000FF00, 0x00F0F000,
                    0x000000F0, 0x00B000B0, 0x0000F0F0, 0x00C8C8C8,
                    0x00787878, 0x00F08020, 0x0050E030, 0x00C08020,
                    0x003060FF, 0x00E030E0, 0x0070F0D0, 0x00FFFFFF };
struct PdAt preobj;
struct CuAt cuat;


        /*
         * Obtain size of device specific dds structure
         */
        DEBUG_0("\n\n\nIn CFGGGA ++++++++debug++++++++++++++++++++++++\n\n\n");
        *size = sizeof(struct gbdds);

        /*
         * allocate the space required for the dds structure
         */
        if( (dds = (struct gbdds *) malloc(*size)) == NULL )
        {
            DEBUG_0("cfg_gga: malloc failed for dds structure");
            return(E_MALLOC);
        }

        /* Initialize the ODM */
        if (odm_initialize () == FAIL)
        {
            /* initialization failed */
            DEBUG_0 ("cfg_gga: odm_initialize() failed\n")
            return (E_ODMINIT);
        }

        if (((lock_id = odm_lock ("/etc/objrepos/config_lock", 0)) == FAIL))
        {
            DEBUG_0 ("cfg_gga: odm_lock() failed\n")
            return (E_ODMLOCK);
        }

        DEBUG_0 ("cfg_gga: ODM initialized and locked\n")

        /*-----------------------------------------------------------------
        | ODM is initialized and locked.
        |-----------------------------------------------------------------*/

        /* Read the needed info from the ODM to find the dev_id */

        rc = get_ODM_info (lname, dev_id, where, bus_name);
        if(rc == 0)
        {
            dds->devid = strtoul(dev_id,NULL,16);

            DEBUG_1("cfg_gga: device id =0x%8.8x\n",dds->devid);

            dds->slot_number = strtoul(where,NULL,10);

            DEBUG_1("cfg_gga: dds->slot_number = %d\n", dds->slot_number);

            DEBUG_1("cfg_gga: bus_name =%s\n",bus_name);
        }
        else
	{
            DEBUG_1 ("cfg_gga: get_ODM_info rc=0x%x\n",rc);
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
                dds->bid = value;
                DEBUG_1("cfg_gga: bid = 0x%8.8x\n", dds->bid);
            }
            else 
	    {
		DEBUG_0 ("cfg_gga: ODM attr: bus_id missing\n");
#ifdef BUS_ID
		dds->bid = BID_VAL(IO_PCI, PCI_IOMEM, 0);
		rc = 0;
		DEBUG_1 ("cfg_gga: assuming bid = %x\n", dds->bid);
#endif
	    }
        }
        if(rc == 0)
        {
            if( get_attrval(lname, "bus_mem_start", (char *)NULL, &value, &rc)
                    != NULL)
            {
                dds->bus_mem_address        = value;
                DEBUG_1("cfg_gga: bus_mem_address = 0x%8.8x\n", dds->bus_mem_address);
            }
            else 
	    {
		DEBUG_0 ("cfg_gga: ODM attr: bus_mem_start missing\n");
	    }
        }

        if(rc == 0)
        {
            if( get_attrval(lname, "scrn_width_mm", (char *)NULL, &value, &rc)
                    == NULL )
            {
                DEBUG_0 ("cfg_gga: ODM attr: screen_width_mm missing\n");
                dds->screen_width_mm = 330;   /* set default */
            }
            else dds->screen_width_mm = value;
            DEBUG_1 ("cfg_gga: screen_width_mm = %d\n", dds->screen_width_mm);

            if( get_attrval(lname, "scrn_height_mm", (char *)NULL, &value, &rc)
                    == NULL )
            {
                DEBUG_0 ("cfg_gga: ODM attr: screen_height_mm missing\n");
                dds->screen_height_mm = 250;   /* set default */
            }
            else dds->screen_height_mm = value;
            DEBUG_1 ("cfg_gga: screen_height_mm = %d\n", dds->screen_height_mm);

            if( get_attrval(lname, "display_mode", (char *)NULL, &value, &rc)
                    == NULL )
            {
                DEBUG_0 ("cfg_gga: ODM attr: display_mode missing\n");
                dds->display_mode = 0x201;   /* set default */
            }
            else dds->display_mode = value;
            DEBUG_1 ("cfg_gga: display_mode = %d\n", dds->display_mode);

            if((cusatt = get_attrval(lname, "display_id", (char *)NULL, &value, &rc))
                    != NULL )
            {
                dds->display_id = value;
                DEBUG_1 ("cfg_gga: display_id = 0x%8.8x\n", dds->display_id);
            }
            else 
	    {
		DEBUG_0 ("cfg_gga: ODM attr: display_id missing\n");
	    }
        }

        if(rc == 0)
        {
            if((rc = get_instance(lname, &value)) == E_OK )
            {
                dds->display_id |= value;
                sprintf(cusatt->value, "0x%08x", dds->display_id);
                if (putattr(cusatt) < 0)
                {
                    DEBUG_0("cfg_gga: putattr failed\n");
                    rc = E_ODMUPDATE;
                }
            }
            else 
	    {
		DEBUG_0 ("cfg_gga: get_instance failed\n");
	    }
        }

        if(rc == 0)
        {
            /*
             * Read in the color table.
             */
            for( i=0; i<16; i++)
            {
                sprintf(temp, "ksr_color%d", i+1);
                if( get_attrval(lname, temp, (char *)NULL, &value, &rc) == NULL )
                {
                    DEBUG_1 ("cfg_gga: ODM attr: ksr_color %d missing\n", i+1);
                    dds->ksr_color_table[i] = default_color_table[i];
                }
                else dds->ksr_color_table[i] = value;
                DEBUG_2 ("cfg_gga: ksr_color_table[%d]\t= 0x%8.8x\n", i+1, dds->ksr_color_table[i]);
            }

            if( get_attrval(lname, "belongs_to", temp, &value, &rc) != NULL )
            {
                /*
                 * Build a customized dependency object for this display
                 * and the lft it belongs to.
                 */
                if( (rc = build_depend(temp, lname)) < 0 )
                {
                    DEBUG_2("cfg_gga: Couldn't add dependency for %s -> %s\n", 
			temp, lname);
                }
                else
                {
                    DEBUG_2("cfg_gga: Added dependency for %s -> %s\n", 
			temp, lname);
                }
            }
            else 
	    {
		DEBUG_0 ("cfg_gga: ODM attr: belongs_to missing\n");
	    }
        }

        if(rc == 0)
        {
	   for (i = 0 ; i < 3 ; i ++)
	   {
              DEBUG_0("extract LFT's DPMS time-outs");
              /*
               * Search the CuAt for LFT's DPMS time-outs values 
               */
              sprintf(temp, "name = lft0 and attribute = pwr_mgr_t%d",i+1);
              if ((rc = (int)odm_get_first(CuAt_CLASS, temp, &cuat)) == 0)
              {
                   DEBUG_1("found no CuAt object for crit=%s\n", temp);
           	   /*
            	   * Didn't find CuAt so search the PdAt for the default value
            	   */

           	   sprintf(temp, "uniquetype = lft/node/lft and attribute = pwr_mgr_t%d",i+1);
           	   if ((rc = (int)odm_get_first(PdAt_CLASS, temp, &preobj)) == 0)
           	   {
                	DEBUG_1("found no PdDv object for crit=%s\n", temp);
           	   }
           	   else if (rc == -1)
           	   {
                	DEBUG_1("ODM failure getting PdDv, crit=%s\n",temp);
           	   }
		   else
		   {
		        dds->dpms_timeouts[i] = atoi(preobj.deflt);
			rc = 0;
		   }
              }
              else if (rc == -1)
              {
                   DEBUG_1("ODM failure getting CuAt, crit=%s\n",temp);
              }
	      else
	      {
	           dds->dpms_timeouts[i] = atoi(cuat.value);
		   rc = 0;
              }
	   }
        }

        if(rc == 0)
        {
            /*
              Copy logical device name to dds for error logging by device.
            */
            strcpy(dds->component, lname);

            *dds_out = (char *)dds;
        }

        odm_unlock(lock_id);

        odm_terminate( );

        return(rc);
}

/*
 * NAME: build_subtype
 *
 * FUNCTION:
 *   build_subtype will create the _subtype and dsp_name CuAt to be 
 *   used by lsdev & lsdisp.
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device dependent configuration routine.
 *
 * RETURNS:
 *   The return vales are
 *     0 - for success
 *     positive return code for failure
 */
build_subtype(lname, dds)
char *lname;                     /* logical name of device */
struct gbdds *dds;               /* pointer to dds structure for return */
{
int             rc;             /* return status */
int             i;              /* loop control variable*/
int             lock_id;        /* odm_lock id returned by odm_lock */
char            crit[80];       /* search criteria string */
ulong           ulong_val;      /* temp variable to get value into */
char            string_val[256];/* temporary string variables */
struct CuAt     *get_attrval(); /* routine to get attribute value */
struct CuAt     *cusatt;        /* customized attribute ptr and object */



       /* Initialize the ODM */
        if (odm_initialize () == FAIL)
        {
            /* initialization failed */
            DEBUG_0 ("cfg_gga: odm_initialize() failed\n")
            return (E_ODMINIT);
        }

        if (((lock_id = odm_lock ("/etc/objrepos/config_lock", 0)) == FAIL))
        {
            DEBUG_0 ("cfg_gga: odm_lock() failed\n")
            return (E_ODMLOCK);
        }

        DEBUG_0 ("cfg_gga: ODM initialized and locked\n")

        /*-----------------------------------------------------------------
        | ODM is initialized and locked.
        |-----------------------------------------------------------------*/

	if ( (( cusatt = get_attrval( lname, "_subtype", string_val,
                    &ulong_val, &rc ) ) != NULL )
                    &&
                    ( rc == E_OK ) )
	{
		if ( dds->vram_config == 0 ) 
			strcpy( cusatt->value, "74" );
		else if ( dds->vram_config == 1 ) 
			strcpy( cusatt->value, "75" );
		else if ( dds->vram_config == 2 ) 
			strcpy( cusatt->value, "78" );
		else 
	        {
		   rc = E_ODMUPDATE;
		   goto done;
		}

		if ( putattr( cusatt ) < 0 )
		{
			rc = E_ODMUPDATE;
			goto done ;
		}
	}

	if ( (( cusatt = get_attrval( lname, "dsp_name", string_val,
                    &ulong_val, &rc ) ) != NULL )
                    &&
                    ( rc == E_OK ) )
	{
		if ( dds->vram_config != 1 ) 
			strcpy( cusatt->value, "S15" );
		else strcpy( cusatt->value, "H10" );

		if ( putattr( cusatt ) < 0 )
		{
			rc = E_ODMUPDATE;
		}
	}
done:
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
    long  create_flags;


    create_flags = ( S_IFMPX | S_IFCHR | S_IRUSR | S_IWUSR | \
                     S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );

    return(mk_sp_file(devno | VIDEO_MINOR, lname, create_flags));
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

        cfgdata.kmid = kmid;
        cfgdata.devno = devno;
        cfgdata.cmd = CFG_QVPD;
        cfgdata.ddsptr = (char *)vpd_data;
        cfgdata.ddslen = 256;

        if( (rc = sysconfig( SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd) ))
                == -1 )
        {
                DEBUG_0("query_vpd: sysconfig failed\n");
                return(E_VPD);
        }
        else
                for(i=0; i<256; i++)
                                vpd[i] = vpd_data[i];

        return(E_OK);
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
        return(E_OK);
}

/*
 * NAME     : device_specific
 *
 * FUNCTION : This function allows for graphics specific code to be
 *            executed.
 *
 * NOTES :    For boxes that have s3/864 graphics adapter on the planar,
 *            we need to some processing in the ODM. 
 *             
 *
 * RETURNS : 0        - success
 *           non-zero - error
 */

int device_specific(lname, location_code)
char * lname, * location_code;
{
	int rc = 0;
	char location_slot[5];
       /* 
          location code is formated as follows: "xx-yy".  "xx" has to be "04" for
          PCI bus adapter.  "yy" is the slot number where the device is attached
          to the bus.  The value of zero indicates the device in on the system
          planar.  Note this slot number is not the same as the "connwhere" value
	  which the driver configuration methods calls the slot number.

          Check to make sure caller is a PCI type adapter.  If it is not, we
          don't need to do anything.  It is also good to check caller a graphics
          device but it is not done.

        */	
	if (strncmp("04",location_code,2) != 0)
	{
	   return(0);    
	}

	sprintf(location_slot,"%s",location_code+3);  /* skip "xx-" */

       /*
          This call is necessary for PCI graphics adapter only (except S3/864)
          When this device is detected to be configured for the very first time
          and it is not not on the planar (i.e., its slot # is non-zero) and if
          the console is defined to be the tty then we forces console finder to
          ask users to specify the new console upon the next IPL.  On the other
          hand, the same condition but the console is the tty then and the default
          display is a graphics planar, then we change the default display to be
          of this device
        */

        rc = assign_dsp(lname,location_slot);

        return rc;
}

