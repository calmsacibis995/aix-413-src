static char sccsid[] = "@(#)33	1.12.1.4  src/bos/usr/sbin/rmdev/rmdev.c, cmdcfg, bos411, 9428A410j 10/13/93 09:21:24";
/*
 * COMPONENT_NAME: (CMDCFG)  Device specific config support cmds
 *
 * FUNCTIONS: rmdev, rmdev_parse, recurse_rmdev
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * PSEUDO CODE
 *	parse user's input
 *	check for errros from the user
 *	get ready to use the ODM
 *	get the specified customized device
 *	get the method names from the corresponding predefined object
 *	process the user's request
 *	initialize the parameter string
 *	if (current status != DEFINED)
 *		if (current status = AVAILABLE) and (there is a stop method)
 *			invoke the stop method
 *		invoke the unconfigure method
 *	if (delete device)
 *		invoke the undefine method
 *	cleanup before exit
 *	display a message relating successful completion
 *****************************************************************************/

#include <stdio.h>
#include <sys/audit.h>
#include <sys/priv.h>
#include <sys/signal.h>
#include "hlcmds.h"

#define DBG	0
#define USAGE MSGSTR(CFG_RMDEV_USAGE)


/*****************************************************************************/
/*			MODULE GLOBAL VARIABLES				     */
/*****************************************************************************/
char logical_name[NAMESIZE]; 	/* type name specified by user */
int delete_device	= FALSE;/* >0 if "-d" specified */
int just_stop_device	= FALSE;/* >0 if "-S" specified */
int recurse_specified	= FALSE;/* >0 if "-R" specified */
int need_savebase	= FALSE;/* >0 if runtime attribute exists */


/*****************************************************************************/
/*				START OF CODE				     */
/*****************************************************************************/
main( argc, argv )
int argc;				/* # args in argv */
char *argv[];				/* command line arguments */
{  char criteria[MAX_CRITELEM_LEN];	/* search criteria */
   char temp1[100];			/* temp string */
   struct CuDv *cudv;			/* ptr to array of CuDv records */
   struct listinfo cudv_info;		/* CuDv listinfo */
   struct sigaction action;		/* parameters for sigaction */
   struct sigaction oldaction;		/* parameters for sigaction */
   struct PdAt *pdat;			/* ptr to array of PdAt records */
   struct listinfo pdat_info;		/* PdAt info */

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


   /* lock the database */
   if (odm_lock( CONFIG_LOCK, LOCK_TIMEOUT ) == -1)
      do_error( ERRSTR(CFG_ODM_LOCK),"rmdev",
		NULL, NULL, EXIT_FATAL, "DEV_Remove" );

   /* initialize internal state */
   auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
   privilege(PRIV_LAPSE);
#ifdef MSG
   setlocale(LC_ALL,"");
#endif

   /* initialize internal variables */
   logical_name[0] = '\0';

   /* parse user's input */
   rmdev_parse( argc, argv );

   /* check for errors from the user */
   if (logical_name[0] == '\0')
      do_error( ERRSTR(CFG_ARGOROPT_MISSING),"rmdev",
		"-l", USAGE, EXIT_FATAL, "DEV_Remove");
   else if (delete_device && just_stop_device)
     do_error(  ERRSTR(CFG_ARGOROPT_CONFLICT),"rmdev",
               "-d -S", USAGE, EXIT_FATAL, "DEV_Remove");

   /* get ready to use the ODM */
   odm_initialize();

   /* get the specified cudvomized device */
   sprintf( criteria, "name = '%s'", logical_name );
   if ((int)(cudv = (struct CuDv *)
	       odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 2 )) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS),"rmdev",
  		"CuDv", NULL, EXIT_FATAL, "DEV_Remove");
   else if (!cudv_info.num)
      do_error( ERRSTR(CFG_ODM_NOCUDV),"rmdev",
  		criteria, NULL, EXIT_FATAL, "DEV_Remove");
   else if (cudv_info.num > 1)
      do_error( ERRSTR(CFG_ODM_NONUNIQUE),"rmdev",
  		criteria, NULL, EXIT_FATAL, "DEV_Remove" );

   /* should we call savebase??? - check for runtime attr */
   if ((int)(pdat = (struct PdAt *)
               odm_get_list(PdAt_CLASS,PHASE1_DISALLOWED,&pdat_info,1,1)) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS), "rmdev", 
  		"PdAt", NULL, EXIT_FATAL, "DEV_Remove" );
   else if (pdat_info.num)
   {  /* runtime attr exists */
      need_savebase = TRUE;
   }

   /* process the user's request */
   recurse_rmdev(cudv, logical_name);

   /* cleanup before exit */
   odm_terminate();

   exit( 0 );
}


/*------------------------------- recurse_rmdev -------------------------------
 * NAME: recurse_rmdev 
 *                                                                    
 * FUNCTION: If recurse specified, then rmdev any children also
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
recurse_rmdev(cudv, logical_name)
struct CuDv *cudv;
char *logical_name;
{
   struct CuDv *cudvp, *cudvpsave;
   char criteria[MAX_CRITELEM_LEN];     /* search criteria */
   struct listinfo cudv_info;
   char temp[100];			/* temp string */
   char temp1[100];			/* temp string */
   char *params;			/* parameter string */
   int max = 0;				/* max chars in params */
   int device_removed = FALSE;		/* >0 if device removed */
   int child;
   char *out_ptr = NULL;                /* pointers to buffers for stdout */
   char *err_ptr = NULL;                /* pointer to buffer for stderr*/
   int error = 0;                       /* error code */


   if ( cudv->PdDvLn == NULL )
      do_error( ERRSTR(CFG_ODM_NOCUPD),"rmdev",
  		logical_name, NULL, EXIT_FATAL, "DEV_Remove");
   else if ( cudv->status == DIAGNOSE )
      do_error( ERRSTR(CFG_DEV_DIAGNOSE), "rmdev", 
	        logical_name, NULL, EXIT_FATAL, "DEV_Remove");

   if(recurse_specified) { /* descend the tree for children */
       sprintf( criteria, "parent='%s'", cudv->name);
       if ( (cudvp = (struct CuDv *)
           odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 2 )) < 0)
		do_error( ERRSTR(CFG_ODM_ACCESS),"mkdev","CuDv",
		    NULL, EXIT_FATAL, "DEV_Configure" );
	cudvpsave = cudvp;
	for (child=0; child<cudv_info.num; child++) {
	    recurse_rmdev(cudvp, cudvp->name);  /* rmdev child */
	    cudvp++;
        } 
	if (cudv_info.num)
	    odm_free_list(cudvpsave, &cudv_info);
   }

   /* initialize the method parameter string */
   sprintf(temp,"-l %s ", logical_name);
   catstr( &params, temp, &max );

   /* what is the current state of the device ??? */
   if (cudv->status != DEFINED)
   {  /* check to see if it needs to be stopped first */
      if ( (cudv->status == AVAILABLE) && (cudv->PdDvLn->Stop[0] != '\0') )
      {  /* call the stop method */
	 privilege(PRIV_ACQUIRE);
         if ((error = odm_run_method(cudv->PdDvLn->Stop,params,
				     &out_ptr,&err_ptr)) == -1)
            do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"rmdev",
	               cudv->PdDvLn->Stop, NULL, EXIT_FATAL, "DEV_Stop" );

         meth_error(error,err_ptr,EXIT_FATAL,"DEV_Stop",cudv->PdDvLn->Stop);

	 /* auditing */
	 auditlog( "DEV_Stop", 0, logical_name, strlen(logical_name)+1 );
	 privilege(PRIV_LAPSE);
      }

      if (!just_stop_device)
      {  /* call the Unconfigure method */
         privilege(PRIV_ACQUIRE);
         if ((error = odm_run_method(cudv->PdDvLn->Unconfigure,params,
				     &out_ptr,&err_ptr)) == -1)
            do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"rmdev",
	               cudv->PdDvLn->Unconfigure,NULL,
		       EXIT_FATAL,"DEV_Unconfigure");

         meth_error(error,err_ptr,EXIT_FATAL,"DEV_Unconfigure",
		    cudv->PdDvLn->Unconfigure);

         /* auditing */
         auditlog("DEV_Unconfigure",0,logical_name,strlen(logical_name)+1);
         privilege(PRIV_LAPSE);
      }
   }

   if (delete_device)
   {  /* call the Undefine method */
      privilege(PRIV_ACQUIRE);
      if ((error = odm_run_method(cudv->PdDvLn->Undefine,params,
				  &out_ptr,&err_ptr)) == -1)
         do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"rmdev",
	            cudv->PdDvLn->Undefine, NULL, EXIT_FATAL, "DEV_Remove" );

      meth_error(error,err_ptr,EXIT_FATAL,"DEV_Remove",cudv->PdDvLn->Undefine);

      /* auditing */
      auditlog( "DEV_Remove", 0, logical_name, strlen(logical_name)+1 );
      privilege(PRIV_LAPSE);

      device_removed = TRUE;
   }

   /* should we call savebase??? */
   if (need_savebase)
   {  /* runtime attr exists - go ahead and call savebase */
      if ( error = odm_run_method( "/usr/sbin/savebase", "", &out_ptr, &err_ptr ))
         do_error( ERRSTR(CFG_SAVEBASE_FAILED), "rmdev",
		   err_ptr, NULL, EXIT_NOT, "DEV_Remove" );
   }

   if (!device_removed)
   {  /* get the current status */
      sprintf( criteria, "name = '%s'", logical_name );
      if ((int)(cudv = (struct CuDv *)
	          odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 1 )) < 0)
         do_error( ERRSTR(CFG_ODM_ACCESS),"rmdev",
  		   "CuDv", NULL, EXIT_FATAL, "DEV_Remove");
      if (cudv_info.num == 1)
	 stat_str( cudv->status, temp1 );
      else
         stat_str( -1, temp1 );
   }
   else
      strcpy( temp1, MSGSTR(CFG_MSG_DELETED) );

   /* display a message relating successful completion */
   sprintf( temp, "%s %s\n", logical_name, temp1);
   printf( temp );
}



/*------------------------------- rmdev_parse --------------------------------
 * NAME: rmdev_parse
 *                                                                    
 * FUNCTION: process arguments entered by the user
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
rmdev_parse( argc, argv )
int argc;			/* num args in argv */
char *argv[];			/* user's input */
{  
   FILE *fp;			/* file pointer */
   int c;			/* char from getopt */
   extern int optind;		/* from getopt */
   extern char *optarg;		/* from getopt */

   while ((c = getopt(argc,argv,"qhdSl:f:R")) != EOF)
   {  
      switch (c)
      {   case 'q' : /* quiet mode - no output */
		     fclose( stdout );
		     fclose( stderr );
		     break;
	  case 'h' : /* help message */
      		     printf(MSGSTR(CFG_RMDEV_USAGE));
		     exit(0);
		     break;
	  case 'd' : /* delete the device */
		     delete_device = TRUE;
		     break;
	  case 'S' : /* stop the device */
		     just_stop_device = TRUE;
		     break;
	  case 'l' : /* logical name */
         	     strcpy( logical_name, optarg );
		     break;
	  case 'f' : /* input from file */
		     args_from( optarg, rmdev_parse, "rmdev" );
		     break;
	  case 'R' : /* recurse switch */
		     recurse_specified = TRUE;
		     break;
	  default  : /* syntax error */
		     do_error( MSGSTR(CFG_RMDEV_USAGE),"rmdev",
				NULL,NULL,EXIT_FATAL,"DEV_Remove");
		     break;
      }/*switch (ch)*/
   }/*while getopt */

   /* anything left to process - error if there is */
   if (optind < argc)
      do_error( ERRSTR(CFG_MISC_SYNTAX_ERR),"rmdev",
		argv[optind],USAGE,EXIT_FATAL,"DEV_Remove");
}

