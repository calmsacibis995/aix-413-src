static char sccsid[] = "@(#)89	1.11  src/bos/usr/sbin/lsattr/lsattr.c, cmdcfg, bos41J, 9520A_all 4/27/95 13:26:29";
/*
 *   COMPONENT_NAME: CMDCFG
 *
 *   FUNCTIONS: Fprint
 *		main
 *		parse_args
 *		syntax
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include "hlcmds.h"
#include "lscf.h"
char *cmdname = "lsattr";

int Dflag = FALSE;
int Eflag = FALSE;
int Rflag = FALSE;

/*
 *	NAME:      main ( lsattr )
 *	FUNCTION:  list information about attributes of predefined and 
 *                 customized devices in the system
 *      EXECUTION ENVIRONMENT: 
 *                 high level user command
 */

main(argc,argv)
int argc;
char *argv[];
{
char uniquetype[BUFSIZE];
int rc = 0;

#ifdef MSG
   setlocale(LC_ALL,"");
#endif
				/* parse user's input */
	parse_args( argc, argv );

        if (hflag)
		syntax();

			/* get information from odm database */
	initialize_cfgodm();

	if (entry_name){
		if( entry_class || entry_subclass || entry_type){
			usage_error1(CFG_MISC_SYNTAX_ERR,
				"-l / -c,-s, -t",
				CFG_LSATTR_USAGE);
			}
		rc = get_cu_dv(entry_name,NULL,NULL,NULL,NULL);
		if(!cudv){
			error1(CFG_ODM_NOCUDV,entry_name);
			}
		else if(cu_info.num > 1){
			error1( CFG_ODM_NONUNIQUE,entry_name);
			}
		strcpy(uniquetype,cudv->PdDvLn_Lvalue);
		pddv = cudv->PdDvLn;
		pd_info.num = 1;

		/* Get the parent CuDv in the case that attributes of */
		/* type=E are stored under the parent's uniquetype    */
		if (strcmp(cudv->parent,"")) {
			rc = get_par_cu_dv(cudv->parent);
		 	if(!parcudv){
				error1(CFG_ODM_NOCUDV,cudv->parent);
			}

		}
	}
	else {
		get_pd_dv(entry_class,entry_subclass,entry_type);
		if(!pddv){
			sprintf(uniquetype,"%s/%s/%s",
				entry_class?entry_class:"*",
				entry_subclass?entry_subclass:"*",
				entry_type?entry_type:"*");
			error1(CFG_ODM_NOPDDV,
				uniquetype);
			}
		else if(pd_info.num > 1){
			sprintf(uniquetype,"%s/%s/%s",
				entry_class?entry_class:"*",
				entry_subclass?entry_subclass:"*",
				entry_type?entry_type:"*");
			error1( CFG_ODM_NONUNIQUE,uniquetype);
			}
		sprintf(uniquetype,"%s",
			  pddv->uniquetype);
	}

	if(entry_attrs)
	     	rc = get_attrs(  entry_attrs, entry_name, uniquetype);
	else
	     	getallattrs(  entry_name, uniquetype);

				/* db work over - release lock & odm */	
	terminate_cfgodm();
				/* print results */
	if ( rc != -1 ) {
		if(Rflag ){
			if( entry_attrs && nattrs==1)
				Rprint();
			else {
				usage_error1(CFG_MISC_SYNTAX_ERR,
					"-a",
					CFG_LSATTR_USAGE);
				}
			}
		else if (Fflag)
			Fprint();
		else if(Dflag){
			if(colon_fmt)
				DOprint();
			else {
				entry_fmt =
			    	"attribute deflt description user_settable";
				Fprint();
				}
			}
		else if(Eflag){
			if(colon_fmt)
				EOprint();
			else {
			  entry_fmt =
		    	    "attribute value description user_settable";
			  Fprint();
		 	 }
			}
		else
			usage_error1(CFG_ARGOROPT_MISSING,
				"-R -D -E -F",CFG_LSATTR_USAGE);
	}
	exit(rc);
}
Fprint()
{
int i;
	Finit();
	if(columnize){
		if(print_hdr)
			Fparse(GETHDRSZ,Attr_MODE,NULL);
		for(i=0;i<nattrs;i++)
			Fparse(GETVALSZ,Attr_MODE,combined_attr + i);
			}
	if(print_hdr){
			Fparse(PRINTHDR,Attr_MODE,NULL);
		printf("\n");
		}
	for(i=0;i<nattrs;i++)
		Fparse(PRINTVAL,Attr_MODE,combined_attr + i);
}
/*
 *      NAME: parse_args
 *      FUNCTION: gets the arguments from the command line,
 *      possibly recursing to allow input from a file or stdin
 */
parse_args(argc,argv)
int argc;
char *argv[];
{ int c;
  extern int optind;
  extern char *optarg;

	while ((c = getopt(argc,argv,"f:l:c:s:t:a:DEROF:hH")) != EOF)
	{  switch (c)
	   {  case 'f' :
			 args_from( optarg, parse_args, "lsattr" );
			 break;
	      case 'l' :
			 if(entry_name)
				usage_error1(CFG_MISC_SYNTAX_ERR,
				   "-l -l",CFG_LSATTR_USAGE);
			 entry_name = optarg;
			 break;
	      case 'c' :
			 if(entry_class)
				usage_error1(CFG_MISC_SYNTAX_ERR,
				    "-c -c",CFG_LSATTR_USAGE);
			 entry_class = optarg;
			 break;
	      case 's' :
			 if(entry_subclass)
				usage_error1(CFG_MISC_SYNTAX_ERR,
				    "-s -s",CFG_LSATTR_USAGE);
			 entry_subclass = optarg;
			 break;
	      case 't' :
			 if(entry_type)
				usage_error1(CFG_MISC_SYNTAX_ERR,
					"-t -t",CFG_LSATTR_USAGE);
			 entry_type = optarg;
			 break;
	      case 'a' :
			 if(!entry_attrs)
			 {  entry_attrs = (char *)malloc(strlen(optarg)+1);
			    strcpy(entry_attrs,optarg);
			 }
			 else
			 {  entry_attrs = (char *)realloc(entry_attrs,
					strlen(entry_attrs) +
					strlen(optarg) + 2);
			    strcat(entry_attrs," ");
			    strcat(entry_attrs,optarg);
			 }
			 break;
	      case 'D' :
			 Dflag = TRUE;
			 break;
	      case 'E' :
			 Eflag = TRUE;
			 break;
	      case 'R' :
			 Rflag = TRUE;
			 break;
	      case 'O' :
			 colon_fmt = TRUE;
			 break;
	      case 'F' :
			 if (entry_fmt)
				usage_error1(CFG_MISC_SYNTAX_ERR,
				   "-F -F",CFG_LSATTR_USAGE);
			 Fflag = TRUE;
			 entry_fmt = optarg;
			 break;
	      case 'h' :
			 hflag++;
			 break;
	      case 'H' :
			 print_hdr++;
			 break;
	      default  :
			 usage_error1(CFG_MISC_SYNTAX_ERR,argv[optind],
				CFG_LSATTR_USAGE);
			 break;
	   }/*switch*/
	}/*while*/

	if (optind < argc)
	   usage_error1(CFG_MISC_SYNTAX_ERR,argv[optind],
				CFG_LSATTR_USAGE);
}/*parse_args*/


/*
 *        NAME: syntax
 *        FUNCTION: print usage message and exits
 */
syntax()
{
	printf(MSGSTR(CFG_LSATTR_USAGE));
        exit(0);
}
