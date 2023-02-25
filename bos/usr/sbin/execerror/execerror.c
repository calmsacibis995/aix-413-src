static char sccsid[] = "@(#)88	1.9.1.1  src/bos/usr/sbin/execerror/execerror.c, cmdcntl, bos41J, 9507C 2/8/95 08:10:20";
/*
 * COMPONENT_NAME: (CMDCNTL) execerror
 *
 * FUNCTIONS: command exec'd when an execload fails
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* this is a prototype program.  It must be converted to NLS */
#include	<stdio.h>
#include	<sys/ldr.h>
#include        <nl_types.h>
#include        "execerr_msg.h"
#define         MSGSTR(num,str) catgets(catd,MS_EXECERROR,num,str)  /*MSG*/
static nl_catd         catd;
#include <locale.h>

/* execerror is exec'd by exec when the load of the real program fails.
 * it is passed the name of the file being exec'd and zero or more
 * loader error message strings.  Each loader error message string
 * contains an error number followed by error data */
extern int errno;
int
main(argc,argv)
int	argc;
char	**argv;
{
	int	i,n,errorid;
	char	s1[256],s2[256],s3[256],s4[256];

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_EXECERR, NL_CAT_LOCALE);
	fprintf(stderr,
		MSGSTR(EXECERROR,"Could not load program %s \n"),argv[1]);
	for(i=2;i<argc;i++){
		*s1=*s2=*s3=*s4=0;
		n=sscanf(argv[i],"%d %255s %255s %255s %255s",&errorid,s1,s2,s3,s4);
		if (n == 0)
			errorid = L_ERROR_SYSTEM;

		/* n is the number of strings.  The first was the error number. */
		switch (errorid) {
			case L_ERROR_TOOMANY:
    				fprintf(stderr,
		MSGSTR(TOOMANY,"Additional errors occurred but are not reported\n"));
				break;
			case L_ERROR_NOLIB:
				if (n==2)
					fprintf(stderr,
		MSGSTR(NOLIB,"Could not load library %s\n"),s1);
				else
					fprintf(stderr,
		MSGSTR(NOLIB1,"Could not load library %s[%s]\n"),s1,s2);
				break;
			case L_ERROR_MEMBER:
				NLfprintf(stderr,
		MSGSTR(MEMBER,"Member %s not found or file not an archive\n"),s1);
				break;
			case L_ERROR_UNDEF:
				fprintf(stderr,
		MSGSTR(UNDEF,"Symbol %s in %s is undefined\n"),s1,s2);
				break;
			case L_ERROR_RLDBAD:
				fprintf(stderr,
		MSGSTR(RLDBAD,"Relocation entry number %s is defective in %s\n"),s1,s2);
				break;
			case L_ERROR_TYPE:
				if (n<3)
					strcpy(s2,"?");
				fprintf(stderr,
                MSGSTR(TYPE,"symbol %s used in %s type does not match exported type\n"),s1,s2);
				break;
			case L_ERROR_FORMAT:
				if (n==1)
					strcpy(s1,argv[1]);
				fprintf(stderr,
		MSGSTR(FORMAT,"%s is not executable or not in correct XCOFF format\n"),s1);
				break;
			case L_ERROR_ALIGN:
    				fprintf(stderr,
		MSGSTR(ALIGN,"alignment of text does not match required alignment\n"));
				break;
			case L_ERROR_ERRNO:
				errno = atoi(s1);
				perror(MSGSTR(EWAS,"Error was"));
				break;
			case L_ERROR_DOMCREAT:
    				fprintf(stderr,
		MSGSTR(DOMCREAT,"Insufficient permission to create loader domain %s\n"),s1);
				break;
			case L_ERROR_DOMADD:
				if (n==3)
    					fprintf(stderr,
		MSGSTR(DOMADD,"Insufficient permission to add shared object %s to loader domain %s\n"),s2,s1);
				else
    					fprintf(stderr,
		MSGSTR(DOMADD1,"Insufficient permission to add shared object %s[%s] to loader domain %s\n"),s2,s3,s1);
				break;
			case L_ERROR_SYSTEM:
			default:
				fprintf(stderr,
		MSGSTR(SYSTEM,"System error - error data is: %s\n"),argv[i]);
				break;
		}
	}
exit(-1);
}
