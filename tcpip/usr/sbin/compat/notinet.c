static char sccsid[] = "@(#)27	1.1  src/tcpip/usr/sbin/compat/notinet.c, tcpip, tcpip411, GOLD410 3/30/94 17:28:09";
/*
 * COMPONENT_NAME: (TCPIP) SRC commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * notinet.c - removes shared memory segment created by inetd whenever SRC 
 *             detects an abnormal termination of inetd.
 *
 */

#include	<stdio.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	"libsrc.h"
#include	<nl_types.h>
#include	"notinet_msg.h"
#include	<locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_NOTINET,n,s)


struct  service_entry *serv;

char 	progname[128];

main(argc, argv)
int	argc;
char 	*argv[];
{
	key_t	srcmemkey;       
	int	srcmemid;
	key_t	telmemkey;       
	int	telmemid;

        setlocale(LC_ALL, "");
        catd = catopen(MF_NOTINET,0);

	strcpy(progname,argv[0]);

	/* SRC passes subsystem name.  Verify before removing memory */
	if (strcmp(argv[1],"inetd")) 
		exit(0);

	/* remove inetd shared memory */
	if ((srcmemkey = ftok(INET_CNFFILE, SRC_FD)) == (key_t) -1) 
                fprintf(stderr, MSGSTR(ERR_FTOK,
                     "%s: ftok for shared memory failed.\n"),progname);
	srcmemid = shmget(srcmemkey,sizeof(struct service_entry),0);
	shmctl(srcmemid, IPC_RMID, (struct shmid_ds *) NULL);

	/* remove telnetd shared memory */
	if ((telmemkey = ftok(TELNETD, SRC_FD)) == (key_t) -1) 
                fprintf(stderr, MSGSTR(ERR_FTOK,
                     "%s: ftok for shared memory failed.\n"),progname);
	telmemid = shmget(telmemkey,sizeof(struct telnetd_entry),0);
	shmctl(telmemid, IPC_RMID, (struct shmid_ds *) NULL);
	
	exit(0);
}

