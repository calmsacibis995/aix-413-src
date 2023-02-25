static char sccsid[] = "@(#)14  1.1  src/rspc/usr/ccs/lib/libpm/pm_sock.c, pwrcmd, rspc41J, 9510A 3/8/95 07:41:10";
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: pmlib_create_sock, pmlib_close_sock, pmlib_recv_reply
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pmlib.h>
#include "pmlib_internal.h"
#include "pmlibdebug.h"


/* prototype */
static void pmlib_close_sock(int, struct sockaddr_un *);
static int  recvfrom2(int, char *, int, int, struct sockaddr *, int *);
extern char *mktemp(char *);


extern int errno;


/*
 * NAME:  pmlib_create_sock
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_recv_reply.
 *      Create and return the descriptor of unix domain connectionless socket.
 *
 * DATA STRUCTURES:
 *      sockaddr_un  - structure for unix domain socket
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Unsuccessfully processed
 */
static int
pmlib_create_sock(struct sockaddr_un *cli_addr, struct sockaddr_un *serv_addr)
{
	int	sockfd;
	int	clilen;

	/*
	 * Fill in the structure "serv_addr" with the address of the
	 * server that we want to send to.
	 */
	memset((char *)serv_addr, 0, sizeof(serv_addr));
	serv_addr->sun_family = AF_UNIX;
	strncpy(serv_addr->sun_path, PMLIB_DAEMON_PATH,
					sizeof(serv_addr->sun_path));

	/*
	 * Open a socket (a UNIX domain datagram socket).
	 */
	if((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0){
		DEBUG_0("pmlib_create_sock: can't open datagram socket\n");
		return PMLIB_ERROR;
	}

	/*
	 * Bind a local address for us.
	 * In the UNIX domain we have to choose our own name (that
	 * should be unique).  We'll use mktemp() to create a unique
	 * pathname, based on our process id.
	 */
	memset((char *)cli_addr, 0, sizeof(struct sockaddr_un));
	cli_addr->sun_family = AF_UNIX;
	strcpy(cli_addr->sun_path, PMLIB_CLIENT_PATH);
	mktemp(cli_addr->sun_path);
	clilen = SUN_LEN(cli_addr);

	if(bind(sockfd, (struct sockaddr *)cli_addr, clilen) < 0){
		DEBUG_0("pmlib_create_sock: can't bind local address\n");
		pmlib_close_sock(sockfd, cli_addr);
		return PMLIB_ERROR;
	}

	return sockfd;
}


/*
 * NAME:  pmlib_close_sock
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_recv_reply.
 *      Close and unlink the descriptor of unix domain socket.
 *
 * DATA STRUCTURES:
 *      sockaddr_un  - structure for unix domain socket
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
static void
pmlib_close_sock(int sockfd, struct sockaddr_un *cli_addr)
{
	close(sockfd);
	if(unlink(cli_addr->sun_path) != 0){
		DEBUG_1("pmlib_close_sock:unlink() failed, errno is %d\n",
					errno);
	}
}


/*
 * NAME:  pmlib_recv_reply
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is pmlib_recv_reply function.
 *      send data for IPC to pm daemon and recv reply.
 *
 * DATA STRUCTURES:
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *      sockaddr_un  - structure for unix domain socket
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Unsuccessfully processed
 */
int
pmlib_recv_reply(struct pmlib_client *clidata, struct pmlib_server *servdata,
			int length)
{
	int			rc;
	int			n;
	int			servlen;
	int			sockfd;
	struct sockaddr_un	cli_addr;
	struct sockaddr_un	serv_addr;
	int			actual_length;

	DEBUG_0("pmlib_recv_reply: called\n");

	if((sockfd = pmlib_create_sock(&cli_addr, &serv_addr)) == PMLIB_ERROR){
		return PMLIB_ERROR;
	}
	/* Socket initialization completed */

	clidata->version = PMLIB_VERSION;
	clidata->cli_uid = getuid();
	clidata->cli_euid = geteuid();
	clidata->cli_pid = getpid();
	servlen = SUN_LEN(&serv_addr);
	n = sizeof(struct pmlib_client);

	if((rc = sendto(sockfd, (char *)clidata, n, 0,
		    	  (struct sockaddr *)&serv_addr, servlen)) != n){
		DEBUG_0("pmlib_recv_reply: sendto error on socket\n");
		DEBUG_1("pmlib_recv_reply: rc = %d\n", rc);
		pmlib_close_sock(sockfd, &cli_addr);
		return PMLIB_ERROR;
	}

	/*
	 * Get data length
	 */
	n = recvfrom2(sockfd, (char *)servdata, sizeof(struct pmlib_server),
					MSG_PEEK, NULL, NULL);
	actual_length = servdata->length;
	if(actual_length < sizeof(struct pmlib_server)){
		DEBUG_1("pmlib : data format error    actual length = %d\n",
								actual_length);
		pmlib_close_sock(sockfd, &cli_addr);
		return PMLIB_ERROR;
	}
	if(actual_length > length){
		DEBUG_1("pmlib : data too long    actual length = %d\n",
								actual_length);
		pmlib_close_sock(sockfd, &cli_addr);
		return PMLIB_ERROR;
	}

	/*
	 * Now read a reply from the socket.
	 */
        n = recvfrom2(sockfd, (char *)servdata, actual_length, 0, NULL, NULL);

	pmlib_close_sock(sockfd, &cli_addr);

	if(n < servdata->length){
		DEBUG_0("pmlib_recv_reply: recvfrom2 error\n");
		DEBUG_2("pmlib_recv_reply: n = %d servdata->length = d\n",
							n, servdata->length);
		return PMLIB_ERROR;
	}

	if((servdata->version != PMLIB_VERSION) ||
			(servdata->ftype != clidata->ftype) ||
			(servdata->ctrl != clidata->ctrl)){
		DEBUG_1("pmlib_recv_reply: version = %d\n", servdata->version);
		DEBUG_1("pmlib_recv_reply: ftype   = %d\n", servdata->ftype);
		DEBUG_1("pmlib_recv_reply: ctrl    = %d\n", servdata->ctrl);
		return PMLIB_ERROR;
	} 

	if(servdata->rc != PMLIB_SUCCESS){
		if(servdata->rc != PMLIB_ERROR){
			errno = servdata->rc;
		}
		DEBUG_0("pmlib_recv_reply: Error Returned\n");
		return PMLIB_ERROR;
	}

	if(servdata->length != n){
		DEBUG_2("pmlib_recv_reply: servdata->length = %d n = %d\n",
							servdata->length, n);
		return PMLIB_ERROR;
	}

	return PMLIB_SUCCESS;
}


/*
 * NAME:  recvfrom2
 *
 * FUNCTION:  Do recvfrom(). Retry if necessary.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      sockaddr_un  - structure for unix domain socket
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
static int
recvfrom2(int sockfd, char *buf, int len, int flags,
			struct sockaddr *sock, int *fromlen)
{
	int	n;

	DEBUG_0("recvfrom2\n");

	n = recvfrom(sockfd, buf, len, flags, sock, fromlen);
	if(n < 0){
		DEBUG_0("pmlib_recv_reply: recvfrom error\n");
		if(errno == EINTR){
			DEBUG_0("pmlib_recv_reply: recvfrom retry\n");
			n = recvfrom(sockfd, buf, len, flags, sock, fromlen);
		}
	}

	return n;
}
