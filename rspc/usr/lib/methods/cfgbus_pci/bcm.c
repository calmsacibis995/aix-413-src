 char sccsid[] = "@(#)13	1.4  src/rspc/usr/lib/methods/cfgbus_pci/bcm.c, rspccfg, rspc41J, 9513A_all 3/28/95 16:08:14";
/*
 *   COMPONENT_NAME: rspccfg Main Bus Configuration module-bcm.c
 *
 *   FUNCTIONS: main
 *              bus_config_method
 *		mk_sp_file
 *	        get_iplcb	
 *	        invoke_adapt_cfg
 *	        output_pkgs
 *	        trace_output
 *	        open_classes	
 *	        close_classes	
 *              set_busid
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/iocc.h>
#include <sys/systemcfg.h>
#include <sys/iplcb.h>
#include <cfgresid.h>

#include "bcm.h"
#include "cfgdebug.h"


extern char *optarg;
int	allpkg = 0 ;
int ipl_phase;
struct CuDv cudv, pcudv;
struct PdDv pddv;
struct  Class   *predev;        /* predefined devices class ptr */
struct  Class   *cusdev;        /* customized devices class ptr */
struct  Class   *preatt;        /* predefined attributes class ptr */
struct  Class   *cusatt;        /* customized attributes class ptr */


main(argc, argv)
int argc;
char **argv;
{
	int i, c, rc;
	int phase = 0;
	char *busname = NULL;
	

	DEBUG_0("in main\n");

	/* check if environment variable set to install all packages */
	if ( !strcmp(getenv("DEV_PKGNAME"),"ALL") )
		allpkg = 1;

	while (EOF != (c = getopt(argc, argv, "12dl:"))) {
		switch (c) {
		case '1':
			phase = PHASE1;
			break;
		case '2':
			phase = PHASE2;
			break;
		case 'd':
			prntflag = TRUE;
			trace_file = fopen( "BUS.out", "w" );
			break;
		case 'l':
			busname = optarg;
			break;
		default :
			exit(E_ARGS);
			break;
		}
	}


	/* Initialize the ODM */
	if (odm_initialize() == -1)
		exit(E_ODMINIT);

	ipl_phase = phase;

	/* Open object classes and keep open for multiple accesses */
	rc = open_classes();
	if (rc == 0)
		rc = bus_config_method(phase, busname);

	/* Close all object classes */
	(void)close_classes();

	/* Terminate the ODM */
	odm_terminate();

	exit( rc );
}


int
bus_config_method(phase, busname) 
int phase;
char *busname;
{
	int rc;
	char bus_devname[100];	/* Bus's /dev name */
	char sstr[80];		/* ODM search string */
	int found=FALSE;	/* Indicates if correct BUC entry found */
	int i;			/* Loop variable for searching for BUC entry */
	int offset;		/* Current offset into BUC table */
	IPL_DIRECTORY	iplcb_dir;	/* IPL control block directory */
	struct buc_info buc;		/* IPL cntrl block buc section */

	DEBUG_0("in bus_config_method\n");


	/* Get the bus's CuDv object */
	sprintf(sstr, "name=%s", busname);
	rc = (int)odm_get_first(CuDv_CLASS, sstr, &cudv);
	if (rc == -1)
		return(E_ODMGET);	/* ODM failure */
	else if (rc == 0)
		return(E_NOCuDv);	/* No CuDv for the bus */

	sprintf( sstr, "uniquetype=%s", cudv.PdDvLn_Lvalue );
	rc = (int)odm_get_first( PdDv_CLASS, sstr, &pddv );
	if( rc == -1 )
		return( E_ODMGET );	/* ODM failure */
	else if( rc == 0 )
		return( E_NOPdDv );	/* No PdDv for the ut */

	/* Get the bus's parent's CuDv object */
	sprintf(sstr, "name=%s", cudv.parent);
	rc = (int)odm_get_first(CuDv_CLASS, sstr, &pcudv);
	if (rc == -1)
		return(E_ODMGET);	/* ODM failure */
	else if (rc == 0)
		return(E_NOCuDvPARENT);	/* No CuDv for the parent */

	/* Set the bus device's LED value */
	if (phase)
		setleds( pddv.led );

	/* Set up bus's /dev name for later use */
	sprintf(bus_devname,"/dev/%s", busname);

	/* Call bus dependent routine. */
	/* For PCI bus this routine gets resid data for PCI bus */
	if ((rc = bus_depend(busname, bus_devname)) != 0)
		return(rc);

	/* Only perform bus config operations if bus is DEFINED */
	if (cudv.status == DEFINED) {

		/* Need to scan BUC table in IPL control block to
		 * find BUC entry corresponding to this bus.  The
		 * bus_dependent_cfg() routine is responsible for
		 * determining if the correct BUC entry has been
		 * found.  The bus_dependent_cfg() routine returns
		 * -1 if BUC entry does not correspond to the bus,
		 * 0 if it does correspond to the bus, and >0 if
		 * there was an error.
		 */

		/*
		 * Read in the IPL Control Block directory
		 * and get offset to first info buc section
		 */
		rc = get_iplcb((char *)&iplcb_dir, 128,
				sizeof(iplcb_dir), MIOIPLCB);
		if (rc)
			return(rc);

		offset = iplcb_dir.buc_info_offset;

		/*
		 * Read in the first buc structure so we can get number
		 * of structs for our loop counter, i
		 */
		rc = get_iplcb((char *)&buc, offset, sizeof(buc), MIOIPLCB);
		if (rc)
			return(rc);

		/*
		 * Loop through buc array structures to find the BUID value
		 */
		for (i = 0; i < buc.num_of_structs; i++) {

			/* Get BUC table entry */
			rc = get_iplcb((char *)&buc, offset, sizeof(buc),
								MIOIPLCB);
			if (rc)
				return(rc);

			/* If BUC entry is for a bus device, call bus dep cfg */
			if ((ulong)buc.device_type == A_BUS_BRIDGE) {
				rc = bus_dependent_cfg(&buc, i, bus_devname);
				if (rc == 0) {
					/* configured bus */
					found = TRUE;
					break;
				}
				else if (rc > 0)
					return(rc);
			}

			/* Point to next BUC entry */
			offset += buc.struct_size;
		} /* end for */

		/* if did not find a buc that matched, can't continue  */
		if ( !found ) {
			return (E_DEVACCESS);
		}

		/* set the bus status to available */
		cudv.status = AVAILABLE;
		if ((int)odm_change_obj(CuDv_CLASS, &cudv) == -1)
			return(E_ODMUPDATE);
	}

	/* discover & define children of the bus */
	rc=define_children( bus_devname, phase, busname );
	  
	return(0);

} /* END bus_config_method */



/*
 * NAME: get_iplcb
 *
 * FUNCTION: Reads "num_bytes" bytes from the IPL control block.
 *           Bytes are read from the offset into the IPL
 *           control block and stored at address "dest".
 *
 * RETURNS:  error code.  0 means no error.
 */
int
get_iplcb(dest, offset, num_bytes, ioctl_type)
char	*dest;
int	offset;
int	num_bytes;
int	ioctl_type;
{
	int		fd;	/* file descriptor */
	MACH_DD_IO	mdd;

	DEBUG_0("in get_iplcb\n");

	if ((fd = open("/dev/nvram", O_RDONLY)) == -1)
		return(E_DEVACCESS);

	mdd.md_addr = offset;
	mdd.md_data = dest;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;


	if (ioctl(fd, ioctl_type, &mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}

	close(fd);
	return(0);
}
/* -------------------------------------------------------------------------*/
/*
 * NAME: invoke_adapt_cfg
 *
 * FUNCTION: called to define children of a bus extender.
 *
 * RETURNS: 0 if customized device information found
 *          >0 if error encountered
 */

int
invoke_adapt_cfg(phase, pddv, cudv)
int     phase;          /* IPL phase */
struct PdDv *pddv;      /* PdDv for card */
struct CuDv *cudv;      /* CuDv for card */

{
        int     rc;
        char    cmdarg[DEFAULTSIZE];
        char    *out_ptr = NULL;
        char    *err_ptr = NULL;

	DEBUG_0("in invoke_adapt_cfg\n");


        if (phase != 0)
                sprintf(cmdarg, "-%d -l %s", phase, cudv->name);

        else
                sprintf(cmdarg, "-l %s", cudv->name);

        rc = odm_run_method(pddv->Configure, cmdarg, &out_ptr, &err_ptr);

        /* if outptr not null and length not 0, print ONLY pkg names */
        if (out_ptr != NULL && strlen(out_ptr) != 0 )
                output_pkgs(out_ptr); 

        if (prntflag) {
                fprintf(trace_file,"invoke: %s %s\nrc = %d\n",
                                        pddv->Configure, cmdarg, rc);
                trace_output( out_ptr, err_ptr);
        }

        return(rc);
}

/* ------------------------------------------------------------------------- */
/*
 * NAME: output_pkgs
 *
 * FUNCTION: This function parses the output returned from a cfg method
 * and print the package name to stdout.  Package names begin with ":"

 * and may be interspersed with logical device names.  The package name
 * is used by cfgmgr to determine installation packages.
 *
 *
 * RETURNS: 0
 *
 */
int
output_pkgs( outptr )
char    *outptr;                        /* ptr to stdout string produced by */
{                                       /* cfg method                       */
        int     i, k;
        char    buf[FNAME_SIZE];        /* temp buffer for pkg name */


	DEBUG_0("in output_pkgs\n");

        i = k = 0 ;
        /* loop through output from run method */

        while (outptr[i] != NULL) {
                /* ':' designates beginning of a pkg name */
                if (outptr[i]  != ':' ){
                        i++;
                        continue;
                }
                /* found start of pkg name, build buffer to print */
                while ( outptr[i] != ' '  &&
                        outptr[i] != '\n' &&
                        outptr[i] != '\t' &&
                        outptr[i] != ','  ) {
                        buf[k] = outptr[i];
                        i++;
                        k++;
                }
                buf[k] = '\0';
                /* print pkg name */
                printf("%s ",buf);
                k=0;
        } /* end while */
        return (0);

}


/* ------------------------------------------------------------------------- */
/*
 * NAME: trace_output
 *
 * FUNCTION: This function parses the output returned from a cfg method
 * and print the package name to stdout.  Package names begin with ":"
 * and may be interspersed with logical device names.  The package name

 * is used by cfgmgr to determine installation packages.
 *
 *
 * RETURNS: 0
 *
 */
int
trace_output( outptr, errptr )
char    *outptr;                        /* ptr to stdout string */
char    *errptr;                        /* ptr to stderr string */

{
	DEBUG_0("in trace_output\n");


        if (outptr != NULL && strlen(outptr) != 0 )
                fprintf(trace_file,"*** stdout ***\n%s\n", outptr);
        else
                fprintf(trace_file,"*** no stdout ***\n");

        if (errptr != NULL && strlen(errptr) != 0 )
                fprintf(trace_file,"*** stderr ***\n%s\n", errptr);
        else
                fprintf(trace_file,"*** no stderr ***\n");

        return(0);

}

/*
 * NAME: open_classes
 *
 * FUNCTION: Opens the four object classes: PdDv, CuDv, PdAt, CuAt.
 * 	     They are kept open for multiple accesses.
 *
 * RETURNS:  error code.  0 means no error.
 */
int
open_classes ( void )
{

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1)
		return(E_ODMOPEN);

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
		return(E_ODMOPEN);

	/* open predefined attribute object class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1)
		return(E_ODMOPEN);

	/* open customized attribute object class */
	if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1)
		return(E_ODMOPEN);

	return(0);
}


/*
 * NAME: close_classes
 *
 * FUNCTION: Closes the four object classes: PdDv, CuDv, PdAt, CuAt.
 *
 * RETURNS:  error code.  0 means no error.
 */
int
close_classes ( void )
{
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	return(0);
}


/*
 * NAME: set_busid
 *
 * FUNCTION: Set up "bus_id" attribute for the bus
 *
 * RETURNS: 0 if successful, >0 on error
 */
int
set_busid(buid)
ulong   buid;	/* Bus Id or BUID for bus */
{
	struct  CuAt     *cuat;
	ulong   cuat_busid;
	int     cnt;

        if (prntflag) fprintf(trace_file, "bus id = %x\n",buid);

        /* Check bus_id in CuAt, redefine it if required */
        cuat = getattr(cudv.name, "bus_id", FALSE, &cnt);
        if (cuat == NULL)
                return(E_NOATTR);

        /* See if current busid is correct */
        cuat_busid = strtoul(cuat->value, (char **)NULL, 0);
        if (cuat_busid == buid)
                /* The bus_id is already correct */

               return (0);

        /* Need to set busid to correct value */
        sprintf(cuat->value, "0x%x", buid);

        putattr(cuat) ;
        return(0);
}
