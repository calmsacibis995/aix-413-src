static char sccsid[] = "@(#)74	1.1  src/bos/usr/sbin/pse/utils/sthd/setoptst.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:59";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** setoptst.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include "sthd.h"
#include <sys/stream.h>
#include <pse/echo.h>

extern	int	errno;

void
setoptst (state)
	int	state;
{
	char	buf[256];
	int	fd;
	int	i1;
	struct stroptions * strop;
	struct strioctl	stri;
	eblk_t	* ep;

	if (state & (ERROR | HANGUP)) {
		printf("Can not test setopts in ERROR or HANGUP mode");
		return;
	}

	printf("Testing M_SETOPTS message\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s faile", echo_name);
		return;
	}

	printf("requesting M_SETOPTS message to change read options\n");
	stri.ic_cmd = ECHO_GENMSG;
	stri.ic_len = sizeof(eblk_t) + sizeof(struct stroptions) - 2;
	stri.ic_timout = -1;
	stri.ic_dp = buf;
	ep = (eblk_t *)buf;
	ep->eb_type = M_SETOPTS;
	ep->eb_flag = 0;
	ep->eb_len = sizeof(struct stroptions);
	strop = (struct stroptions *)&ep[1];
	strop->so_flags = SO_READOPT;
	strop->so_readopt = RMSGN;
	if (stream_ioctl(fd, I_STR, &stri) == -1)
		printe(true, "I_STR ioctl failed");

	printf("checking on read option\n");
	if (stream_ioctl(fd, I_GRDOPT, &i1) != -1) {
		if (i1 == (RMSGN | RPROTNORM))
			printf("read option change verified\n");
		else
			printe(false, "get read opt returned %d when expecting %d", i1, RMSGN);
	} else
		printe(true, "get read opt failed");

	stream_close(fd);
}
