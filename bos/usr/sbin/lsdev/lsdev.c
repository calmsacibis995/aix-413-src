static char sccsid[] = "@(#)86	1.7  src/bos/usr/sbin/lsdev/lsdev.c, cmdcfg, bos411, 9428A410j 5/19/93 08:55:49";
/*
 *   COMPONENT_NAME: CMDDEV
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

char *cmdname = "lsdev";

/*
 *	NAME:      main ( lsdev )
 *	FUNCTION:  list information about predefined and 
 *                 customized devices in the system
 *      EXECUTION ENVIRONMENT: 
 *                 high level user command
 */

main(argc,argv)
int argc;
char *argv[];
{
int i;			/* scratch */

#ifdef MSG
   setlocale(LC_ALL,"");
#endif
	parse_args( argc, argv );

        if (hflag)
		syntax();

	if(Pflag && Cflag){
		usage_error1(CFG_ARGOROPT_CONFLICT, "-P -C",CFG_LSDEV_USAGE);
		}
	else if(!Pflag && !Cflag){
		usage_error1(CFG_ARGOROPT_MISSING,  "-P -C",CFG_LSDEV_USAGE);
		}

	initialize_cfgodm();
		
	if(Pflag){
		if(entry_state)
			usage_error1(CFG_ARGOROPT_CONFLICT,
				"-P -S", CFG_LSDEV_USAGE);
		if(entry_name)
			usage_error1(CFG_ARGOROPT_CONFLICT,
				"-P -l", CFG_LSDEV_USAGE);
		get_pd_dv(entry_class,entry_subclass,entry_type);
		terminate_cfgodm();
		cudv = NULL;
		if(rflag){
			rinit();
			for(i=0;i<pd_info.num;i++)
				rlist(PdDv_MODE,pddv + i,entry_col);
			rprint();
			exit(0);
			}
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
		get_cu_dv(entry_name,entry_class, entry_subclass,
			entry_type,entry_state);
		terminate_cfgodm();
		pddv = NULL;
		if(rflag){
			rinit();
			for(i=0;i<cu_info.num;i++)
				rlist(CuDv_MODE,cudv + i,entry_col);
			rprint();
			exit(0);
			}
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

	while ((c = getopt(argc,argv,"f:CPF:Hhl:S:c:s:t:r:")) != EOF)
	{  switch (c)
	   {  case 'f' : /* args from file */
			 args_from( optarg, parse_args, "lsdev" );
			 break;
	      case 'C' :
			 Cflag = TRUE;
			 break;
	      case 'P' :
			 Pflag = TRUE;
			 break;
	      case 'F' :
			 if(entry_fmt)
				usage_error1(CFG_ARGOROPT_CONFLICT, "-F -F",
					CFG_LSDEV_USAGE);
			 entry_fmt = optarg;
			 Fflag = TRUE;
			 break;
	      case 'H' :
			 print_hdr++;
			 break;
	      case 'h' :
			 hflag++;
			 break;
	      case 'l' :
			 if(entry_name)
				usage_error1(CFG_ARGOROPT_CONFLICT, "-l -l",
					CFG_LSDEV_USAGE);
			 entry_name = optarg;
			 break;
	      case 'S' :
			 if(entry_state)
				usage_error1(CFG_ARGOROPT_CONFLICT,"-S -S",
					CFG_LSDEV_USAGE);
			 entry_state = optarg;
			 break;
	      case 'c' :
			 if(entry_class)
				usage_error1(CFG_ARGOROPT_CONFLICT,"-c -c",
					CFG_LSDEV_USAGE);
			 entry_class = optarg;
			 break;
	      case 's' :
			 if(entry_subclass)
				usage_error1(CFG_ARGOROPT_CONFLICT,"-s -s",
					CFG_LSDEV_USAGE);
			 entry_subclass = optarg;
			 break;
	      case 't' :
			 if(entry_type)
				usage_error1(CFG_ARGOROPT_CONFLICT,"-t -t",
					CFG_LSDEV_USAGE);
			 entry_type = optarg;
			 break;
	      case 'r' :
			 if(rflag)
				usage_error1(CFG_ARGOROPT_CONFLICT,"-r -r",
					CFG_LSDEV_USAGE);
			 rflag = TRUE;
			 entry_col = optarg;
			 break;
	      default  : 
			 usage_error1(CFG_MISC_SYNTAX_ERR,argv[optind],
					CFG_LSDEV_USAGE);
			 break;
	   }/*switch*/
	}/*while*/

	/* check for other junk */
	if (optind < argc)
	   usage_error1(CFG_MISC_SYNTAX_ERR,argv[optind],CFG_LSDEV_USAGE);

}/*parse_args*/


/*
 *        NAME: syntax
 *        FUNCTION: print usage message and exits
 */
syntax()
{
	printf(MSGSTR(CFG_LSDEV_USAGE));
   	exit(0);
}
