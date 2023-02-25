static char sccsid[] = "@(#)47  1.3 src/rspc/usr/bin/ntsc/ntsc.c, pcivca, rspc41J, 9507C 1/27/95 04:27:57";
/*
 *   COMPONENT_NAME: (PCIVCA)  PCI Video Capture Adapter Device Driver
 *
 * FUNCTIONS: ntsc
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#define _ILS_MACROS
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <locale.h>
#include "ntsc_msg.h"
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#define MAXPARMS 2
#define MSGSTR(Num,Str) catgets(catd,MS_NTSC,Num,Str)
#define	VCA_XSERVER			3
#define	VCA_NTSC			4
#define VCA_IOC_NTSC_SW                 _IOW('v',1,struct ntsc_sw *)


nl_catd catd;
struct  ntsc_sw
{
        int     ntsc_out;           /* ntsc output on/off */
};

main(argc,argv)
int 	argc;
char 	**argv;
{
int vflag = 0;              /* 'v' parameter */
struct  ntsc_sw ntsc;
int     fd, rc;
int     out, c;


	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_NTSC, NL_CAT_LOCALE);
					
	while ((c=getopt(argc,argv,"v")) != EOF) {
		switch (c) {
		      case 'v':
				vflag++;
				break;
		      default:
				break;
		}
	}

	if (argc < 2) {
      		fprintf(stderr, MSGSTR(USAGE, "usage: ntsc [-v] [on|of]\n"));
      		exit(2);
    	}

	if ( strcmp("on",argv[optind]) == 0 )
		out = 1;
	else if ( strcmp("off",argv[optind]) == 0 )
		out = 0;
	else {
      		fprintf(stderr, MSGSTR(USAGE, "usage: ntsc [-v] [on|of]\n"));
      		exit(2);
    	}

       	if ((fd = openx("/dev/vca0", O_RDWR, 0, VCA_NTSC)) < 0 ){
		if (( vflag ) && ( out )) {
	    		fprintf(stderr, MSGSTR(CANTOPEN, "Can not switch to ntsc on while overlay is active.\n"));
			printf("%s", MSGSTR(CANTON, "ntsc on failed.\n"));
		} else if (( vflag ) && ( out == 0 ))
			printf("%s", MSGSTR(CANTOFF, "ntsc off failed.\n"));
            	exit(2);
       	}
       	ntsc.ntsc_out = out;

       	if ((rc = ioctlx(fd, VCA_IOC_NTSC_SW, &ntsc, VCA_NTSC))< 0){
		if (( vflag ) && ( out )) {
             	   if ( errno == EBUSY )
			fprintf(stderr, MSGSTR(CANTIOCTL, "Already set to ntsc on. Use ntsc off first.\n"));
		   printf("%s", MSGSTR(CANTON, "ntsc on failed.\n"));
		} else if (( vflag ) && ( out == 0 ))
			printf("%s", MSGSTR(CANTOFF, "ntsc off failed.\n"));
               	close(fd);
               	exit(2);
       	}
	if (( vflag ) && ( out ))
		printf("%s", MSGSTR(NTSCON, "ntsc on succeeded.\n"));
	else if (( vflag ) && ( out == 0 ))
		printf("%s", MSGSTR(NTSCOFF, "ntsc off succeeded.\n"));

	close(fd);
	exit(0);
}
