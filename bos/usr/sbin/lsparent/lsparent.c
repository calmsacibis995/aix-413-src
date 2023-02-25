static char sccsid[] = "@(#)88	1.7  src/bos/usr/sbin/lsparent/lsparent.c, cmdcfg, bos411, 9428A410j 5/19/93 08:56:51";
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
#include "hlcmds.h"
#include "lscf.h"
char *cmdname = "lsparent";

/*
 *	NAME:      main ( lsparent )
 *	FUNCTION:  list possible parent devices to
 *                 which connections can be made 
 *      EXECUTION ENVIRONMENT: 
 *                 high level user command
 */
main(argc,argv)
int argc;
char *argv[];
{
int i;	/* scratch */

#ifdef MSG
   setlocale(LC_ALL,"");
#endif
	parse_args( argc, argv );

        if (hflag)
		syntax();

	if(Pflag && Cflag){
		usage_error1(CFG_ARGOROPT_CONFLICT,"-P -C",CFG_LSPARENT_USAGE);
		}
	else if(!Pflag && !Cflag){
		usage_error1(CFG_ARGOROPT_MISSING, "-P -C",CFG_LSPARENT_USAGE);
		}
		
				/* get info from odm database */
	initialize_cfgodm();
	if(entry_name){
	        get_cu_dv(entry_name, entry_class,
			entry_subclass,entry_type,entry_state);
		entry_key = get_subclass(entry_name);
		cudv = NULL;
		}
	else if(!entry_key)
		usage_error1(CFG_ARGOROPT_MISSING,"-k -l",CFG_LSPARENT_USAGE);
		
	get_conn(NULL,entry_key);
	derive_pd_from_conn();		

					/* print results */
	if(Pflag){
				/* release lock and odm database */
		terminate_cfgodm();
		if(!Fflag)
			entry_fmt = "class type subclass description";
		Finit();
		if(columnize){
			if(print_hdr)
				Fparse(GETHDRSZ,PdDv_MODE,NULL);
			for(i=0;i<pd_info.num;i++)
				Fparse(GETVALSZ,PdDv_MODE,pddv + i);
			}
		if(print_hdr){
				Fparse(PRINTHDR,PdDv_MODE,NULL);
			printf("\n");
			}
		for(i=0;i<pd_info.num;i++)
			Fparse(PRINTVAL,PdDv_MODE,pddv + i);
		}
	else if(Cflag){
		derive_cu_from_pd();
				/* release lock and odm database */
		terminate_cfgodm();
		if(!Fflag)
			entry_fmt = "name status location description";
		Finit();
		if(columnize){
			if(print_hdr)
				Fparse(GETHDRSZ,CuDv_MODE,NULL);
			for(i=0;i<cu_info.num;i++)
				Fparse(GETVALSZ,CuDv_MODE,cudv + i);
			}
		if(print_hdr){
				Fparse(PRINTHDR,CuDv_MODE,NULL);
			printf("\n");
			}
		for(i=0;i<cu_info.num;i++)
			Fparse(PRINTVAL,CuDv_MODE,cudv + i);
		}

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

	while ((c = getopt(argc,argv,"f:CPF:Hhk:l:")) != EOF)
	{  switch (c)
	   {  case 'f' :
			 args_from( optarg, parse_args, "lsparent" );
			 break;
	      case 'C' :
			 Cflag = TRUE;
			 break;
	      case 'P' :
			 Pflag = TRUE;
			 break;
	      case 'F' :
			 if(entry_fmt)
				usage_error1(CFG_MISC_SYNTAX_ERR, "-F -F",
					CFG_LSPARENT_USAGE);
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
				usage_error1(CFG_MISC_SYNTAX_ERR,
					"-k -k",CFG_LSPARENT_USAGE);
			 entry_key = optarg;
			 break;
	      case 'l' :
			 if(entry_name)
				usage_error1(CFG_MISC_SYNTAX_ERR,
					"-l -l",CFG_LSPARENT_USAGE);
			 entry_name = optarg;
			 break;
	      default  :
			usage_error1(CFG_MISC_SYNTAX_ERR,argv[optind],
				CFG_LSPARENT_USAGE);
			 break;
	   }/*switch*/
	}/*while*/

	/* check for left-overs */
	if (optind < argc)
	   usage_error1(CFG_MISC_SYNTAX_ERR,argv[optind],CFG_LSPARENT_USAGE);

}/*parse_args*/


/*
 *        NAME: syntax
 *        FUNCTION: print usage message and exits
 */
syntax()
{
	printf(MSGSTR(CFG_LSPARENT_USAGE));
	exit(0);
}
