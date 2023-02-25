static char sccsid[] = "@(#)28	1.1  src/tcpip/usr/sbin/compat/stinet.c, tcpip, tcpip411, GOLD410 3/30/94 17:28:20";
/*
 * COMPONENT_NAME: (TCPIP) SRC commands
 *
 * FUNCTIONS:  getconfig, do_ftp, do_telnet, do_finger, do_login
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
 * stinet.c - provides SRC long status information for the requested inetd
 *            subserver (i.e. ftp, login, exec, etc.).
 */

#include        <stdio.h>
#include        <string.h>
#include        <sys/stat.h>
#include        <sys/types.h>
#include        <sys/ipc.h>
#include        <sys/shm.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <arpa/inet.h>
#include        <errno.h>
#include	<odmi.h>
#include        "libffdb.h"
#include        "libsrc.h"
#include        "inetodm.h"

#include <nl_types.h>
#include "stinet_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_STINET,n,s)

char 	progname[128]; 

struct	service_entry *srcserv;
key_t	srcmemkey;
int	srcmemid;

struct  InetServ inetserv;
	
extern	int errno;
extern	int odmerrno;

main(argc, argv)
	int argc;
	char *argv[];
{
	char	portnum[128];
	int	x, y, printflg = FALSE;
	int	port;

        setlocale(LC_ALL, "");
        catd = catopen(MF_STINET,0);

	
	/* save command line arguments */
	strcpy(progname,argv[0]);
	strcpy(portnum,argv[1]);

	port = atoi(portnum);
	if (port == 1512) {
		strcpy(portnum, "512");
	}

	/* create shared memory key and attach shared memory created by inetd */
	if ((srcmemkey = ftok(INET_CNFFILE, SRC_FD)) == (key_t) -1) {
                fprintf(stderr, MSGSTR(ERR_FTOK,
		     "%s: ftok for shared memory failed.\n"),progname);
		exit(1);
	}

	if ((srcmemid = shmget(srcmemkey,sizeof(struct service_entry),0)) < 0) {
                fprintf(stderr, MSGSTR(ERR_SHMGET,
		     "%s: shmget for shared memory failed.\n"),progname);
		exit(1);
	}

	if ((srcserv = (struct service_entry *) shmat(srcmemid, 0, 0)) < 0) {
                fprintf(stderr, MSGSTR(ERR_SHMAT,
		     "%s: shmat for shared memory failed.\n"),progname);
		exit(1);
	}

	/* print inetserv info */
	fprintf(stdout,"%-12s %-24s%-24s%s\n", MSGSTR(SRC_MSG1,"Service"),
		MSGSTR(SRC_MSG2,"Command"), MSGSTR(SRC_MSG3,"Description"),
		MSGSTR(SRC_MSG4,"Status"));

	/* get the inetserv entry for the port number passed via argv[1] */
	getconfig(portnum);

	/* process "special" subservers that have additional status info */
	if (inetserv.portnum == 21)	/* ftp */
		do_ftp(inetserv.command);
	if (inetserv.portnum == 79)	/* finger */
		do_finger(inetserv.command);
	if (inetserv.portnum == 513)	/* login */
		do_login(inetserv.command);

	/* traverse shared memory extracting/printing the appropriate info */
	if (inetserv.state == 3l) 
	    for (x = 0; x < MAXCONN; x++) 
	        if (!(strcmp(inetserv.servname,srcserv->serv[x].service)))
		    for (y = 0; y < MAXCONN; y++)
                        if (srcserv->serv[x].connect[y].pid != -1) 
                            if (!kill(srcserv->serv[x].connect[y].pid,0)) {
		    		if (!printflg) {
				    fprintf(stdout,"\n%-24s %-24s%-24s\n",
					MSGSTR(SRC_MSG7,"Pid"), 
					MSGSTR(SRC_MSG8,"Inet Address"),
					MSGSTR(SRC_MSG9,"Hostname"));
				    printflg++;
		    		}
                                fprintf(stdout," %-24d%-24s%-24s\n",
			            srcserv->serv[x].connect[y].pid,
				    inet_ntoa(srcserv->serv[x].connect[y].inetaddr),
				    srcserv->serv[x].connect[y].hostname);
			    	if (inetserv.portnum == 23)	/* telnet */
				    do_telnet(srcserv->serv[x].connect[y].pid);
			     }

        /* detach the shared memory segment prior to exiting */
        if (shmdt(srcserv) < 0) {
                fprintf(stderr, MSGSTR(ERR_SHMDT,
		     "%s: shmdt for shared memory failed.\n"),progname);
                exit(1);
        }

}


getconfig(portnum)
	char	*portnum;
{
	char 	criteria[128];
	int	rc,
		firstflg = 1,
		match = FALSE;
	char 	*p;

	/* setup odm environment and perform odm initialization */
	if ((rc = odm_initialize()) < 0) {
		fprintf(stderr, 
			MSGSTR(ERR_INIT, "%s: error initializing odm.\n"), 
			progname);
		exit(odmerrno);
	}

	/* setup odm environment and perform odm initialization */
	if ((rc = odm_set_path(InetServ_PATH)) < 0) {
		fprintf(stderr, 
			MSGSTR(ERR_PATH, 
			"%s: error setting odm default path.\n"), 
			progname);
		exit(odmerrno);
	}

	/* open the inetserv object class */
	if ((rc = (int)odm_open_class(InetServ_CLASS)) < 0) {
		fprintf(stderr, 
			MSGSTR(ERR_OPEN, "%s: error opening odm object: %s.\n"),
			progname, "InetServ");
		exit(odmerrno);
	}

	/* setup search criteria and query the InetServ database */
	sprintf(criteria,"portnum = %s", portnum);
	while (!match && (rc=(int)odm_get_obj(InetServ_CLASS,criteria,&inetserv,firstflg)) > 0) {
		firstflg = 0;

		p = (inetserv.state == 3l) ? 
			MSGSTR(SRC_MSG5,"active") : MSGSTR(SRC_MSG6,"inactive");
		fprintf(stdout," %-12s%-24s%-24s%s\n",
			inetserv.servname,inetserv.command,inetserv.description,p);
	}

	/* close inetserv object class and terminate odm access */
	if ((rc = odm_close_class(InetServ_CLASS)) < 0) {
		fprintf(stderr, 
			MSGSTR(ERR_CLOSE,"%s: error closing odm object: %s.\n"),
			progname, "InetServ");
		exit(odmerrno);
	}

	/* terminate odm access with a graceful cleanup */
	if ((rc = odm_terminate()) < 0) {
		fprintf(stderr, 
			MSGSTR(ERR_TERM, "%s: error terminating odm.\n"), 
			progname);
		exit(odmerrno);
	}
}


do_ftp(buffer)
	char	*buffer;
{
	char 	*p;
	int	debug = 0, 
		logging = 0, 
		tracing = 0;
	
	p = buffer;
	while ((p = strchr(p, '-')) != (char *) NULL) {
		p++;
		if (*p == 'v' || *p == 'd')
			debug++;
		else if (*p == 'l')
			logging++;
		else if (*p == 's')
			tracing++;
	}
	p = (debug) ? MSGSTR(SRC_MSG5,"active") : MSGSTR(SRC_MSG6,"inactive");
	fprintf(stdout,"\n%-12s %s\n", MSGSTR(SRC_MSG10,"Debug"), p);
	p = (logging) ? MSGSTR(SRC_MSG5,"active") : MSGSTR(SRC_MSG6,"inactive");
	fprintf(stdout,"%-12s %s\n", MSGSTR(SRC_MSG11,"Logging"), p);
	p = (tracing) ? MSGSTR(SRC_MSG5,"active") : MSGSTR(SRC_MSG6,"inactive");
	fprintf(stdout,"%-12s %s\n", MSGSTR(SRC_MSG12,"Tracing"), p);
}


do_telnet(pid)
	int	pid;
{
	struct	telnetd_entry *telentry;
	key_t	telmemkey;
	int	telmemid;
	int	x;

	/* create shared mem key and attach shared memory created by telnetd */
	if ((telmemkey = ftok(TELNETD, SRC_FD)) == (key_t) -1) {
                fprintf(stderr, MSGSTR(ERR_FTOK,
		     "%s: ftok for shared memory failed.\n"),progname);
		exit(1);
	}

	if ((telmemid = shmget(telmemkey,sizeof(struct telnetd_entry),0)) < 0) {
                fprintf(stderr, MSGSTR(ERR_SHMGET,
		     "%s: shmget for shared memory failed.\n"),progname);
		exit(1);
	}

	if ((telentry = (struct telnetd_entry *) shmat(telmemid, 0, 0)) < 0) {
                fprintf(stderr, MSGSTR(ERR_SHMAT,
		     "%s: shmat for shared memory failed.\n"),progname);
		exit(1);
	}

	for (x = 0; x < MAXCONN; x++) {
		if (telentry->conn[x].pid == pid) {
			fprintf(stdout," terminal type = %s\n\n",
				telentry->conn[x].termtype);
			break;
		}
	}

        /* detach the shared memory segment prior to exiting */
        if (shmdt(telentry) < 0) {
                fprintf(stderr, MSGSTR(ERR_SHMDT,
		     "%s: shmdt for shared memory failed.\n"),progname);
                exit(1);
	}
}


do_finger(buffer)
	char	*buffer;
{
	char 	*p;
	int	debug = 0; 
	
	p = buffer;
	while ((p = strchr(p, '-')) != (char *) NULL) {
		p++;
		if (*p == 's')
			debug++;
	}
	p = (debug) ? MSGSTR(SRC_MSG5,"active") : MSGSTR(SRC_MSG6,"inactive");
	fprintf(stdout,"\n%-12s %s\n", MSGSTR(SRC_MSG10,"Debug"), p);
}


do_login(buffer)
	char	*buffer;
{
	char 	*p;
	int	debug = 0; 
	
	p = buffer;
	while ((p = strchr(p, '-')) != (char *) NULL) {
		p++;
		if (*p == 's')
			debug++;
	}
	p = (debug) ? MSGSTR(SRC_MSG5,"active") : MSGSTR(SRC_MSG6,"inactive");
	fprintf(stdout,"\n%-12s %s\n", MSGSTR(SRC_MSG10,"Debug"), p);
}
