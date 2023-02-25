static char sccsid[] = "@(#)85	1.2.1.3  src/tcpip/usr/ccs/lib/libtcp_ocs/ocs_vtsup.c, tcpip, tcpip41J, 9512A_all 3/13/95 16:41:18";
/*
 * COMPONENT_NAME: TCPIP ocs_vtsup.c
 *
 * FUNCTIONS: findvhost odm_error_exit process_vhost talk_to_lm
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * CHANGE_ACTIVITY:
 *
 * $L1=OEOCS    HOM???? 941231 PDXO: Change to check for -1 as error
 *			return from ODM routines.  Check for < 0
 *			is incorrect since routines return an address.
 *			Also change return value from the address of
 *			an array to the array and casts of NULL pointer
 *			return values to agree with routine declarations.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/errno.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/syslog.h> 
#include "lm.h"    
#include "ocsvhost.h" 
#include "ocsadmin.h" 

#define ODMLOCKFILE	"/etc/objrepos/config_lock"
#define OCSADMINDEV	"/dev/ocsadmin"
#define OCSvhost_PATH   "/etc/objrepos"
#define BUFSIZE		256
#define MAXHOSTNODES	3
#define BZERO		0

#include <nl_types.h>
#include "libtcp_ocs_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_LIBTCP_OCS, n, s)

char prog_name[] = "ocs_vtsup";               
char *inet_ntoa();                         

/*
 * Open the OCS administration device to determine if
 * the "hostname" is connected.  If not, then try the
 * "alt_hostname_one".  If that's not connected, try
 * "alt_hostname_two".  If there are no host nodes
 * connected, then exit.
 */
caddr_t
find_host_to_connect(possible_host_node_list, max_host_cnt)
char *possible_host_node_list;
int max_host_cnt;
{
        int afd;                  /* OCS admin device fd */
        int host_cnt;
        static char hostnode_to_connect[HOSTNMSIZE];
        char temp_hostname[HOSTNMSIZE];
        struct hostent *hostnode_ptr;
        struct sockaddr_in sin_hostnode;
	struct  ocsioctl    ocsioctl;
	int hold_o_unit;
	int OCS_GET_TBMID_errno;

        if ((afd = open(OCSADMINDEV, O_RDWR)) < BZERO){
		syslog(LOG_ERR,
			MSGSTR(ERR_ADMIN,
			"%s: error opening OCS administration device: %d.\n"),
			prog_name, errno);
	        return((char *)NULL);	
	        } /* End open admin device check */	

	for( host_cnt = 0; host_cnt < max_host_cnt; host_cnt++){
		bcopy(possible_host_node_list, temp_hostname,
                               	 sizeof(temp_hostname));
                hostnode_ptr = gethostbyname(temp_hostname);
	        if (hostnode_ptr == NULL) {
                    syslog(LOG_ERR,
                       MSGSTR(ERR_GETHOST, "%s: gethostbyname() error: %d.\n"),
                                prog_name, h_errno);
                    close(afd);
                    return((char *)NULL);
                } /* End of if (hostnode_ptr == NULL) check */

	        bzero(&sin_hostnode, sizeof (struct sockaddr_in));
	        sin_hostnode.sin_port        = BZERO;
	        sin_hostnode.sin_len         = sizeof(struct sockaddr_in);
	        sin_hostnode.sin_family      = hostnode_ptr->h_addrtype;
	        sin_hostnode.sin_addr.s_addr = *((u_long *)hostnode_ptr->h_addr);
                ocsioctl.o_addr = (caddr_t) &sin_hostnode;
                ocsioctl.o_bufsize = sizeof(struct sockaddr_in);
                ocsioctl.o_unit = BZERO;
                /*
	         * If the Host is in a defined state, there is no
                 * TBMID (tbmid = 0) and ioctl cmd OCS_GET_TBMID returns
                 * EINVAL. But this is not an error in our case.
                 * It just means that the Host is not connected,
                 * and look at the next Host name in the list.
                 * This is bogus.
                 */		
                if ((ioctl(afd, OCS_GET_TBMID, &ocsioctl)) < BZERO){
                     if (errno != EINVAL){
                         syslog(LOG_ERR,
                            MSGSTR(ERR_TBMID,
                            "%s: ioctl() OCS_GET_TBMID error occurred: %d.\n"),
                            prog_name, errno);
                         close(afd);
                         return((char *)NULL);
                     } /* End of  if (errno != EINVAL) */
                } /* End of  OCS_GET_TBMID error */

                OCS_GET_TBMID_errno = errno; /* save errno value */
                if (OCS_GET_TBMID_errno != EINVAL){
                    hold_o_unit = ocsioctl.o_unit;
                    bzero(&ocsioctl, sizeof(ocsioctl));
                    ocsioctl.o_unit = hold_o_unit;
		    if ((ioctl(afd, OCS_IS_TBM_CONNECTED, &ocsioctl)) < BZERO){
                        syslog(LOG_ERR,
                           MSGSTR(ERR_TBMCON,
                           "%s: ioctl() OCS_IS_TBM_CONNECTED error occurred: %d.\n"),
                           prog_name, errno);
                        close(afd);
                        return((char *)NULL);
                    } /* End of  OCS_IS_TBM_CONNECTED error */

	            if (ocsioctl.o_bufsize){
		        bcopy(possible_host_node_list, hostnode_to_connect,
                               	 sizeof(hostnode_to_connect));
                        close(afd);
                        return(hostnode_to_connect); /* @L1C */
                    } /* End of if (ocsioctl.o_bufsize) */
                } /* End of if (OCS_GET_TBMID.errno != EINVAL) */
	        possible_host_node_list += HOSTNMSIZE;
        } /* End of for loop */

         if (host_cnt == max_host_cnt){
		syslog(LOG_ERR,
			MSGSTR(ERR_NOHOST,
			"%s: No host nodes are connected: %d.\n"),
			prog_name, errno);
                close(afd);
                return((char *)NULL);
	     } /* End of no more host nodes to check */

} /* End of find_host_to_connect() function */

/*
 *
 */
caddr_t
find_ocsnode_to_bind(hostnode_to_connect, lock_id)
char *hostnode_to_connect;
int lock_id;
{
    struct  CuAt    *cuat;      /* customized attribute ptr */
    char    crit[BUFSIZE];
    char    uniquetype[BUFSIZE];
    struct  listinfo    listinfo;
    int     rc;
    static char ocsnode_to_bind[HOSTNMSIZE];

	/* open the CuAt object class */
	if ((rc = (int)odm_open_class(CuAt_CLASS)) == -1){ /* @L1C */
	    syslog(LOG_ALERT,
		     MSGSTR(ERR_OPEN, "%s: error opening odm object %s: %d.\n"),
			     prog_name, "CuAt", odmerrno);
            odm_error_exit(odmerrno, lock_id);
	}

	/* get entry in customized database */

        sprintf(crit, "name = '%s' AND attribute = 'nodename'", hostnode_to_connect);
        cuat = (struct CuAt *) odm_get_list(CuAt_CLASS, crit, &listinfo, 1, 1);
        if ((int) cuat == -1){
            syslog(LOG_ALERT,
	            MSGSTR(ERR_CUAT,
                    "%s: odm_get_list() error occurred: %s: %d.\n"),
                    prog_name, "CuAt", odmerrno);
            odm_error_exit(odmerrno, lock_id);
            }

	/* close CuAt object class and terminate odm access */
	if ((odm_close_class(CuAt_CLASS)) == -1) { /* @L1C */
		syslog(LOG_ALERT,
			MSGSTR(ERR_CLOSE,
			"%s: error closing odm object %s: %d.\n"), prog_name,
			"CuAt", odmerrno);
                odm_error_exit(odmerrno, lock_id);
	}

        /* attribute found in customized database */
        if (listinfo.num > 0) {
            strncpy(ocsnode_to_bind, cuat->value, sizeof(ocsnode_to_bind));
            return(ocsnode_to_bind); /* @L1C */
        }
	else {
            syslog(LOG_ERR,
	            MSGSTR(ERR_OCSNODE,
                    "%s: No OCS node name associated with: %s.\n"),
                    prog_name, hostnode_to_connect);
            return((char *)NULL);
            }

} /* End of find_ocsnode_to_bind() function */

/*
 * Finds the virtual host from
 * the "OCSvhost" odm object class that
 * equals the destination internet address
 * found in the local telnet request socket. 
 *
 * If not found, then a null pointer is returned.
 */                                                                  
struct OCSconnection *
findvhost(local)
struct sockaddr_in *local;
{
	int	i;
	int	rc;	
	char	*lock_file = ODMLOCKFILE;	/* ODM lock file */
	int	lock_id;			/* ODM lock identifier */
	int	timeout = 1;			/* ODM lock timeout in sec */
	int	retry_max = 5;			/* ODM lock retry maximum  */
	char	*odm_path;
	static struct OCSvhost ocsvhost;
	static struct OCSconnection ocsconnection;
	char sstring[BUFSIZE];		/* ODM search criteria pointer */
	int found_vhost = FALSE;        /* OCSvhost entry matched */
        char *hostnode_to_connect = NULL;
        char *ocsnode_to_bind = NULL;
        char *host_buf;
        char *host_buf_save;
        int max_host_cnt = BZERO;



	if (local->sin_addr.s_addr == BZERO) {   
	   		syslog(LOG_ERR,
			MSGSTR(ERR_ADDR, "%s: socket address equals zero.\n"),
				prog_name);
			exit(1);
	}

	if (local->sin_family != AF_INET) {   
	   		syslog(LOG_ERR,
			MSGSTR(ERR_FAMILY,
				"%s: socket address family error: %d.\n"),
				prog_name, local->sin_family);
			exit(1);
	}

	/* setup odm environment and perform odm initialization */
	if ((odm_initialize()) == -1) { /* @L1C */
		syslog(LOG_ALERT,
			MSGSTR(ERR_INIT, "%s: error initializing odm: %d.\n"),
			prog_name, odmerrno);
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
					MSGSTR(ERR_TERM,
					"%s: error terminating odm: %d.\n"),
					prog_name, odmerrno);
			}
		}
		return((struct OCSconnection *) NULL); /* @L1C */
	}

	/* set path for object classes */
	if ((odm_path = odm_set_path(OCSvhost_PATH)) < (char *) BZERO) {
		syslog(LOG_ALERT,
			MSGSTR(ERR_PATH,
			"%s: error setting odm default path: %d.\n"),
			prog_name, odmerrno);
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
					MSGSTR(ERR_TERM,
					"%s: error terminating odm: %d.\n"),
					prog_name, odmerrno);
			}
		}
		return((struct OCSconnection *) NULL); /* @L1C */
	}

	/* lock the database */
	for (i = retry_max; i >= 0; i--) {
	   lock_id = odm_lock(lock_file, timeout);
	   if (lock_id >= BZERO)
		break;
		if (i == BZERO) {      /* end of loop - no lock obtained */
			syslog(LOG_ALERT,
				MSGSTR(ERR_LOCK,
				"%s: error locking odm database: %d.\n"),
				prog_name, odmerrno);
		if (odm_path > BZERO)
			free(odm_path);
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
					MSGSTR(ERR_TERM,
					"%s: error terminating odm: %d.\n"),
					prog_name, odmerrno);
			}
		}
		return((struct OCSconnection *) NULL); /* @L1C */
		}
        }

	/* open the ocsvhost object class */
	rc = (int)odm_open_class(OCSvhost_CLASS);
	/* Assume no OCS Support:
	 * when OCSvhost object class does not exist: ODMI_CLASS_DNE
	 */
	if ((rc == -1) && (odmerrno == ODMI_CLASS_DNE)) { /* @L1C */
		if (odm_path > BZERO)
			free(odm_path);
		if ((odm_unlock(lock_id)) == -1) { /* @L1C */
			syslog(LOG_ALERT,
				MSGSTR(ERR_UNLOCK,
				"%s: error unlocking odm database: %d.\n"),
				prog_name, odmerrno);
			exit(1);
		}
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
					MSGSTR(ERR_TERM,
					"%s: error terminating odm: %d.\n"),
					prog_name, odmerrno);
			}
		}
		return((struct OCSconnection *) NULL); /* @L1C */
	} else if (rc == -1) { /* @L1C */
		syslog(LOG_ALERT,
		     MSGSTR(ERR_OPEN, "%s: error opening odm object %s: %d.\n"),
			     prog_name, "OCSvhost", odmerrno);
		if (odm_path > BZERO)
			free(odm_path);
		if ((odm_unlock(lock_id)) == -1) { /* @L1C */
			syslog(LOG_ALERT,
				MSGSTR(ERR_UNLOCK,
				"%s: error unlocking odm database: %d.\n"),
				prog_name, odmerrno);
			exit(1);
		}
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
					MSGSTR(ERR_TERM,
					"%s: error terminating odm: %d.\n"),
					prog_name, odmerrno);
			}
		}
		return((struct OCSconnection *) NULL); /* @L1C */
	}


	/*
	 * Search all objects in the OCSvhost object class with
	 * this internet address passed. (destination address in the
	 * telnet request)
         * Refer to the "OCS Virtual Host Support" menu through SMIT.
	 *
	 * Note that rc equals BZERO when all objects have been read and
	 * there is no matching internet addresses.  rc equals -1 when
	 * there is an error from the odm_get_first() call.
	 */
	sprintf(sstring,"vh_inetaddr = '%s'",inet_ntoa(local->sin_addr.s_addr));
	rc =(long)odm_get_first(OCSvhost_CLASS,sstring,&ocsvhost);
	if (rc > BZERO)
	    found_vhost = TRUE;
	else if (rc == -1) {   /* @L1C */
	    syslog(LOG_ALERT,
		MSGSTR(ERR_SEARCH,"%s: error searching odm object %s: %d.\n"),
		prog_name, "OCSvhost", odmerrno);
		if (odm_path > BZERO)
			free(odm_path);
		if ((odm_close_class(OCSvhost_CLASS)) == -1) { /* @L1C */
			syslog(LOG_ALERT,
				MSGSTR(ERR_CLOSE,
				"%s: error closing odm object %s: %d.\n"),
				prog_name, "OCSvhost", odmerrno);
		}
		if ((odm_unlock(lock_id)) == -1) { /* @L1C */
			syslog(LOG_ALERT,
				MSGSTR(ERR_UNLOCK,
				"%s: error unlocking odm database: %d.\n"),
				prog_name, odmerrno);
			exit(1);
		}
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
					MSGSTR(ERR_TERM,
					"%s: error terminating odm: %d.\n"),
					prog_name, odmerrno);
			}
		}
	    return((struct OCSconnection *) NULL); /* @L1C */
        }


	/* close OCSvhost object class and terminate odm access */
	if ((odm_close_class(OCSvhost_CLASS)) == -1) { /* @L1C */
		syslog(LOG_ALERT,
			MSGSTR(ERR_CLOSE,
			"%s: error closing odm object %s: %d.\n"), prog_name,
			"OCSvhost", odmerrno);
		if (odm_path > BZERO)
			free(odm_path);
		if ((odm_unlock(lock_id)) == -1) { /* @L1C */
			syslog(LOG_ALERT,
				MSGSTR(ERR_UNLOCK,
				"%s: error unlocking odm database: %d.\n"),
				prog_name, odmerrno);
			exit(1);
		}
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
				MSGSTR(ERR_TERM,
				"%s: error terminating odm: %d.\n"),
				prog_name, odmerrno);
			}
		}
		return((struct OCSconnection *) NULL); /* @L1C */
	}

        /*
         * If the OCSvhost entry is found that matches
         * the destination address in the telnet request,
         * then find the a host that's connected and its
         * associated OCS node.
         */
        if (found_vhost == TRUE){
            bcopy(ocsvhost.protoname, ocsconnection.protoname,
                        sizeof(ocsvhost.protoname));
            /*
             * Load array of possible host nodes to check
             * the OCS administration device to determine
             * that a connection has been established to
             * the host node.
             */
	    host_buf = (caddr_t)malloc(HOSTNMSIZE * MAXHOSTNODES);
            if (host_buf == NULL) {
                syslog(LOG_ERR,
                        MSGSTR(ERR_HARRAY, "%s: malloc error on Host array.\n"),
                        prog_name);
		if (odm_path > BZERO)
			free(odm_path);
		if ((odm_unlock(lock_id)) == -1) { /* @L1C */
			syslog(LOG_ALERT,
				MSGSTR(ERR_UNLOCK,
				"%s: error unlocking odm database: %d.\n"),
				prog_name, odmerrno);
			exit(1);
		}
		if ((odm_terminate()) == -1) { /* @L1C */
			if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
				syslog(LOG_ALERT,
				MSGSTR(ERR_TERM,
				"%s: error terminating odm: %d.\n"),
				prog_name, odmerrno);
			}
		}
                exit(1);
            } /* End of malloc() error check */

	    host_buf_save = host_buf;
            if (ocsvhost.hostname[0] != NULL){
	        bcopy(ocsvhost.hostname, host_buf,
			sizeof(ocsvhost.hostname));
	        host_buf += HOSTNMSIZE;
                max_host_cnt++;
            }
            if (ocsvhost.alt_hostname_one[0] != NULL){
	        bcopy(ocsvhost.alt_hostname_one, host_buf,
			sizeof(ocsvhost.alt_hostname_one));
	        host_buf += HOSTNMSIZE;
                max_host_cnt++;
            }
            if (ocsvhost.alt_hostname_two[0] != NULL){
	        bcopy(ocsvhost.alt_hostname_two, host_buf,
			sizeof(ocsvhost.alt_hostname_two));
                max_host_cnt++;
            }

	    host_buf = host_buf_save;
            hostnode_to_connect = find_host_to_connect(host_buf, max_host_cnt);
            free(host_buf);       /* free dynamically allocated space */
            if (hostnode_to_connect != NULL){
                bcopy(hostnode_to_connect, ocsconnection.hostname,
                        sizeof(ocsconnection.hostname));
                /*
                 * Now, open the Customized database to obtain the OCS
                 * Node Name associated with the Host Name, which has
                 * been determined to be connected.  Refer to the "OCS
                 * Host Name Support" menu through SMIT.
                 */
                ocsnode_to_bind = find_ocsnode_to_bind(hostnode_to_connect,
                                                       lock_id);
                if (ocsnode_to_bind != NULL){
                    bcopy(ocsnode_to_bind, ocsconnection.ocsnodename,
                        sizeof(ocsconnection.ocsnodename));
                } /* End of if (ocsnode_to_bind != NULL) */
	    } /* End of if (hostnode_to_connect != NULL) */
	} /* End of if (found_vhost == TRUE) */

	/* unlock the database */
	if ((odm_unlock(lock_id)) == -1) { /* @L1C */
		syslog(LOG_ALERT,
				MSGSTR(ERR_UNLOCK,
				"%s: error unlocking odm database: %d.\n"),
				prog_name, odmerrno);
		exit(1);
	}
	


        if (odm_path > BZERO)
                free(odm_path);
	/* terminate odm access with a graceful cleanup */
	if ((odm_terminate()) == -1) { /* @L1C */
		if (odmerrno != ODMI_UNLOCK){   /* ODMI_UNLOCK is OK */
			syslog(LOG_ALERT,
				MSGSTR(ERR_TERM,
				"%s: error terminating odm: %d.\n"),
				prog_name, odmerrno);
			return((struct OCSconnection *) NULL); /* @L1C */
			}
	}

        /*
         * Perform login locally on RISC System 6000
         */
        if (found_vhost == FALSE)
	    return((struct OCSconnection *) NULL);
        /*
         * found_vhost == TRUE but problems finding Host node
         * to connect to or OCS node to bind to
         */
	else if (ocsnode_to_bind == NULL)  {
            exit(1);
		}
        else
        /*
         * Perform login on AIX/ESA
         * We have valid Host and OCS node internet addresses
         */
	   {
    	    return(&ocsconnection);
            }

}  /* End of findvhost() */

/*
 * Closes any open object class and terminates ODM in
 * the event of an ODM error.
 */
int
odm_error_exit(odmexitcode, lock_id)
int odmexitcode;             /* ODM errno value */
int lock_id;		     /* ODM lock identifier */
{
	/* Close open object classes */
	odm_close_class(OCSvhost_CLASS);
	odm_close_class(CuAt_CLASS);

	/* unlock the database */
	odm_unlock(lock_id);

	/* Terminate the ODM */
	odm_terminate();

	exit(odmexitcode);

} /* End of odm_error_exit() */

/*
 * Virtual Host processing
 */
int
process_vhost(OCScon_ptr, line, lm_vtp_var_ptr)
struct OCSconnection *OCScon_ptr;
char *line;    				/* terminal name - pty slave */
LM_VTP_VAR *lm_vtp_var_ptr;    	/* vtp specific variables for LM */
{
	int rc;
	int socket_type;               /* socket type for lm socket() call */
	int proto_type;                /* protocol type for lm socket() call */
	int lm_serv_addr_len;          /* length of lm address structure */
	struct hostent *h_ptr;					
	struct servent *s_ptr;	       /* LM service structure */				 	
	union _lm_serv_addr {
		struct sockaddr_in tcp;     /* internet socket address desc */
					    /* OCS domain socket address desc */
	} lm_serv_addr;
	

    	/*
	 * Populate address of server we want to connect to;
	 * that is the Login Monitor (lm)
	 */
	lm_serv_addr_len = sizeof lm_serv_addr;
	bzero((char *) &lm_serv_addr, lm_serv_addr_len);

	/*
	 * If the protocol is tcp, then invoke gethostbyname()
	 * to obtain the internet address of the host machine,
	 * where the Login Monitor resides.
	 *
	 * If the protocol is "odtp", then invoke xxx().
	 */
	if (((strcmp(OCScon_ptr->protoname, "tcp")) == BZERO) ||
	    ((strcmp(OCScon_ptr->protoname, "TCP")) == BZERO)) {
		s_ptr = getservbyname("lm", OCScon_ptr->protoname);
		if (s_ptr == NULL){
	   		syslog(LOG_ERR,
			MSGSTR(ERR_GETSERV, "%s: getservbyname() error: %d.\n"),
				prog_name, errno);
			exit(1);
			}
		lm_serv_addr.tcp.sin_port = s_ptr->s_port;
		h_ptr = gethostbyname(OCScon_ptr->hostname);
		if (h_ptr == NULL) {
	   		syslog(LOG_ERR,
			MSGSTR(ERR_GETHOST, "%s: gethostbyname() error: %d.\n"),
				prog_name, h_errno);
			exit(1);
		}
		/* if 4.3 BSD, then sin_len does not exist */
	    	lm_serv_addr.tcp.sin_len         = sizeof(struct sockaddr_in);
		lm_serv_addr.tcp.sin_family      = h_ptr->h_addrtype;
		lm_serv_addr.tcp.sin_addr.s_addr = *((u_long *)h_ptr->h_addr);
	    	lm_serv_addr_len = sizeof(struct sockaddr_in);
		socket_type = SOCK_STREAM;
		proto_type = BZERO;
	}  /* End of tcp protocol check */
	/*else  if (((strcmp(OCScon_ptr->protoname, "odtp")) == BZERO) ||
	         ((strcmp(OCScon_ptr->protoname, "ODTP")) == BZERO)){  */
		/* make odtp service call */
		/* set lm_serv_addr here */
		/* } End of odtp protocol */
	else {  /* default */
	       syslog(LOG_ERR,
		MSGSTR(ERR_INVPROTO, "%s: invalid protocol name: %s.\n"),
			prog_name, OCScon_ptr->protoname);
		exit(1);
	}  /* End of invalid protocol type */

	/*
	 * Talk to the Login Monitor on the host machine
	 *
	 * If communication with the Login Monitor succeeds:
	 *   - RTY Server starts login process on terminal line
	 *   - telnetd can now do a select on the pty master as
	 *     usual.
	 */
	talk_to_lm(&lm_serv_addr, lm_serv_addr_len, OCScon_ptr->ocsnodename,
                    socket_type, proto_type, line, lm_vtp_var_ptr);
}

/*
 * Connection-oriented stream delivery service
 * to the Login Monitor (lm) on the virtual
 * host machine
 *
 * If an error occurs, the process exits with the errno value.
 * Otherwise, returns BZERO for success.
 */
int
talk_to_lm(lm_serv_addr, lm_serv_addr_len, ocsnode_name, socket_type, proto_type, line, lm_vtp_var_ptr)
struct sockaddr *lm_serv_addr;  /* LM server socket address */
int lm_serv_addr_len;  /* LM server socket address length */
char *ocsnode_name;   /* OCS node name to bind to */
int socket_type;    /* socket type used for lm communication */
int proto_type;     /* protocol type used for lm communication */
char *line;			/* terminal name - pty slave */
LM_VTP_VAR *lm_vtp_var_ptr;    	/* vtp specific variables for LM */
{
	int rc = BZERO;
	int nbytes = BZERO;  /* Nbr of bytes actually read */
	int nbytes_left;     /* Nbr of bytes left to read */
	int flags = BZERO;
	int  lm_client_sockfd;
	struct lm_message_hdr lm_msg_hdr;
	struct lm_response lm_resp;
	caddr_t lm_message;            /* entire message content for LM */
	int vtp_var_size = BZERO;
	uchar_t *lm_resp_ptr;
	struct hostent *ocsnode_ptr;
	struct sockaddr_in sin_ocsnode;

	
	lm_resp_ptr = (char *)&lm_resp;

   	/*
	 * Open a socket
	 */
	if ((lm_client_sockfd = socket(lm_serv_addr->sa_family,
					socket_type, proto_type)) < BZERO) {	
		syslog(LOG_ERR,
			MSGSTR(ERR_LMSOCK,
			"%s: error opening socket to LM: %d.\n"),
			prog_name, errno);
	   exit(errno);
	 } /* End create socket check */	

	/*
	 * Must bind the socket to the specific
	 * inet address of the OCS node, which
         * is basically the network interface address.
	 * This address is specified on the AIX/ESA
	 * machine at configuration time and
	 * validated.
	 */
	ocsnode_ptr = gethostbyname(ocsnode_name);
	if (ocsnode_ptr == NULL) {
            syslog(LOG_ERR,
                   MSGSTR(ERR_GETHOST, "%s: gethostbyname() error: %d.\n"),
                                prog_name, h_errno);
            exit(1);
        }

	bzero(&sin_ocsnode, sizeof (struct sockaddr_in));
	sin_ocsnode.sin_port        = BZERO;
	sin_ocsnode.sin_len         = sizeof(struct sockaddr_in);
	sin_ocsnode.sin_family      = ocsnode_ptr->h_addrtype;
	sin_ocsnode.sin_addr.s_addr = *((u_long *)ocsnode_ptr->h_addr);

	if ((bind(lm_client_sockfd, &sin_ocsnode, sizeof(struct sockaddr)))
								     < BZERO) {
		syslog(LOG_ERR,
			MSGSTR(ERR_BINDLA,
			"%s: error binding local address: %d.\n"),
			prog_name, errno);
		exit(errno);
	} /* End bind of local address */

	/*
	 * Establish connection to server.
	 */
	if ((connect(lm_client_sockfd, lm_serv_addr, lm_serv_addr_len))
								    < BZERO) {
		syslog(LOG_ERR,
			MSGSTR(ERR_CONNLM,
			"%s: error connecting socket to LM: %d.\n"),
			prog_name, errno);
		exit(errno);
	} /* End connect to server check */

	/*
	 * Populate the Login Monitor message header
	 */
	bzero((char *) &lm_msg_hdr, sizeof(lm_msg_hdr));
	lm_msg_hdr.lm_msgtype = TN_OPEN_TERMINAL;
	strncpy(lm_msg_hdr.lm_devname, line, sizeof(lm_msg_hdr.lm_devname));

	/*
	 * Determine the length for the variable-length
	 * Login Monitor message based on the virtual
	 * terminal protocol type
	 */
	switch(lm_vtp_var_ptr->vtp_type) {
		case(TELNETD_VTP):
			lm_msg_hdr.lm_size = (sizeof(lm_msg_hdr)
					+ sizeof(TELNETD_VAR)
					+ sizeof(lm_vtp_var_ptr->vtp_type));
			vtp_var_size = (sizeof(TELNETD_VAR)
					+ sizeof(lm_vtp_var_ptr->vtp_type));
			break;
		case(RLOGIND_VTP):
			lm_msg_hdr.lm_size = (sizeof(lm_msg_hdr)
					+ sizeof(RLOGIND_VAR)
					+ sizeof(lm_vtp_var_ptr->vtp_type));
			vtp_var_size = (sizeof(RLOGIND_VAR)	
					+ sizeof(lm_vtp_var_ptr->vtp_type));
			break;
		default:
	   		syslog(LOG_ERR,
				MSGSTR(ERR_VTPTYPE,
				"%s: protocol type error: %d.\n"),
				prog_name, lm_vtp_var_ptr->vtp_type);
			exit(1);
	} /* End of switch */

	/*
	 * Dynamically allocate space for the variable-length
	 * LM message.
	 */
	lm_message = (caddr_t)malloc(lm_msg_hdr.lm_size);
	if (lm_message == NULL) {
		syslog(LOG_ERR,
			MSGSTR(ERR_MALLOC, "%s: malloc error on LM message.\n"),
			prog_name);
		exit(1);
	} /* End of malloc() error check */

	/*
	 * Build the entire Login Monitor message for
	 * transmission
	 */
	bcopy(&lm_msg_hdr, lm_message, sizeof(lm_msg_hdr));
	bcopy(lm_vtp_var_ptr, lm_message + sizeof(lm_msg_hdr), vtp_var_size);

	/*
	 * Send Login Monitor a request to open the
	 * correct ESA device
	 */
	if (send(lm_client_sockfd, lm_message, lm_msg_hdr.lm_size, flags)
								    < BZERO) {
	   syslog(LOG_ERR,
		MSGSTR(ERR_SENDLM, "%s: error transmitting msg to LM: %d.\n"),
				prog_name, errno);
		close(lm_client_sockfd);
	        free(lm_message);	
		exit(errno);
	}

	/*
	 * Wait for acknowledgement that the AIX/ESA
	 * device (slave side of the pty) was successfully
	 * opened by the Login Monitor/RTY Server
	 */
	nbytes_left = sizeof(lm_resp);
	while((nbytes = recv(lm_client_sockfd, lm_resp_ptr, nbytes_left,flags))
							     != nbytes_left) {
		if (nbytes <= BZERO) {
			syslog(LOG_ERR,
				MSGSTR(ERR_RCVLM,
				"%s: error receiving msg from LM: %d.\n"),
				prog_name, errno);
		   close(lm_client_sockfd);
	           free(lm_message);	
		   exit(errno);
		}  /* End of recv() error */
		lm_resp_ptr += nbytes;
		nbytes_left -= nbytes;
	}   /* End of while recv() loop */

	if (lm_resp.status != LM_OPEN_SUCCESS) {
	   syslog(LOG_ERR,
		MSGSTR(ERR_STATLM,
				"%s: status error from LM: %d.\n"),
				prog_name, lm_resp.status);
		close(lm_client_sockfd);
	        free(lm_message);	
		exit(errno);
	}

	close(lm_client_sockfd);
	free(lm_message);	/* free dynamically allocated space */

} /* End of talk_to_lm() */
