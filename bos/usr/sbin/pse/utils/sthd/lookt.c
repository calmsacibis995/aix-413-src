static char sccsid[] = "@(#)64	1.1  src/bos/usr/sbin/pse/utils/sthd/lookt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:43";
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
 ** lookt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

extern	int	errno;

static	boolean	look_check(   int fd, char * module_name, char * ret_buf, int ret_val, int errno_val   );

void
lookt (state)
	int	state;
{
	char	buf[128];
	int	fd;

	printf("Testing look ioctl\n");
	if ((fd = stream_open(nuls_name, 2)) == -1) {
		printe(true, "open of %s failed", nuls_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, pass_name) == -1) {
		printe(true, "push of %s failed", pass_name);
		stream_close(fd);
		return;
	}

	if (state & ERROR) {
		printf("testing look in error state\n");
		if (set_error_state(fd, EIO))
			look_check(fd, pass_name, buf, -1, EIO);
		stream_close(fd);
		return;
	}

	printf("testing valid look\n");
	look_check(fd, pass_name, buf, 0, 0);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing look with bogus address\n");
	look_check(fd, pass_name, BOGUS_ADDRESS, -1, EFAULT);
#endif

	stream_close(fd);
}

static boolean
look_check (fd, module_name, ret_buf, ret_val, errno_val)
	int	fd;
	char	* module_name;
	char	* ret_buf;
	int	ret_val, errno_val;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_LOOK, ret_buf)) {
	case 0:
		if (ret_val == 0) {
			if (strcmp(ret_buf, module_name) == 0)
				return printok("look returned the correct module_name");
			return printe(false, "look returned incorrect module_name '%s'", ret_buf);
		}
		return printe(false, "look returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == errno_val)
				return printok("look failed as expected with errno %d", errno);
			return printe(true, "look failed incorrectly, expected errno %d", errno_val);
		}
		return printe(true, "look failed");
	default:
		if (ret_val == i1)
			return printok("look returned %d as expected");
		return printe(false, "look returned %d when expecting %d", i1, ret_val);
	}
}
