static char sccsid[] = "@(#)26	1.2  src/tcpip/usr/sbin/compat/inetserv.c, tcpip, tcpip411, GOLD410 7/14/94 10:32:32";
/*
 * COMPONENT_NAME: (TCPIP) SRC commands
 *
 * FUNCTIONS:  validate_portnum, validate_waitstate, validate_protocol,
 *             validate_socktype, get_inet_obj, purge_inet_obj, add_inet_obj,
 *             modify_inet_obj, startup_odm, wrapup_odm, usage
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
 * inetserv.c - Modifies/Displays InetServ object information derived from the
 *              /etc/services and /etc/inetd.conf files. If object modification
 *              is done, the inetexp command is called to update the 
 *              /etc/services and /etc/inetd.conf files to reflect the 
 *              modifications made in the InetServ database. 
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<odmi.h>
#include	"libffdb.h"
#include	"libsrc.h"
#include	"inetodm.h"

#include <nl_types.h>
#include "inetserv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_INETSERV,n,s)
 
#define		SHOW_SRV	0		/* show service object info */
#define		SHOW_CNF	1		/* show config object info */
#define 	CHANGE		2		/* change an object */
#define		ADD		3		/* add/activate an object */
#define		DEACTIVATE_ALL	4		/* deactivate all objects */
#define		DEACTIVATE_ONE	5		/* deactivate an object */

#define		INTERNAL	"internal"
#define		SRV_HEADER	"Service\t\tPort/Protocol\tAliases"
#define		CNF_HEADER1	"Service\tSocket\tProtocol Wait/\tUser\tServer Program\tServer Program"
#define		CNF_HEADER2	"Name\tType\t\t Nowait\t\t\t\tArguments"
#define		DASH_HEADER	 "-------------------------------------------------------------------------------"

struct	SERV_PARM 
	{
	char	servname[20];
	char	protocol[10];
	long	state;
	};
struct	SERV_PARM *serv_parm[MAXRECORDS];
int	serv_parm_cnt = 0;   

char 	progname[128],			/* = argv[0] at run time */
	outrec[MAXRECLEN];		/* temporary storage for output recs */

struct	InetServ inetserv;		/* storage area for ODM data */

extern 	char 	*optarg;
extern 	int 	optind;
extern	int	odmerrno;
int    	Zflag = 0,		/* for printing in smit format */
	iflag = 0; 

main(argc, argv)
	int 	argc;
	char 	**argv;
	{
	char	*cp = (char *)NULL,
		*cp2 = (char *)NULL,
		*service = (char *)NULL,
		*socket = (char *)NULL,
		*protname = (char *)NULL,
		*portnum = (char *)NULL,
		*alias_arg = (char *)NULL,
		*username = (char *)NULL,
		*waitind = (char *)NULL,
		*servpath = (char *)NULL,
		*servargs = (char *)NULL,
		*new_name = (char *)NULL,
		*new_proto = (char *)NULL;

    	int 	c = 0, 
		action = -1,
		Dflag = 0, 
		Iflag = 0, 
		Pflag = 0, 
		Rflag = 0, 
		Sflag = 0, 
		Uflag = 0, 
		Vflag = 0, 
		Xflag = 0, 
		aflag = 0, 
		cflag = 0, 
		dflag = 0, 
		nflag = 0, 
		rflag = 0, 
		pflag = 0, 
		sflag = 0, 
		tflag = 0, 
		uflag = 0, 
		vflag = 0, 
		wflag = 0; 

	long	state = 0l;

	setlocale(LC_ALL, "");
	catd = catopen(MF_INETSERV,0);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc &&
	   (c = getopt(argc, argv, "acdisDSIXv:p:n:t:w:u:U:V:R:P:r:Z")) != EOF) 
		{
		switch (c) 
			{
			case 'D':
				if (aflag || dflag || cflag || sflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			Dflag++;
	    			break;

			case 'I':
				if (Sflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			Iflag++;
	    			break;

			case 'S':
				if (Iflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			Sflag++;
	    			break;

			case 'P':
	    			Pflag++;
				new_proto = optarg;
				validate_protocol(new_proto);
	    			break;

			case 'R':
	    			Rflag++;
				servargs = optarg;
				if (strlen(servargs) > 50) {
				    usage(MSGSTR(BAD_ARGS,
				    "Service arguments (-R) must be 50 characters or less"));
				}
	    			break;

			case 'U':
	    			Uflag++;
				username = optarg;
	    			break;

			case 'V':
	    			Vflag++;
				new_name = optarg;
	    			break;

			case 'X':
				if (vflag || pflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			Xflag++;
	    			break;

			case 'Z':
				if (aflag || dflag || cflag || Dflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
				Zflag++;
				break;

			case 'a':
				if (dflag || cflag || sflag || Dflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			aflag++;
	    			break;

			case 'c':
				if (dflag || aflag || sflag || Dflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			cflag++;
	    			break;

			case 'd':
				if (cflag || aflag || sflag || Dflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			dflag++;
	    			break;

			case 'i':
	    			iflag++;
	    			break;

			case 'n':
	    			nflag++;
				portnum = optarg;
				validate_portnum(portnum);
	    			break;

			case 'p':
				if (Xflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			pflag++;
				protname = optarg;
				validate_protocol(protname);
	    			break;

			case 'r':
	    			rflag++;
				servpath = optarg;
	    			break;

			case 's':
				if (cflag || aflag || dflag || Dflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			sflag++;
	    			break;

			case 't':
	    			tflag++;
				socket = optarg;
				validate_socktype(socket);
	    			break;

			case 'u':
				uflag++;
				alias_arg = optarg;
	    			break;

			case 'v':
				if (Xflag)
	    				usage(MSGSTR(INCOMP_SW,
					"incompatible command line switches."));
	    			vflag++;
				service = optarg;
	    			break;

			case 'w':
	    			wflag++;
				waitind = optarg;
				validate_waitstate(waitind);
	    			break;

			case '?':
	    			usage(0);

			default:
	    			usage(MSGSTR(UNK_SWITCH,
					"unknown switch: %c."),c);
			}
    		}

    	if (optind != argc)
		usage(MSGSTR(NOT_PARSED,"some parameters were not parsed."));


	if (!(Dflag || aflag || cflag || dflag || sflag))
		usage(MSGSTR(SPEC_SW1,
"you must specify one of the following options: -a, -c, -d, -D, or -s."));

	if (iflag && !sflag)
	    	usage(MSGSTR(INCOMP_SW, "incompatible command line switches."));

	if (tflag)
		{
		if (!strcmp(socket, STREAM))
			{
			wflag++;
			waitind = TCP_NOWAIT;
			}
		}

	if (rflag)
		if (!strcmp(servpath, INTERNAL))
			{
			Rflag++;
			servargs = BLANK;
			}

    	if (aflag)
		{
		if (Sflag)
			{
			if (!(vflag && pflag && nflag))
				usage(MSGSTR(SPEC_SW2,
				"you must specify the -v, -p, and -n options."));
			if (tflag || wflag || Uflag || rflag || Rflag)
	    			usage(MSGSTR(INCOMP_SW,
				"incompatible command line switches."));
			}
		else if (Iflag)
			{
			if (!(vflag && pflag && tflag && wflag && Uflag && rflag))
				usage(MSGSTR(SPEC_SW3,
"you must specify the -v, -p, -t, -w, -U, and -r options."));
			if (nflag || uflag)
	    			usage(MSGSTR(INCOMP_SW,
				"incompatible command line switches."));
			}
		else
			usage(MSGSTR(SPEC_SW4,
"you must specify either the -S or -I option."));
			 
		startup_odm();

		if (get_inet_obj(ADD, service, protname, &state))
			{
			if (Sflag)
				{
				for (c = 0; c < serv_parm_cnt; c++)
					if (serv_parm[c]->state == 1l ||
					    serv_parm[c]->state == 3l)
						{
						wrapup_odm();
						fprintf(stderr,MSGSTR(OBJ_SRV1,"\"%s/%s\" service object already exists\n"), service, protname);
						exit(1);
						}
				state = 1l;
				}

			if (Iflag)
				{
				for (c = 0; c < serv_parm_cnt; c++)
					if (serv_parm[c]->state == 3l)
						{
						wrapup_odm();
						fprintf(stderr,MSGSTR(OBJ_CNF1,
"\"%s/%s\" configuration object already exists\n"), service, protname);
						exit(1);
						}
				state = 3l;
				}

			modify_inet_obj(ADD,state,service,protname,portnum,
				socket,alias_arg,username,servpath,servargs,
				waitind,service,protname);
			}
		else
			{
			if (Sflag)
				state = 1l;
				 
			if (Iflag)
				{
				wrapup_odm();
				fprintf(stderr,MSGSTR(OBJ_SRV2,
"\"%s/%s\" service object does not exists, cannot add configuration\n"), 
					service, protname);
				exit(1);
				}

			add_inet_obj(state,service,protname,portnum,socket,
				alias_arg,username,servpath,servargs,waitind);
			}

		wrapup_odm();
		}


    	if (cflag)
		{
		if (Sflag || Iflag)
			action = CHANGE;
		else
			usage(MSGSTR(SPEC_SW4,
			"you must specify either the -S or -I option."));

		if (!(vflag && pflag))
			usage(MSGSTR(SPEC_SW5,
				"you must specify the -v and -p options."));

		if (Sflag)
			{
			if (tflag || wflag || Uflag || rflag || Rflag)
	    			usage(MSGSTR(INCOMP_SW,
				"incompatible command line switches."));
			if (!(Vflag || Pflag || nflag || uflag))
				usage(MSGSTR(SPEC_SW6,"you must specify the -V, -P, -n, or -u option(s)."));
			}

		if (Iflag)
			{
			if (nflag || uflag)
	    			usage(MSGSTR(INCOMP_SW,
				"incompatible command line switches."));
			if (!(Vflag || Pflag || tflag || wflag || Uflag || rflag || Rflag))
				usage(MSGSTR(SPEC_SW7,
"you must specify the -V, -P, -t, -w, -U, -r, or -R option(s)."));
			}

		startup_odm();
		state = -1l;
		c = modify_inet_obj(action,state,service,protname,portnum, 
			socket,alias_arg,username,servpath,servargs, 
			waitind,new_name,new_proto);
		wrapup_odm();
		if (!c)
			{
			fprintf(stderr,MSGSTR(OBJ_CNF2,
				"\"%s/%s\" object does not exists\n"), 
				service, protname);
			exit(1);
			}
		}


    	if (dflag)
		{
		if (!(Sflag || Iflag))
			usage(MSGSTR(SPEC_SW4,
			"you must specify either the -S or -I option."));

		if (!(vflag || pflag || Xflag))
			usage(MSGSTR(SPEC_SW8,
		"you must specify the -X option or the -v and -p options."));
		else if (Xflag)
			{
			startup_odm();
			action = DEACTIVATE_ALL;
			get_inet_obj(action, NULL, NULL, NULL, &state);
			state = (Sflag) ? 0l : 1l;
			for (c = 0; c < serv_parm_cnt; c++)
				{
				modify_inet_obj(action,state,
					serv_parm[c]->servname,
					serv_parm[c]->protocol,
					NULL,NULL,NULL,NULL,NULL, 
					NULL,NULL,NULL,NULL);
				free(serv_parm[c]);
				}
			wrapup_odm();
			}
		else if (!(vflag && pflag))
			usage(MSGSTR(SPEC_SW5,
			"you must specify the -v and -p options."));
		else
			{
			startup_odm();
			action = DEACTIVATE_ONE;
			if (get_inet_obj(action,service,protname,&state))
				{
				if (Sflag)
					state = 0l;
				else if (Iflag)
					if (state != 0l)
						state = 1l;

				modify_inet_obj(action,state,service,protname,
					NULL,NULL,NULL,NULL,NULL, 
					NULL,NULL,NULL,NULL);
				wrapup_odm();
				}
			else
				{
				wrapup_odm();
				fprintf(stderr,MSGSTR(OBJ_CNF2,
					"\"%s/%s\" object does not exists\n"), 
					service, protname);
				exit(1);
				}
			}
		}


    	if (Dflag)
		{
		if (!(vflag || pflag || Xflag))
			usage(MSGSTR(SPEC_SW8,
		"you must specify the -X option or the -v and -p options."));
		else if (Xflag)
			{
			}
		else if (!(vflag && pflag))
			usage(MSGSTR(SPEC_SW5,
				"you must specify the -v and -p options."));
	
		startup_odm();
		purge_inet_obj(service, protname);
		wrapup_odm();
		}


    	if (sflag)
		{
		if (Sflag) {
			action = SHOW_SRV;
			if (Zflag)
				printf("#service:port:proto:aliases\n");
		} else if (Iflag) {
			action = SHOW_CNF;
			if (Zflag)
				printf("#service:socket:proto:wait:user:prog:args\n");
		} else
			usage(MSGSTR(SPEC_SW4,
			"you must specify either the -S or -I option."));

		if (!(vflag || pflag || Xflag))
			usage(MSGSTR(SPEC_SW8,
		"you must specify the -X option or the -v and -p options."));
		else if (Xflag)
			{
			}
		else if (!(vflag && pflag))
			usage(MSGSTR(SPEC_SW5,
				"you must specify the -v and -p options."));

		startup_odm();
		c = get_inet_obj(action, service, protname, &state);
		wrapup_odm();
		if (!c) 
			{
			if (Zflag) 
				{
				if (action == SHOW_SRV)
					printf("%s::%s:\n",service,protname);
				else 
					{
					if (!strcmp(protname,"tcp"))
						{
						cp = STREAM;
						cp2 = TCP_NOWAIT;
						}
					else
						{
						cp = DGRAM;
						cp2 = TCP_WAIT;
						}
					printf("%s:%s:%s:%s:::\n",
						service,cp,protname,cp2);
					}
				}
			else
				exit(1);
			}
		}
	else
		{
		if (execl(INETEXP, INETEXP, 0) < 0)
			{
			fprintf(stderr, 
				MSGSTR(ERR_EXEC,"error executing %s."),INETEXP);
			exit(-1);
			}
		}

	exit(0);
	}


validate_portnum(portnum)
	char	*portnum;
	{
	if (strspn(portnum, "0123456789") != strlen(portnum))
		usage(MSGSTR(BAD_PORT,"port number must be numeric."));
	}
  

validate_waitstate(waitind)
	char	*waitind;
	{
	if (!(strncmp(waitind, TCP_WAIT, strlen(TCP_WAIT))))
		{
		}
	else if (!(strncmp(waitind, TCP_NOWAIT, strlen(TCP_NOWAIT))))
		{
		}
	else
		usage(MSGSTR(BAD_WAIT,
		"command line contains invalid wait state: %s."),waitind);
	}
  

validate_protocol(protname)
	char	*protname;
	{
	if (!(strncmp(protname, TCP, strlen(TCP))))
		{
		}
	else if (!(strncmp(protname, UDP, strlen(UDP))))
		{
		}
	else
		usage(MSGSTR(BAD_PROTO,
		"command line contains invalid protocol: %s."),protname);
	}
  

validate_socktype(socket)
	char	*socket;
	{
	if (!(strncmp(socket, STREAM, strlen(STREAM))))
		{
		}
	else if (!(strncmp(socket, DGRAM, strlen(DGRAM))))
		{
		}
	else if (!(strncmp(socket, SUNRPC_TCP, strlen(SUNRPC_TCP))))
		{
		}
	else if (!(strncmp(socket, SUNRPC_UDP, strlen(SUNRPC_UDP))))
		{
		}
	else
		usage(MSGSTR(BAD_SOCKET,
		"command line contains invalid socket type: %s."),socket);
	}
  

int get_inet_obj(action, service, protname, old_state)
	int	action;
	char	*service,
		*protname;
	long	*old_state;
	{
	char 	buf[20],
		criteria[128];
	int 	rc,
		firstflg = TRUE,
		header_printed = FALSE,
		match = FALSE;

	if (service && protname)
		sprintf(criteria,
			"servname = '%s' and protocol = '%s'",service,protname);
	else
		criteria[0] = '\0';
	

	while ((rc=(int)odm_get_obj(InetServ_CLASS,criteria,&inetserv,firstflg)) > 0)
		{
		firstflg = FALSE;
		if (action == SHOW_SRV)
			{
			if (!strncmp(inetserv.socktype,SUNRPC,strlen(SUNRPC)))
				{
				}
			else if (inetserv.state && inetserv.portnum != -1)
				{
				if (!header_printed && !Zflag)
					{
					printf("%s\n", MSGSTR(SRV_HEADER_MSG,
								   SRV_HEADER));
					printf("%s\n", DASH_HEADER);
					header_printed = TRUE;
					}
				if (strlen(inetserv.servname) > 7)
					strcpy(buf, "\t");
				else
					strcpy(buf, "\t\t");
				if (Zflag) {
					header_printed = TRUE;
				        printf("%s:%d:%s:%s\n",
					inetserv.servname,inetserv.portnum,
					inetserv.protocol,inetserv.alias);
				} else					
				        printf("%s%s%d/%s\t\t%s %s\n",
					inetserv.servname,buf,inetserv.portnum,
					inetserv.protocol,inetserv.alias,
					inetserv.description);
				}
			}
		else if (action == SHOW_CNF)
			{
			if ((!iflag && inetserv.state == 3)
			  || (iflag && inetserv.state == 1))
				{
				if (!header_printed && !Zflag)
					{
					printf("%s\n", MSGSTR(CNF_HEADER1_MSG,
								  CNF_HEADER1));
					printf("%s\n", MSGSTR(CNF_HEADER2_MSG,
								  CNF_HEADER2));
					printf("%s\n", DASH_HEADER);
					header_printed = TRUE;
					}
				if (strlen(inetserv.socktype) > 7)
					strcpy(buf, BLANK);
				else
					strcpy(buf, "\t");
				if (!iflag || (iflag && (strlen(inetserv.socktype) != 0)))
				    {
				    if (Zflag) {
					header_printed = TRUE;
					printf("%s:%s:%s:%s:%s:%s:%s\n",
					inetserv.servname,inetserv.socktype,
					inetserv.protocol,inetserv.waitstate, 
					inetserv.user,inetserv.path,
					inetserv.command);
				    } else
					printf("%s\t%s%s%s\t%s\t%s\t%s %s\n",
					inetserv.servname,inetserv.socktype,buf,
					inetserv.protocol,inetserv.waitstate, 
					inetserv.user,inetserv.path,
					inetserv.command);
				    }
				}
			}
		else if (action == DEACTIVATE_ALL)
			{
			if (inetserv.state == 3l || inetserv.state == 1l)
				{
				serv_parm[serv_parm_cnt] = (struct SERV_PARM *)
					malloc(sizeof(struct SERV_PARM));
				if (serv_parm[serv_parm_cnt] == 
					(struct SERV_PARM *)NULL)
					{
					fprintf(stderr,
						MSGSTR(ERR_MALLOC,
						"error allocating memory."));
					exit(-1);
					}
				strcpy(serv_parm[serv_parm_cnt]->servname,
					inetserv.servname);
				strcpy(serv_parm[serv_parm_cnt]->protocol,
					inetserv.protocol);
				serv_parm_cnt++;
				}
			}
		else if (action == ADD)
			{
			serv_parm[serv_parm_cnt] = (struct SERV_PARM *)
				malloc(sizeof(struct SERV_PARM));
			if(serv_parm[serv_parm_cnt] == (struct SERV_PARM *)NULL)
				{
				fprintf(stderr, MSGSTR(ERR_MALLOC,
					"error allocating memory."));
				exit(-1);
				}
			strcpy(serv_parm[serv_parm_cnt]->servname,
				inetserv.servname);
			strcpy(serv_parm[serv_parm_cnt]->protocol,
				inetserv.protocol);
			serv_parm[serv_parm_cnt]->state = inetserv.state;
			serv_parm_cnt++;
			match = TRUE;
			}
		else
			{
			*old_state = inetserv.state;
			match = TRUE;
			}
		}

	if (action == SHOW_SRV || action == SHOW_CNF) 
		match = (header_printed) ? 1 : 0;
	return(match);
	}


purge_inet_obj(service, protname)
	char	*service,
		*protname;
	{
	int 	rc;
	char	criteria[128];

	if (service && protname)
		sprintf(criteria,
			"servname = '%s' and protocol = '%s'",service,protname);
	else
		criteria[0] = '\0';
	
	if  ((rc = odm_rm_obj(InetServ_CLASS, criteria)) < 0)
		{
		fprintf(stderr,
			MSGSTR(ERR_DELETE,"error deleting inetserv object."));
		exit(odmerrno);
		}
	}


add_inet_obj(state,service,protname,portnum,socket,alias_arg,username,servpath,servargs,waitind)
	long	state;
	char	*service,
		*protname,
		*portnum,
		*socket,
		*alias_arg,
		*username,
		*servpath,
		*servargs,
		*waitind;
	{
	int 	rc;

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
	inetserv.state = state;
	strncpy(inetserv.servname, service, sizeof (inetserv.servname) - 1);
	if (socket != (char *) NULL)
		strncpy(inetserv.socktype, socket, 
			sizeof (inetserv.socktype) - 1);
	strncpy(inetserv.protocol, protname, 
		sizeof (inetserv.protocol) - 1);
	if (waitind != (char *) NULL)
		strncpy(inetserv.waitstate, waitind, 
			sizeof (inetserv.waitstate) - 1);
	if (username != (char *) NULL)
		strncpy(inetserv.user, username, 
			sizeof (inetserv.user) - 1);
	if (servpath != (char *) NULL)
		strncpy(inetserv.path, servpath, 
			sizeof (inetserv.path) - 1);
	if (servargs != (char *) NULL) 
		strncpy(inetserv.command, servargs, 
			sizeof (inetserv.command) - 1);
	inetserv.portnum = atol(portnum);
	if (alias_arg != (char *) NULL) 
		strncpy(inetserv.alias, alias_arg, 
			sizeof (inetserv.alias) - 1);

	/* add the object to the database */
	if ((rc = odm_add_obj(InetServ_CLASS, &inetserv)) < 0)
		{
		fprintf(stderr, 
			MSGSTR(ERR_ADD, "%s: error adding odm object.\n"), 
			progname);
		exit(odmerrno);
		}
	}


int
modify_inet_obj(action,state,service,protname,portnum,socket,alias_arg,username,servpath,servargs,waitind,new_name,new_proto)
	int	action;
	long	state;
	char	*service,
		*protname,
		*portnum,
		*socket,
		*alias_arg,
		*username,
		*servpath,
		*servargs,
		*waitind,
		*new_name,
		*new_proto;
	{
	char	buf[MAXRECLEN],
		criteria[128];
	int	x = 0,
		firstflg = TRUE,
		match = FALSE,
		rc;

	if (service && protname)
		sprintf(criteria,
			"servname = '%s' and protocol = '%s'",service,protname);
	else
		criteria[0] = '\0';

	
	while ((rc=(int)odm_get_obj(InetServ_CLASS,criteria,&inetserv,firstflg)) > 0)
		{
		if (firstflg && action == CHANGE)
			match = TRUE;
		firstflg = FALSE;

		/* store values in the InetServ structure */
		if (action == ADD)
			{
			inetserv.state = state;
			action = CHANGE;
			state = -1l;
			}
		else if (state != -1l)
			inetserv.state = state;
		if (new_name != (char *) NULL)
			strncpy(inetserv.servname, new_name,
				sizeof (inetserv.servname) - 1);
		if (socket != (char *) NULL)
			strncpy(inetserv.socktype, socket,
				sizeof (inetserv.socktype) - 1);
		if (new_proto != (char *) NULL)
			strncpy(inetserv.protocol, new_proto,
				sizeof (inetserv.protocol) - 1);
		if (waitind != (char *) NULL)
			strncpy(inetserv.waitstate, waitind,
				sizeof (inetserv.waitstate) - 1);
		if (username != (char *) NULL)
			strncpy(inetserv.user, username,
				sizeof (inetserv.user) - 1);
		if (servpath != (char *) NULL)
			strncpy(inetserv.path, servpath,
				sizeof (inetserv.path) - 1);
		if (servargs != (char *) NULL) 
			strncpy(inetserv.command, servargs,
				sizeof (inetserv.command) - 1);
		if (portnum != (char *) NULL) 
			inetserv.portnum = atol(portnum);
		if (alias_arg != (char *) NULL) 
			strncpy(inetserv.alias, alias_arg,
				sizeof (inetserv.alias) - 1);
		odm_change_obj(InetServ_CLASS, &inetserv);
		}
	return(match);
	}


startup_odm()
	{
	int	rc;

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
	}



wrapup_odm()
	{
	int	rc;

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
	}


int usage(a, b, c, d)
	char	*a,
		*b, 
		*c, 
		*d;
	{
	if (a)
    		fprintf(stderr, a, b, c, d);
    	fprintf(stderr, 
		MSGSTR(USAGE1,
		"\nusage:\t%s -a -S -v servname -p protname -n portnum [-u \"alias...\"]\n"),
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE2,
		"\t%s -a -I -v servname -p protname -t socket -w waitind\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE3,
		"\t\t-U username -r serverpath [-R \"serverargs...\"]\n"));
    	fprintf(stderr, 
		MSGSTR(USAGE4,
		"\t%s -c -S -v servname -p protname -V new-name | -P new-prot\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE5,
		"\t\t[-n portnum] [-u \"alias...\"]\n"));
    	fprintf(stderr, 
		MSGSTR(USAGE6,
		"\t%s -c -I -v servname -p protname -V new-name | -P new-prot\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE7,
		"\t\t[-t socktype] [-w waitind] [-U username]\n"));
    	fprintf(stderr, 
		MSGSTR(USAGE8,
		"\t\t[-r serverpath] [-R \"serverargs...\"]\n"));
    	fprintf(stderr, 
		MSGSTR(USAGE9,
		"\t%s -d {-S | -I} {-v servname -p protname | -X}\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE10,
		"\t%s -s [-i]  {-S | -I} {-v servname -p protname | -X} [-Z]\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE11,
		"\t%s -D {-v servname -p protname | -X}\n"), 
		progname);
    	exit(2);
	}
