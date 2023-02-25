static char sccsid[] = "@(#)70	1.1  src/bos/usr/sbin/pse/utils/sthd/pusht.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:54";
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
 ** pusht.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

#define	ERR_STATES	(ERROR | HANGUP)

#ifndef errno
extern	int	errno;
#endif

static	boolean	push_check(   int fd, char * module_name, int ret_val, int expected_errno   );

void
pusht (state)
	int	state;
{
	char	buf[128];
	int	fd;
	int	i1;

	printf("Testing push ioctl\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (state & ERROR) {
		printf("testing push in error state\n");
		if (!set_error_state(fd, EIO))
			return;
		push_check(fd, pass_name, -1, EIO);
		stream_close(fd);
		return;
	}

	if (state & HANGUP) {
		printf("testing push in hangup state\n");
		if (!set_hangup_state(fd))
			return;
		push_check(fd, pass_name, -1, EINVAL);
		stream_close(fd);
		return;
	}

	printf("testing valid push of '%s'\n", pass_name);
	push_check(fd, pass_name, 0, 0);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing push with bogus address\n");
	push_check(fd, BOGUS_ADDRESS, -1, EFAULT);
#endif

	printf("testing push of bogus module name\n");
	push_check(fd, "foo", -1, EINVAL);

	printf("testing push of module which fails in open\n");
	push_check(fd, errm_name, -1, ENXIO);

	printf("writing and reading data to make sure the stream is still ok\n");
	if ((i1 = stream_write(fd, buf, sizeof(buf))) == sizeof(buf)) {
		printf("write ok\n");
		if ((i1 = stream_read(fd, buf, sizeof(buf))) == sizeof(buf))
			printf("read ok\n");
		else
			printe(false, "read returned %d when expecting %d", i1, sizeof(buf));
	} else
		printe(false, "write returned %d when expecting %d", i1, sizeof(buf));

	stream_close(fd);
}

static boolean
push_check (fd, module_name, ret_val, expected_errno)
	int	fd;
	char	* module_name;
	int	ret_val, expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_PUSH, module_name)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "push failed");
		if (errno != expected_errno)
			return printe(true, "push failed incorrectly, expected errno %d", expected_errno);
		return printok("push failed as expected with errno %d", errno);
	case 0:
		if (ret_val != 0)
			return printe(false, "push returned 0 when expecting %d", ret_val);
		return printok("push succeeded");
	default:
		if (ret_val != i1)
			return printe(false, "push returned %d when expecting %d", i1, ret_val);
		return printok("push returned %d as expected", i1);
	}
}
