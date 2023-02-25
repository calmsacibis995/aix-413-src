static char sccsid[] = "@(#)57	1.1  src/bos/usr/sbin/pse/utils/sthd/fdint.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:29";
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
 ** fdint.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

#ifndef errno
extern	int	errno;
#endif

static	boolean	fdinsert_check(   int fd, struct strfdinsert * strfdp, int ret_val, int expected_errno   );

void
fdinsertt (state)
	int	state;
{
	char	buf[256];
	int	fd;
	int	expected_errno;
	int	i1;
	int	ret_val;
	struct strfdinsert	strfd;

	printf("Testing fdinsert\n");
	/* TODO: fix error, hangup and ondelay testing */

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	nread_check(fd, 0, 0, 0);
	ret_val = (state & (ERROR | HANGUP)) ? -1 : 0;

#ifndef NO_BOGUS_ADDRESSES
	if ( (state & (ERROR | HANGUP)) == 0 ) {
		printf("testing fdinsert with bogus address\n");
		if (ret_val == 0)
			expected_errno = EFAULT;
		fdinsert_check(fd, BOGUS_ADDRESS, -1, expected_errno);
	}
#endif

	printf("testing fdinsert with bogus flags\n");
	if (ret_val == 0)
		expected_errno = EINVAL;
	strfd.ctlbuf.len = sizeof(caddr_t);
	strfd.ctlbuf.buf = buf;
	strfd.databuf.len = 0;
	strfd.flags = -1;
	strfd.fildes = fd;
	strfd.offset = 0;
	fdinsert_check(fd, &strfd, -1, expected_errno);

	printf("testing fdinsert with short data\n");
	strfd.ctlbuf.len = 0;
	strfd.flags = 0;
	fdinsert_check(fd, &strfd, -1, expected_errno);

	printf("testing fdinsert with non-aligned offset\n");
	strfd.ctlbuf.len = sizeof(caddr_t);
	strfd.offset = 1;
	fdinsert_check(fd, &strfd, -1, expected_errno);

	if ( (state & (ERROR | HANGUP)) == 0 ) {
		printf("testing fdinsert with bogus fd\n");
		strfd.offset = 0;
		strfd.fildes = -1;
		fdinsert_check(fd, &strfd, -1, expected_errno);
	}

	printf("testing valid fdinsert with data\n");
	strfd.fildes = fd;
	i1 = 0;
	strfd.ctlbuf.buf = (char *)&i1;
	strfd.ctlbuf.len = sizeof(i1);
	strfd.databuf.buf = buf;
	strfd.databuf.len = sizeof(buf);
	if (fdinsert_check(fd, &strfd, ret_val, expected_errno)) {
		if (ret_val == 0) {
			strfd.ctlbuf.maxlen = sizeof(i1);
			strfd.databuf.maxlen = sizeof(buf);
			if (getmsg(fd, &strfd.ctlbuf, &strfd.databuf, &ret_val) == 0)
				printf("retrieved fdinsert message, insertted q == 0x%x\n", i1);
			else
				printe(true, "getmsg failed");
		}
	}
	stream_close(fd);
}

static boolean
fdinsert_check (fd, strfdp, ret_val, expected_errno)
	int	fd;
	struct strfdinsert	* strfdp;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_FDINSERT, (char *)strfdp)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "fdinsert failed");
		if (errno != expected_errno)
			return printe(true, "fdinsert failed incorrectly, expecting errno %d", expected_errno);
		return printok("fdinsert failed as expected with errno %d", errno);
	case 0:
		if (ret_val != 0)
			return printe(false, "fdinsert succeeded when expecting %d", ret_val);
		return printok("fdinsert succeeded");
	default:
		if (ret_val != i1)
			return printe(false, "fdinsert returned %d when expecting %d", i1, ret_val);
		return printok("fdinsert return %d as expected", i1);
	}
}
