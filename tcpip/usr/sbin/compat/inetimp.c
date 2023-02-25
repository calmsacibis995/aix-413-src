static char sccsid[] = "@(#)23	1.5  src/tcpip/usr/sbin/compat/inetimp.c, tcpip, tcpip411, 9439C411a 9/29/94 17:50:42";
/*
 * COMPONENT_NAME: (TCPIP) SRC commands
 *
 * FUNCTIONS:  get_cnf_rec, get_nfs_rec, validate_inrec, validate_srvrec,
 *             parse_input_record, add_inetserv_obj.
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
 * inetimp - takes the /etc/services and /etc/inetd.conf files and creates
 *           the "InetServ" odm object class.
 */                                                                   

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/times.h>
#include	<netdb.h>
#include	<odmi.h>
#include	"libffdb.h"
#include	"libsrc.h"
#include	"inetodm.h"

#include <nl_types.h>
#include "inetimp_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_INETIMP,n,s)

#define	SRV_NAME_OFF	0
#define	SRV_PORT_OFF	1
#define	SRV_PROTO_OFF	2
#define	SRV_ALIAS_OFF	3

#define	CNF_NAME_OFF	0
#define	CNF_SOCK_OFF	1
#define	CNF_PROTO_OFF	2
#define	CNF_WAIT_OFF	3
#define	CNF_USER_OFF	4
#define	CNF_PATH_OFF	5
#define	CNF_CMND_OFF	6
#define	CNF_ARGS_OFF	7

#define	MAXFLDS		20

int	srvrec_ndx = 0,			/* index into srvrec pointers */
	cnfrec_ndx = 0,			/* index into cnfrec pointers */
   	srv_active = 0,
	cnf_active = 0;
int	conf_used[MAXRECLEN];		/* true == conf record already used */

char	*description = (char *)NULL,
    	*srvrec[MAXRECORDS],		/* buffers for INET_SRVFILE records */
	*cnfrec[MAXRECORDS],		/* buffers for INET_CNFFILE records */
	*srvflds[MAXFLDS],		/* parsed srvfile record fields */
	*cnfflds[MAXFLDS],		/* parsed cnffile record fields */
     	progname[128];			/* = argv[0] at run time */

struct	InetServ inetserv;

extern	int odmerrno;

struct	srv_entries 
	{
	char	servname[20];
	char	protocol[10];
	long	state;
	} service[MAXRECORDS];
int	srvndx = 0;

main(argc, argv)
int 	argc;
char 	**argv;
{
    char	inrec[MAXRECLEN],	/* input buffer for input records */
    *cp = (char *)NULL;	/* utility char pointer */
    
    int	x = 0,			/* general purpose index */
    y = 0,			/* general purpose index */
    rc = 0;			/* return code from ODM routines */
    
    FILE 	*srvfp = (FILE *)NULL,	/* stream pointer to INET_SRVFILE */
    *cnffp = (FILE *)NULL,	/* stream pointer to INET_CNFFILE */
    *ff_open();
    
    setlocale(LC_ALL, "");
    catd = catopen(MF_INETIMP,0);
    
    /* save program name, for error messages, etc. */
    strcpy(progname, *argv);
    
    /* initialize pointers to NULL */
    /* initialize pointers to NULL */
    for (x = 0; x < MAXRECORDS; x++)
    {
	srvrec[x] = (char *)NULL;
	cnfrec[x] = (char *)NULL;
    }
    
    /* read and store every record in INET_SRVFILE */
    srvrec_ndx = 0;
    srvfp = ff_open(INET_SRVFILE, "r+");	
    while (fgets(inrec, MAXRECLEN, srvfp) > 0)
    {
	if (srvrec_ndx > MAXRECORDS) {
	    fprintf(stderr, 
		    MSGSTR(ERR_RECNUM,
			   "%s: Too many records in %s, maximum is %d.\n"), 
		    progname, INET_SRVFILE, MAXRECORDS);
	    exit(-1);
	}
	if ((cp = strchr(inrec, '\n')) != NULL)
	    *cp = '\0'; 
	if ((cp = strchr(inrec, '/')) != NULL)
	    *cp = ' '; 
	if (!validate_inrec(inrec, TRUE))
	{
	    srvrec[srvrec_ndx] = (char *) malloc(strlen(inrec) + 1);
	    if (srvrec[srvrec_ndx] == (char *)NULL)
	    {
		fprintf(stderr, 
			MSGSTR(ERR_MALLOC,
			       "%s: error allocating memory.\n"), 
			progname);
		exit(-1);
	    }
	    memset(srvrec[srvrec_ndx], '\0', strlen(inrec) + 1);
	    strcpy(srvrec[srvrec_ndx], inrec);
	    srvrec_ndx++;
	}
    }
    ff_close(INET_SRVFILE, srvfp);
    srvrec[srvrec_ndx] = (char *)NULL;
    
    
    /* read and store every record in INET_CNFFILE */
    cnfrec_ndx = 0;
    cnffp = ff_open(INET_CNFFILE, "r+");	
    while (fgets(inrec, MAXRECLEN, cnffp) > 0)
    {
	if (cnfrec_ndx > MAXRECORDS) {
	    fprintf(stderr, 
		    MSGSTR(ERR_RECNUM,
			   "%s: Too many records in %s, maximum is %d.\n"), 
		    progname, INET_CNFFILE, MAXRECORDS);
	    exit(-1);
	}

	if ((cp = strchr(inrec, '\n')) != NULL)
	    *cp = '\0'; 
	if (!validate_inrec(inrec, FALSE))
	{
	    cnfrec[cnfrec_ndx] = (char *) malloc(strlen(inrec) + 1);
	    if (cnfrec[cnfrec_ndx] == (char *)NULL)
	    {
		fprintf(stderr, 
			MSGSTR(ERR_MALLOC,
			       "%s: error allocating memory.\n"), 
			progname);
		exit(-1);
	    }
	    memset(cnfrec[cnfrec_ndx], '\0', strlen(inrec) + 1);
	    strcpy(cnfrec[cnfrec_ndx], inrec);
	    cnfrec_ndx++;
	}
    }
    ff_close(INET_CNFFILE, cnffp);
    cnfrec[cnfrec_ndx] = (char *)NULL;
    
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
    
    /* purge old object class since we are rebuilding it anew */
    rc = odm_rm_class(InetServ_CLASS);
    
    /* create the inetserv object class */
    if ((rc = odm_create_class(InetServ_CLASS)) < 0)
    {
	fprintf(stderr, 
		MSGSTR(ERR_CREATE, 
		       "%s: error creating odm object: %s.\n"),
		progname, "InetServ");
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
    
    
    /* for every record read from INET_SRVFILE, create inetserv object */
    for (x = 0; x < srvrec_ndx; x++)
    {
	cp = srvrec[x];
	SKIPDELIM(cp);
	for (y = 0; y < MAXFLDS; y++)
	    srvflds[y] = (char *)NULL;
	srv_active = TRUE;
	if (*cp == '#')
	{
	    cp++;
	    srv_active = TRUE;  /* XXX should never be false */
	}
	description = strchr(cp, '#');
	parse_input_record(cp, srvflds);
	get_cnf_rec();
	if (!strcmp(srvflds[SRV_NAME_OFF], SUNRPC))
	    get_nfs_recs();
    }
    
    /* now check for any conf records that haven't already been added */
    for (x = 0; x < cnfrec_ndx; x++)
	if (! conf_used[x]) {  /* add it */
	    strcpy(inrec, cnfrec[x]);
	    cp = inrec;
	    SKIPDELIM(cp);
	    for (y = 0; y < MAXFLDS; y++)
		cnfflds[y] = srvflds[y] = (char *)NULL;
	    if (*cp == '#')
		cp++;
	    parse_input_record(cp, cnfflds);
	    cnf_active = (inrec[0] == '#') ? FALSE : TRUE;
	    srv_active = TRUE;  /* XXX force state = 3 or 1 */
	    add_inetserv_obj(FALSE);
	}
    
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
    
    exit(0);
}


int get_cnf_rec()    
	{
	char	*cp,
		rec[MAXRECLEN];
	int	x, y,
		match = FALSE;

	for (x = 0; x < cnfrec_ndx; x++)
		{
		strcpy(rec, cnfrec[x]);
		cp = rec;
		SKIPDELIM(cp);
		for (y = 0; y < MAXFLDS; y++)
			cnfflds[y] = (char *)NULL;
		if (*cp == '#')
			cp++;
		parse_input_record(cp, cnfflds);
		if (!strcmp(srvflds[SRV_NAME_OFF], cnfflds[CNF_NAME_OFF]))
			{
			if (!strcmp(srvflds[SRV_PROTO_OFF], 
				    cnfflds[CNF_PROTO_OFF]))
				{
				cnf_active = (rec[0] == '#') ? FALSE : TRUE;
				match = TRUE;
				conf_used[x] = match; /* mark record as used */
				add_inetserv_obj(FALSE);
				}
			}
		else 
			{
			y = 0;
			while (srvflds[SRV_ALIAS_OFF+y] != (char *)NULL)
				{
				if (!strcmp(srvflds[SRV_ALIAS_OFF+y], 
			    		cnfflds[CNF_NAME_OFF]))
					{
					if (!strcmp(srvflds[SRV_PROTO_OFF], 
				    		cnfflds[CNF_PROTO_OFF]))
							{
							cnf_active = 
								(rec[0] == '#')
								? FALSE : TRUE;
							match = TRUE;
							/* mark record used */
							conf_used[x] = match;
							add_inetserv_obj(FALSE);
							}
					}
				y++;
				}
			}
		}
	if (!match)
		{
		cnf_active = FALSE;
		for (y = 0; y < MAXFLDS; y++)
			cnfflds[y] = (char *)NULL;
		add_inetserv_obj(FALSE);
		}
	}


int get_nfs_recs()    
	{
	char	*cp,
		rec[MAXRECLEN];
	int	x, y;

	for (x = 0; x < cnfrec_ndx; x++)
		{
		strcpy(rec, cnfrec[x]);
		cp = rec;
		SKIPDELIM(cp);
		for (y = 0; y < MAXFLDS; y++)
			cnfflds[y] = (char *)NULL;
		cnf_active = TRUE;
		if (*cp == '#')
			{
			cp++;
			cnf_active = FALSE;
			}
		parse_input_record(cp, cnfflds);
		if (!strncmp(cnfflds[CNF_SOCK_OFF], SUNRPC, strlen(SUNRPC)))
			if (!strcmp(srvflds[SRV_PROTO_OFF], 
				    cnfflds[CNF_PROTO_OFF])) {
				conf_used[x] = TRUE; /* mark record as used */
				add_inetserv_obj(TRUE);
			}
		}
	}


int validate_inrec(rec, srvflag)    
	char	*rec;
	int	srvflag;
	{
	char	*flds[MAXFLDS],
		buf[MAXRECLEN],
		*cp;
	int	x = 0,
   		comment_rec = FALSE;

	while (x < MAXFLDS)
		flds[x++] = (char *)NULL;

	strcpy(buf, rec);
	cp = buf;
	SKIPDELIM(cp);
	if (*cp == '\0')
		return(1);
	if (*cp == '#')
		{
		comment_rec = TRUE;
		cp++;
		}
	parse_input_record(cp, flds);

	if (srvflag)
		return(validate_srvrec(flds, comment_rec));
	else
		return(validate_cnfrec(flds, comment_rec));
	}


int validate_srvrec(flds, comment_rec)
	char	*flds[];
	int	comment_rec;
	{
	/* protocol must be either "tcp" or "udp" */
	if (!strcmp(flds[SRV_PROTO_OFF], TCP))
		{
		}
	else if (!strcmp(flds[SRV_PROTO_OFF], UDP))
		{
		}
	else 
		{
		if (!comment_rec)
			fprintf(stderr, 
				MSGSTR(ERR_PROTO,
				"%s: %s contains invalid protocol: %s.\n"),
				progname, INET_SRVFILE, flds[SRV_PROTO_OFF]);
		return(1);
		}
	return(0);
	}


int validate_cnfrec(flds, comment_rec)
	char	*flds[];
	int	comment_rec;
	{
	/* socket type must be "stream","dgram","sunrpc_tcp" or "sunrpc_udp" */
	if (!strcmp(flds[CNF_SOCK_OFF], STREAM))
		{
		}
	else if (!strcmp(flds[CNF_SOCK_OFF], DGRAM))
		{
		}
	else if (!strcmp(flds[CNF_SOCK_OFF], SUNRPC_TCP))
		{
		}
	else if (!strcmp(flds[CNF_SOCK_OFF], SUNRPC_UDP))
		{
		}
	else 
		{
		if (!comment_rec)
			fprintf(stderr, 
				MSGSTR(ERR_SOCKET,
				"%s: %s contains invalid socket type: %s.\n"),
				progname, INET_CNFFILE, flds[CNF_SOCK_OFF]);
		return(1);
		}

	/* protocol field must be either "tcp" or "udp" */
	if (!strcmp(flds[CNF_PROTO_OFF], TCP))
		{
		}
	else if (!strcmp(flds[CNF_PROTO_OFF], UDP))
		{
		}
	else 
		{
		if (!comment_rec)
			fprintf(stderr, 
				MSGSTR(ERR_PROTO,
				"%s: %s contains invalid protocol: %s.\n"),
				progname, INET_CNFFILE, flds[CNF_PROTO_OFF]);
		return(1);
		}

	/* wait state must be either "wait" or "nowait" */
	if (!strcmp(flds[CNF_WAIT_OFF], TCP_WAIT))
		{
		}
	else if (!strcmp(flds[CNF_WAIT_OFF], TCP_NOWAIT))
		{
		}
	else 
		{
		if (!comment_rec)
			fprintf(stderr, 
				MSGSTR(ERR_WAIT,
				"%s: %s contains invalid wait state: %s.\n"),
				progname, INET_CNFFILE, flds[CNF_WAIT_OFF]);
		return(1);
		}
	return(0);
	}


parse_input_record(rec, flds)
	char	*rec,
	    	**flds;
	{
	char	*cp;

	SKIPDELIM(rec);
	while (*rec != '\0' && *rec != '#')
		{
		cp = rec;
		SKIPTOKEN(cp);
		*flds++ = rec;
		if (*cp != '\0')
			{
			*cp = '\0';
			rec = cp + 1;
			}
		else
			rec = cp;
		SKIPDELIM(rec);
		}
	*flds = (char *)NULL;
	}


add_inetserv_obj(nfs_record)
	int	nfs_record;
	{
	int	x = 0,
		match = FALSE,
		rc;
	struct servent *srvbynam;

	/* clear old values from last add */
	inetserv.state = 0l;
	memset(inetserv.servname, '\0', sizeof(inetserv.servname));
	memset(inetserv.socktype, '\0', sizeof(inetserv.socktype));
	memset(inetserv.protocol, '\0', sizeof(inetserv.protocol));
	memset(inetserv.waitstate, '\0', sizeof(inetserv.waitstate));
	memset(inetserv.user, '\0', sizeof(inetserv.user));
	memset(inetserv.path, '\0', sizeof(inetserv.path));
	memset(inetserv.command, '\0', sizeof(inetserv.command));
	inetserv.portnum = 0l;
	memset(inetserv.alias, '\0', sizeof(inetserv.alias));
	memset(inetserv.description, '\0', sizeof(inetserv.description));
	memset(inetserv.statusmethod, '\0', sizeof(inetserv.statusmethod));


	/* store values in the InetServ structure */
	if (srv_active)
		inetserv.state = (cnf_active) ? 3l : 1l;

	if (nfs_record || ! srvflds[SRV_NAME_OFF])
		strcpy(inetserv.servname, cnfflds[CNF_NAME_OFF]);
	else
		strcpy(inetserv.servname, srvflds[SRV_NAME_OFF]);

	if (cnfflds[CNF_SOCK_OFF] != (char *) NULL)
		strcpy(inetserv.socktype, cnfflds[CNF_SOCK_OFF]);

	if (! srvflds[SRV_PROTO_OFF])
		strcpy(inetserv.protocol, cnfflds[CNF_PROTO_OFF]);
	else
		strcpy(inetserv.protocol, srvflds[SRV_PROTO_OFF]);

	if (cnfflds[CNF_WAIT_OFF] != (char *) NULL)
		strcpy(inetserv.waitstate, cnfflds[CNF_WAIT_OFF]);

	if (cnfflds[CNF_USER_OFF] != (char *) NULL)
		strcpy(inetserv.user, cnfflds[CNF_USER_OFF]);

	if (cnfflds[CNF_PATH_OFF] != (char *) NULL)
		strcpy(inetserv.path, cnfflds[CNF_PATH_OFF]);

	if (cnfflds[CNF_CMND_OFF] != (char *) NULL) 
		{
		strcpy(inetserv.command, cnfflds[CNF_CMND_OFF]);
		x = 1;
		while (cnfflds[CNF_CMND_OFF + x] != (char *)NULL) 
			{
			strcat(inetserv.command, BLANK);
			strcat(inetserv.command, cnfflds[CNF_CMND_OFF + x]);
			x++;
			}
		}

	if (! srvflds[SRV_PORT_OFF]) {
		srvbynam = getservbyname(inetserv.servname, inetserv.protocol);
		if (srvbynam)
			inetserv.portnum = (long)srvbynam->s_port;
		else
			inetserv.portnum = -1;
	}
	else
		inetserv.portnum = atol(srvflds[SRV_PORT_OFF]);

	if (srvflds[SRV_ALIAS_OFF] != (char *) NULL) 
		{
		strcpy(inetserv.alias, srvflds[SRV_ALIAS_OFF]);
		x = 1;
		while (srvflds[SRV_ALIAS_OFF + x] != (char *)NULL) 
			{
			strcat(inetserv.alias, BLANK);
			strcat(inetserv.alias, srvflds[SRV_ALIAS_OFF + x]);
			x++;
			}
		}

        if (description != (char *) NULL) {
                /* Need to limit the comments to 39 characters plus
                 * a NULL, for the ODM database.
                 */
                strncpy(inetserv.description, description, 39);
                inetserv.description[39] = '\0';
        }

	if (!nfs_record)
		strcpy(inetserv.statusmethod, TCP_STAT);

	for (x = 0; x < srvndx; x++)
		{
		if (!strcmp(inetserv.servname, service[x].servname) &&
		    !strcmp(inetserv.protocol, service[x].protocol))
			{
			match = TRUE;
			if (inetserv.state == 3l && service[x].state == 3l)
				inetserv.state = 1l;
			}
		}
	if (!match)
		{
	 	strcpy(service[srvndx].servname, inetserv.servname);
	 	strcpy(service[srvndx].protocol, inetserv.protocol);
		service[srvndx].state = inetserv.state;
		srvndx++;
		}

	/* add the object to the database */
	if ((rc = odm_add_obj(InetServ_CLASS, &inetserv)) < 0)
		{
		fprintf(stderr, 
			MSGSTR(ERR_ADD, "%s: error adding odm object.\n"), 
			progname);
		exit(odmerrno);
		}

	}
