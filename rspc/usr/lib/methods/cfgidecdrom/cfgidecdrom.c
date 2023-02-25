static char sccsid[] = "@(#)36  1.4  src/rspc/usr/lib/methods/cfgidecdrom/cfgidecdrom.c, rspccfg, rspc41J, 9516A_all 4/17/95 11:43:29";
/*
 * COMPONENT_NAME: (rspccfg) IDE CD-ROM drive
 *			     	Config method
 *
 * FUNCTIONS: generate_minor, make_special_files, download_microcode,
 * FUNCTIONS: query_vpd, define_children, build_dds
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */                                                                   
#define _KERNSYS
#define _RS6K_SMP_MCA
#include <sys/systemcfg.h>
#undef _RS6K_SMP_MCA
#undef _KERNSYS
#include	<stdio.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/device.h>
#include	<sys/sysconfig.h>
#include	<sys/cfgdb.h>
#include	<sys/stat.h>
#include	<sys/scsi.h>
#include	<sys/sysmacros.h>
#include	<fcntl.h>
#include	<malloc.h>
#include	<sys/watchdog.h>
#include	"idecdromdd.h"
#include	<sys/ide.h>
#include	<sys/cfgodm.h>
#include	"cfgdebug.h"
#include	"cfghscsi.h"
#include	"cf.h"
#include 	<string.h>
#include 	<sys/pm.h>

/* global data */

struct	idecdrom_dds	dds_var; /* Global dds structure for IDE-CD-ROM. */
				 /* A ptr to this data is passed back to */
				 /* caller of build_dds.                 */

/* global data for this file */
#define	INQSIZE	255
#define	SEG_SIZE	1000000		/* statistics segment size */
#define	MAX_REQUEST	0x20000		/* Max # bytes / request   */
#define	MAX_COALESCE	0x20000		/* Max # bytes to coalesce */
static	uchar	inq_data[INQSIZE];	/* Storage area for inquiry data */


static void get_vpd_value(uchar*, uchar*, char*, short*, char) ;
int    get_inquiry_data(struct CuDv*, int, uchar*);
static int get_tokens(char*, char*, short*, short*, int*, int*, char*) ;

int    get_dev_id( char*, uchar*);

/*
 * NAME: generate_minor
 *
 * FUNCTION: Generates a minor number for the IDE CD-ROM 
 *	     drive being configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 *
 * RETURNS:
 * The 0, or errno if an error occurs.
 * The minor number is passed back via the pointer in the last parameter
 * 
 */

long generate_minor( logical_name, major_no, minor_dest )
char *logical_name;		/* Name of device			*/
long major_no;			/* Major no. already generated		*/
long *minor_dest;		/* Address to store minor number	*/
{
	long	*genret;
	long *genminor();

	/* Request a minor number. Only one number is required, and	*/
	/* there is no preferred number, nor is there a boundary	*/

	DEBUG_0( "generate_minor()\n" )

	if(( genret=genminor( logical_name, major_no, -1, 1, 1, 1 )) == NULL )
		return E_MINORNO;

	*minor_dest = *genret;
	return 0;
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: Generates two special files for the IDE CD-ROM 
 *	     drive being configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 * 2. The files generated are /dev/<logical_name>, and /dev/r<logical_name>
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */

#define MSP_MAXLEN	14	/* Maximum length of a file name	*/

int make_special_files( logical_name, devno )
char	*logical_name;
dev_t	devno;
{
	char basename[MSP_MAXLEN+1];
	char sstr[100];
	struct CuDv cusobj;
	int perm_mask;
	int rc;

	DEBUG_0( "make_special_files()\n" )

	/* Check that there is enough room in basename for r<logical_name> */

	if( strlen(logical_name) > (MSP_MAXLEN-1) )
		return E_MAKENAME;


	sprintf(sstr,"name = %s",logical_name);
	rc = (int) odm_get_obj(CuDv_CLASS,sstr,&cusobj,ODM_FIRST);
	if ((!rc) || (rc < 0)) {
		/*  Error finding device in CuDv so fail this routine*/
		return E_MAKENAME;
	}
	if (!strncmp(cusobj.PdDvLn_Lvalue,"cdrom",5)) {
		/* 
		 * This is a CD-ROM Device so make the special files 
		 * with the appropriate permissions(444).            
		 */
		perm_mask = S_IRUSR | S_IRGRP | S_IROTH;
	}
	else {
		/* Unknown device class */
		return E_MAKENAME;
	}
	/* Create the character special file */

	sprintf( basename, "r%s", logical_name );

	if( rc = mk_sp_file( devno, basename,
			( perm_mask | S_IFCHR ) ) )
		return(rc);

	/* Generate block file */

	sprintf( basename, "%s", logical_name );

	return( mk_sp_file( devno, basename,
		( perm_mask | S_IFBLK ) ) );
}

/*
 * NAME: download_microcode
 *
 * FUNCTION: Downloads micro-code to device
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *	assumed.
 * 2. The IDE CD-ROMs only use micro-code 
 *	for diagnostics (if at all), therefore the diagnostic programs will 
 *	download their own micro-code.
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */

int download_microcode( lname  )
char	*lname;
{
	DEBUG_0( "download_microcode()\n" );

	return 0;
}

/*
 * NAME: query_vpd
 *
 * FUNCTION: Obtains Vital Product Data From IDE-CD-ROM Device
 *	     and places it in the customized database
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */


int query_vpd( cusobj, kmid, devno, vpd_dest )
struct  CuDv    *cusobj;        /* Pointer to customized object rec             
*/
mid_t   kmid;                   /* Kernel module I.D. for Dev Driver    */
dev_t   devno;                  /* Concatenated Major & Minor No.s      */
char    *vpd_dest;              /* Destination for formatted vpd        */
{
    char        *fmt_str;
    int         vpd_len ;
    int         rc;
    char	device_type;
    int         len ;                   /* length of data returned.     */

    fmt_str = "MF0808C,TM1010C,RL2004C,Z00008X" ;

    vpd_len = idevpd(cusobj, vpd_dest, fmt_str) ;
    if (vpd_len > 0) {
    /* vpd_len > 0 is good, got some/all VPD data */
#ifdef CFGDEBUG
        hexdump(vpd_dest, vpd_len) ;
#endif
        rc = 0 ;
    }
    else {
        DEBUG_0("query_vpd: No VPD received from scvpd\n") ;
        rc = E_VPD ;
    }
    return(rc);
} /* END query_vpd */


/*
 * NAME: define_children
 *
 * FUNCTION: Scans for child devices
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 * 2. The IDE CD-ROMs do not have child 
 *	devices, so this routine always succeeds
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */

int define_children( lname, cusdev, phase, children )
char	*lname;
long	cusdev;
int		phase;
char	*children;
{
	DEBUG_0( "define_children()\n" )
	return 0;
}

/*
 * NAME: build_dds
 *
 * FUNCTION: Device dependent dds build for IDE-CD-ROM
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded routine.
 *
 * RETURNS:	0 if succeed, or errno if fail
 */

#define		CUSTOM	0	/* Customized attribute current value	*/

#ifdef CLOSE_RET
#undef CLOSE_RET
#endif
#define	CLOSE_RET(retval) {	\
		odm_close_class(PdAt_CLASS);	\
		odm_close_class(CuAt_CLASS);	\
		return retval;			\
	}

int build_dds( lname, dds_data_ptr, dds_len )
char	*lname;			/* Logical name				*/
char	**dds_data_ptr;		/* Address to store pointer to dds	*/
int	*dds_len;		/* Address to store length of dds	*/
{
	struct	idecdrom_dds	*dds;	/* ptr for dds for IDE-CD-ROM        */
	char	search_str[128];	/* Search criterion 		     */
	struct	Class	*c_att_oc;	/* Customized Attribute Object Class */
	struct	Class	*p_att_oc;	/* Predefined Attribute Object Class */
	struct	CuDv	cusobj;		/* Customized Device Object 	     */
	int	rc;			/* Return code 			     */
	long	major_no,		/* Major number of adapter 	     */
		minor_no;		/* Major No. for adapter 	     */
	int	how_many;
	int	bc;
	long	*getminor();
	long	genmajor();
	char    data_value[30];
	struct  CuAt   *cuat;
	int	queue_depth;
	uchar   prevent_dflt;
	int	rw_timeout;
	char    tmpstr[20];
	int	i;
	uchar	dev_id;


	DEBUG_0( "build_dds()\n" )

	dds = &dds_var;
	bzero(dds, sizeof(struct idecdrom_dds));
        dds->segment_cnt = 0;
        dds->byte_count = 0;
        dds->segment_size = SEG_SIZE;
	dds->max_request = MAX_REQUEST;


	/* Open required Object Classes */

	if( (int)( c_att_oc = odm_open_class( CuAt_CLASS ) ) == -1 )
	{
		DEBUG_0( "build_dds() for IDE-CD-ROM: Error opening CuAt\n")
		return E_ODMOPEN;
	}

	if( (int)( p_att_oc = odm_open_class( PdAt_CLASS ) ) == -1 )
	{
		DEBUG_0( "build_dds() for IDE-CD-ROM: Error opening PdAt\n")
		return E_ODMOPEN;
	}

	/* Read CuDv for IDE device */

	sprintf( search_str, "name = '%s'", lname);

	DEBUG_1( "Performing odm_get_obj(CuDv,%s)\n", search_str )

	rc = (int)odm_get_obj(CuDv_CLASS,search_str,&cusobj,ODM_FIRST);

	if( rc == 0 )
	{
		DEBUG_1( "cfgsccd: Record not found in CuDv: %s\n",search_str)
		CLOSE_RET( E_NOCuDv )
	} else if ( rc == -1 )
	{
		DEBUG_0( "build_dds() for IDE-CD-ROM: Error Reading CuDv\n")
		CLOSE_RET( E_ODMGET )
	}

	DEBUG_1( "odm_get_obj(CuDv,%s) succeeded\n", search_str )


		
		if (!strncmp(cusobj.PdDvLn_Lvalue,"cdrom",5)) {
			
			/* CD-ROM drive */
			
			/* dds->dev_type = DK_CDROM; */
			prevent_dflt = FALSE;
			rw_timeout = DK_TIMEOUT;
			
		}
		else {
			/* Invalid device class */
			return (E_WRONGDEVICE);
		}
	

		
		/* Read the attributes from the customized & predefined classes */

		if ((rc=getatt(&dds->start_timeout,'h',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "start_timeout",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if start unit timeout is not in ODM, just assume
			 * default.
			 */
			dds->start_timeout = DK_START_TIMEOUT;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		if ((rc=getatt(&dds->rw_timeout,'h',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "rw_timeout",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if read/write time out attribute is not in ODM, 
			 * just assume default.
			 */
			dds->rw_timeout = rw_timeout;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		

		if( ( dds->mode_data_length = - ( rc =
						 getatt(dds->mode_data, 'b', c_att_oc, p_att_oc, lname,
							cusobj.PdDvLn_Lvalue, "mode_data", NULL))) < 0 )
			CLOSE_RET(rc)
				
				if( ( dds->mode_default_length = - ( rc =
								    getatt(dds->mode_default_data, 'b', c_att_oc, p_att_oc, lname,
									   cusobj.PdDvLn_Lvalue, "mode_default", NULL))) < 0 )
					/*
					 * If mode default attribute doesn't exist, don't fail,
					 * just ignore
					 */
					dds->mode_default_length = 0;
		
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "audio_supported",NULL)) == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just set
			 * it to TRUE.
			 */
			dds->play_audio = TRUE;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			if ((!strcmp(data_value,"yes"))) 
				dds->play_audio = TRUE;
			else 
				dds->play_audio = FALSE;
		}
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "m2f1_supported",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->cd_mode2_form1_flag = TRUE;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			if ((!strcmp(data_value,"yes"))) 
				dds->cd_mode2_form1_flag = TRUE;
			else 
				dds->cd_mode2_form1_flag = FALSE;
		}
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "m2f2_supported",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->cd_mode2_form2_flag = TRUE;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			if ((!strcmp(data_value,"yes"))) 
				dds->cd_mode2_form2_flag = TRUE;
			else 
				dds->cd_mode2_form2_flag = FALSE;
		}
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "cdda_supported",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->cd_da_flag = FALSE;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			if ((!strcmp(data_value,"yes"))) 
				dds->cd_da_flag = TRUE;
			else 
				dds->cd_da_flag = FALSE;
		}
		
		if ((rc=getatt(&dds->multi_session,'c',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "multi_session",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->multi_session = TRUE;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		
        if ((rc=getatt(&dds->block_size,'i',c_att_oc, p_att_oc,
                       lname,cusobj.PdDvLn_Lvalue,
                       "block_size",NULL)) == E_NOATTR) {
		
                /*
                 * Do not fail if block_size is not in ODM, just assume
                 * default.
                 */
                dds->block_size = DK_BLOCKSIZE;
        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }
	

        if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc,
                       lname,cusobj.PdDvLn_Lvalue,
                       "prevent_eject",NULL)) == E_NOATTR) {
                /*
                 * don't fail if this attribute is not there, just set
                 * it to FALSE.
                 */
                dds->prevent_eject = prevent_dflt;
        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }
        else {
                if ((!strcmp(data_value,"yes")))
                        dds->prevent_eject = TRUE;
                else
                        dds->prevent_eject = FALSE;
        }

       if ((rc=getatt(&dds->max_coalesce,'i',c_att_oc, p_att_oc,
                       lname,cusobj.PdDvLn_Lvalue,
                       "max_coalesce",NULL)) == E_NOATTR) {
                /*
                 * Do not fail if max_coalesce is not in ODM, just assume
                 * default.
                 */
                dds->max_coalesce = MAX_COALESCE;
        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }


	/* device idle time */
	if ((rc=getatt(&dds->pm_dev_itime, 'i',c_att_oc, p_att_oc,
			lname,cusobj.PdDvLn_Lvalue,
			"pm_dev_itime",NULL)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_itime = 0.
		 */
		dds->pm_dev_itime = 0;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}
	/* device standby time */
	if ((rc=getatt(&dds->pm_dev_stime, 'i',c_att_oc, p_att_oc,
			lname,cusobj.PdDvLn_Lvalue,
			"pm_dev_stime",NULL)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_stime = 0.
		 */
		dds->pm_dev_stime = 0;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}
	/* device attribute */
	if ((rc=getatt(&dds->pm_dev_att, 'i',c_att_oc, p_att_oc,
			lname,cusobj.PdDvLn_Lvalue,
			"pm_dev_att",NULL)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_att = 0. (no attribute)
		 * If no attribute, it is handled as PM_OTHER_DEVICE.
		 */
		dds->pm_dev_att = PM_OTHER_DEVICE;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}

	/* Get ide_location and save in DDS */
	if ( get_dev_id( (char *)&(cusobj.connwhere), &dev_id) )
                return( E_INVCONNECT );
	dds->device_id = dev_id;

	strncpy( dds->resource_name, lname, sizeof(dds->resource_name) );


	/* Read CuDv for parent (IDE-Adapter) */

	sprintf( search_str, "name = '%s'", cusobj.parent );

	DEBUG_1( "performing odm_get_obj(CuDv,%s)\n", search_str )

	if( ( rc=(int)odm_get_obj(CuDv_CLASS,search_str,&cusobj,TRUE)) == 0 )
	{
		DEBUG_1( "build_dds() Record not found in CuDv: %s\n",
			search_str)
		CLOSE_RET( E_NOCuDvPARENT)
	} else if ( rc == -1 )
	{
		DEBUG_0( "build_dds() for IDE-CD-ROM: Error Reading CuDv\n")
		CLOSE_RET( E_ODMGET )
	}

	DEBUG_0( "odm_get_obj() succeeded\n" )

	/* Generate Parent's devno */

	major_no = genmajor( cusobj.ddins );
	minor_no = *getminor( major_no, &how_many, cusobj.name );

	dds->adapter_devno = makedev( major_no, minor_no );

	DEBUG_1( "dds->adapter_devno = %ld\n", dds->adapter_devno)

	*dds_data_ptr = (char *)dds;		/* Store address of struct */
	*dds_len = sizeof(struct idecdrom_dds);	/* Store size of structure */

#ifdef CFGDEBUG
	dump_dds( *dds_data_ptr, *dds_len );
#endif 

	DEBUG_0( "build_dds() returning 0\n" )

	CLOSE_RET(0)
}


 /*
 *---------------------------------------------------------------
 * The following defines are used in get_vpd_value to identify
 * the offsets to place the various parts of a formatted VPD
 * value.
 *---------------------------------------------------------------
 */
#define STAR_OFFSET    0
#define KEYWORD_OFFSET 1
#define LENGTH_OFFSET  3
#define DATA_OFFSET    4
#define VPD_OVERHEAD   DATA_OFFSET

/*
 *---------------------------------------------------------
 * Forward function definitions
 *---------------------------------------------------------
 */
/*
*/
/*
 ************************************************************************
 * NAME     : IDE VPD (idevpd)
 * FUNCTION : Get IDE inquiry data, parse it out into IBM VPD
 *            format, and then save that in CuVPD
 * EXECUTION ENVIRONMENT : This routine is to be run from IDE
 *            device configure methods only.
 * NOTES :
 ************************************************************************
 */
int
idevpd(struct CuDv      *cudv,          /* Ptr to cudv of dvc being config'd */
      char              *vpd_str,       /* ptr to vpd str               */
      char              *def_format)    /* default VPD format string    */
{
    uchar       inq_data[INQSIZE] ;    /* space for inquiry data fr dvc*/
    int         inq_len       ;         /* len of inquiry data          */

    uchar       temp_buf[INQSIZE] ;    /* temp buffer so can calc len  */
    char        *vpd_end  ;             /* ptr to next free byte.       */
    ushort      vpd_len   ;             /* current len of formatted VPD */

    char        sstr[64]  ;             /* search string for ODM queries*/
    char        keyword[3];             /* VPD keyword.                 */
    char        more      ;             /* Loop control flag.           */
    int         code_page ;             /* inq code page value is in    */
    int         prev_page ;             /* code page of previous value  */
    char        dtype     ;             /* data type of the VPD field   */
    short       offset    ;             /* offset of value in inq data. */
    short       len       ;             /* len of value in inq data     */

    int         rc        ;             /* Rtn code from ODM routines   */
    struct PdAt attr      ;             /* PdAt for type 'V' format attr*/

    char        *format_str ;           /* Ptr to format str in use     */

    /* BEGIN scvpd */

    inq_len   = 0 ;
    prev_page = NO_PAGE ;
    code_page = 0;                      /* initialize for first inq loop*/
    vpd_len = 0   ;
    vpd_end = vpd_str;			/* Ptr to last byte written     */

   /*
    *----------------------------------------------------------------------
    * Get the 1st instance of a type='V' attribute for this device.
    * Then go into a loop so that we can handle multiple instances
    * of the type='V' attribute (loop exits when ODM returns 0 - no
    * objects returned).
    *----------------------------------------------------------------------
    */
    sprintf(sstr, "uniquetype=%s and type=V", cudv->PdDvLn_Lvalue) ;
    rc = (int)odm_get_first(PdAt_CLASS, sstr, &attr) ;
   
    do
    {
       /*
        *--------------------------------------------------------------
        * Set format_str to appropriate format string based on
        * results of getting the V attribute.
        *--------------------------------------------------------------
        */
        format_str = ((rc == 0) || (rc == -1)) ? def_format : attr.deflt ;

        more = (char)get_tokens(format_str, keyword, &offset,
                                &len, &code_page, &prev_page, &dtype) ;
        while (more)
        {
           /*
            *--------------------------------------------------------------
            * Must get inquiry data from the device if the code page
            * is different from the last code page specified in the
            * format string.
            * The initial pass, the prev_page is set to be different.
            *--------------------------------------------------------------
            */
            if (prev_page != code_page) 
            {
                inq_len = get_inquiry_data(cudv, code_page, inq_data) ;
            }
           /*
            *--------------------------------------------------------------
            * If there was an error getting inquiry data from the
            * device, inq_len will be zero; therefore, must check
            * to be sure we have inquiry data to work with.
            *--------------------------------------------------------------
            */
	    if ((offset >= 0) && ((offset + len) <= inq_len))
            {
		
                get_vpd_value((uchar *)&temp_buf, inq_data + offset,
                              keyword, &len, dtype ) ;
		if ( (vpd_len + len + VPD_OVERHEAD) > VPDSIZE ) {
			/*
                         *--------------------------------------------
			 * don't have enough room for any more VPD in
			 * buffer for this keyword and the following
			 * words-- return length 
                         *--------------------------------------------
	                 */
			return (vpd_len);
		}
		else {
			bcopy( &temp_buf, vpd_end, len+VPD_OVERHEAD);
			vpd_len += len + VPD_OVERHEAD;
			vpd_end += len + VPD_OVERHEAD;
		}
            }
           /*
            *--------------------------------------------------------------
            * Get the next keyword tokens.
            * If no more tokens in this string, check the "values" field.
            * If we are already processing the "values" field, then
            * we are done with this attribute.
            *--------------------------------------------------------------
            */
            if (!get_tokens(NULL,keyword, &offset, &len,&code_page,
                            &prev_page, &dtype))
            {
                if (format_str == attr.deflt)
                {
                    format_str = attr.values ;
                    more = (char)get_tokens(format_str, keyword, &offset,
                                            &len, &code_page, &prev_page,
                                            &dtype ) ;
                }
                else
                {
                    more = FALSE ;
                }
            }
        } /* END while */

        if (format_str != def_format)
        {
           /*
            *--------------------------------------------------------
            * Check for more "type=V" attributes for this device -
            * ONLY if not using the default format string.
            *--------------------------------------------------------
            */
            rc = (int)odm_get_next(PdAt_CLASS, &attr) ;
        }
    } while ((rc != 0) && (rc != -1)) ;

    return(vpd_len) ;
} /* END scvpd */

/*
 ************************************************************************
 * NAME     : get_vpd_value
 * FUNCTION : Adds indicated inquiry data to formated VPD string with
 *            given keyword prefix.
 * EXECUTION ENVIRONMENT : This routine is with in this module only.
 * NOTES :
 * RETURNS  :
 *            Length of the formated value just added.
 ************************************************************************
 */
static void
get_vpd_value(uchar     *vpd     ,      /* addr at which to put VPD value*/
              uchar     *inq_data,      /* addr of inquiry data to add  */
              char      *keyword ,      /* keyword prefix to use        */
              short     *len,           /* length of inquiry data to add*/
              char      dtype)          /* data type                    */
{

    int       i;
    int       cnt=0;
    int       vpd_offset=0;
    char      hex_digits[] = "0123456789ABCDEF";

    /* BEGIN get_vpd_value */

    bzero(vpd, INQSIZE);

   /*
    *-------------------------------------------------------------
    * Set the VPD value "header".  Has the format of "*KKL"
    * where
    *   *  = character '*'
    *   KK = 2 character VPD keyword
    *   L  = 1 byte length in shorts (16 bit words).
    *
    * Then copy the inquiry data in place after the 'L'.
    *-------------------------------------------------------------
    */

    vpd[STAR_OFFSET] = '*' ;
    bcopy(keyword, &vpd[KEYWORD_OFFSET], 2)  ;

    /*
    *-------------------------------------------------------------
    *  Check the data type of the keyword field.  If it is hex 'X',
    *  change each nibble to get the ascii equivalent and increment
    *  the length.
    *-------------------------------------------------------------
    */

    if( dtype == 'X' || dtype == 'x' ) {
      for ( i=0; i< *len; i++) {

         vpd[DATA_OFFSET + vpd_offset++] = hex_digits[(inq_data[i] & 0xf0 )
                                           >> 4] ;
         vpd[DATA_OFFSET + vpd_offset++] = hex_digits[(inq_data[i] & 0x0f )];
         cnt++ ;
      }  /* end for */
      *len += cnt;                   /* increment the total length         */

    } /* end if */
    else {                           /* dtype is already ascii, no convert */

       bcopy(inq_data, &vpd[DATA_OFFSET], *len) ;

    } /* end else */

    if ( *len%2 != 0 ) {                    /* if len is odd        */
        vpd[DATA_OFFSET + *len] = 0x00;     /* pad with a null      */
        (*len)++;                           /* take one more byte   */
    }

    vpd[LENGTH_OFFSET] = (uchar)((*len +VPD_OVERHEAD) >> 1) ;

} /* END get_vpd_value */

/*
 ***************************************************************
 * NAME     : issue_idecdrom_inquiry
 * FUNCTION : Performs the low level work to get the inquiry
 *            data from the device.
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called only from within
 *    IDE CD-ROM device configuration methods.
 * NOTES    :
 *    - This routine expects the adapter to be opened and started
 *      by the caller.
 * RETURNS:
 *      0           if succeed
 *      E_DEVACCESS if fail
 ***************************************************************
 */
int
issue_idecdrom_inquiry(int     adapfd  ,
                   uchar  *inqdata ,
                   uchar  location ,
                   int    code_page,
		   int    *error)
{
    struct ide_inquiry inq;
    int         try;
    int         tried_async = 0 ;
    int         rc;

/* BEGIN issue_idecdrom_inquiry */

    /* Zero out inquiry command structure */
    bzero(&inq, sizeof(struct ide_inquiry));


    *error = 0;

    /* build inquiry command structure */
    inq.ide_device = location;
    inq.inquiry_len = (uchar)INQSIZE;
    inq.inquiry_ptr = inqdata;

   /*
    *-------------------------------------------------------------------
    *     get_extended  - indicates extended inquiry requested
    *             (TRUE= extended; FALSE=standard).
    *     code_page_num - identifies the code page requested if this is an
    *             extended unquiry.
    *-------------------------------------------------------------------
    */
    if (code_page != (int)NO_PAGE)
    {
        inq.get_extended = TRUE ;
        inq.code_page_num = (uchar)code_page ;
    }

    for (try=0; try<3; try++)
    {
        rc = 0;
        rc = ioctl(adapfd, IDEIOINQU, &inq);
        if (rc == 0)
        {
            return(0);;      /* Successful completion! */
        }

        /* Handle error conditions */
        if (try > 0)
        {
            if ((errno == ENOCONNECT) && (tried_async == 0))
            {
                DEBUG_0("issue_idecdrom_inquiry: inquiry returned ENOCONNECT\n") ;
                DEBUG_0("....now trying ASYNC\n") ;
                inq.flags = SC_ASYNC;
                tried_async = 1;
            }
            else
            {
                DEBUG_1("issue_idecdrom_inquiry: inquiry returned errno: %d\n",
                        errno) ;
                break;
            }
        }
    } /* END for loop */
    if (errno == ENODEV) 
	    *error = ENODEV;
    else
	    *error = -1;

    return(E_DEVACCESS);
} /* END issue_idecdrom_inquiry */



/*
 ************************************************************************
 * NAME     : Get Inquiry Data
 * FUNCTION : Set up and request the adapter for inquiry data.
 * EXECUTION ENVIRONMENT : This routine is to be run from IDE
 *            device configure methods only.
 * NOTES :
 * RETURNS  :
 *            Length of data obtained.
 *            Zero indicates failure.
 ************************************************************************
 */
int
get_inquiry_data(struct CuDv *cudv ,    /* Dvc's customized object      */
                 int    code_page,      /* code page of inq data to get */
                 uchar* inq_data)       /* Rtn addr of ptr to inq bfr   */
{

    uchar       location ;              /* IDE location of device.      */
    int         len ;                   /* length of data returned.     */

    int         fd  ;                   /* file desc for scsi adptr     */
    int         rc  ;
    char        adp_name[32] ;          /* spec file name of scsi adptr */
    int 	error;			/* Error from issue_idecdrom_inquiry*/

/* BEGIN get_inquiry_data */

    /*location = (uchar)strtoul(cudv->connwhere,NULL,10);*/
    location = strtoul(cudv->connwhere,NULL,16);

    sprintf(adp_name, "/dev/%s", cudv->parent) ;
    /*if ((fd = open(adp_name, O_RDWR)) >= 0)*/
    if ((fd = open(adp_name, O_RDONLY)) >= 0)
    {
        if (ioctl(fd, IDEIOSTART, location) == 0)
        {
            /* now get inquiry data */
            if ((rc = issue_idecdrom_inquiry(fd,inq_data,location,code_page,&error)) == 0)
            {
		if (code_page == NO_PAGE) {
                                        /* length of inq data is in position
                                           4; add 5 to it to get total len
                                           of inq data in the buffer       */
               		len = inq_data[4] + 5 ;
		}
		else
		{
                                        /* length of inq data is in position
                                           3; add 4 to it to get total len
                                           of inq data in the buffer       */
               		len = inq_data[3] + 4 ;
		}
#ifdef CFGDEBUG
                /* look at inquiry data */
                hexdump(inq_data, (long)len);
#endif
            }
            else /* error getting inquiry data from drive */
            {
                DEBUG_0("get_inq_data: error issueing inquiry; ") ;
                DEBUG_3("rc=%d adap=%s location %d\n",
                        rc, adp_name, location) ;
                len = 0 ;
            }

            if (ioctl(fd, IDEIOSTOP, location) < 0)
            {
                DEBUG_1("Failed to IDEIOSTOP device at location %d \n",
                (int)location) ;
            }
        }
        else /* Error starting device */
        {
            DEBUG_2(
                "get_inq_data: Failed IDEIOSTART for location %d ;errno=%d\n",
                location,errno) ;
            len = 0 ;
        }
        close(fd) ;
    }
    else /* error opening adapter */
    {
        DEBUG_2("get_inq_data: error opening %s, errno=%d\n", adp_name, errno) ;
        len = 0 ;
    }
    return(len) ;
} /* END get_inquiry_data */

/*
 ***********************************************************************
 * NAME     : get_tokens
 * FUNCTION : break out the pieces of the VPD format string that
 *            is passed in.  (see NOTES for details for the format)
 * EXECUTION ENVIRONMENT : Should only be used as part of IDE
 *            VPD processing.
 * NOTES    :
 *     -  The format of the format string is a comma seperated
 *        string of keyword location identifiers.  The format of
 *        of the keyword location identifier is 1 of 2 formats (the
 *        2 formats can be intermixed within the same string).
 *        a) KKOOLLD
 *             where
 *               KK is the 2 character keyword name to be associated
 *                  with the data.
 *               OO is the 2 hexadecimal digit offset into the inquiry data
 *                  where the VPD keyword value resides.
 *               LL is the 2 hexadecimal digit length of the keyword value
 *                  in the inquiry data.  This length is in bytes.
 *               D  is the 1 character data type, C for char ASCII, X hex.
 *                  This will be used for conversion.
 *        b) KKOOLLPPD
 *             where
 *               KK, OO, and LL are as defined in 'a)' above.
 *               PP is the 2 hexadecimal digit inquiry code page that
 *                  contains this VPD keyword value.  If this field is
 *                  NOT present, it is assumed that the value is in
 *                  the default inquiry data.
 *               D  is the 1 character data type, C for char ASCII, X hex.
 *                  This will be used for conversion.
 ***********************************************************************
 */
static int
get_tokens(char   *format_str,          /* the format string            */
           char   *keyword,             /* return var for keyword name  */
           short  *offset ,             /* return var for offset        */
           short  *len    ,             /* return var for len of value  */
           int    *page   ,             /* return var for code page     */
           int    *prev_page,           /* return var for previous c page*/
           char   *dtype)               /* return var for string data typ*/
{
    char   *loc_id;			/* beginning of this keyword     */
    int    rc    ;
    char   part[3];
	 

    /* BEGIN get_tokens */

    bzero(part, sizeof(part) );

    loc_id = strtok(format_str, ", ") ;   /* search for next , or blank */

    if (loc_id != NULL)
    {
                                        /* get the C or X data type   */
        *dtype = loc_id[strlen(loc_id) -1] ; 
	strncpy(keyword, loc_id, 2);
	*offset = (short)strtol(strncpy(part,loc_id+2,2),NULL,16);

	*len =    (short)strtol(strncpy(part,loc_id+4,2),NULL,16);

        *prev_page = *page ;            /* save previous value     */
                                        /* to know when to switch  */
 	if (strlen(loc_id) > 7 ) {      /* there is code page info */
		*page = (int)strtol(strncpy(part,loc_id+6,2), NULL,16);
	}
	else {
		*page = NO_PAGE;
	}
        rc = TRUE ;
    }
    else
    {
        rc = FALSE ;
    }
    return(rc) ;
} /* END get_tokens */

/*
 ***********************************************************************
 * NAME: get_dev_id
 *
 * FUNCTION: Extracts the device ID from the connwhere string provided.
 *
 * EXECUTION ENVIRONMENT:
 *	Dynamically loaded & run by generic config
 *
 * NOTES:
 *	1. This code is designed to be loadable, therefore no global variables 
 *	   are assumed.
 *	2. The basic function is to convert the input string
 *	   a uchar values (i.e. the device ID)
 *
 * RETURNS:
 *	 0 for success
 *	-1 for failure
 ***********************************************************************
 */
int
get_dev_id( ideaddr, dev_id )
char	*ideaddr;
uchar	*dev_id;
{
	if (*ideaddr == '\0') return -1;

/* We utilize the behavior of strtoul which stops converting characters at
   the first non-base character Thus after the start position is set, the
   conversion stops either at the ',' or at the NULL end-of-string */

	*dev_id = (uchar)strtoul(ideaddr,NULL,10);

	return 0;
}


