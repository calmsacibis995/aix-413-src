static char sccsid[] = "@(#)83  1.8  src/rspc/usr/lib/methods/cfgsys_rspc/cfgsys_rspc.c, rspccfg, rspc41J, 9516B_all 4/21/95 11:19:29";
/*
 *   COMPONENT_NAME: (rspccfg) RSPC System Configuration Method
 *
 *   FUNCTIONS: close_odm_and_terminate
 *		err_exit
 *		get_dev_objs_by_name
 *		get_key
 *		mdd_get
 *		open_odm_and_init
 *		parse_params
 *		set_attrs
 *		setattr
 *		
 *		sysplanar
 *		proc
 *		mem
 *		L2cache
 *		bus
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* interface:
   cfgsys_rspc -l <logical_name> [-<1|2>]
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <fcntl.h>
#include <sys/systemcfg.h>

/* Required for IPL-control block: */
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>

/* NVRAM addresses */
#include <sys/nvdd.h>

/* Local header files */
#include "cfgdebug.h"
#include <cfgresid.h> 

/* external functions */
struct	CuAt	*getattr();
int get_resid_mem(long long *,long long *);

#define SYS0_NAME		"sys0"
#define SYS0_UTYPE		"sys/node/rspc"

#define SYSPLANAR_NAME		"sysplanar0"
#define SYSPLANAR_UTYPE		"planar/sys/sysplanar_rspc"

#define SYSPLANAR_NAME          "sysplanar0"
#define SYSPLANAR_UTYPE1        "planar/sys/sysplanar_rspc"

#define MEM_NAME                "mem0"
#define MEM_UTYPE               "memory/sys/totmem"

#define PROC_UTYPE              "processor/sys/proc_rspc"

#define L2CACHE_NAME            "L2cache0"
#define L2CACHE_UTYPE           "memory/sys/L2cache_rspc"

#define BUS0_NAME               "bus0"
#define BUS_UTYPE               "bus/sys/pci"


#define DEVPKG_PREFIX           "devices"       /* device package prefix */

#define PROCESSORDEVICE         0x80            /* residual bus_id value   */
#define BRIDGECONTROLLER        6               /* residual base_type value*/
#define SYSTEMPERIPHERAL        8               /* residual base_type value*/
#define L2CACHESUB              4               /* residual sub_type value */

#define SYS0_LED		0x811
#define SYSPLANAR_LED           0x811

struct  Class 	*cusdev;	/* customized devices class ptr */
struct	Class	*predev;	/* predefined devices class ptr */
struct	Class	*cusatt;	/* customized attribute class ptr */
struct	Class	*preatt;	/* predefined attribute class ptr */

/* global variables */
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */
int     allpkg = 0;             /* packaging flag               */
long	long    memsize;	/* total memory size */
long	long    good_memsize;	/* total good memory size */

IPL_DIRECTORY	iplcb_dir;	/* IPL control block directory */
IPL_INFO	iplcb_info;	/* IPL control block info section */

/* Input parameters */
char	*logical_name;		/* logical name to configure	*/
int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2	*/

/* Provide prototypes for functions that return no value */
void open_odm_and_init();
void close_odm_and_terminate();
void parse_params();
 
/*
 * NAME: main
 *
 * FUNCTION: Configure system object, define sysplanar device.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This function (program) is invoked by the configuration manager.
 *	Entries are placed in the customized devices object class when they
 *	do not already exist.
 *	
 * 	This cfgmethod configures the sys0 object, defines and configures
 * 	the sysplanar0, mem0, procN, and L2cache0 devices. 
 *	
 *
 */


/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
   int	  rc;			
   void   err_exit();	
   struct CuDv cudv;		/* customized device object storage */
   struct PdAt pdat;		/* predefined attribute object storage */
   char	  sstring[256];		/* search criteria pointer */
   uint   model;		/* model type		   */

   /* parse the parameters */
   parse_params(argc, argv);
   
   /* set the LED values to sys0's values */
   if (ipl_phase != RUNTIME_CFG)
       setleds(SYS0_LED);
   
   /* initialize odm and open the databases */
   open_odm_and_init();


   /* Get IPLCB information and store in global var */
   /* Read in the IPL Control Block directory */
   rc = mdd_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB);
   if (rc) {
   	err_exit(rc);
   }
 
   /* Read in the IPL info section of the Control Block 
      for later use */
   rc = mdd_get(&iplcb_info,iplcb_dir.ipl_info_offset,
   		sizeof(iplcb_info),MIOIPLCB);
   if (rc) {
   	err_exit(rc);
   }

   /*** PROCESS SYS0 DEVICE ***/
   /* Get sys0 device's CuDv and PdDv objects */

   sprintf(sstring, "name = %s", SYS0_NAME);
   rc = (int)odm_get_first(cusdev,sstring,&cudv);

   if (rc == -1) {
      /* ODM failure */
      DEBUG_0("cfgsys: ODM failure getting CuDv object\n");
      err_exit(E_ODMGET);
   }

   if (rc == 0) {
	DEBUG_1("cfgsys: failed to find CuDv object for %s\n",sstring);
	err_exit(E_NOCuDv);
   }

   /* Determine the memory size for sys0 & mem0 attributes
      Get get the values from residual data library routine.	     */

   rc = get_resid_mem(&memsize, &good_memsize);
   if (rc) {
	DEBUG_0("Error obtaining mem size from residual data\n");
	memsize = 0;
	good_memsize = 0;
   }

   DEBUG_1("Memory size from residual=%lld\n",memsize);
   DEBUG_1("Good Memory size from residual=%lld\n",good_memsize);

   if (ipl_phase == PHASE1) {
   	/* In phase 1 set up sys0's system attributes in database */
   	rc = set_attrs(logical_name,&iplcb_info,&iplcb_dir,model);
   } 
   else {
   	/* This is phase 2 or run time */
   	/* Set kernel parameters to their customized values */
   	rc = setvar(&cudv, ipl_phase);
   	DEBUG_1("setvar returned, rc=%d\n",rc);
   }
   if (rc) {
   	err_exit(rc);
   }

   /* sys0 is now configured, make sure it is AVAILABLE */
   if (cudv.status != AVAILABLE) {
	/* Set to same if currently missing */
	if (cudv.chgstatus == MISSING) {
		cudv.chgstatus = SAME;
	}

	/* Set location code */
	strcpy(cudv.location,"00-00");

	/* Set status to available */
	cudv.status = AVAILABLE;

	/* Update CuDv object */
	rc = (int)odm_change_obj( cusdev, &cudv );
	if (rc == -1) {
		DEBUG_1("Error changing CuDv entry for %s\n",cudv.name);
		err_exit(E_ODMUPDATE);
	}
   }

   /* define and configure the sysplanar device */
   rc=sysplanar();

   close_odm_and_terminate();

   exit(rc);
}



/*
 * sysplanar routine - always defines and configures the sysplanar0 device.
 *		       If the customized device information is not correct,
 *		       this routine corrects the information to match
 *		       the rspc configuration.
 *	 	  
 *		       sysplanar also detects and defines its children
 *		       memory, processor(s), L2cache, and any top-level
 *		       buses.   It uses residual data to determine what
 *		       children are present.
 */
int
sysplanar()
{

   int    rc;
   struct CuDv cudv;            /* customized device object storage     */
   char   sstring[256];         /* odm search string                    */
   ulong  num_dev;              /* residual data number of devices      */
   CFG_DEVICE  *devp;           /* ptr to residual data device array    */


   DEBUG_0("Entering sysplanar routine\n");

   /*** PROCESS SYSPLANAR0 DEVICE ***/
    
   /* check the PdDv and CuDv objects */
   rc = get_dev_objs_by_name(SYSPLANAR_NAME, SYSPLANAR_UTYPE,
                             logical_name,"0",&cudv);

   if (rc) {
        err_exit(rc);
   }

   /* if need to correct CuDv fields then do an odm_change */

   if (cudv.status != AVAILABLE) {
	
	if (cudv.chgstatus == MISSING)
		cudv.chgstatus = SAME;
	cudv.status = AVAILABLE;

	/* Update CuDv object */
	rc = (int)odm_change_obj( cusdev, &cudv );
	if (rc == -1) {
		DEBUG_1("Error changing CuDv entry for %s\n",
              		 cudv.name)
		err_exit(E_ODMUPDATE);
	}
    }

   /* get total number of devices and device array from resid data */
   rc=get_resid_dev(&num_dev, &devp);
   if (rc)
        return(rc);
   DEBUG_1("total number of devices = %d\n",num_dev);

   /* define sysplanar's children */
   if (ipl_phase != PHASE1) {

        /* define/configure memory device */
        rc = mem();
        if (rc) {
                err_exit(rc);
        }
        /* configure processor(s) device */
        rc = proc();
        if (rc) {
                err_exit(rc);
        }


        /* configure L2cache device */
        rc = L2cache(num_dev, devp);
        if (rc) {
                err_exit(rc);
        }
   }

   /* detect/define bus device(s) */
   rc = bus(num_dev, devp);

   free(devp);                  /* must free since library mallocs */
   if (rc) {
        err_exit(rc);
   }
   return(0);
}




/* get_dev_objs_by_name
 *		  
 *	desired_name for device
 *	utype
 *	pname
 *	connection
 *	cudv
 *
 *	This will get a CuDv based on the name field.  If a CuDv
 *	with the appropriate name exists, this routine will change
 *	its other attributes to that of the parameters passed in.
 *	i.e., if bus0 exists in the CuDv database, we will
 *	update it to have the passed in parent_name, uniquetype
 *	and connwhere.   
 *
 *	If a CuDv for this name does not exist (e.g., proc0),
 *	it will create one.
 *
 *	Currently, this routine is used for the following devices:
 *		sysplanar0
 *		mem0
 *		procN
 *		L2cache0
 *		  
 */

int
get_dev_objs_by_name(desired_name, utype, pname, connection, cudv)
char	*desired_name;		/* Device name				*/
char	*utype;			/* Device predefined uniquetype		*/
char	*pname;			/* Parent device logical name 		*/
char	*connection;		/* Connection location on parent device	*/
struct  CuDv	*cudv;		/* Pointer to CuDv object 		*/

{
	int	i;		/* loop control variable */
	int	rc;		/* ODM return code */
	char	sstring[256];	/* the ODM search string */
	struct  PdDv pddv;	/* PdDv structure for processing */
	char	*out1;		/* string for odm_run_method */
	char 	*out2;		

	/* get PdDv based on uniquetype */
	
	sprintf(sstring, "uniquetype = '%s'", utype);
	rc = (int)odm_get_first(predev, sstring, &pddv);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_1("cfgsys: failed to find PdDv object for %s\n",utype);
		return(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_1("cfgsys: ODM failure getting PdDv for %s\n",utype);
		return(E_ODMGET);
	}
	

	for (i=0; i<=1; i++) { 
		/* get CuDv based on desired_name 	*/
		sprintf(sstring, "name = '%s'", desired_name);
		rc = (int)odm_get_first(cusdev,sstring,cudv);
	
		if (rc == -1) {
			/* ODM failure */
			DEBUG_0("cfgsys: ODM failure getting CuDv object\n");
			return(E_ODMGET);
		}
		
		if (rc == 0)
		{
			if (i==1)
				/* We've already tried this once - give up */
				return(E_NOCuDv);
			
			/* No CuDv object found, so define one by 
			   running define method, force location code  */
	
			sprintf( sstring, 
				 "-c %s -s %s -t %s -p %s -w %s -l %s -L %s -d",
				 pddv.class, pddv.subclass, pddv.type,
				 pname, connection, desired_name,
			         "00-00");
	
			DEBUG_2( "Invoking %s method with params:\n%s\n",
						pddv.Define, sstring )
	
			rc = odm_run_method(pddv.Define,sstring,&out1,&out2);
			if (rc) {
				return(E_ODMRUNMETHOD);
			}

			/* now loop back through - we should
			 *  find this cudv just created next time 
			 */
		}
		else {
			/* we found the CuDv! */
			break;
		}
	}

        /* if desired uniquetype not the same as existing, fix CuDv */

        if (strcmp(utype, cudv->PdDvLn_Lvalue) ){
                DEBUG_1 ("Updating fields for %s \n", cudv->name);

                /* set location code */
                strcpy(cudv->location,"00-00");

                /* set up parent name */
                strcpy(cudv->parent,pname);

                /* set up uniquetype */
                strcpy(cudv->PdDvLn_Lvalue,utype);

                /* set up connwhere value */
                strcpy(cudv->connwhere,connection);

                /* clean-up, delete existing CuAt objects for
                   this device */
                sprintf (sstring, "name=%s",cudv->name);

                rc = (int)odm_rm_obj(CuAt_CLASS,sstring);

                if (rc == -1)
                        return(E_ODMDELETE);

		/* Update CuDv object */
		rc = (int)odm_change_obj(cusdev, cudv);
		if (rc == -1) {
			DEBUG_1("Error changing CuDv entry for %s\n",
                 		cudv->name)
			err_exit(E_ODMUPDATE);
		}

        }

	return(0);	
}


/*
 * mem - this routine configures the memory device.  There is only
 *	 one memory device configured, regardless of the number
 *	 of simms, etc.  The size attributes of the mem0 device
 *	 is determined by residual data. 
 */
int
mem()
{
        int     rc;
        struct  CuDv cudv;      /* customized device object storage */
	char	totalmem[10];	/* for setting mem0 size attributes */


        DEBUG_0("Entering memory routine\n");

        rc = get_dev_objs_by_name(MEM_NAME, MEM_UTYPE,
                                  SYSPLANAR_NAME, "M", &cudv);

	if (!rc && cudv.status != AVAILABLE) {

		/* memory sizes already retrieved from residual data*/
		/* memory in M BYTES */

   		sprintf(totalmem,"%lld", good_memsize>>20);
        	DEBUG_1("good memory size attr=%s\n",totalmem)
        	rc=setattr(MEM_NAME, "goodsize", totalmem);
		if (rc)
			return (rc);

   		sprintf(totalmem,"%lld", memsize>>20);
        	DEBUG_1("total memory size attr=%s\n",totalmem)
        	rc=setattr(MEM_NAME, "size", totalmem);
		if (rc)
			return (rc);

		/* if need to correct CuDv fields then do an odm_change */

        	if (cudv.status != AVAILABLE) {

			if (cudv.chgstatus == MISSING) 
                		cudv.chgstatus = SAME;
                	cudv.status = AVAILABLE;

                	/* Update CuDv object */
                	rc = (int)odm_change_obj( cusdev, &cudv );
                	if (rc == -1) {
                        	DEBUG_1("Error changing CuDv entry for %s\n",
                        		 cudv.name)
                        	err_exit(E_ODMUPDATE);
                	}
        	}
	}

        return (rc);

}/*end of mem */



/* proc - obtains residual data and configures the number of
 *        CPU processors defined.   This routine also customizes
 *	  the state and type attributes per processor.
 *
 */
int
proc()
{
	int	i,rc;
	struct	CuDv cudv;	/* customized device object storage     */
	CFG_CPU	*cpup;		/* residual data buffer for CPUs        */
	ushort	num_cpus;	/* residual data number of CPUs         */
	char	proc_name[16];  /* processor name buffer                */
	char	proc_conn[16];	/* processor connwhere buffer           */
	char	state[10];	/* processor state in char format       */
	char    type[10];	/* type in char format 			*/

        DEBUG_0("Entering proc routine\n");

        /* get number of cpus and CPU array from residual data */
        rc=get_resid_cpu(&num_cpus, &cpup);
	if (rc)
		return (rc); 

        DEBUG_1("Actual number of cpus= %d\n",num_cpus);
        DEBUG_1("rc from get_resid_cpu= %d\n",rc);

	for (i=0; i<num_cpus; i++){

		/* create the processor device name & connection*/
		sprintf(proc_name, "proc%d", i);
                sprintf(proc_conn, "P%d", i);
        	DEBUG_3("Proc[%d] state=%d, type=%d\n",
			 i,(cpup+i)->cpustate,(cpup+i)->cputype);

                rc = get_dev_objs_by_name(proc_name, PROC_UTYPE,
                                          SYSPLANAR_NAME,
                                          proc_conn, &cudv);

		if (!rc && cudv.status != AVAILABLE) {

			/* set the state custom attr for this proc */
			switch ( (cpup+i)->cpustate) {
				case  0 : strcpy(state,"enable");
					   break;
				case  1 : strcpy(state,"disable");
					   break;
				case  2 : strcpy(state,"faulty");
					   break;
				/* if not 0, 1, 2, error */
				default  : return (E_DEVACCESS);  
			}
       			rc=setattr( proc_name, "state", state);
			if (rc) {
       				free(cpup);
				return(rc);
			}
			/* set the type custom attr for this proc */
       			rc=setattr(proc_name,"type",(cpup+i)->cpuname);
			if (rc) {
       				free(cpup);
				return(rc);
			}

			/* if need to correct CuDv fields then do so  */
       	 		if ( cudv.status != AVAILABLE) {

				if (cudv.chgstatus == MISSING) 
               				cudv.chgstatus = SAME;
               			cudv.status = AVAILABLE;

               			/* Update CuDv object */
               			rc = (int)odm_change_obj( cusdev, 
							  &cudv );
               			if (rc == -1) {
                       			DEBUG_1("Error changing CuDv entry for %s\n",
                       				 cudv.name)
                       			err_exit(E_ODMUPDATE);
               			}	
       			}
		}
	}
        free(cpup);             /* must free since lib routine mallocs */
        return(rc);

} /* end of processor */

/*
 * L2cache - this routine configures the L2cache device.  This is
 *	     only performed if one L2cache device is listed in 
 *	     residual data.   The L2cache size attribute is obtained
 *	     from the L2cache descriptor from residual data.
 */
int
L2cache(num_dev, devp)
ulong   num_dev;                /* number of devices in residual data*/
CFG_DEVICE *devp;               /* ptr to residual data device array */
{
        int     i,rc=0;
        struct  CuDv cudv;      /* customized device object storage  */
	ulong	size;		/* L2 cache size in resid data	     */
	char	csize[10];	/* size in char format		     */


        DEBUG_0("Entering L2cache routine\n");

        for (i=0; i<num_dev; i++){
                if ((devp+i)->deviceid.basetype==SYSTEMPERIPHERAL &&
                    (devp+i)->deviceid.subtype ==L2CACHESUB) {

                        rc = get_dev_objs_by_name(L2CACHE_NAME, L2CACHE_UTYPE,
                                                  SYSPLANAR_NAME, "L", &cudv);

			if (!rc && cudv.status != AVAILABLE) {
				/* get L2cache size from residual data for 
			   	   size attribute */
				rc=get_L2cache_size(i, 'a', &size);
				if (rc) 
					return (rc);

        			sprintf( csize, "%d", size);
        			rc=setattr( L2CACHE_NAME, "size", csize);
				if (rc)
					return(rc);
				/* if need to correct CuDv fields then 
				   do an odm_change */

 			        if (cudv.status != AVAILABLE) {

					if (cudv.chgstatus == MISSING) 
               		 			cudv.chgstatus = SAME;
                			cudv.status = AVAILABLE;

                			/* Update CuDv object */
                			rc =(int)odm_change_obj(cusdev, &cudv);
                			if (rc == -1) {
                        			DEBUG_1("Error changing CuDv entry for %s\n",
                        	 		cudv.name)
                        			err_exit(E_ODMUPDATE);
                			}
        			}
			}
                        /* found 1 L2cache device, done */
                        break;
                }
        }

        return (rc);

}/*end of L2cache */


/* bus
 *
 *      This routine handles the processing of the bus device(s) listed
 *      in residual data.
 */
int
bus(num_dev, devp)
ulong   num_dev;                /* number of devices in residual data   */
CFG_DEVICE *devp;               /* ptr to residual data device array    */
{
        int     i,rc;
        struct  CuDv cudv;      /* customized device object storage     */
        struct  PdDv pddv;      /* predefined device object storage     */
        char    sstring[256];   /* odm search string                    */
        char    *out1;          /* string for odm_run_method            */
        char    *out2;
        CFG_pci_descriptor_t  *bridged;    
				/* return information on pci bus descript*/
	int	numpkts;	/* number of packets returned		*/


DEBUG_0("Entering bus routine\n");

/* loop through all devices to find bus types */
for (i=0; i<num_dev; i++){
        DEBUG_1 ("\n\n***Device number[%d]: \n",i);
        DEBUG_0 ("   Device Id Info    \n");
        DEBUG_1 ("\tBusId - %d \n",(devp+i)->deviceid.busid);
        DEBUG_1 ("\tDevId - 0x%8.8x\n",(devp+i)->deviceid.devid);
        DEBUG_1 ("\tSerial - %d \n",(devp+i)->deviceid.serialnum);
        DEBUG_1 ("\tFlags - %d \n",(devp+i)->deviceid.flags);
        DEBUG_1 ("\tBaseType - %d \n",(devp+i)->deviceid.basetype);
        DEBUG_1 ("\tSubType - %d \n",(devp+i)->deviceid.subtype);
        DEBUG_1 ("\tInterface - %d \n",(devp+i)->deviceid.interface);
        DEBUG_1 ("\tSpare - %d \n",(devp+i)->deviceid.spare);


        DEBUG_0 ("   Bus Access Info    \n");
        DEBUG_1 ("\t[0] - %d \n",(devp+i)->busaccess.pnpaccess.csn);
        DEBUG_1 ("\t[1] - %d \n",(devp+i)->busaccess.pnpaccess.logicaldevnumber)
;
        DEBUG_1 ("\t[2] - %d \n",(devp+i)->busaccess.pnpaccess.readdataport);

        DEBUG_0 ("   PNP Heap Access Info    \n");
        DEBUG_1 ("\tAllocated  - 0x%8.8X\n",(devp+i)->allocatedoffset);
        DEBUG_1 ("\tPossible   - 0x%8.8X\n",(devp+i)->possibleoffset);
        DEBUG_1 ("\tCompatible - 0x%8.8X\n",(devp+i)->compatibleoffset);
        DEBUG_1 ("\tPNP number - %s\n",(devp+i)->pnpid);

        /* get all processor type devices from residual data */
        if ((devp+i)->deviceid.busid   ==PROCESSORDEVICE &&
            (devp+i)->deviceid.basetype==BRIDGECONTROLLER) {

                /* devid is pnp number */
                sprintf(sstring,"subclass=sys AND devid=%s",(devp+i)->pnpid);

                rc = (int)odm_get_first(predev, sstring, &pddv);

                if (rc==-1) {
                        /* ODM failure */
                        DEBUG_1("cfgsys_rspc: ODM failure getting PdDv for %s\n",				 sstring);
                        return(E_ODMGET);
                }
                if (rc==0 || allpkg ) {
                        /* No PdDv object for this device */
                        DEBUG_1("cfgsys_rspc: print pkg name for %s\n",
				 sstring);
                        /* print pkg name */
                        fprintf(stdout, ":%s.sys.%s ",DEVPKG_PREFIX,
                                        (devp+i)->pnpid);

                }
                if (rc !=0) {
                        /* PdDv exists, check for CuDv */

                        /* to obtain CuDv,need bus number to build
                           the connwhere field. Get number from
                           residual data PCI Bridge Descriptor       */

                        rc = get_pci_descriptor(i, 'a', &numpkts, &bridged);
			DEBUG_1("cfgsys_rspc: rc from get_pci_descriptor = %d\n",rc);

                        if (rc) {
                                return (rc);
                        }

                        /* Bridge Desc. contains the bus number */

                        sprintf(sstring,"PdDvLn=%s AND connwhere=%d.%d",
                                pddv.uniquetype, (devp+i)->deviceid.subtype,
                                bridged->busnum);

                        rc = (int)odm_get_first(cusdev, sstring, &cudv);
                        if (rc == -1) {
                                /* ODM failure */
                                DEBUG_0("cfgsys_rspc: ODM failure getting CuDv object\n");
                                return(E_ODMGET);
                        }

                        if (rc == 0) {
                                /* No CuDv object found so define one  */

                                sprintf( sstring, "-c %s -s %s -t %s -p %s -w %d.%d -d -L %s",
					pddv.class, pddv.subclass,
                                        pddv.type, SYSPLANAR_NAME,
                                        (devp+i)->deviceid.subtype,
                                        bridged->busnum,
					"00-00");

                                DEBUG_2( "Invoking %s method with params:\n%s\n",
					  pddv.Define, sstring);

                                rc = odm_run_method(pddv.Define,sstring,
                                                    &out1,&out2);

                                if (rc) {
                                        return(E_ODMRUNMETHOD);
                                }

                        }
                        else { /* found CuDv */
                                out1=cudv.name;
				DEBUG_0("cfgsys_rspc:found bus CuDv object\n");
                                /* Set to same if currently missing */
                                if (cudv.status != AVAILABLE ) {
                                   if (cudv.chgstatus == MISSING) {
                                        cudv.chgstatus = SAME;
                                        /* Update CuDv object */
                                        rc=(int)odm_change_obj(cusdev, &cudv);
                                        if (rc==-1) {
                                                DEBUG_1("Error changing CuDv entry for %s\n", cudv.name)
                                                err_exit(E_ODMUPDATE);
                                        }
                                    }
                                }
                        }
                        /* print object name to stdout */
                        fprintf(stdout, "%s\n", out1);
                } /* end PdDv exists */

        }/* end if processor */
}/* end for */

return(0);

} /* end of bus processing */


    
/*
 * NAME: set_attrs
 *
 * FUNCTION: Sets sys0's attributes in the database
 *
 * RETURNS:  error code.  0 means no error.
 */
int
set_attrs(lname,iplcb_ptr,iplcb_dir,model)
char		*lname;		/* pointer to name of sys object */
IPL_INFO	*iplcb_ptr;	/* IPL control block info section */
IPL_DIRECTORY	*iplcb_dir;	/* IPL Directory 		*/
uint		model;		/* model type being configured 	*/
{
	char	modelcode[16];	/* for setting model code attribute */
	char	keylock[10];	/* for setting keylock attribute */
	uchar	fwversion[8];	/* for setting firmware (residual) 
				   version attribute */

	int	key;		/* key setting */
	int	rc;		/* return code from subroutine calls */
	char	totalmem[10];	/* for setting sys0 realmem attribute */

	/* set the system attributes */
	DEBUG_0("Setting system attributes\n");

	/* get model code from IPL control block */
	DEBUG_1("modelcode=0x%08x\n",iplcb_ptr->model);
	sprintf( modelcode, "0x%08x", iplcb_ptr->model);
	setattr( lname, "modelcode", modelcode );

	/* Set keylock attribute */
	DEBUG_0("Getting keylock state\n")
        if (rc = get_key(&key)) return(rc);
	switch(key)
	{
	case 0:	/* Undefined */
	case 1:
		strcpy( keylock, "secure" );
		break;
	case 2:
		strcpy( keylock, "service" );
		break;
	case 3:
		strcpy( keylock, "normal" );
		break;
	default:
		DEBUG_0("IMPOSSIBLE VALUE IN SWITCH STATEMENT???\n")
		;
	}
	DEBUG_1("keylock=%s\n",keylock)
	setattr( lname, "keylock", keylock );

	/* Get the residual version & revision */
	rc=get_resid_version(fwversion); 
	if (rc)
		return(rc);

	DEBUG_1("firmware version.revision=%s\n",fwversion)
	setattr( lname, "fwversion", fwversion );

	/* memsize already retrieved from residual data, create an
	   attribute - size in Kbytes */

   	sprintf(totalmem,"%lld", memsize>>10);

	/* If error on residual memory, memsize will = 0!! */
	DEBUG_1("realmem=%s\n",totalmem)
	setattr( lname, "realmem", totalmem );

	return(E_OK);
}



/*
 * NAME: setattr
 *
 * FUNCTION: To set an attribute for a device
 *
 * NOTES: This function uses getattr, and putattr, so default values won't
 *	be placed in CuAt.
 *	The value passed in should be in the same base ( e.g. "0x0" ) as
 *	the default, for putattr to recognize it correctly.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
setattr( lname, aname, avalue )
char	*lname;			/* name of device attribute belongs to */
char	*aname;			/* attribute name */
char	*avalue;		/* attribute value */

{
	struct	CuAt	*cuat;	/* customized attribute object */
	int	how_many;	/* variable needed for getattr call */

	/* routines called */
	struct	CuAt	*getattr();
	int		putattr();

	DEBUG_3("setattr(%s,%s,%s)\n",lname,aname,avalue);

	/* get current attribute setting */
	cuat = getattr(lname, aname, 0, &how_many);
	if (cuat == (struct CuAt *)NULL ) {
		DEBUG_0("ERROR: getattr() failed\n");
		return(E_NOATTR);
	}

	DEBUG_1("Value received by getattr: %s\n", cuat->value );

	/* Only update CuAt object if attr value really changed */
	if (strcmp(cuat->value,avalue)) {
		/* copy new attribute value into cuat structure */
		strcpy( cuat->value, avalue );

		if (putattr( cuat )) {
			DEBUG_0("Error: putattr() failed\n");
			return(E_ODMUPDATE);
		}
	}

	return(0);
}

/*
 * NAME: get_key
 *
 * FUNCTION: Reads keylock state via ioctl and stores it at "dest".
 *
 * RETURNS:  error code.  0 means no error.
 */
int 
get_key(dest)
int *dest;
{
     int        fd;
     MACH_DD_IO mdd;
     ulong      key = 0;

     if ((fd = open("/dev/nvram", O_RDONLY)) == -1) {
             DEBUG_0("Unable to open /dev/nvram")
             return(E_DEVACCESS);
     }

     DEBUG_0("Getting keylock state\n")

     mdd.md_incr = MV_WORD;
     mdd.md_size = 1;
     mdd.md_data = (uchar *)&key;
     if (ioctl(fd, MIOGETKEY, &mdd) == -1) {
             DEBUG_0("Error getting keylock state via MIOGETKEY ioctl")
             close(fd);
             return(E_DEVACCESS);
     }

     close(fd);
     *dest = key & 0x00000003;
     return(0);
}


/*
 * NAME: mdd_get
 *
 * FUNCTION: Reads "num_bytes" bytes from nvram or IPL control block.
 *	     Bytes are read from the offset into nvram or the IPL 
 *           control block and stored at address "dest". 
 *                                                                    
 * RETURNS:  error code.  0 means no error.
 */  

int
mdd_get(dest, offset, num_bytes, ioctl_type)
char	*dest;
int	offset;
int	num_bytes;
int	ioctl_type;
{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;


	if ((fd = open("/dev/nvram", O_RDONLY)) == -1) { 
                DEBUG_0("Unable to open /dev/nvram");
	        return(E_DEVACCESS);
        }

        DEBUG_2("mdd_get: offset=%08x, size=%08x\n",offset,num_bytes);
        DEBUG_1("mdd_get: ioctl type = %d\n",ioctl_type);

        mdd.md_addr = offset;
	mdd.md_data = dest;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;

	DEBUG_0("Calling mdd ioctl\n");

	if (ioctl(fd, ioctl_type, &mdd) == -1) { 
                DEBUG_0("Error reading IPL-Ctrl block");
                close(fd);
                return(E_DEVACCESS);
        }


	close(fd);
	return(0);
}


/*
 * NAME: parse_params
 *                                                                    
 * FUNCTION: parses the parameters and puts them in global variables
 *                                                                    
 */  
void
parse_params(argc, argv)
int argc;			/* num args in argv */
char *argv[];			/* user's input */
{
   int  errflg,c;  	        /* used in parsing parameters   */
   int	rc;			/* return code */

   /* Get the packaging environment variable */
   if (!strcmp(getenv("DEV_PKGNAME"),"ALL"))
        allpkg = 1;

   /*****                                                          */
   /***** Parse Parameters                                         */
   /*****                                                          */

   ipl_phase = RUNTIME_CFG;
   errflg = 0;
   logical_name = NULL;
   
   while ((c = getopt(argc,argv,"l:12")) != EOF) 
   {
   	switch (c) 
   	{
   	case 'l':
   		if (logical_name != NULL)
   			errflg++;
   		logical_name = optarg;
   		break;
   	case '1'  :
   		if (ipl_phase != RUNTIME_CFG)
   			errflg++;
   		ipl_phase = PHASE1;
   		break;
   	case '2'  :
   		if (ipl_phase != RUNTIME_CFG)
   			errflg++;
   		ipl_phase = PHASE2;
   		break;
   	default:
   		errflg++;
   	}
   }

   if (errflg) {
   	/* error parsing parameters */
   	DEBUG_0("cfgsys: command line error\n");
   	exit(E_ARGS);
   }

   /*****                                                          */
   /***** Validate Parameters                                      */
   /*****                                                          */
   /* logical name must be specified */
   if (logical_name == NULL) {
   	DEBUG_0("cfgsys: logical name must be specified\n");
   	exit(E_LNAME);
   }
   
   DEBUG_1("Configuring device: %s\n",logical_name);

   return;

}


/*
 * NAME: open_odm_and_init
 *                                                                    
 * FUNCTION: open the odm object classes
 *		and initialize odm
 *                                                                    
 */  
void
open_odm_and_init()
{
   /* start up odm */
   if (odm_initialize() == -1) {
   	/* initialization failed */
   	DEBUG_0("cfgsys: odm_initialize() failed\n");
   	exit(E_ODMINIT);
   }
   DEBUG_0 ("ODM initialized\n");

   /* open customized devices object class */
   if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
   	DEBUG_0("cfgsys: open class CuDv failed\n"); 
   	err_exit(E_ODMOPEN);
   }

   /* open predefined devices object class */
   if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
   	DEBUG_0("cfgsys: open class PdDv failed\n");
   	err_exit(E_ODMOPEN);
   }

   /* open predefined attribute object class */
   if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1) {
   	DEBUG_0("cfgsys: open class PdAt failed\n");
   	err_exit(E_ODMOPEN);
   }
   
   /* open customized attribute object class */
   if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1) {
   	DEBUG_0("cfgsys: open class PdDv failed\n");
   	err_exit(E_ODMOPEN);
   }

   DEBUG_0 ("Object classes opened\n");

   return;

}

/*
 * NAME: close_odm_and_terminate
 *                                                                    
 * FUNCTION: close the odm object classes
 *		and terminate odm
 *                                                                    
 */  
void
close_odm_and_terminate()
{
   /* close classes and terminate ODM */
   DEBUG_0( "Closing classes, and terminating odm\n");

   if (odm_close_class(predev) == -1) {
   	DEBUG_0("cfgsys: close object class PdDv failed");
	err_exit( E_ODMCLOSE );
   }

   if (odm_close_class(preatt) == -1) {
   	DEBUG_0("cfgsys: close object class PdAt failed");
	err_exit( E_ODMCLOSE );
   }

   if (odm_close_class(cusdev) == -1) {
	DEBUG_0("cfgsys: close object class CuDv failed");
	err_exit( E_ODMCLOSE );
   }

   if (odm_close_class(cusatt) == -1) {
	DEBUG_0("cfgsys: close object class CuAt failed");
	err_exit( E_ODMCLOSE );
   }

   odm_terminate();

   return;

}


/*
 * NAME: err_exit
 *                                                                    
 * FUNCTION: Closes all object classes and terminates ODM on fatal errors.
 *                                                                    
 * RETURNS: NONE
 */  

void
err_exit(error_code)
int	error_code;
{
	odm_close_class(PdDv_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_close_class(CuDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_terminate();
	exit(error_code);
}

