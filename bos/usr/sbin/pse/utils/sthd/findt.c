static char sccsid[] = "@(#)58	1.1  src/bos/usr/sbin/pse/utils/sthd/findt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:31";
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
 ** findt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

#ifndef errno
extern	int	errno;
#endif

static	boolean	find_check(   int fd, char * module_name, int ret_val, int expected_errno   );

void
findt (state)
	int	state;
{
	int	fd;
	char	* module_name;

	printf("Testing I_FIND ioctl\n");

	if ((fd = stream_open(nuls_name, 2)) == -1) {
		printe(true, "open of %s failed", nuls_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, pass_name) == -1) {
		printe(true, "push of %s failed", pass_name);
		return;
	}

	if (state & ERROR) {
		if (set_error_state(fd, EIO)) {
			printf("testing find on '%s' in error state", pass_name);
			find_check(fd, pass_name, -1, EIO);
		}
		stream_close(fd);
		return;
	}

	printf("testing find on '%s'\n", pass_name);
	find_check(fd, pass_name, 1, 0);

#ifndef NO_BOGUS_ADDRESSES	
	printf("testing find with bogus address for name\n");
	find_check(fd, BOGUS_ADDRESS, -1, EFAULT);
#endif

	printf("testing find with name that's too long\n");
	find_check(fd, "abcdefghijklmnopqrstuvwxyzabcdefghi", -1, EINVAL);

	module_name = "sc";
	printf("testing find with name of non-pushed module (%s)\n", module_name);
	find_check(fd, module_name, 0, 0);

	printf("testing find with device name\n");
	find_check(fd, nuls_name, -1, EINVAL);

	stream_close(fd);
}

static boolean
find_check (fd, module_name, ret_val, expected_errno)
	int	fd;
	char	* module_name;
	int	ret_val, expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_FIND, module_name)) {
	case 1:
		if (ret_val == 1)
			return printok("find succeeded");
		return printe(false, "find succeeded when expecting %d", ret_val);
	case 0:
		if (ret_val == 0)
			return printok("find returned 0 as expected");
		return printe(false, "find returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == expected_errno)
				return printok("find failed as expected with errno %d", errno);
			return printe(true, "find failed incorrectly, expected errno %d", expected_errno);
		}
		return printe(true, "find failed");
	default:
		return printe(false, "find returned %d when expecting %d", i1, ret_val);
	}
}
