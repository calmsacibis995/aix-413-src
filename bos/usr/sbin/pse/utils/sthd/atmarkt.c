static char sccsid[] = "@(#)52	1.1  src/bos/usr/sbin/pse/utils/sthd/atmarkt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:17";
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
 ** atmarkt.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include "sthd.h"
#include <sys/stream.h>
#include <pse/echo.h>

#ifndef errno
extern	int	errno;
#endif

staticf	boolean	atmark_check(   int fd, int arg, int ret_val, int expected_errno   );
staticf	boolean	send_marked_msg(   int fd   );

#define	BUFFER_SIZE	128
static	char	buf[256];

void
atmarkt (state)
	int	state;
{
	int	fd;
	int	i1;

	printf("Testing I_ATMARK ioctl\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (state & ERROR) {
		if (set_error_state(fd, EIO)) {
			printf("testing atmark on '%s' in error state", pass_name);
			atmark_check(fd, ANYMARK, -1, EIO);
		}
		stream_close(fd);
		return;
	}

	printf("setting read mode to RMSGN\n");
	if (stream_ioctl(fd, I_SRDOPT, RMSGN) == -1) {
		printe(true, "stream_ioctl failed");
		stream_close(fd);
		return;
	}

	printf("testing atmark with invalid request\n");
	atmark_check(fd, -1, -1, EINVAL);

	printf("writing a marked message\n");
	if (!send_marked_msg(fd)) {
		stream_close(fd); 
		return;
	}

	printf("writing a regular message\n");
	if ((i1 = stream_write(fd, buf, BUFFER_SIZE)) != BUFFER_SIZE) {
		printe(true, "write returned %d when expecting %d", i1, BUFFER_SIZE);
		stream_close(fd); 
		return;
	}
	printf("write successful\n");

	printf("writing a second marked message\n");
	if (!send_marked_msg(fd)) {
		stream_close(fd); 
		return;
	}

	printf("writing a second regular message\n");
	if ((i1 = stream_write(fd, buf, BUFFER_SIZE)) != BUFFER_SIZE) {
		printe(true, "write returned %d when expecting %d", i1, BUFFER_SIZE);
		stream_close(fd); 
		return;
	}
	printf("write successful\n");

	printf("testing ANYMARK atmark with multiple marked messages on the queue\n");
	atmark_check(fd, ANYMARK, 1, 0);

	printf("testing LASTMARK atmark with multiple marked messages on the queue\n");
	atmark_check(fd, LASTMARK, 0, 0);

	printf("reading first marked message\n");
	if (stream_read(fd, buf, BUFFER_SIZE) == -1) {
		printe(true, "read failed");
		stream_close(fd);
		return;
	}

	printf("testing ANYMARK atmark with one marked message in the middle of the queue\n");
	atmark_check(fd, ANYMARK, 0, 0);

	printf("testing LASTMARK atmark with one marked message in the middle of the queue\n");
	atmark_check(fd, LASTMARK, 0, 0);

	printf("reading first regular message\n");
	if (stream_read(fd, buf, BUFFER_SIZE) == -1) {
		printe(true, "read failed");
		stream_close(fd);
		return;
	}

	printf("testing ANYMARK atmark with one marked message at the front of the queue\n");
	atmark_check(fd, ANYMARK, 1, 0);

	printf("testing LASTMARK atmark with one marked message at the front of the queue\n");
	atmark_check(fd, LASTMARK, 1, 0);

	printf("reading second marked message\n");
	if (stream_read(fd, buf, BUFFER_SIZE) == -1) {
		printe(true, "read failed");
		stream_close(fd);
		return;
	}

	printf("reading second regular message\n");
	if (stream_read(fd, buf, BUFFER_SIZE) == -1) {
		printe(true, "read failed");
		stream_close(fd);
		return;
	}

	printf("testing ANYMARK atmark on empty queue\n");
	atmark_check(fd, ANYMARK, 0, 0);

	printf("testing LASTMARK atmark on empty queue\n");
	atmark_check(fd, LASTMARK, 0, 0);

	stream_close(fd);
}

staticf boolean
atmark_check (fd, arg, ret_val, expected_errno)
	int	fd;
	int	arg;
	int	ret_val, expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_ATMARK, (char *)arg)) {
	case 1:
		if (ret_val == 1)
			return printok("atmark returned 1 as expected");
		return printe(false, "atmark succeeded when expecting %d", ret_val);
	case 0:
		if (ret_val == 0)
			return printok("atmark returned 0 as expected");
		return printe(false, "atmark returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == expected_errno)
				return printok("atmark failed as expected with errno %d", errno);
			return printe(true, "atmark failed incorrectly, expected errno %d", expected_errno);
		}
		return printe(true, "atmark failed");
	default:
		return printe(false, "atmark returned %d when expecting %d", i1, ret_val);
	}
}

static boolean
send_marked_msg (fd)
	int	fd;
{
	struct strioctl	stri;
	eblk_t	* ep;

	stri.ic_cmd = ECHO_GENMSG;
	stri.ic_len = BUFFER_SIZE + sizeof(eblk_t);
	stri.ic_timout = -1;
	stri.ic_dp = buf;
	ep = (eblk_t *)buf;
	ep->eb_type = M_DATA;
	ep->eb_flag = MSGMARK;
	ep->eb_len = BUFFER_SIZE;
	if (stream_ioctl(fd, I_STR, &stri) == -1)
		return printe(true, "I_STR to send marked message failed");
	return printok("marked message sent");
}
