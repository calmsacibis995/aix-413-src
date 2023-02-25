static char sccsid[] = "@(#)18  1.13  src/bos/usr/sbin/auditstream/auditstream.c, cmdsaud, bos411, 9428A410j 11/12/93 14:41:13";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: auditstream command
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	"stdio.h"
#include	"unistd.h"
#include	"fcntl.h"
#include	"sys/audit.h"
#include	"sys/errno.h"
#include	"sys/priv.h"
#include	"fcntl.h"
#include 	"locale.h"

#define	AUDITDEV	"/dev/audit"
#define	NULLDEV		"/dev/null"

#include "auditstm_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_AUDITSTM,n,s)

#include	"audutil.h"

DEBUGNAME("AUDITSTREAM")

extern int	optind;
extern char	*optarg;

int	cflag = 0;
struct {
	char *classes;
	int len;
} ext_buf;

main(int ac, char *av[]){

	int fd;
	int out;
	int c;


        /* 
	 * Shut down unnecessary fd's 
	 */
 
	/*
 	 * if associated with a tty then close 
	 * to protect against frevoke
	 * (exit1 does a freopen())
	 */

/*
	if(isatty(STDERR_FILENO)){

		fclose(stderr);

	}

	if(isatty(STDIN_FILENO)){

		fclose(stdin);

	}
*/

        kleenup(3, 0, 0);
	setpgrp();

	setlocale(LC_ALL, "");
	catd = catopen(MF_AUDITSTM, NL_CAT_LOCALE);
	
	/* 
	 * Privilege check
	 */

	if(privcheck(SET_PROC_AUDIT)){ 
		errno = EPERM;

		exit1(errno, 
		MSGSTR(ERRPRIV, 
		"** SET_PROC_AUDIT privilege required"), 
		0); 

	}

	/* 
	 * Clean device special buffer
	 * for audit class definition
	 */

	ext_buf.classes = NULL;
	ext_buf.len = 0;

	if(ac > 3){
		errno = EINVAL;

		exit1(errno, 
		MSGSTR(USAGE, 
		"Usage: auditstream [-c class[,class]*]"),
		0);
	}
		
        /*
         * Suspend auditing
         */

        if(auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0){
                errno = EPERM;

                exit1(errno,
                MSGSTR(ERRSUSPEND,
                "** cannot suspend auditing "),
                0);

        }

	while(1){

		c = getopt (ac, av, "c:");
		if(c == EOF) break;

		switch(c){
			case 'c':
				cflag = 1;
				ext_buf.classes = optarg;
				break;
			default:
                		errno = EINVAL;

                		exit1(errno,
				MSGSTR(USAGE, 
				"Usage: auditstream [-c class[,class]*]"),
                		0);
		}
	}

	/* 
	 * Audit class definition
	 */

	if(ext_buf.classes){

		char	*p;
		extern char	*xalloc ();

		ext_buf.len = strlen(ext_buf.classes) + 2;
		p = xalloc(ext_buf.len);
		strcpy(p, ext_buf.classes);
		ext_buf.classes = p;
		ext_buf.classes[ext_buf.len - 1] = NULL;

		for(p = ext_buf.classes; *p; p++){
			if (*p == ',')
				*p = NULL;
		}
	}

	fd = openx(AUDITDEV, O_RDONLY, 0, cflag ? &ext_buf : 0);
	if(fd < 0){

		perror(AUDITDEV);
		exit(errno);

	}

	while(1){

		char	buf[BUFSIZ];
		int	cnt;

		cnt = read(fd, buf, sizeof (buf));

		if(cnt <= 0){

			break;
		}

		write(1, buf, cnt);
	}

	exit(0);
}
