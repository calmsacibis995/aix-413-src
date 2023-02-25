static char sccsid[] = "@(#)65	1.1  src/bos/usr/sbin/pse/utils/sthd/nreadt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:45";
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
 ** nreadt.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include "sthd.h"

#ifndef errno
extern	int	errno;
#endif

void
nreadt (state)
	int	state;
{
	char	buf[256];
	int	fd;

	printf("Testing nread ioctl\n");
	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (state & ERROR) {
		printf("testing nread in error state\n");
		if (set_error_state(fd, EIO))
			nread_check(fd, -1, 0, EIO);
		stream_close(fd);
		return;
	}

	if (stream_write(fd, buf, sizeof(buf) / 2) != (sizeof(buf) / 2)) {
		printe(true, "write failed");
		stream_close(fd);
		return;
	}
	printf("wrote %d bytes\n", sizeof(buf) / 2);
	nread_check(fd, 1, sizeof(buf) / 2, 0);

	if (stream_write(fd, buf, sizeof(buf)) == sizeof(buf)) {
		printf("wrote %d bytes\n", sizeof(buf));
		nread_check(fd, 2, sizeof(buf) / 2, 0);
	} else
		printe(true, "write failed");

	if (state & HANGUP)
		set_hangup_state(fd);
	else if (state & ONDELAY)
		set_ondelay_mode(fd);

	if (stream_read(fd, buf, sizeof(buf) / 2) != (sizeof(buf) / 2)) {
		printe(true, "read failed");
		return;
	}
	printf("read %d bytes\n", sizeof(buf));
	nread_check(fd, 1, sizeof(buf), 0);

	if (stream_read(fd, buf, sizeof(buf) / 2) != (sizeof(buf) / 2)) {
		printe(true, "read failed");
		return;
	}
	printf("read %d bytes\n", sizeof(buf) / 2);
	nread_check(fd, 1, sizeof(buf) / 2, 0);

	if (stream_read(fd, buf, sizeof(buf) / 2) != (sizeof(buf) / 2)) {
		printe(true, "read failed");
		return;
	}
	printf("read %d bytes\n", sizeof(buf) / 2);
	nread_check(fd, 0, 0, 0);

	stream_close(fd);
}

boolean
nread_check (fd, expected_qcnt, expected_qbytes, expected_errno)
	int	fd, expected_qcnt, expected_qbytes;
	int	expected_errno;
{
	int	qcnt, qbytes;

	if ((qcnt = stream_ioctl(fd, I_NREAD, &qbytes)) == -1) {
		if (expected_qcnt != -1)
			return printe(true, "nread failed");
		if (errno != expected_errno)
			return printe(true, "nread failed incorrectly, expected errno %d", expected_errno);
		return printok("nread failed as expected with errno %d", errno);
	}
	if (qcnt == expected_qcnt  &&  qbytes == expected_qbytes)
		return printok("nread returned count %d and %d bytes", qcnt, qbytes);
	printe(false, "nread returned count %d and %d bytes, expected count %d, %d bytes"
	, qcnt, qbytes, expected_qcnt, expected_qbytes);
	return false;
}
