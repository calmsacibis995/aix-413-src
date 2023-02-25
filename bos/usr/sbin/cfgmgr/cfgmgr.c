static char sccsid[] = "@(#)52	1.33.1.20  src/bos/usr/sbin/cfgmgr/cfgmgr.c, cmdcfg, bos41J, 9523C_all 6/9/95 19:46:18";
/*
 *   COMPONENT_NAME: (CMDCFG) Generic config support cmds
 *
 *   FUNCTIONS: LEDS
 *		configure_dev
 *		configure_node
 *		configure_list
 *		main
 *		nlsmsg
 *		noctrlch
 *		showerr
 *		sort_seq
 *		add_pkglist
 *		inst_pkg
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define  _ILS_MACROS

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <odmi.h>
#include <sys/cfgdb.h>
#include <sys/priv.h>	/* privilege */
#include <sys/audit.h>	/* auditing */
#include <sys/signal.h>
#include <sys/cfgodm.h> /* classes structure dcls */
#include <sys/mdio.h>  	/* machine device driver  */
#include <cf.h>
#include <sys/time.h>


#define	CFGMGR_CATALOG		"cfgmgr.cat"
#define CFGMGR_MSG_SET		1
#define CFGMGR_ERR_SET		2

#ifdef  MSG
#include <nl_types.h>
#include <locale.h>
#endif

/*---------------------------- message strings ------------------------------*/
#define MSG_OUT_INFO		1
#define MSG_NO_OUT		2
#define MSG_ERR_INFO		3
#define MSG_NO_ERR		4
#define MSG_PHASE		5
#define MSG_BAD_PHASE		8
#define MSG_TOP_LEVEL		9
#define MSG_RC			10
#define MSG_SAVEBASE		11
#define MSG_CONFIG_DEV		12
#define MSG_INVOKING		13
#define MSG_METHERR		14
#define	MSG_PKGINSTALL		18
#define	MSG_INSTALLFRM		19

char *msgstr[] =
{
"",
"****************** stdout ***********\n%s\n",
"****************** no stdout ***********\n",
"****************** stderr ***********\n%s\n",
"****************** no stderr ***********\n",
"phase = %d\n",
"cfgmgr is running in PHASE2\n",
"cfgmgr is running in PHASE2MAINT\n",
"cfgmgr : unknown phase = %d\n",
"invoking top level program -- \"%s\"\n",
"return code = %d\n",
"calling savebase\n",
"attempting to configure device '%s'\n",
"invoking %s %s\n",
"Method error (%s):\n",
"",
"",
"",
"Packages to install %s...\n",
"Installing devices packages from: %s\n\twith devinstall switches:%s\n"
};

/*------------------------------ error messages ----------------------------*/
/* WARNING - these error numbers correspond to led values which are REGISTERED*/
/*    in some documents, so they cannot be changed!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define ERR_SYNTAX			1	/* syntax error in cmnd line */
#define ERR_CONFLICT			2	/* option conflict */
#define ERR_ODM_INIT			3	/* can't initialize ODM */
#define ERR_GET_RULES			4	/* can't access config rules object */
#define ERR_GET_CUDV			5	/* can't get data from customized device obj. */
#define ERR_GET_CUDVDR			6	/* can't get data from device driver object */
#define ERR_PHASE1_ILLEGAL 		7	/* phase1 illegal at this point */
#define ERR_NO_SEQ			8	/* can't find sequence rule */
#define ERR_UPDATE_ODM			9	/* can't update odm data base */
#define ERR_SAVEBASE			10	/* savebase failure */
#define ERR_GET_PDAT			11	/* can't access the PdAt class */
#define ERR_NO_MEMORY			12	/* can't allocate system memory */
#define ERR_CONFIGMTH			13	/* no config method for device */
#define ERR_LOCK_ACC			14	/* lock acquire error */
#define ERR_FWRITE			16	/* File write */ 
#define ERR_FOPEN			17	/* File open */ 
#define ERR_VERIFY			18	/* install error */ 
#define ERR_BOSBOOT			19	/* install error */ 
#define ERR_INSTALL			20	/* install error */ 
#define ERR_MISSING			21	/* missing package error */ 

char *errstr[] =
{
"",
"cfgmgr: 0514-600 Usage error : %s\nUsage: cfgmgr [-f|-s|-p phase] [-l name] [-v] [-i device]\n",
"cfgmgr: 0514-601 Flag conflict : %s\nUsage: cfgmgr [-f|-s|-p phase] [-l name] [-v] [-i device]\n",
"cfgmgr: 0514-602 Cannot initialize the ODM data base.\n",
"cfgmgr: 0514-603 Cannot access the Config_Rules object class in the\n\tdevice configuration database.\n",
"cfgmgr: 0514-604 Cannot access the CuDv object class in the\n\tdevice configuration database.\n",
"cfgmgr: 0514-605 Cannot access the CuDvDr object class in the\n\tdevice configuration database.\n",
"cfgmgr: 0514-606 Running PHASE1 at this point is not valid.\n",
"cfgmgr: 0514-607 There are no sequence rules to execute for the\n\tspecified phase.\n",
"cfgmgr: 0514-608 Cannot update an object in the device\n\tconfiguration database.\n",
"cfgmgr: 0514-609 Unable to save the base customized information\n\ton /dev/ipldevice.\n%s\n",
"cfgmgr: 0514-610 Cannot access the PdAt object class in the\n\tdevice configuration database.\n",
"cfgmgr: 0514-611 There is not enough memory available now.\n",
"cfgmgr: 0514-612 Unable to configure the %s device because\n\tit either does not exist in the customized device database or it does not\n\thave a corresponding entry in the predefined device database.\n",
"cfgmgr: 0514-613 Unable to acquire the device configuration database\n\tlock. The errno is %d.\n",
"cfgmgr: 0514-614 Warning - unable to determine the \"type_of_boot\"\n",
"cfgmgr: 0514-616 Unable to write the file %s\n",
"cfgmgr: 0514-617 Unable to open the file %s\n",
"cfgmgr: 0514-618 Bosboot verification failed. Resolve the problem and re-run bosboot\n",  
"cfgmgr: 0514-619 Bosboot failed. Resolve the problem and re-run bosboot\n",  
"cfgmgr: 0514-620 The following device packages are required for device\n\tsupport but were not found on the current installation,\n\tor encountered an error during installation.\n",
"cfgmgr: 0514-621 WARNING: The following device packages are required for \n\tdevice support but are not currently installed.\n"
};

/*---------------------------------- device packaging ----------------------*/
#define	DEVPKGFILE	"/tmp/device.pkgs"
#define	DEVPKGFAIL	"/var/adm/dev_pkg.fail"
#define DEVINSTALL	"/usr/sbin/devinstall"
#define BOSBOOT		"/usr/sbin/bosboot"
#define BOSBOOTARGS	" -a -d /dev/ipldevice "
#define VERIFYARGS	" -v -d /dev/ipldevice "
#define FNAME_SIZ	256
#define OLD_PKG		0
#define NEW_PKG		1
#define DEL_PKG		3

/*---------------------------------- exit status ---------------------------*/
#define EXIT_NOT	-1
#define EXIT_OK		0
#define EXIT_FATAL	1

/*---------------------------------- leds ----------------------------------*/
int show_leds;				/* >0 if leds to be shown */
#define LEDS_BASE	0x520		/* first cfgmgr led value */
#define LEDS_PROGRAM	0x538		/* led value to signify a program being run */
#define LEDS_RETURN	0x539		/* led value to signify a program being run */
#define LEDS(num)	{if (show_leds) setleds(num);}

/*--------------------------------- global vars -----------------------------*/
int 		verbose;		/* >0 if verbose output desired	*/
short		phase;			/* current phase 		*/
int		runtime = 0;		/* =0 default is boot phase     */
extern	char	*malloc();		/* memory allocation routine 	*/
extern	int	errno;			/* errno 			*/
int 		lock_id;		/* odm lock id			*/
int 		*cfgmgr_cat;
int 		*cmdcfg_cat;

/*----------------------- global vars for device installation ----------------*/
struct	pkgname {
	char	name[FNAME_SIZ];
	int	status;
	struct  pkgname *next;
}	*pkglist, *badpkglist ;		/* pkgname list and failed pkgname list */

int		inst_mode = 0;
int		allpkg = 0;		/* Mark DEV_PKGNAME flag is set */
char		first[3] = "-s";	/* Flag for re-creating dev_pkg.fail*/
int		pkgcnt = 0;


/*
* NAME : nlsmsg
*
* FUNCTION : returns a pointer to the requested string, which is obtained from
*		either the message catalog (if available), or the default messages
*
* PARAMETERS :
*	set	-	message set
*	msg	-	message num
*
* RETURNS VALUES:
*
* GLOBAL VARIABLES REFERENCED : none
*/
char *nlsmsg( msgset, msgnum )
int msgset;
int msgnum;
{	char *default_msg;
	int  *cat_fd;

	/* init where the msgs & defaults come from */
	switch (msgset)
	{	case CFGMGR_ERR_SET	:	default_msg = errstr[msgnum];
						cat_fd = cfgmgr_cat;
						break;
	 	case CFG_METH_SET	:	default_msg = meth_err_msg[msgnum];
						cat_fd = cmdcfg_cat;
						break;
		default			:	default_msg = msgstr[msgnum];
						cat_fd = cfgmgr_cat;
						break;
	}

	/* is the catalog open ???? */
	if (cat_fd == (int *)-1)
	{	/* NO! - use the default messages */
		return( default_msg );
	}
	else
		return( (char *)catgets((struct _catalog_descriptor*)cat_fd,
			msgset,msgnum,default_msg) );
	}


/*
* NAME:  cfgmgr
*
* FUNCTION:
*	Configuration manager is responsible for device configuration
*	during IPL.  The config manager is divided into two phases.  The
*	first phase is involved when the kernel is brought into the system and
*	only the boot file system is active.  The boot file system is usually
*	a subset of a full functional root file system.  The responsibility
*	of the config manager in phase I is to configure base devices so
*	that the root device(s) can be configured and be ready for operation.
*	After the root device(s) is/are configured, the init program switches
*	the root directory to the new device(s).  The second phase of config
*	manager is to configure the rest of the devices in the system
*	after the root file system is up and running.  The config manager
*	is a simple routine which is mainly driven by the config rules
*	object class in the data base.
*	The cfgmgr can also be invoked during run time to configure any
*	detectable devices which are not being configured during the IPL time.
*	Those devices may be turned off at IPL time or newly added devices.
*
*
* EXECUTION ENVIRONMENT:
*
*
* DATA STRUCTURES: config rule object class
*		   device predefined and customized object classes
*
* INPUT PARAMETERS:
*			argv -- input parameters
*				1st parm -- "-f"	first phase
*					 or "-s"	second phase
*				"-n"			no initialization
*			     -- none -- run time option
*
* RETURN VALUE DESCRIPTION:  0	-- no errors 
*			      -1 -- fatal error 
*
* EXTERNAL PROCEDURES CALLED:
*
*/
int
main(argc, argv)
int	argc;
char	*argv[];
{
	int 	rc;				/* return code */
	char	*cp;				/* temporary character ptr */
	int	c;				/* option character */
	struct 	PdAt pdat;			/* PdAt object */
	extern 	char *optarg;			/* getopt */
	extern 	int optind;			/* getopt */
  	struct 	sigaction action;		/* parameters of sigaction */
	struct 	sigaction oldaction;
	char 	*inuclients;			/* ptr to value of INUCLIENTS env var */
	FILE	*fpkg;
	struct 	pkgname *p;
	char	instdev[FNAME_SIZ] = "";
	int	bosboot_flag;			/* indicator for running bosboot*/
	int	bad_pkgs;
        char 	*logical_name = NULL;           /* for -l option only */
	char 	*out_ptr;
	char 	*err_ptr;
        time_t	timeval;


	/* the INUCLIENTS environment variable is set by the instlclient*/
	/*command and is used to signal that a diskless client's root 	*/
	/*directory is mounted as the root directory, so things are not	*/
	/*what they appear to be normally			 	*/
	/* cfgmgr should not be run in that kind of environment	 	*/

	inuclients = (char *)getenv("INUCLIENTS");
	if ( (inuclients != NULL) && (strcmp(*inuclients,"")) )
		exit( 0 );

	/*  Get device installation parameters from environment */
	pkglist = badpkglist = NULL;
	if( !strcmp(getenv("DEV_PKGNAME"),"ALL") )  allpkg = 1;

	/*  Get runtime parameter from environment */
	/*  if variable is null, then must be runtime */
	if( !strcmp(getenv("SYSCFG_PHASE"), "") )  runtime = 1;

	/* ignore these signals */
	memset(&action, '\0', sizeof (struct sigaction));
	action.sa_handler = SIG_IGN;
	sigaction (SIGQUIT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGINT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGTERM, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGHUP, &action, &oldaction);

	/* initialize internal state */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	priv_lapse();  

	/* initialize values */
	bosboot_flag = verbose = phase = FALSE;	
	cfgmgr_cat = cmdcfg_cat = (int *)-1;
	show_leds = TRUE;

	/* initialize the ODM */
	if ( odm_initialize() ) 
	   showerr (ERR_ODM_INIT, NULL,EXIT_FATAL);


	if (runtime) {	
		/* init for NLS use - deliberately avoid this for phase 1 */
#ifdef MSG
			setlocale(LC_ALL,"");
			cfgmgr_cat = (int *) catopen(CFGMGR_CATALOG,NL_CAT_LOCALE);
			cmdcfg_cat = (int *) catopen(CFG_MSG_CATALOG,NL_CAT_LOCALE);
#endif
	}

	/* parse the command line */
	while (( c = getopt(argc, argv, "sfvdp:l:i:")) != EOF ) {
	   switch (c)
	   {  case 's' :	if (phase)
			    		showerr( ERR_CONFLICT,"-s|-p",EXIT_FATAL );
			 	phase = PHASE2;
			 	break;

	      case 'f' :	if (phase)
			    		showerr( ERR_CONFLICT,"-f|-p",EXIT_FATAL );
				else if (runtime)
	      				showerr(ERR_PHASE1_ILLEGAL,NULL,EXIT_FATAL);
			 	phase = PHASE1;
			 	break;

	      case 'p' :	if (phase)
			    		showerr( ERR_CONFLICT,"-f|-s",EXIT_FATAL );
				phase = atoi( optarg );
				break;
	      case 'd' :
	      case 'v' :	verbose = TRUE;
				break;

	      case 'l' :	logical_name = optarg;
				break;

	      case 'i' :	inst_mode = 1;
				strncpy(instdev, optarg, strlen(optarg));
				break;

	      default  :	showerr( ERR_SYNTAX, NULL,EXIT_FATAL );
			 	break;
	   } /*end switch*/
	}

	/* check for other args */
	if (optind < argc)
	   showerr( ERR_SYNTAX, argv[optind],EXIT_FATAL );

	/* if no phase specified, use PHASE2 as the default */
	/* when no phase is specified, do not show leds */
	if (phase == 0)
	{	phase = PHASE2;
		show_leds = FALSE;
	}

	/* config all devices according to the sequence rules */

	while( 1 ) {

       	 	if (logical_name) {                     /* was -l specified? */
        	        configure_list(logical_name);   /* start configuring 
							   at given device   */
         	} else {
                	/* config all dev according the sequence rules */
        	        configure_node();
         	}

		if( !inst_mode ) break;           /* Not installation - done */

		if( (rc = inst_pkg(instdev)) < 0 ) break;	

		/* if devinstall successful for at least 1 pkg, do bosboot */
		if (rc == 1) bosboot_flag=TRUE;
			

	} /* while */	


	if (runtime)
	{  /* call savebase */

	   /* show that a program is being executed */
	   LEDS( LEDS_PROGRAM )

	   if (verbose)
	   {  printf("-----------------------------------------------------------------------\n");
	      printf(nlsmsg(CFGMGR_MSG_SET,MSG_SAVEBASE));
	   }

      	   rc = odm_run_method("/usr/sbin/savebase","",&out_ptr,&err_ptr);
           if (time(&timeval) == -1)
              perror("time");
           else
              printf("Date: %s", ctime(&timeval));

	   /* show that we have returned from the program */
	   LEDS( LEDS_RETURN )

	   retmsg( rc, out_ptr, err_ptr);

	   if (rc)
	      showerr( ERR_SAVEBASE, err_ptr, EXIT_FATAL );
	}

	/* blank out leds */
	setleds( 0xfff );

	/* output DEVPKGFILE */
	if ( allpkg ) {
		if((fpkg = fopen(DEVPKGFILE,"w")) == NULL )
			showerr( ERR_FOPEN, DEVPKGFAIL,EXIT_FATAL );
		for( p = pkglist; p != NULL; p = p->next )  
		    if(fprintf(fpkg,"%s\n", p->name) < 0 )
			showerr( ERR_FWRITE, DEVPKGFAIL,EXIT_FATAL );
		fclose(fpkg);
        }

	if ( inst_mode && bosboot_flag && runtime )  {
		/*Run bosboot before exiting if there is any
		  successfully installed packages */

		if(odm_run_method(BOSBOOT,VERIFYARGS,NULL,NULL)) 
			showerr( ERR_VERIFY, NULL,EXIT_FATAL);	
		/* Verify succeeds,so run the actual one */	
		if(odm_run_method(BOSBOOT,BOSBOOTARGS,NULL,NULL))
			showerr( ERR_BOSBOOT,NULL,EXIT_FATAL);
	}

	odm_terminate();

	/* if not install mode, not DEV_PKGNAME=ALL, and at runtime
	   print out a warning that some packages may be missing */
	if (!inst_mode && !allpkg && runtime && pkglist ) {

		/* Need to make a special check for missing 'isa_sio'
		   packages.  These are not to be treated as missing
		   packages.  The 'bad_pkgs' flag will indicate if
		   a non isa_sio package is missing.  In this case,
		   the message will be displayed. */
		bad_pkgs = 0;
		for( p = pkglist; p != NULL; p = p->next ) {
			if (!strncmp(p->name,"devices.isa_sio.",16))
				p->status = DEL_PKG;
			else
				bad_pkgs = 1;
		}

		if (bad_pkgs) {
			/* Had missing packages (other than isa_sio pkgs) */
    			fprintf(stderr,nlsmsg(CFGMGR_ERR_SET,ERR_MISSING));
			for( p = pkglist; p != NULL; p = p->next ) {
				if (p->status != DEL_PKG)
		    			fprintf(stderr,"%s\n", p->name); 
			}
		}
	}
	/* close the message catalogs */
	if (cfgmgr_cat != (int *) -1)
		catclose( (struct _catalog_descriptor *)cfgmgr_cat );
	if (cmdcfg_cat != (int *) -1)
		catclose( (struct _catalog_descriptor *)cmdcfg_cat );

	exit( EXIT_OK );

}  



/*
*  NAME:  configure_node
*
*  FUNCTION:  configure devices under a node in the sequence rule
*
*  RETURN VALUE:
*		0  -- successful
*		non-zero -- failed
*
*/
int
configure_node()
{
	char 	**name_list;
	char 	*name;
	char 	*out_ptr;
	char 	*err_ptr;
	int 	len;
	int 	seq;
	int 	i,j;
	int 	rc;
	int 	entries;
	struct 	Config_Rules **list;
	struct  Config_Rules **sort_seq();
	int 	max_rules;
        time_t	timeval;

	char 	*test;

	test = NULL;

	/* sort the rules by sequence number */
	list = sort_seq( &max_rules );

#ifdef DBG_RULES
	printf("\n");
	/* just want to show the rules which would be executed and stop */
	for (seq=0; seq < max_rules; seq++)
		printf("%-50s  0x%-4x  %d\n",
				 list[seq]->rule,list[seq]->phase,list[seq]->seq);
	printf("\n");
	exit(0);
#endif

	/* acquire data base lock */
	if ( (lock_id=odm_lock( "/etc/objrepos/config_lock", ODM_WAIT)) == -1)
	   showerr(ERR_LOCK_ACC, errno, EXIT_NOT);

	/* execute each rule */
	for (seq=0; seq < max_rules; seq++)
	{  if (verbose)
	   {  printf("-----------------------------------------------------------------------\n");
	      printf(nlsmsg(CFGMGR_MSG_SET,MSG_TOP_LEVEL),list[seq]->rule);
	   }

	   /* show that a program is being executed */
	   LEDS( LEDS_PROGRAM )

	   /* execute this program */
           if (time(&timeval) == -1)
              perror("time");
           else
              printf("Date: %s", ctime(&timeval));
	   rc = odm_run_method( list[seq]->rule, "", &out_ptr, &err_ptr );
           if (time(&timeval) == -1)
              perror("time");
           else
              printf("Date: %s", ctime(&timeval));

	   /* show that we have returned from the program */
	   LEDS( LEDS_RETURN )

	   retmsg( rc, out_ptr, err_ptr);

	   /* check for odm_run_method failure */
	   if (rc == -1)
	       rc = E_ODMRUNMETHOD;

	   /* check for an error */
	   rc = ((rc < 0) || (rc > E_LAST_ERROR)) ? E_LAST_ERROR : rc;
	   if (rc)
	   {  fprintf(stderr,nlsmsg(CFGMGR_MSG_SET,MSG_METHERR),list[seq]->rule);
		fprintf(stderr,nlsmsg(CFG_METH_SET,rc));
		if (err_ptr != NULL)
		{  /* print whatever the method wrote */
			if ((*err_ptr != '\0') && (*err_ptr != '\n'))
	                    fprintf(stderr,"%s\n",err_ptr);
		}
	   }
	   if (err_ptr != NULL)
		free( err_ptr );

	   if (out_ptr != NULL ) {
		if (strlen(out_ptr) != 0 )
	   		/* Configure the devices found */
	   		configure_list(out_ptr);
		else
			free(out_ptr);
	   }


	}/* end for (seq < max_rules) */

	/* unlock the database */
	odm_unlock(lock_id);


	return (0);
}


/*
* NAME:  configure_list
*
* FUNCTION:
*	To invoke the configure methods for a list of devices and 
*	their children.
*
* EXECUTION ENVIRONMENT:
*
* DATA STRUCTURES:
*
* INPUT PARAMETERS:
*	devices -- list of device logical names
*
* RETURN VALUE DESCRIPTION:   0 -- successful
*			      non-zero --  failed
*
* EXTERNAL PROCEDURES CALLED:
*		individual device config methods
*/
configure_list(char *devices)
{
	char 	**name_list;
	char 	*name;
	char 	*out_ptr;
	char 	*err_ptr;
	int 	len;
	int 	i,j;
	int 	rc;
	int 	entries;

	/* malloc space for name_list, which will store the addresses */
	/*   of the out_ptr buffers which will be returned from the   */
	/*   configure methods */
	if ( (name_list = (char **) malloc ( 10 * sizeof (char *))) == 0 )
	      showerr(ERR_NO_MEMORY, NULL, EXIT_FATAL);

	/* initialize the name_list */
	entries = 1;
	name_list[0] = devices;

	/* process each list of names of devices to be configured */
	for (j=0 ; j < entries; j++ )
	{  /* separate the names */
	      len = strlen( name_list[j] );
	      i = 0;

	      while (i < len)
	      {  /* point to the current name */
			name = &(name_list[j][i]);

			/* look for a separator char */
			for (; i < len; i++)
				if (	(name_list[j][i] == ' ') ||
						(name_list[j][i] == ',') ||
						(name_list[j][i] == '\n') ||
						(name_list[j][i] == '\t') )
				{  /* take everything up to this point as a program */
					name_list[j][i] = '\0';

					/* increment past this point for next loop*/
					i++;
					break;
				}

			/* check for null name - just in case */
			if (name[0] == '\0')
				continue;
			if (name[0] == ':' ) { /* package name */
				addpkglist( &name[1], &pkglist);
				continue;
			}

			/* configure this device */
			rc = configure_dev( name, &out_ptr );

			/* any output from that configure method??? */
			if( (out_ptr != (char *) 0) )
			   if (strlen(out_ptr) != 0) 
			   {  /* add this to the names list */
				name_list[entries++] = out_ptr;

				if ( (entries%10) == 0 )
				{  /* allocate more memory for the name_list */
					if ((name_list = (char **) realloc(name_list, 
					   (entries+10) * sizeof (char *))) == NULL )
		  			   showerr(ERR_NO_MEMORY, NULL,EXIT_FATAL);
		 		}
			   }

			   else free(out_ptr);

	      }/* end while (i < len) */

	      /* done with this list */
	      free( name_list[j] );

	}/* end for (j < entries) */

	/* free up the name_list */
	free( (void *)name_list );
	return(rc);
}


/*
* NAME:  configure_dev
*
* FUNCTION:
*	To invoke the configure method of the device.
*
* EXECUTION ENVIRONMENT:
*
* DATA STRUCTURES:
*
* INPUT PARAMETERS:
*	device -- device logical name
*	names-ret -- device name ptr for the list of device names
*			to be returned to be configured
*
* RETURN VALUE DESCRIPTION:  0 -- successful
*			      non-zero --  failed
*
* EXTERNAL PROCEDURES CALLED:
*		individual device config method
*/
configure_dev( device, out_ptr)
char *device;
char **out_ptr;
{ char	 criteria[MAX_CRITELEM_LEN];
  struct CuDv *cudv;
  struct listinfo cudv_info;
  char 	*err_ptr;
  int 	rc;
  char 	params[100];

	*out_ptr = 0;

	if (verbose)
	{  printf("-----------------------------------------------------------------------\n");
	   printf (nlsmsg(CFGMGR_MSG_SET,MSG_CONFIG_DEV),device);
	}

	/* get the PdDv info for this device */
	sprintf (criteria, "name = '%s'",device);
	if ((int)(cudv = (struct CuDv *)odm_get_list(CuDv_CLASS,criteria,
						&cudv_info,1,2)) == -1)
	{  showerr( ERR_GET_CUDV, device, EXIT_NOT );
	   printf("\n");
	   return( -1 );
	}

	if ( (cudv_info.num != 1) || (!cudv[0].PdDvLn) )
	{  showerr ( ERR_CONFIGMTH , device, EXIT_NOT);
	   printf("\n");
	   return( -1 );
	}

        /* If the device is in the diagnose state, just return 0 */
        if ( cudv[0].status == DIAGNOSE )
	    return(0);

	/* initialize the method parameter string */
	if (phase == PHASE1)
	   sprintf( params, "-1 -l %s", device );
	else if (show_leds)
	   sprintf( params, "-2 -l %s", device );
	else
	   sprintf( params, "-l %s", device );

	/* security */
	priv_acquire();  

	if (verbose)
	   printf(nlsmsg(CFGMGR_MSG_SET,MSG_INVOKING), 
		  cudv[0].PdDvLn->Configure, params);

	/* show that a program is being executed */
	LEDS( LEDS_PROGRAM )

	/* invoke the configure method */
        if (time(&timeval) == -1)
           perror("time");
        else
           printf("Date: %s", ctime(&timeval));
 	rc = odm_run_method(cudv[0].PdDvLn->Configure,params,out_ptr,&err_ptr);
        if (time(&timeval) == -1)
           perror("time");
        else
           printf("Date: %s", ctime(&timeval));

	/* show that we have returned from the program */
	LEDS( LEDS_RETURN )

	retmsg(rc, *out_ptr, err_ptr);

	/* check for odm_run_method failure */
	if (rc == -1)
	    rc = E_ODMRUNMETHOD;

	/* check for an error */
	rc = ((rc < 0) || (rc > E_LAST_ERROR)) ? E_LAST_ERROR : rc;
	if (rc)
	{  fprintf(stderr,nlsmsg(CFGMGR_MSG_SET,MSG_METHERR),
		cudv[0].PdDvLn->Configure);
	   fprintf(stderr,nlsmsg(CFG_METH_SET,rc));
	   auditlog( "DEV_Configure", AUDIT_FAIL,device,strlen(device)+1 );
	}
	else
	   auditlog( "DEV_Configure", AUDIT_OK, device, strlen(device)+1 );

	/* out-security */
	priv_lapse();  

	/* print stderr stuff */
	if (err_ptr != NULL)
	{  if ((*err_ptr != '\0') && (*err_ptr != '\n'))
	      fprintf(stderr,"%s\n",err_ptr);
	   free( err_ptr );
	}

	return( rc );
}

/*
* NAME : showerr
*
* FUNCTION : Display error message on console or front panel led's display
*		depending whether the cfgmgr is invoked at IPL time or
*		run time.
*
* PARAMETERS :
*	arg -- character arg to be displayed
*
* RETURNS VALUES:
*
* GLOBAL VARIABLES REFERENCED : none
*/
showerr(errnum, arg, exit_status)
int  errnum;		/* error number */
char *arg;		/* character to be displayed */
int  exit_status;	/* exit status code */
{

	/* write an error message */
	fprintf( stderr, nlsmsg(CFGMGR_ERR_SET,errnum), arg);

	if (exit_status == EXIT_NOT)
	   return( 0 );
	else
	{  /* assuming fatal error */
	   odm_unlock(lock_id);
	   odm_terminate();

	   /* close the message catalogs */
	   if (cfgmgr_cat != (int *) -1)
		catclose((struct _catalog_descriptor *) cfgmgr_cat );
           if (cmdcfg_cat != (int *) -1)
		catclose((struct _catalog_descriptor *) cmdcfg_cat );

	   /* turn off the leds */
	   setleds( 0xfff );

	   /* bye-bye */
	   exit( exit_status );
	}
}

noctrlch( str )
char *str;
{ int i;

	if (str == NULL)
	   return( 0 );

	if (str[0] == '\0')
	   return( 0 );

	for (i=0; str[i] != '\0'; i++)
	   if ((!isprint(str[i])) && (str[i] != '\n'))
	      return( 0 );

	return( 1 );
}


struct Config_Rules **sort_seq( max_rules )
int *max_rules;
{ 
	int 	i,j;
	struct 	Config_Rules *tmp_me;
	struct 	Config_Rules **list;
	char 	criteria[MAX_CRITELEM_LEN];
	struct 	Config_Rules *rules;
	struct 	listinfo rules_info;

	if (verbose)
	   printf(nlsmsg(CFGMGR_MSG_SET,MSG_PHASE),phase);

	/* get all the rules for this phase */
	sprintf(criteria,"phase = %d",phase);
	if ((int)(rules = (struct Config_Rules *)
		odm_get_list(Config_Rules_CLASS,criteria, &rules_info,1,1)) == -1)
	   showerr( ERR_GET_RULES, NULL, EXIT_FATAL );
	else if ( rules_info.num == 0 )
	   showerr( ERR_NO_SEQ , NULL, EXIT_FATAL );

	/* malloc the appropriate space for the ordered list */
	if ((list = (struct Config_Rules **)
	    malloc( rules_info.num * sizeof(struct Config_Rules *) )) == 0)
	   showerr( ERR_NO_MEMORY, NULL,EXIT_FATAL );

	for (i=0; i < rules_info.num; i++)
	{
		/* convert list to pointers */
		list[i] = &(rules[i]);
	}

	/* prioritize the list based on the sequence number */
	/* put rules with "seq == 0" at the end of the list */
	for (i=0; i < (rules_info.num -1); i++)
		for (j=i+1; j < rules_info.num; j++)
		{  if (list[j]->seq == 0)
				continue;
			if ( (list[j]->seq < list[i]->seq) || (list[i]->seq == 0) )
			{  /* swap 'em */
				tmp_me = list[i];
				list[i] = list[j];
				list[j] = tmp_me;
			}
		}

	*max_rules = rules_info.num;

	return( list );
}


/*
*
*	NAME: retmsg
*
*	FUNCTION:	displays the messages returned from odm_run_method	
*
*	PARAMETERS:	return code, stdout-buffer pointer, stderr-buffer pointer
*
*	RETURNED VALUES:	none
*
*/
retmsg( rc,out,err)
int rc;
char *out, *err;
{
	if (verbose)
	{  printf(nlsmsg(CFGMGR_MSG_SET,MSG_RC),rc);
	   if (noctrlch(out))
	      printf(nlsmsg(CFGMGR_MSG_SET,MSG_OUT_INFO),out);
	   else
	      printf(nlsmsg(CFGMGR_MSG_SET,MSG_NO_OUT));
	   if (noctrlch(err))
	      printf(nlsmsg(CFGMGR_MSG_SET,MSG_ERR_INFO),err);
	   else
	      printf(nlsmsg(CFGMGR_MSG_SET,MSG_NO_ERR));
	}
}
/*
* NAME:  addpkglist
*
* FUNCTION: add a package name to the list.
*    It makes sure the same name has not been
*    in the list and then adds the name in 
*    alphabetically sorted order. 
*
* INPUT PARAMTERS: 	package name, list header
*
* RETURN VALUE DESCRIPTION:  exit when malloc fails
*
* EXTERNAL PROCEDURES CALLED: none
*
*/

addpkglist(name, pkgl)
char   *name;
struct pkgname **pkgl;
{
 struct pkgname *node, *new_node, *prev_node;
 int    rc;

 node = *pkgl;  	/* list header */
 prev_node = NULL; 	/* previous node */
 rc = -1;  		/* non-zero value */

 /* loop until one of the return conditions is met */
 while( 1 ) 
 {

     /* If the list is null OR if we have found the 
  	correct place in the list, then insert this name */

     if ( !node || (rc = strcmp(node->name, name)) >= 0 ) 
     {
 	 /* if strcmp returned 0, then this name is already in the list */
  	if (!rc) 
   		return; 

  	/* not a duplicate - we need to create a new one */
        new_node = (struct pkgname *) malloc( sizeof( struct pkgname) );
  	if( !new_node )
   		showerr(ERR_NO_MEMORY,NULL,EXIT_FATAL);

  	/* fill in the new node with data */
  	strcpy( new_node->name, name);
  	new_node->status = NEW_PKG; 
  
  	/* make the new node point to the node we found
   	   with strcmp - its alphabetical antecedent */
  	new_node->next = node;

  	/* if the node is going to be the first in the list */
  	/* set up package list and return.                  */
  	if( !*pkgl || node == *pkgl )           /* 1st node */
  	{ 
   		*pkgl = new_node;
   		return;
  	}
  
  	/* set up the prev_node pointer to hook the new node in */
  	prev_node->next = new_node;
  	return;

     } /* if !node */ 

     /* we need to loop again, so set up the previous node pointer 
	to point to the one we're currently looking at....  */

     prev_node = node; 

     /* ...and examine the next node in the list */
     node  = node->next;
 } /* end while */

} 

/*
*
*	NAME: inst_pkg
*
*	FUNCTION:	call devinstall to install a set of device pacakges
*
*	PARAMETERS:	install device
*
*	RETURNED VALUES:
*			-1	no new package names
*
*			any return code from devinstall
*
*
*
*/
inst_pkg(instdev)
char *instdev;
{
   int    new,rc;
   struct pkgname *p;
   FILE   *fpkg;
   char	  indev_arg[256];

	new = 0;

	/* Write new packages to a pkgfile */
	if( (fpkg = fopen(DEVPKGFILE,"w")) == NULL)
		showerr( ERR_FOPEN, DEVPKGFAIL,EXIT_FATAL );

	for( p = pkglist; p != NULL; p = p->next ) { 
	      if ( p->status == OLD_PKG ) continue;
	      if( fprintf(fpkg,"%s\n", p->name) < 0 )
		  showerr( ERR_FWRITE, DEVPKGFAIL,EXIT_FATAL );
	      if(!new && verbose ) 
	          printf(nlsmsg(CFGMGR_MSG_SET,MSG_PKGINSTALL));
	      if( verbose ) printf("%s\n",p->name);
	      p->status = OLD_PKG;
	      new = 1;
	}

	fclose(fpkg);

	if( !new ) return -1 ;

	/* if install time do not re-create DEVPKGFAIL file */
	if ( !runtime ) strcpy(first,"  "); 

	/* if cfgmgr called with verbose, then call devinstall with 
	   the verbose flag set.  Suppress bosboot. */
	if (verbose)
		sprintf(indev_arg,"-v -b -f%s -d%s %s ",DEVPKGFILE, instdev,first);
	else
		sprintf(indev_arg," -b -f%s -d%s %s ",DEVPKGFILE, instdev,first);
	
	strcpy(first,"  "); 

	if( verbose ) {
		printf(nlsmsg(CFGMGR_MSG_SET,MSG_INSTALLFRM),instdev,indev_arg);
	}

	rc = odm_run_method(DEVINSTALL,indev_arg,NULL,NULL);

	return rc;
}

priv_lapse()
{
	priv_t	priv;

	getpriv(PRIV_INHERITED, &priv, sizeof(priv_t));
	setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));
}

priv_acquire()
{
	priv_t	priv;

	getpriv(PRIV_MAXIMUM, &priv, sizeof(priv_t));
	setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));
}

