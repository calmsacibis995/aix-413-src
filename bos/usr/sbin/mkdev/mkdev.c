static char sccsid[] = "@(#)30	1.14.2.6  src/bos/usr/sbin/mkdev/mkdev.c, cmdcfg, bos411, 9428A410j 3/5/94 11:28:36";
/*
 * COMPONENT_NAME: (CMDCFG)  Device specific config support cmds
 *
 * FUNCTIONS: mkdev, mkdev_parse, recurse_cfg, getcudvlist
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
#include <ctype.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <sys/signal.h>
#include <sys/statfs.h>
#include "hlcmds.h"

#define USAGE MSGSTR(CFG_MKDEV_USAGE)

struct CuDv *getcudvlist();


/*****************************************************************************/
/*			MODULE GLOBAL VARIABLES				     */
/*****************************************************************************/
char class[NAMESIZE];		/* class */
char subclass[NAMESIZE];	/* subclass */
char type[NAMESIZE];		/* type name specified by user */
char parent[NAMESIZE];		/* parent name specified by user */
char where[NAMESIZE]; 		/* connection location on parent */
char logical_name[NAMESIZE]; 	/* type name specified by user */
int define_specified     = FALSE;/* >0 if define method to be called*/
int configure_specified  = TRUE;/* >0 if configure meth to be called*/
int start_specified      = TRUE;/* >0 if start method to be called*/
int recurse_specified = FALSE;  /* recursively configure ancestors */
int read_from_stdin      = FALSE;/* >0 while read from stdin specified*/
int attrs_present	= FALSE;/* >0 if -a specified */
char *attrs		= NULL; /* ptr to attr string */
int max_attrs		= 0;	/* max size of attrs */
int need_savebase	= FALSE;/* set if savebase needed */

/*****************************************************************************/
/*				START OF CODE				     */
/*****************************************************************************/
main( argc, argv )
int argc;				/* # args in argv */
char *argv[];				/* command line arguments */
{  char criteria[MAX_CRITELEM_LEN];	/* search criteria */
   char *params;			/* method parameter string */
   int max;				/* max chars allowed in params */
   char temp[100];			/* temp string */
   char temp1[100];			/* temp string */
   int error = 0;			/* error code */
   struct PdDv *pddv;			/* ptr to array of PdDv records*/
   struct listinfo pddv_info;		/* listinfo for PdDv */
   struct PdAt *pdat;			/* ptr to array of PdAt records */
   struct listinfo pdat_info;		/* PdAt info */
   struct CuDv *cudv;			/* ptr to array of CuDv records */
   struct listinfo cudv_info;		/* CuDv listinfo */
   int last;				/* last char in logical_name */
   char *out_ptr = NULL;		/* pointers to buffers for stdout */
   char *err_ptr = NULL;		/* pointer to buffer for stderr*/
   struct sigaction action;		/* parameters for sigaction */
   struct sigaction oldaction;		/* parameters for sigaction */
   struct statfs statb;

   statfs("/", &statb);
   if(statb.f_bavail == 0)
   {
     fprintf(stderr, "Not enough space in the root directory\n");
     exit(EXIT_FATAL);
   }
   statfs("/tmp", &statb);
   if(statb.f_bavail == 0)
   {
     fprintf(stderr, "Not enough space in the /tmp directory\n");
     exit(EXIT_FATAL);
   }

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
      do_error( ERRSTR(CFG_ODM_LOCK),"mkdev",
		NULL, NULL, EXIT_FATAL, "DEV_Create" );

   /* initialize internal state */
   auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
   privilege(PRIV_LAPSE);
#ifdef MSG
   setlocale(LC_ALL,"");
#endif

   /* initialize internal variables */
   class[0] = subclass[0] = type[0] = parent[0] = 
	where[0] = logical_name[0] = '\0';
   max = max_attrs = 0;
   catstr( &attrs, " -a \"", &max_attrs );

   /* parse user's input */
   mkdev_parse( argc, argv );

   /* check for errors from the user */
   if (!define_specified)
   {  if (logical_name[0] == '\0')
         do_error(   ERRSTR(CFG_ARGOROPT_MISSING),"mkdev","-t | -c | -s | -l",
  		     USAGE,  EXIT_FATAL, "DEV_Create" );
      else if (attrs_present)
         do_error(   ERRSTR(CFG_ARGOROPT_CONFLICT),"mkdev","-a",
  		     USAGE,  EXIT_FATAL, "DEV_Create" );
   }

   /* get ready to use the ODM */
   odm_initialize();

   /* get information from the ODM for the specified device */

   if (define_specified)
   {
      /* initialize the criteria */
      criteria[0] = '\0';
      if (type[0] != '\0')
         sprintf( criteria, "type = '%s'", type );
      if (subclass[0] != '\0')
      {  if (criteria[0] == '\0')
            sprintf( temp, "subclass = '%s'", subclass );
	 else
            sprintf( temp, " AND subclass = '%s'", subclass );
         strcat( criteria, temp );
      }
      if (class[0] != '\0')
      {  if (criteria[0] == '\0')
            sprintf( temp, "class = '%s'", class );
         else
            sprintf( temp, " AND class = '%s'", class ); 
         strcat( criteria, temp );
      }

      /* get the specified pddvefined device */
      if ((int)(pddv = (struct PdDv *)
		  odm_get_list( PdDv_CLASS, criteria, &pddv_info, 1, 1 )) < 0)
         do_error( ERRSTR(CFG_ODM_ACCESS),"mkdev",
		   "PdDv", NULL,  EXIT_FATAL, "DEV_Create" );
      else if (pddv_info.num == 0)
         do_error( ERRSTR(CFG_ODM_NOPDDV),"mkdev",
		   criteria, NULL,  EXIT_FATAL, "DEV_Create" );
      else if (pddv_info.num > 1)
         do_error( ERRSTR(CFG_ODM_NONUNIQUE),"mkdev",
		   criteria, NULL,  EXIT_FATAL, "DEV_Create" );

      /* initialize the define_method parameter string */
      sprintf(temp,"-c %s -s %s -t %s",
	        pddv->class, pddv->subclass, pddv->type );
      catstr( &params, temp, &max );
      if (parent[0] != '\0')
      {  sprintf(temp," -p %s", parent);
         catstr( &params, temp, &max );
      }
      if (where[0] != '\0')
      {  sprintf(temp," -w %s", where);
         catstr( &params, temp, &max );
      }
      if (logical_name[0] != '\0')
      {  sprintf(temp," -l %s", logical_name);
         catstr( &params, temp, &max );
      }

      /* initialize error message string */
      sprintf(temp,"mkdev (%s)",pddv->Define);

      /* invoke the define method */
      if (pddv->Define[0] == '\0')
         do_error(  ERRSTR(CFG_ODM_NO_DEFINE),"mkdev",
	            pddv->Define, NULL, EXIT_FATAL, "DEV_Create" );
      privilege(PRIV_ACQUIRE);
      if ((error=odm_run_method(pddv->Define,params,&out_ptr,&err_ptr)) == -1)
         do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"mkdev",
	            pddv->Define, NULL, EXIT_FATAL, "DEV_Create" );

      meth_error(error,err_ptr,EXIT_FATAL,"DEV_Create",pddv->Define);

      /* assuming logical name returned on stdout (in out_ptr) */
      last = strlen(out_ptr) - 1;
      if (isspace(out_ptr[last]))
         out_ptr[last] = '\0';
      sprintf( logical_name, "%s", out_ptr );

      /* auditing */
      *params = '\0';
      sprintf( temp, "%s,%s,%s,%s,", pddv->class, pddv->type,
	  	 logical_name,parent );
      catstr( &params, temp, &max );
      auditlog( "DEV_Create", 0, params, strlen(params)+1 );

      if (attrs_present)
      {  /* call the change method */
         *params = '\0';
	 sprintf( temp, "-l %s ", logical_name );
	 catstr( &params, temp, &max );
	 catstr( &params, attrs, &max );
   
         /* end-quote the attribute string */
         catstr( &params, "\" ", &max );
   
         /* initialize error message string */
         sprintf(temp1,"mkdev (%s)", pddv->Change );

         /* invoke the change method */
         if (pddv->Change[0] == '\0')
            do_error(  ERRSTR(CFG_ODM_NO_CHANGE),"mkdev",
	               pddv->Change, NULL, EXIT_FATAL, "DEV_Create" );
         if ((error=odm_run_method(pddv->Change,params,&out_ptr,&err_ptr))==-1)
            do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"mkdev",
	               pddv->Change, NULL, EXIT_FATAL, "DEV_Create" );

         if ( meth_error(error,err_ptr,EXIT_NOT,"DEV_Create",pddv->Change) )
         {  /* method error - remove the device we just defined */

            /* init error message string */
            sprintf(temp1,"mkdev (%s)", pddv->Undefine );

	    /* invoke the undefine method */ 
            if (pddv->Undefine[0] == '\0')
               do_error(  ERRSTR(CFG_ODM_NO_UNDEFINE),"mkdev",
	                  pddv->Undefine, NULL, EXIT_FATAL, "DEV_Create" );
	    if ((error = odm_run_method(pddv->Undefine,temp,
					&out_ptr,&err_ptr)) == -1)
               do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"mkdev",
	                  pddv->Undefine, NULL, EXIT_FATAL, "DEV_Create" );

            meth_error(error,err_ptr,EXIT_FATAL,"DEV_Create",pddv->Undefine);

	    exit( EXIT_FATAL );
	 }/* if change method error */

         /* auditing */
         auditlog( "DEV_Change", 0, params, strlen(params)+1 );

	 attrs_present = FALSE;
	 free( attrs );
      } /* attrs_present */

      privilege(PRIV_LAPSE);

      /* free up temp strings */
      free( params );
      free( attrs );
      max = max_attrs = 0;

   }/* define_specified */

   /* get the specified customized device */
   sprintf( criteria, "name = '%s'", logical_name );
   if ((int)(cudv = (struct CuDv *)
	       odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 2 )) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS),"mkdev","CuDv",
                NULL, EXIT_FATAL, "DEV_Configure" );
   else if (!cudv_info.num )
      do_error( ERRSTR(CFG_ODM_NOCUDV),"mkdev",criteria,
                NULL, EXIT_FATAL, "DEV_Configure" );
   else if (cudv_info.num > 1)
      do_error( ERRSTR(CFG_ODM_NONUNIQUE),"mkdev",criteria,
                NULL, EXIT_FATAL, "DEV_Configure" );
    else if ( cudv->PdDvLn == NULL )
      do_error( ERRSTR(CFG_ODM_NOCUPD),"mkdev",logical_name,
		NULL, EXIT_FATAL, "DEV_Configure" );
   else if ( cudv->status == DIAGNOSE )
      do_error( ERRSTR(CFG_DEV_DIAGNOSE), "mkdev", logical_name,
                NULL, EXIT_FATAL, "DEV_Configure");

   /* check the customized status */
   if (cudv->status == DEFINED)
   {  /* the default is to bring the device to the available state */
      /*    which means that if there is a start method, execute it */
      /*    after a configure is done */
      /* if "-S" option was specified, this means don't bring to the */
      /*    available state; so, if there is not start method, then */
      /*    "-S" means don't configure */
      if (!start_specified && !cudv->PdDvLn->Start[0])
         configure_specified = FALSE;
   }
   else if (cudv->status == STOPPED)
   {  configure_specified = FALSE;
   }
   else if (cudv->status == AVAILABLE)
   {  
      start_specified = FALSE;
   }

   /* should we call savebase??? - check for runtime attr */
   if ((int)(pdat = (struct PdAt *)
               odm_get_list(PdAt_CLASS,PHASE1_DISALLOWED,&pdat_info,1,1)) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS), "mkdev", 
  		"PdAt", NULL, EXIT_FATAL, "DEV_Create" );
   else if (pdat_info.num)
   {  /* runtime attr exists - need savebase */
      need_savebase = TRUE;
   }

   /* process the user's request */
   recurse_cfg(cudv, define_specified, configure_specified, 
	start_specified, logical_name);

   /* cleanup before exit */
   odm_terminate();

   exit( 0 );
}



/*------------------------------- recurse_cfg --------------------------------
 * NAME: recurse_cfg
 *                                                                    
 * FUNCTION: If recurse specified, check parent and try to configure if needed
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
recurse_cfg(cudv, define_specified, configure_specified, 
		start_specified, logical_name)
struct CuDv *cudv;
int define_specified, configure_specified, start_specified;
char *logical_name;
{
   struct CuDv *cudvp;			/* CuDv for parent */
   char criteria[MAX_CRITELEM_LEN];	/* search criteria */
   char temp[100];			/* temp string */
   char temp1[100];			/* temp string */
   char *out_ptr = NULL;		/* pointers to buffers for stdout */
   char *err_ptr = NULL;		/* pointer to buffer for stderr*/
   int error = 0;			/* error code */
   struct listinfo cudv_info;		/* CuDv listinfo */
      
   /* Check to see if the parent exists AND if it is in diag state */
   if ( cudv->parent[0] != '\0' )
   {
      sprintf( criteria, "name='%s'", cudv->parent);
      cudvp = getcudvlist(criteria, cudv->parent);
      if ( cudvp->status == DIAGNOSE )
          do_error( ERRSTR(CFG_PAR_DIAGNOSE), "mkdev", cudv->parent,
	            NULL, EXIT_FATAL, "DEV_Configure");
   }
   if (configure_specified) {
      if( recurse_specified && (cudv->parent[0] != '\0') ) {
            if (cudvp->status == DEFINED) {	/* parent is defined */
		recurse_cfg(cudvp, FALSE, TRUE, TRUE,
			cudvp->name);  /* config parent */
	    }
	/* falls through if parent is already configured */
      } 
	
      /* init error message string */
      sprintf(temp1,"mkdev (%s)", cudv->PdDvLn->Configure );

      /* initialize the parameter string */
      sprintf( temp, "-l %s", cudv->name);

      /* invoke the configure method */
      if (cudv->PdDvLn->Configure[0] == '\0')
         do_error(  ERRSTR(CFG_ODM_NO_CONFIGURE),"mkdev",
                    cudv->PdDvLn->Configure, NULL, EXIT_FATAL, "DEV_Create" );
      privilege(PRIV_ACQUIRE);

      if ((error = odm_run_method(cudv->PdDvLn->Configure,temp,
				  &out_ptr,&err_ptr) ) == -1) {
         do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"mkdev",
	            cudv->PdDvLn->Configure, NULL, EXIT_FATAL, "DEV_Create" );
	}

      if ( meth_error(error,err_ptr,EXIT_NOT,"DEV_Create",
		      cudv->PdDvLn->Configure) )
      { 
	 if (define_specified)
	 { 
            /* init error message string */
            sprintf(temp1,"mkdev (%s)", cudv->PdDvLn->Undefine );

	    /* invoke the undefine method */ 
            if (cudv->PdDvLn->Undefine[0] == '\0')
               do_error(  ERRSTR(CFG_ODM_NO_UNDEFINE),"mkdev",
                          cudv->PdDvLn->Undefine, NULL, EXIT_FATAL, 
			  "DEV_Create" );
	    if ((error = odm_run_method(cudv->PdDvLn->Undefine,temp,
					&out_ptr,&err_ptr)) == -1)
               do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"mkdev",
	                  cudv->PdDvLn->Undefine,NULL,EXIT_FATAL,"DEV_Create");

            meth_error(error,err_ptr,EXIT_FATAL,"DEV_Create",
		       cudv->PdDvLn->Undefine);
	 }

	 exit( EXIT_FATAL );
      }
      privilege(PRIV_LAPSE);
   } /* configure_specified */

   /* check the current status */
   sprintf( criteria, "name = '%s'", logical_name );
   if ((int)(cudv = (struct CuDv *)
	       odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 2 )) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS),"mkdev","CuDv",
                NULL, EXIT_FATAL, "DEV_Configure" );
   else if (!cudv_info.num )
      do_error( ERRSTR(CFG_ODM_NOCUDV),"mkdev",criteria,
                NULL, EXIT_FATAL, "DEV_Configure" );
   else if (cudv_info.num > 1)
      do_error( ERRSTR(CFG_ODM_NONUNIQUE),"mkdev",criteria,
                NULL, EXIT_FATAL, "DEV_Configure" );
   else if(cudv->PdDvLn == NULL)
      do_error( ERRSTR(CFG_ODM_NOPDDV),"mkdev",logical_name,
		NULL, EXIT_FATAL,"DEV_Configure" );

   if ((!cudv->PdDvLn->Start[0]) ||
       (cudv->status == AVAILABLE))
      start_specified = FALSE;

   if (start_specified)
   {  /* initialize error message string */
      sprintf(temp1,"mkdev (%s)", cudv->PdDvLn->Start );

      /* initialize the parameter string */
      sprintf( temp, "-l %s ", logical_name);

      /* invoke the start method */
      if (cudv->PdDvLn->Start[0] == '\0')
         do_error(  ERRSTR(CFG_ODM_NO_START),"mkdev",
                    cudv->PdDvLn->Start, NULL, EXIT_FATAL, "DEV_Create" );
      privilege(PRIV_ACQUIRE);
      if ((error = odm_run_method( cudv->PdDvLn->Start, temp, 
				   &out_ptr, &err_ptr)) == -1)
         do_error(  ERRSTR(CFG_ODM_RUN_METHOD),"mkdev",
	            cudv->PdDvLn->Start, NULL, EXIT_FATAL, "DEV_Create" );

      meth_error(error,err_ptr,EXIT_FATAL,"DEV_Start",cudv->PdDvLn->Start);

      /* auditing */
      auditlog( "DEV_Start", 0, logical_name, strlen(logical_name)+1 );

      privilege(PRIV_LAPSE);
   }

   /* should we call savebase??? */
   if (need_savebase)
   {  /* runtime attr exists - go ahead and call savebase */
      if ( error = odm_run_method( "/usr/sbin/savebase", "", &out_ptr, &err_ptr ))
         do_error( ERRSTR(CFG_SAVEBASE_FAILED), "mkdev",
		   err_ptr, NULL, EXIT_NOT, "DEV_Create" );
   }

   /* get current status */
   sprintf( criteria, "name = \'%s\'", logical_name );
   if ((int)(cudv = (struct CuDv *)
	       odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 1 )) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS),"mkdev","CuDv",
                NULL, EXIT_FATAL, "DEV_Configure" );
   if (cudv_info.num == 1)
      stat_str( cudv->status, temp1 );
   else
      stat_str( -1, temp1 );

   /* display a message relating successful completion */
   sprintf(temp,"%s %s\n", logical_name, temp1 );
   printf( temp );

} /* recurse_cfg */


/*------------------------------- getcudvlist --------------------------------
 * NAME: getcudvlist
 *                                                                    
 * FUNCTION: Call odm_get_list on CuDv to get the parent device.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: Checks for errors.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
struct CuDv *
getcudvlist( criteria, logical_name )
char *criteria;
char *logical_name;
{
   struct CuDv *cudv;
   struct listinfo cudv_info; 

   if ( (cudv = (struct CuDv *)
	      odm_get_list( CuDv_CLASS, criteria, &cudv_info, 1, 2 )) < 0)
      do_error( ERRSTR(CFG_ODM_ACCESS),"mkdev","CuDv",
                NULL, EXIT_FATAL, "DEV_Configure" );
   else if (!cudv_info.num )
      do_error( ERRSTR(CFG_ODM_NOCUDV),"mkdev",criteria,
                NULL, EXIT_FATAL, "DEV_Configure" );
   else if (cudv_info.num > 1)
      do_error( ERRSTR(CFG_ODM_NONUNIQUE),"mkdev",criteria,
                NULL, EXIT_FATAL, "DEV_Configure" );
    else if ( cudv->PdDvLn == NULL )
      do_error( ERRSTR(CFG_ODM_NOCUPD),"mkdev",logical_name,
		NULL, EXIT_FATAL, "DEV_Configure" );
    return cudv;
}


/*------------------------------- mkdev_parse --------------------------------
 * NAME: mkdev_parse
 *                                                                    
 * FUNCTION: parse user's input
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
mkdev_parse( argc, argv )
int argc;			/* num args in argv */
char *argv[];			/* user's input */
{  
   FILE *fp;			/* file pointer */
   int c;			/* char from getopt */
   extern int optind;		/* from getopt */
   extern char *optarg;		/* from getopt */
   int i;

   while ((c = getopt(argc,argv,"qhdc:s:t:p:w:l:f:a:SR")) != EOF)
   {  
      switch (c)
      {   case 'q' : /* quiet mode - no output */
		     fclose( stdout );
		     fclose( stderr );
		     break;
	  case 'h' : /* help message */
      		     printf(MSGSTR(CFG_MKDEV_USAGE));
		     exit(0);
		     break;
	  case 'c' : /* class name */
         	     strcpy( class, optarg );
		     define_specified = TRUE;
		     break;
	  case 's' : /* subclass */
         	     strcpy( subclass, optarg );
		     define_specified = TRUE;
		     break;
	  case 't' : /* type */
	 	     strcpy( type, optarg );
		     define_specified = TRUE;
		     break;
	  case 'p' : /* parent */
         	     strcpy( parent, optarg );
		     break;
	  case 'w' : /* where */
         	     strcpy( where, optarg );
		     break;
	  case 'l' : /* logical name */
         	     strcpy( logical_name, optarg );
		     break;
	  case 'd' : /* define only specified */
      		     define_specified = TRUE;
         	     configure_specified = FALSE;
	 	     start_specified = FALSE;
		     break;
	  case 'f' : /* input from file */
		     args_from( optarg, mkdev_parse, "mkdev" );
		     break;
	  case 'a' : /* attributes */
         	     catstr( &attrs, optarg, &max_attrs );
         	     catstr( &attrs, " ", &max_attrs );
		     attrs_present = TRUE;
		     break;
	  case 'S' : /* don't start */
	 	     start_specified = FALSE;
		     break;
	  case 'R' : /* recursive config - only if not -d or -S */
	 	     recurse_specified = TRUE;
		     break;
	  default  : /* syntax error */
		     do_error( MSGSTR(CFG_MKDEV_USAGE),"mkdev",
				NULL,NULL,EXIT_FATAL,"Dev_Create");
		     break;
      }/*switch (ch)*/
   }/*while getopt */

   /* only accept -R if not -d or -S */
   if (!start_specified && recurse_specified)
	do_error( MSGSTR(CFG_MKDEV_USAGE),"mkdev",
		NULL,NULL,EXIT_FATAL,"Dev_Create");

   /* anything left to process - error if there is */
   if (optind < argc)
      do_error( ERRSTR(CFG_MISC_SYNTAX_ERR),"mkdev",
		argv[optind],USAGE,EXIT_FATAL,"Dev_Create");
}

