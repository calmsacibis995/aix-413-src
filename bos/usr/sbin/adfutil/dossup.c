static char sccsid[] = "@(#)88	1.5  src/bos/usr/sbin/adfutil/dossup.c, cmdadf, bos411, 9428A410j 6/15/90 16:57:47";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (dossup) getdosdir(), getdosfile()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "symbol.h"
#include <fcntl.h>
#include <string.h>
#include <dos.h>
#include "externs.h"

extern int errno;

/*
 * NAME: getdosdir
 *                                                                    
 * FUNCTION: 
 *    Scan dos device A: (default:/dev/fd0) for idfile. idfile might be
 * of the form "A:*.*".  This routine returns the name of the last file 
 * found in odfile, and the count of the number of files found.
 *                                                                    
 * NOTES:
 *
 *
 * RETURN:
 *    0, 1, or n : number of files found using search criteria "idfile"
 *
 */  

int 
getdosdir(odfile, idfile)
char *odfile, *idfile;
{
    DOSFIND srch;
    int dcnt = 0;
    char sfile[15];
    char *nfile;
    char *dosfirst(), *dosnext();

    sprintf(sfile, "%s", idfile);
    nfile = dosfirst(&srch, sfile, S_ALL);
    while (nfile) {
	    /* printf("\tFound %s file\n", nfile); */
	dcnt++;
	strcpy(odfile, nfile);
	nfile = dosnext(&srch);
    }
    return dcnt;
}

/*
 * NAME: getdosfile
 *                                                                    
 * FUNCTION: 
 *    copy dos file from device (if specified) to aixfile
 *                                                                    
 * NOTES:
 *    This procedure forks off a child to do the dos processing because
 * "dosinit" changes the environment in an undesired fashion.  My primary
 * reason for the fork was because the "dosinit" changes the file descriptors
 * for ALL files (why?) and this was causing a problem in a later stage of
 * the program.
 *
 * DATA STRUCTURES:
 *    if dosdevice is defined DOS_A is changed in the child environment
 *
 * RETURN:
 *    0 : an error occured
 *    1 : normal Exit
 *
 */  

getdosfile(dosdevice, sfile, aixfile, fascii)
char *dosdevice, *sfile, *aixfile;
int fascii;
{
    int pid, status;
    int sav_error;
    int nf;
    int fd;
    DOSFILE dosfd;
    int tsize;
    char tbuf[1024];
    char envbuf[255];
    char dosfile[15];

    if ((!sfile) || (!aixfile))
	return false;
    if (0 == (pid = fork())) {
	/* 
	   We need to fork of a seperate process here because "dos services"
	   muck up the open and closed file pointers.
	*/
	if (dosdevice) {
	    /* init dosdevice if specified */
	    sprintf(envbuf, "DOS_A=%s", dosdevice);
	    putenv(envbuf);
	}
	if (0 > dosinit()) {
	   sav_error = doserrno;
	   fprintf(stderr, NL_MSG(DOS_INIT, 
                   "adfutil[getdosfile:dosinit] device(%s) doserrno(%d)"),
                    dosdevice, doserrno);
	   exit(sav_error);
	}

	if (index(sfile, '*') || index(sfile, '?')) {   
	    /* Wild card for file name */
	    if (0 == (nf = getdosdir(dosfile, sfile))) {    
		/* no files found */
	        fprintf(stderr, NL_MSG(DOS_NONE, 
                       "adfutil[getdosdir] no files found, file(%s)\n"),
		        sfile);
		exit(1);
	    } else if (nf > 1) {
		/* multiple files found */
	        fprintf(stderr, NL_MSG(DOS_SOME,
                       "adfutil[getdosdir] (%d) files found, file(%s)\n"),
		        nf, sfile);
		exit(1);
	    }
	} else strcpy(dosfile, sfile);

	if (0 > (fd = open(aixfile, O_CREAT|O_RDWR, 0666))) {
	    sav_error = errno;
	    sprintf(tbuf, "adfutil[getdosfile:open] file(%s)", aixfile);
	    perror(tbuf);
	    exit(sav_error);
	}
	if (0 > (dosfd = dosopen(dosfile, O_RDONLY|(fascii?DO_ASCII:0), 0))) {
	    sav_error = doserrno;
	    fprintf(stderr,NL_MSG(DOS_ERR, 
  "[adfutil] An error occured trying to read \"%s\" from \"%s\". Error = %d\n"),
               dosfile, dosdevice, doserrno);
	    exit(sav_error);
	}
	tsize = dosread(dosfd, tbuf, 1024);
	while (tsize > 0) {
	    write(fd, tbuf, tsize);
	    tsize = dosread(dosfd, tbuf, 1024);
	}
	dosclose(dosfd);
	close(fd);
	exit(0);
    }
    if (0 > wait(&status) && fPrtDebug) {
	fprintf(stderr, NL_MSG(DOS_STAT, 
                "ERROR: wait on [getdosfile] Child(%d) errno(%d)\n"),
                 pid, errno);
	return false;
    }
    if (status != 0 && fPrtDebug) {
	fprintf(stderr, NL_MSG(DOS_STAT2,
                "NOTICE: [getdosfile] Child(%d) exited status(%#X).\n"));
	return false;
    }
    return true;
}
