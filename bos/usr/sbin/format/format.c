static char sccsid[] = "@(#)92	1.13.1.4  src/bos/usr/sbin/format/format.c, cmdfiles, bos411, 9428A410j 11/16/93 08:37:21";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: format, fdformat
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/fd.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/access.h>
#include <locale.h>
#include <stdlib.h>
#include <sys/scdisk.h>

/* format [-fl -d device] */

#define	SECT_SIZE		2
#define	BLKSIZE			512
#define FILLER			((unsigned)0xf6)
#define tty()			isatty(0)
#define trkaddr(cyl,side)	((long) (trksize * (cyl*info.sides + side)))

extern  char	*optarg;
extern	int	optind;
extern  char	*sys_errlist[];
extern  int	sys_nerr;
extern  char	*rindex();
#include <nl_types.h>
#include "format_msg.h"
nl_catd	catd;
#define	MSGS(N,S)	catgets(catd,MS_FORMAT,N,S)
char	*devname;
int	devfd;
int	trksize;
char	lflg;
char    hflg;	/* use for high density special file name, eg, /dev/fd0h */
char    fdhflg;		/* use for -h option in fdformat */
char    fdflg;		/* use for fdformat */
int	verify;		/* turned off by "-f" option */
struct	fdinfo info;	/* floppy info struct */
int     mbflag;         /* Multibyte? */
int	is_scrwopt = 0;	/* is device SCSI read/write optical drive? */

main(argc, argv)
int	argc;
char	*argv[];
{ 	int	c;
	char *name;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_FORMAT, NL_CAT_LOCALE);

        mbflag = (MB_CUR_MAX > 1);

	name = rindex(argv[0], '/');
	if (name) name++;
	else name = argv[0];
	if (!strcmp(name, "fdformat"))
		fdformat_getargs(argc, argv);
	else
		getargs(argc, argv);	/* set flgs according to args */
	open_dev(devname);		/* open device */
	if (! is_scrwopt)
		drive_info();		/* get device specifications */
	if(tty()) {				
	    printf(MSGS(INSERT,"Insert new media for %s\n"),devname);	 /*MSG*/
	    printf(MSGS(ENTER,"and strike ENTER when ready"));           /*MSG*/
	    fflush(stdout);
	    while((c= getchar()) != '\n') putchar(c);
	    if (! is_scrwopt)
		printf(MSGS(INFO,				         /*MSG*/
	    	"Formatting: %d side(s), %d trk/side, %d sect/trk\n"),   /*MSG*/
	      		info.sides, info.ncyls, info.nsects);
	}
	if (is_scrwopt)
		format_scrwopt(devname);/* format scsi read/write optical disk*/
	else
		format_dsk();		/* format the diskette  */
	if(tty()) printf(MSGS(COMPL,"Format completed\n"));		 /*MSG*/
	exit(0);
}

getargs(argc, argv)
int	argc;
char	*argv[];
{ 	register c;
	
	lflg = 0 ;
	verify = 1;
	devname = "/dev/rfd0";			        /* by default */
	while((c=getopt(argc,argv,"d:lfm:")) != EOF) {
	    switch(c) {
	    case 'd':		/* device to format */
		devname = optarg;
		break;
	    case 'l':		/* low density diskette */
		lflg = 1;
		break;
	    case 'f':		/* don't bother to verify */
		verify = 0;
		break;
	    case 'm':		/* future feature */
				/* but for now, err_exit() */
	    default:
		err_exit(MSGS(USAGE,"usage: format [-d device] [-fl]\n"));
	    }
	}
}

fdformat_getargs(argc, argv)
int	argc;
char	*argv[];
{ 	register c;
	
	fdflg = 1;
	lflg = 1;			/* fdformat always do low density */
	verify = 1;
	devname = "/dev/rfd0";			        /* by default */
	while((c=getopt(argc,argv,"h")) != EOF) {
	    switch(c) {
	    case 'h':
		    lflg = 0;
		    fdhflg = 1;
		    break;
	    default:
		err_exit(MSGS(USAGE1,"usage: fdformat [-h] device\n"));
	    }
	}
	if (argc > optind)
		devname = argv[optind];
}

open_dev(dev)
char *dev;
{
	struct devinfo info;

	/* check if the invoker has the write access to the device */
	if (access(dev, W_OK) != 0) {
		perror(dev);
		exit(255);
	}

	/* Open device.
	 * O_NDELAY tells the floppy driver NOT
	 * to try to determine floppy's format.
	 */
	if ((devfd = open(dev, O_RDWR | O_EXCL | O_NDELAY)) < 0) 
	    err_exit(MSGS(OPNERR,					 /*MSG*/
		"format: can't open %s: %s\n"), dev, strerror(errno));   /*MSG*/

	/* Make sure its really a valid device.
	 */
	if ((ioctl(devfd, IOCINFO, (char*)&info)  ==  -1)
	 || ((info.devtype != DD_DISK) && (info.devtype != DD_SCRWOPT))
         || (info.flags != DF_RAND))
	    err_exit(MSGS(NOTDSK,"format: %s: failure accessing device\n"),dev);
	if (info.devtype == DD_SCRWOPT)
		is_scrwopt = 1;		/* device is SCSI r/w drive */
}

drive_info()
{
	if((ioctl(devfd, FDIOCGINFO, (char*)&info)) < 0) 
	    err_exit(MSGS(NOPAR,"format: can't get drive parameters\n"));/*MSG*/

	/* check to see if the devname is a special file name, eg, fd0h */
              if(mbflag)
              {
		if (mbsrchr(devname, 'l') != NULL)
		   lflg = 1;		/* special file name, eg, fd0l */
		else if (mbsrchr(devname, 'h') != NULL)
	                hflg = 1;	/* special file name, eg, fd0h */
              }
              else
              {
		if ((void *)strrchr(devname, 'l') != NULL)
		   lflg = 1;
		else if ((void *)strrchr(devname, 'h') != NULL)
	                hflg = 1;
              }
	
	if (lflg && fdhflg)
	    err_exit(MSGS(CONFLICT2,"Conflicting flag -h and low density special file name.\n"));

	if (fdflg && hflg && !fdhflg)
	    err_exit(MSGS(CONFLICT3,"Specify flag -h with high density special file name.\n"));

	if (lflg && hflg)
	    err_exit(MSGS(CONFLICT1,"Conflicting flag -l and high density special file name.\n"));

	/* overwrite the values set by the ioctl() if the lflg is set */
 	if (lflg) {	
		info.nsects = 9; 		/* lo-density parms */
		if (info.type == D_96) { 	/* 5.25" diskette */
			info.sides  = 2;
			info.ncyls  = 40;
			}
	}

	if((ioctl(devfd, FDIOCSINFO, (char*)&info)) < 0)
	    err_exit(MSGS(NSETPAR, 					 /*MSG*/
			"format: can't set drive parameters\n"));	 /*MSG*/
	
	trksize = info.nsects * BLKSIZE;
}

format_dsk()
{
	register int cyl;

	/* Format and verify each track.
	 */
	for (cyl = 0; cyl < info.ncyls; cyl++) {
		format_trk(cyl, 0);
		format_trk(cyl, 1);
	}
	if (verify)
		for (cyl = 0; cyl < info.ncyls; cyl++) {
			verify_trk(cyl, 0);
			verify_trk(cyl, 1);
		}
}

format_scrwopt(dev)
char *dev;
{
	struct devinfo info;

	if (close (devfd) < 0 )
		err_exit("format: %s\n",strerror(errno));
	
	if ((devfd = openx(dev, O_RDWR | O_EXCL | O_NDELAY, (mode_t)NULL, SC_SINGLE)) < 0) 
	    err_exit(MSGS(OPNERR,					 /*MSG*/
		"format: can't open %s: %s\n"), dev, strerror(errno));   /*MSG*/

	/* Make sure its really a valid device.
	 */
	if ((ioctl(devfd, IOCINFO, (char*)&info)  ==  -1)
	 || ((info.devtype != DD_SCRWOPT))
         || (info.flags != DF_RAND))
	    err_exit(MSGS(NOTDSK,"format: %s: failure accessing device\n"),dev);

	/* Issue the DKFORMAT ioctl to format the media */
	if (ioctl(devfd, DKFORMAT, (char*)&info)  ==  -1)
	    err_exit(MSGS(FAILED,					 /*MSG*/
	    "format: format operation failed: %s\n"), strerror(errno));	 /*MSG*/

	if (close (devfd) < 0 )
		err_exit("format: %s\n",strerror(errno));
}

format_trk(cyl, side)
int cyl, side;
{
	register int c, sect;
	char buf[256];

	for(c=0,sect=1; sect<=info.nsects; ++sect)
	{   buf[c++] = cyl;
	    buf[c++] = side;
	    buf[c++] = sect;
	    buf[c++] = SECT_SIZE;
	}
	if (ioctl(devfd, FDIOCFORMAT, buf) < 0)
	    err_exit(MSGS(FAILED,					 /*MSG*/
	    "format: format operation failed: %s\n"), strerror(errno));  /*MSG*/
}

verify_trk(cyl, side)
int cyl, side;
{
	static char *buf = NULL;	/* buffer for reading track */
	register int i;

	if (buf == NULL)
	    if ((buf = malloc(trksize))  ==  NULL)
		err_exit(MSGS(MEMERR,"format: out of memory\n"));	 /*MSG*/

	/* Read in entire track.
	 */
	lseek(devfd, trkaddr(cyl,side), 0);
	if (read(devfd, buf, trksize) != trksize)
	{   if (errno==EFORMAT || errno==EIO)
		err_exit(MSGS(DSKBAD,					 /*MSG*/
			"format: media bad or incompatible\n"));	 /*MSG*/
	    else
		err_exit(MSGS(GERR, "format: %s\n"), strerror(errno));	 /*MSG*/
	}

	/* The format operation is supposed to initialize
	 * each byte in track to a special filler value (0xf6).
	 * Make sure this is the case.
	 */
	for (i=0; i<trksize; i++)
	    if (buf[i] != FILLER)
		err_exit(MSGS(DSKBAD,					 /*MSG*/
			"format: media bad or incompatible\n"));	 /*MSG*/
}

/*VARARGS1*/
err_exit(s, s1, s2)
char	*s, *s1, *s2;
{
	fprintf(stderr,s, s1, s2);
	exit (255);
}
