static char sccsid[] = "@(#)92	1.1  src/tcpip/usr/lbin/telnetd/release_license.c, tcptelnet, tcpip411, GOLD410 4/5/94 17:52:16";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _FloatingReleaseLicense
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <unistd.h>
#include <sys/file.h>
#include <sys/license.h>


/*
 * NAME:     _FloatingReleaseLicense
 *
 * FUNCTION: Releases a floating license.
 *
 * RETURNS:  Nothing.
 */
void
_FloatingReleaseLicense(pid_t pid)
{
	struct	monitord_request client_request;
	int	fd;

	if ((fd = open(MONITORD_PIPE, O_RDWR|O_NONBLOCK, 0600)) >= 0)
	{
		client_request.login_pid    = pid;
		client_request.request_type = RELEASE_LIC;

		write(fd, &client_request, sizeof(client_request));
		close(fd);
	}
}
