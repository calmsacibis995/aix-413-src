static char sccsid[] = "@(#)14	1.3  src/tcpip/usr/sbin/compat/inetexp.c, tcpip, tcpip411, 9439C411a 9/29/94 17:50:53";
/*
 * COMPONENT_NAME: (TCPIP) SRC  commands
 *
 * FUNCTIONS:  process_inetserv_objects, bld_srvrec, bld_cnfrec, purge_recs
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
 * inetexp - creates the /etc/services and /etc/inetd.conf files from 
 *           the "InetServ" odm object class.
 */                                                                   

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/times.h>
#include	<odmi.h>
#include	"libffdb.h"
#include	"libsrc.h"
#include	"inetodm.h"

#include <nl_types.h>
#include "inetexp_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_INETEXP,n,s)

#define		COMMENT	"#"		/* pointer to comment char string */

long	state = 0l,			/* object state */
	port = 0l;			/* object portnumer */

int	srvrec_ndx = 0,			/* index into srvrec pointers */
	cnfrec_ndx = 0;			/* index into cnfrec pointers */

char	*srvrec[MAXRECORDS],		/* buffer for INET_SRVFILE records */
	*cnfrec[MAXRECORDS],		/* buffer for INET_CNFFILE records */
	outrec[MAXRECLEN],		/* temporary storage for output recs */
	progname[128];			/* = argv[0] at run time */

struct  InetServ inetserv;
	
struct	srv_entries
	{
	char servname[20];
	char protocol[10];
	} service[MAXRECORDS];
int	srvndx = 0; 

extern	int odmerrno;

main(argc, argv)
	int 	argc;
	char 	**argv;
	{
	int	x = 0, rc;		/* general purpose index */

	setlocale(LC_ALL, "");
	catd = catopen(MF_INETEXP, 0);

	/* save program name, for error messages, etc. */
	strcpy(progname, *argv);

	/* setup odm environment and perform odm initialization */
	if ((rc = odm_initialize()) < 0)
		{
		fprintf(stderr, 
			MSGSTR(ERR_INIT, "%s: error initializing odm.\n"), 
			progname);
		exit(odmerrno);
		}

	/* setup odm environment and perform odm initialization */
	if ((rc = odm_set_path(InetServ_PATH)) < 0)
		{
		fprintf(stderr, 
			MSGSTR(ERR_PATH, 
			"%s: error setting odm default path.\n"), 
			progname);
		exit(odmerrno);
		}

	/* open the inetserv object class */
	if ((rc = (int)odm_open_class(InetServ_CLASS)) < 0)
		{
		fprintf(stderr, 
			MSGSTR(ERR_OPEN, "%s: error opening odm object: %s.\n"),
			progname, "InetServ");
		exit(odmerrno);
		}


	/* get all objects in the inetserver object class */
	/* for each one found, build the appropriate records */
	/* for the inetd.conf and services files  */
	process_inetserv_objects();


	/* close inetserv object class and terminate odm access */
	if ((rc = odm_close_class(InetServ_CLASS)) < 0)
		{
		fprintf(stderr, 
			MSGSTR(ERR_CLOSE,"%s: error closing odm object: %s.\n"),
			progname, "InetServ");
		exit(odmerrno);
		}

	/* terminate odm access with a graceful cleanup */
	if ((rc = odm_terminate()) < 0)
		{
		fprintf(stderr, 
			MSGSTR(ERR_TERM, "%s: error terminating odm.\n"), 
			progname);
		exit(odmerrno);
		}


	/* purge old records from services */
	purge_recs(INET_SRVFILE);
	/* write all service records to the /etc/services file */
	for (x = 0; x < srvrec_ndx; x++)
		{
		ff_add(INET_SRVFILE, srvrec[x]);
		free(srvrec[x]);
		}


	/* purge old records from inetd.conf */
	purge_recs(INET_CNFFILE);
	/* write all config records to the /etc/inetd.conf file */
	for (x = 0; x < cnfrec_ndx; x++)
		{
		ff_add(INET_CNFFILE, cnfrec[x]);
		free(cnfrec[x]);
		}


	exit(0);
	}

 
process_inetserv_objects()
{
    long 	rc = 0;
    int	firstflg = 1;
    
    srvrec_ndx = cnfrec_ndx = 0;
    
    while ((rc=(long)odm_get_obj(InetServ_CLASS,"",&inetserv,firstflg)) > 0)
    {
	if (srvrec_ndx > MAXRECORDS) {
	    fprintf(stderr, 
		    MSGSTR(ERR_RECNUM,
			   "%s: Too many records in %s, maximum is %d.\n"), 
		    progname, INET_SRVFILE, MAXRECORDS);
	    exit(-1);
	}
	if (cnfrec_ndx > MAXRECORDS) {
	    fprintf(stderr, 
		    MSGSTR(ERR_RECNUM,
			   "%s: Too many records in %s, maximum is %d.\n"), 
		    progname, INET_CNFFILE, MAXRECORDS);
	    exit(-1);
	}
	    
	srvrec[srvrec_ndx] = (char *)NULL;
	cnfrec[cnfrec_ndx] = (char *)NULL;
	
	firstflg = 0;
	if (!bld_srvrec())
	{
	    srvrec[srvrec_ndx] = strdup(outrec);
	    if (srvrec[srvrec_ndx] == (char *)NULL)
	    {
		fprintf(stderr,
			MSGSTR(ERR_MALLOC,
			       "%s: error allocating memory.\n"),
			progname);
		exit(-1);
	    }
	    srvrec_ndx++;
	}
	if (!bld_cnfrec())
	{
	    cnfrec[cnfrec_ndx] = strdup(outrec);
	    if (cnfrec[cnfrec_ndx] == (char *)NULL)
	    {
		fprintf(stderr,
			MSGSTR(ERR_MALLOC,
			       "%s: error allocating memory.\n"),
			progname);
		exit(-1);
	    }
	    cnfrec_ndx++;
	}
    }
}

 
int bld_srvrec()
	{
	char	buf[128];
	int	x = 0;

	/* if socktype = "sunrpc_udp or "sunrpc_tcp" skip this record */
	if (!strncmp(inetserv.socktype, SUNRPC, strlen(SUNRPC)) ||
	    inetserv.portnum == -1)
		return(1);

	for (x = 0; x < srvndx; x++)
		{
		if (!strcmp(inetserv.servname, service[x].servname) &&
		    !strcmp(inetserv.protocol, service[x].protocol))
			return(1);
		}
	strcpy(service[srvndx].servname, inetserv.servname);
	strcpy(service[srvndx].protocol, inetserv.protocol);
	srvndx++;

	if (inetserv.state)
		strcpy(outrec, inetserv.servname);
	else	
		{
		strcpy(outrec, COMMENT);
		strcat(outrec, inetserv.servname);
		}
	if (strlen(outrec) <= 7)
		strcat(outrec, "\t");
	strcat(outrec, "\t");
	sprintf(buf, "%d", inetserv.portnum);
	strcat(outrec, buf);
	strcat(outrec, "/");
	strcat(outrec, inetserv.protocol);
	strcat(outrec, "\t\t");
	strcat(outrec, inetserv.alias);
	strcat(outrec, BLANK);
	strcat(outrec, inetserv.description);

	return(0);
	}

 
int bld_cnfrec()
	{
	/* if socktype, waitstate, user, and path aren't present, skip record */
	if ((strlen(inetserv.socktype) == 0) && (strlen(inetserv.user) == 0) && 
	    (strlen(inetserv.waitstate) == 0) && (strlen(inetserv.path) == 0))
	 	return(1);

	if (inetserv.state == 3l)
		strcpy(outrec, inetserv.servname);
	else	
		{
		strcpy(outrec, COMMENT);
		strcat(outrec, inetserv.servname);
		}
	strcat(outrec, "\t");
	strcat(outrec, inetserv.socktype);
	if (strlen(inetserv.socktype) > 7)
		strcat(outrec, BLANK);
	else
		strcat(outrec, "\t");
	strcat(outrec, inetserv.protocol);
	strcat(outrec, "\t");
	strcat(outrec, inetserv.waitstate);
	strcat(outrec, "\t");
	strcat(outrec, inetserv.user);
	strcat(outrec, "\t");
	strcat(outrec, inetserv.path);
	strcat(outrec, BLANK);
	strcat(outrec, inetserv.command);

	return(0);
	}


int purge_recs(fname)
	char 	*fname;			/* pointer to file name */
	{
    	char 	tmpfile[128],
		*cp,
		*strstr(),
    	     	buf[MAXRECLEN];

    	FILE 	*fp, 
		*tfp,
		*ff_open();

    	struct 	stat 	sb;

    	if (stat(fname, &sb) != 0) 
		{
		fprintf(stderr,
			MSGSTR(ERR_STAT, "%s: %s does not exist.\n"),
			progname, fname);
		exit(errno);
    		}

    	fp = ff_open(fname, "r+");	/* "r+" locks the file */
    	strcpy(tmpfile, tmpnam(NULL));

    	if ((tfp = fopen(tmpfile, "w")) == 0) 
		{
		fprintf(stderr,
			MSGSTR(ERR_OPEN2, "%s: can't open temp file.\n"),
			progname);
		exit(errno);
    		}

	/* set the mode on the temp file the same as the target */
	if (fchmod(fileno(tfp), sb.st_mode)) {
		perror("chmod");
		exit(errno);
	}

	while (fgets(buf, MAXRECLEN, fp) > 0) 
		{
		if (buf[0] == '\n')
   			fputs(buf, tfp);
		else if (buf[0] == '#')
			{
			if ((cp = strstr(buf, TCP)) != (char *)NULL)
				{
				}
			else if ((cp = strstr(buf, UDP)) != (char *)NULL)
				{
				}
			else
   				fputs(buf, tfp);
			}
    		}

    	ff_close(fname, fp);
    	fclose(tfp);

    	if (rename(tmpfile, fname) == -1) 
		{
		if (errno == EXDEV)
	    		ff_rename_by_copying(tmpfile, fname);
		else 
			{
			fprintf(stderr,
				MSGSTR(ERR_RENAME,
				"%s: can't move temp file back to %s.\n"),
				progname, fname);
	    		exit(errno);
			}
    		}

    	return(0);
	}







