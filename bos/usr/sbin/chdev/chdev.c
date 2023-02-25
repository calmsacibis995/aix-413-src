static char sccsid[] = "@(#)32	1.13.1.4  src/bos/usr/sbin/chdev/chdev.c, cmdcfg, bos411, 9428A410j 2/18/94 14:07:25";
/*
 * COMPONENT_NAME: (CMDCFG)  Device specific config support cmds
 *
 * FUNCTIONS: chdev, chdev_parse
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <sys/signal.h>
#include "hlcmds.h"

#define DBG	0
#define USAGE MSGSTR(CFG_CHDEV_USAGE)



/*****************************************************************************/
/*			MODULE GLOBAL VARIABLES				     */
/*****************************************************************************/
char logical_name[NAMESIZE];    /* type name specified by user */
char *params = NULL;		/* ptr to parameter string */
int max = 0;			/* max chars in params */
char *attrs = NULL;		/* ptr to attribute string */
int max_attrs = 0;		/* max chars in attrs */
int attrs_present = FALSE;	/* >0 if "-a" option specified */
int temp_change = FALSE;	/* >0 if "-T" option specified */
int perm_change = FALSE;	/* >0 if "-P" option specified */
char temp[100];			/* temp string */


/*****************************************************************************/
/*				START OF CODE				     */
/*****************************************************************************/
main( argc, argv )
int argc;				/* # args in argv */
char *argv[];				/* command line arguments */
{  char criteria[MAX_CRITELEM_LEN];	/* search criteria */
   int error;				/* returned method error code */
   struct CuDv *cudv;			/* ptr to array of CuDv records */
   struct listinfo cudv_info;		/* CuDv listinfo */
   struct PdAt *pdat;			/* ptr to array of PdAt records */
   struct listinfo pdat_info;		/* PdAt info */
   char *out_ptr = NULL;  		/* pointers to buffers for */
   char *err_ptr = NULL;		/*    the method's stdout, stderr*/
   struct sigaction action;		/* parameters for sigaction */
   struct sigaction oldaction;		/* parameters for sigaction */

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
      do_error( ERRSTR(CFG_ODM_LOCK),"chdev",
		NULL, NULL, EXIT_FATAL, "DEV_Change" );

   /* initialize internal state */
   auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
   privilege(PRIV_LAPSE);
#ifdef MSG
   setlocale(LC_ALL,"");
#endif

   /* initialize internal variables */
   logical_name[0] = '\0';
   max = max_attrs = 0;
   catstr( &attrs, "-a \"", &max_attrs );

   /* parse user's input */
   chdev_parse( argc, argv );

   /* check for errors from the user */
   if (logical_name[0] == '\0')
      do_error(  ERRSTR(CFG_ARGOROPT_MISSING), "chdev", 
		"-l", USAGE, EXIT_FATAL, "DEV_Change" );
   else if (temp_change && perm_change)
      do_error(   ERRSTR(CFG_ARGOROPT_CONFLICT), "chdev", 
               "-P -T", USAGE, EXIT_FATAL, "DEV_Change" );

   /* get ready to use the ODM */
   odm_initialize();

   /* get the specified cudvomized device */
   sprintf( criteria, "name = '%s'", logical_name );
   if ((int)(cudv = (struct CuDv *)
	       odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 2 )) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS), "chdev", 
  		"CuDv", NULL, EXIT_FATAL, "DEV_Change" );
   if (!cudv_info.num)
      do_error( ERRSTR(CFG_ODM_NOCUDV), "chdev", 
  		logical_name, NULL, EXIT_FATAL, "DEV_Change" );
   else if (cudv_info.num > 1)
      do_error( ERRSTR(CFG_ODM_NONUNIQUE), "chdev", 
  		criteria, NULL, EXIT_FATAL, "DEV_Change" );
   else if (cudv->PdDvLn == NULL)
      do_error( ERRSTR(CFG_ODM_NOCUPD), "chdev", 
  		logical_name, NULL, EXIT_FATAL, "DEV_Change" );
   else if (cudv->status==DIAGNOSE)
      do_error(ERRSTR(CFG_DEV_DIAGNOSE), "chdev", logical_name,
                NULL, EXIT_FATAL, "DEV_Change");

   /* process the user's request */

   if (attrs_present)
   {  /* add attrs to the params string */
      catstr( &params, attrs, &max );
      catstr( &params, " \" ", &max );
   }

   /* invoke the change method */
   privilege( PRIV_ACQUIRE );
   if ((error = odm_run_method(cudv->PdDvLn->Change,params,
			       &out_ptr,&err_ptr)) == -1)
      do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"chdev",
                 cudv->PdDvLn->Change, NULL, EXIT_FATAL, "DEV_Change" );

   meth_error(error,err_ptr,EXIT_FATAL,"DEV_Change",cudv->PdDvLn->Change);

   /* should we call savebase??? - check for runtime attr */
   if ((int)(pdat = (struct PdAt *)
               odm_get_list(PdAt_CLASS,PHASE1_DISALLOWED,&pdat_info,1,1)) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS), "chdev", 
  		"PdAt", NULL, EXIT_FATAL, "DEV_Change" );
   else if (pdat_info.num)
   {  /* runtime attr exists - go ahead and call savebase */
      if ( error = odm_run_method( "/usr/sbin/savebase", "", &out_ptr, &err_ptr ))
         do_error( ERRSTR(CFG_SAVEBASE_FAILED), "chdev",
		   err_ptr, NULL, EXIT_NOT, "DEV_Change" );
   }

   /* auditing */
   auditlog( "DEV_Change", 0, params, strlen(params)+1 );
   privilege( PRIV_LAPSE );

   /* cleanup before exit */
   odm_terminate();

   /* display a message relating successful completion */
   sprintf( temp, MSGSTR(CFG_D_CHANGED), logical_name );
   printf( temp );

   exit( 0 );
}


/*------------------------------- chdev_parse --------------------------------
 * NAME: chdev_parse
 *                                                                    
 * FUNCTION: process the user's input
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
chdev_parse( argc, argv )
int argc;			/* num args in argv */
char *argv[];			/* user's input */
{  
   FILE *fp;			/* file pointer */
   int c;			/* char from getopt */
   extern int optind;		/* from getopt */
   extern char *optarg;		/* from getopt */

   while ((c = getopt(argc,argv,"qhl:p:w:TPf:a:")) != EOF)
   {  
      switch (c)
      {   case 'q' : /* quiet mode - no output */
		     fclose( stdout );
		     fclose( stderr );
		     break;
	  case 'h' : /* help message */
      		     printf(MSGSTR(CFG_CHDEV_USAGE));
		     exit(0);
		     break;
	  case 'l' : /* logical name */
         	     strcpy( logical_name, optarg );
		     sprintf( temp, "-l %s ", logical_name );
		     catstr( &params, temp, &max );
		     break;
	  case 'p' : /* parent name */
		     sprintf( temp, "-p %s ", optarg );
		     catstr( &params, temp, &max );
		     break;
	  case 'w' : /* parent name */
		     sprintf( temp, "-w %s ", optarg );
		     catstr( &params, temp, &max );
		     break;
	  case 'T' : /* parent name */
		     catstr( &params, "-T ", &max );
		     temp_change = TRUE;
		     break;
	  case 'P' : /* parent name */
		     catstr( &params, "-P ", &max );
		     perm_change = TRUE;
		     break;
	  case 'f' : /* input from file */
		     args_from( optarg, chdev_parse, "chdev" );
		     break;
	  case 'a' : /* attributes */
		     if (attrs_present == TRUE)
         	     	catstr( &attrs, " ", &max_attrs );
         	     catstr( &attrs, optarg, &max_attrs );
		     attrs_present = TRUE;
		     break;
	  default  : /* syntax error */
		     do_error( MSGSTR(CFG_CHDEV_USAGE),"chdev",
				NULL,NULL,EXIT_FATAL,"DEV_Change");
		     break;
      }/*switch (ch)*/
   }/*while getopt */

   /* anything left to process - error if there is */
   if (optind < argc)
      do_error( ERRSTR(CFG_MISC_SYNTAX_ERR),"chdev",
		argv[optind],USAGE,EXIT_FATAL,"DEV_Change");
}

