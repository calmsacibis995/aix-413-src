static char sccsid[] = "@(#)87	1.8  src/bos/usr/sbin/lsconn/lsconn.c, cmdcfg, bos411, 9428A410j 11/4/93 12:32:08";
/*
 *   COMPONENT_NAME: CMDCFG
 *
 *   FUNCTIONS: main
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
#include <cf.h>
#include "hlcmds.h"
#include "lscf.h"
char *cmdname = "lsconn";

/*
 *	NAME:      main ( lsconn )
 *	FUNCTION:  list information about possible child connections 
 *                 to predefined and 
 *                 customized devices in the system
 *      EXECUTION ENVIRONMENT: 
 *                 high level user command
 */
main(argc,argv)
int argc;
char *argv[];
{
int i;
char *uniquetype = NULL;
char unique[BUFSIZ];

#ifdef MSG
   setlocale(LC_ALL,"");
#endif

	parse_args( argc, argv );

        if (hflag)
		syntax();

	if(entry_parent && (entry_class || entry_subclass || entry_type)){
		usage_error1(CFG_ARGOROPT_CONFLICT, "-p {-c -s -t}",CFG_LSCONN_USAGE);
		}
	else if(!entry_parent && !(entry_class || entry_subclass || entry_type)){
		usage_error1(CFG_ARGOROPT_MISSING,  "-p {-c -s -t}",CFG_LSCONN_USAGE);
		}

	initialize_cfgodm();
					/* get connection key */
	if(entry_name){
	        get_cu_dv(entry_name, NULL,NULL,NULL,NULL);
		if(!cu_info.num){
			error1(CFG_ODM_NOCUDV,entry_name);
			}
		if(cu_info.num > 1){
			error1(CFG_ODM_NONUNIQUE,entry_name);
			}
		entry_key = get_subclass(entry_name);
		cudv = NULL;
		}
	else if(!entry_key){
		if(!Fflag)
			entry_fmt = "connwhere connkey";
		}
					/* get device-> crit for conns */
	if(entry_parent){
		get_cu_dv(entry_parent,NULL,NULL,NULL,NULL);
		if(!cu_info.num){
			error1(CFG_ODM_NOCUDV,entry_parent);
			}
		if(cu_info.num > 1){
			error1(CFG_ODM_NONUNIQUE,entry_parent);
			}
		uniquetype = cudv->PdDvLn_Lvalue;
		}
	else  {
		get_pd_dv(entry_class,entry_subclass,entry_type);
		if(!pd_info.num){
			sprintf(unique,"%s/%s/%s",
				entry_class?entry_class:"*",
				entry_subclass?entry_subclass:"*",
				entry_type?entry_type:"*");
			error1(CFG_ODM_NOPDDV,
				unique);
			}
		if(pd_info.num != 1){
			sprintf(unique,"%s/%s/%s",
				entry_class?entry_class:"*",
				entry_subclass?entry_subclass:"*",
				entry_type?entry_type:"*");
			error1(CFG_ODM_NONUNIQUE,unique);
			}
		uniquetype = pddv->uniquetype;
		}


					/* get conns using crit */
	get_conn(uniquetype,entry_key);

	terminate_cfgodm();
				/* print results */
	if(!entry_fmt)
		entry_fmt = "connwhere";
	Finit();
	if(columnize){
		if(print_hdr)
			Fparse(GETHDRSZ,PdCn_MODE,NULL);
			for(i=0;i<conn_info.num;i++)
				Fparse(GETVALSZ,PdCn_MODE,conn + i);
		}
	if(print_hdr){
		Fparse(PRINTHDR,PdCn_MODE,NULL);
		printf("\n");
		}
	for(i=0;i<conn_info.num;i++)
		Fparse(PRINTVAL,PdCn_MODE,conn + i);

	exit(0);
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

	while ((c = getopt(argc,argv,"f:F:Hhk:l:p:c:s:t:")) != EOF)
	{  switch (c)
	   {  case 'f' :
			 args_from( optarg, parse_args, "lsconn" );
			 break;
	      case 'F' :
			 if(entry_fmt)
				usage_error1(CFG_MISC_SYNTAX_ERR,"-F -F",
					CFG_LSCONN_USAGE);
			 entry_fmt = optarg;
			 Fflag = TRUE;
			 break;
	      case 'H' :
			 print_hdr++;
			 break;
	      case 'h' :
			 hflag++;
			 break;
	      case 'k' :
			 if(entry_key)
				usage_error1(CFG_MISC_SYNTAX_ERR,"-k -k",
					CFG_LSCONN_USAGE);
			 entry_key = optarg;
			 break;
	      case 'l' :
			 if(entry_name)
				usage_error1(CFG_MISC_SYNTAX_ERR, "-l -l",
					CFG_LSCONN_USAGE);
			 entry_name = optarg;
			 break;
	      case 'p' :
			 if(entry_parent)
				usage_error1(CFG_MISC_SYNTAX_ERR,"-p -p",
					CFG_LSCONN_USAGE);
			 entry_parent = optarg;
			 break;
	      case 'c' :
			 if(entry_class)
				usage_error1(CFG_MISC_SYNTAX_ERR,"-c -c",
					CFG_LSCONN_USAGE);
			 entry_class = optarg;
			 break;
	      case 's' :
			 if(entry_subclass)
				usage_error1(CFG_MISC_SYNTAX_ERR,"-s -s",
					CFG_LSCONN_USAGE);
			 entry_subclass = optarg;
			 break;
	      case 't' :
			 if(entry_type)
				usage_error1(CFG_MISC_SYNTAX_ERR,"-t -t",
					CFG_LSCONN_USAGE);
			 entry_type = optarg;
			 break;
	      default  :
	   		 usage_error1(CFG_MISC_SYNTAX_ERR,optarg,
				      CFG_LSCONN_USAGE);
			 break;
	   }/*switch*/
	}/*while*/

	/* check for left-overs */
	if (optind < argc)
	   usage_error1(CFG_MISC_SYNTAX_ERR,optarg,CFG_LSCONN_USAGE);
}


/*
 *        NAME: syntax
 *        FUNCTION: print usage message and exits
 */
syntax()
{
	printf(MSGSTR(CFG_LSCONN_USAGE));
   	exit(0);
}
