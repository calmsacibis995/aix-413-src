static char sccsid[] = "@(#)35	1.19  src/bos/usr/sbin/adfutil/main.c, cmdadf, bos411, 9437A411a 9/12/94 14:36:27";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (main) main(), usage()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "symbol.h"
#include "global.h"
#include <locale.h>

#define fEOF 0		/* yylex returns 0 on EOF */

#define USAGE "\
usage : \n\
adfutil [ -c XXXX | -a adapter ][ -d device | -f file ][ -q ][ -m [file(s)] ]\n\
\t-a : adapter name \n\
\t-c : PS/2 hex cardid \n\
\t-d : device specification (default: /dev/fd0) \n\
\t-f : filename of adf on filesystem, cardid must match if specified \n\
\t-m : microcode only \n\
\t-q : quiet mode \n"

extern FILE *yyin;
extern int errno;

char yytext[2046];

int ffile = false;

/*
 * NAME: main
 *                                                                    
 * FUNCTION: adfutil entry point
 *                                                                    
 * NOTES:
 *    parse command line arguments
 *    initialize hash table 		(tabinit)
 *    parse input file 			(yyparse)
 *    add data to database 		(genodm)
 *
 * DATA STRUCTURES:
 *
 * RETURN: never returns.  Exit value = number of error or 0
 *	
 */  

main(argc, argv)
int argc;
char **argv;
{
    char *sb;
    char dosfile[20];
    int rc, argerr;
    int fcard = false;
    int pid;
    FILE *dfd;

/* externs for use with getopt */
    extern	int	optind  ;
/*    extern	int	*optarg ; redeclaration */
    extern 	int	optopt  ;
    extern	int	opterr  ;


/* BEGIN main */

    setlocale(LC_ALL,"");  		/* set National Language locale */
    ADF_ERR = catopen(MF_ADF, NL_CAT_LOCALE);

    yydebug = false;
    fPrtDebug = false;
    fquiet = false;
    iCardid = 0xffff;
    argerr = 0;

    opterr = 0 ;            		/* turn off msgs from getopt */
    

    /* I.a) Parse Args */

    while ((rc = getopt(argc, argv, "a:c:d:f:gm:opqz")) != EOF)
    {
	switch (rc) 
	{
	  case 'a': /* -a adapter_name */
	    sbAdap = optarg ;
	    break;
	  case 'c': /* -c card_id */
	    rc = sscanf(optarg, "%x", &iCardid);
	    fcard = true;
	   
	    if ((rc < 1) || (4 != strlen(optarg))) 
	    {
		fprintf(stderr, NL_MSG(CARD_ID, "Invalid cardid \"%s\".\n")
			,optarg);
		usage(1);
	    }
	    break;
	  case 'd': /* -d device */
	    sbDevice = optarg ;
	    if (NULL == (dfd = fopen(sbDevice, "r"))) 
	    {
		fprintf(stderr, NL_MSG(DEV_OPEN, 
		        "Can't open device \"%s\".\n"), sbDevice);
		usage(1);
	    }
	    fclose(dfd);
	    break;
	  case 'f': /* -f file name */
	    sbFile = optarg ;

	    if (NULL == (yyin = fopen(sbFile, "r"))) 
	    {
		fprintf(stderr, NL_MSG(FILE_OPEN, 
			    "Can't open file \"%s\".\n"), sbFile);
		usage(1);
	    }
	    ffile = true;
	    break;
	  case 'm': /* -m [ file [ file(s) ] ] */
	    fucode = true;
	    ucode = (struct fl*)makefl(optarg) ;

	    while ((optind < argc) && (argv[optind][0] != '-')) 
	    {
		attachfl(argv[optind++], ucode);
	    }
	    break;
	  case 'o':
	    odmdebug = true;
	    break;
	  case 'p':
	    fPrtDebug = true;
	    break;
	  case 'q': /* -q (toggle) */
	    fquiet = true;
	    break;
	  case 'z':
	    yydebug = true;
	    break;
	  default:
	    switch (optopt)
	    {
	      case 'a':
		fprintf(stderr, 
			NL_MSG(AOPT_MISS,
			   "-a option requires argument - adapter name\n"));
		break ;
	      case 'c':
		fprintf(stderr, 
			NL_MSG(COPT_MISS,
			   "-c option requires argument - cardid\n")) ; 
		break ;
	      case 'd':
		fprintf(stderr, 
			NL_MSG(DOPT_MISS,
			   "-d option requires argument - device name\n")) ;
		break ;
	      case 'f':
		fprintf(stderr, 
			NL_MSG(FOPT_MISS,
			   "-f option requires argument - file name\n")) ;
		break ;
	      case 'm':   /* -m has an OPTIONAL argument */
		fucode = true ;
		argerr-- ;			/* to offset argerr++ below */
		break ;
	      default:
		fprintf(stderr, 
			NL_MSG(INV_ARG_C, "Invalid argument \"%c\".\n"),
			optopt);
	        break ;
	    }
	    argerr++;
	    break;
	}
    }

    if (argerr)
    {
	usage(1) ;
    }

    if (ffile && sbDevice) {
	fprintf(stderr, NL_MSG(FILE_OPT, 
	    "-f option cannot be specified with -d option.\n"));
	usage(1);
    }

    /* I.b) determine and get ADF input file */

    rc = 1;
    if (!ffile) {
	pid = getpid();
	sbFile = (unsigned char*)malloc(128);
	sprintf(sbFile, "/tmp/adf%d", pid);
	if (fcard)
	    sprintf(dosfile, "A:@%X.ADF", iCardid);
	else if (sbAdap) {
	    iCardid = getadap(sbAdap);
	    if (iCardid)
		sprintf(dosfile, "A:@%X.ADF", iCardid);
	    else 
		strcpy(dosfile, "A:@*.ADF");
	} else 
	    strcpy(dosfile, "A:@*.ADF");
	rc = getdosfile(sbDevice, dosfile, sbFile, 1);
	if (!rc && !fucode) {
	    fprintf(stderr, NL_MSG(ADF_NONE, "No ADF input file.\n"));
	    exit(1);
	} 
	if (rc && NULL == (yyin = fopen(sbFile, "r"))) {
	    fprintf(stderr, NL_MSG(FILE_OPEN, 
		    "Can't open file \"%s\".\n"), sbFile);
	    exit(1);
	}
    }

    /* II.a) Process ADF data file */

    tabinit();

    if (yydebug) fprintf(stderr, "Starting Phase 1 ... ");
    if (rc) 
	yyparse();

    /* III) Populate Predefine */

    switch (cErrors) {
    case 0:
	fprintf(stderr, NL_MSG(ZERO_ERRS, "No errors detected in adf.\n"));
	if (yydebug) 
	    fprintf(stderr, "Starting Phase 2 ... ");
	if (!fucode) 
	    genodm();
	else 
	    getmicro(ucode, adf_ucode);
	break;
    case 1:
	fprintf(stderr, NL_MSG(ONE_ERR, "One error detected in adf.\n"));
	break;
    default:
	fprintf(stderr, 
	    NL_MSG(MANY_ERRS, "%d errors detected in adf.\n"), cErrors);
	break;
    }

    if ((!ffile) && sbFile)
	unlink(sbFile);
    exit(cErrors);
}

/*
 * NAME: usage
 *                                                                    
 * FUNCTION: print usage message, exit
 *                                                                    
 * RETURN: never
 *	
 */  

usage(estat)
int estat;	/* exit status */
{
    fprintf(stderr, "%s", NL_MSG(USAGE_ERR, USAGE));
    exit(estat);
} /* END main */

/* EOF */
