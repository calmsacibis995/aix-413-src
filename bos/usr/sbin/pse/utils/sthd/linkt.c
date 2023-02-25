static char sccsid[] = "@(#)61	1.1  src/bos/usr/sbin/pse/utils/sthd/linkt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:38";
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
 ** linkt.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include "sthd.h"

#ifndef errno
extern	int	errno;
#endif

staticf	void	test_bad_values(   void   );
staticf	void	test1(   void   );
staticf	void	test2(   void   );
staticf	void	test3(   void   );
staticf	void	test4(   void   );
	int	link_test(   int fd, int arg_fd, int ret_val, int expected_errno   );
	boolean	unlink_test(   int fd, int muxid, int ret_val, int expected_errno   );

void
linkt (state)
	int	state;
{
	if (state & (ERROR | HANGUP)) {
		printf("Cannot test multiplexing ioctls in ERROR or HANGUP state");
		return;
	}

	printf("Testing multiplexing ioctls\n");
	test_bad_values();
	PAUSE_HERE();

	test1();
	PAUSE_HERE();

	test2();
	PAUSE_HERE();

	test3();
	PAUSE_HERE();

	test4();
}

staticf void
test_bad_values ()
{
	int	mux_fd, echo_fd;

	printf("testing link and unlink with various bogus parameters\n");
	if ((mux_fd = stream_open("/dev/tmux", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux failed");
		return;
	}

	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		printe(true, "stream_open of /dev/echo failed");
		return;
	}

	printf("testing link with bogus fd arg\n");
	link_test(mux_fd, -1, -1, EBADF) != -1;

	printf("testing link with non-multiplexor driver\n");
	link_test(echo_fd, mux_fd, -1, EINVAL);

	printf("testing unlink with bogus muxid\n");
	unlink_test(mux_fd, 0, -1, EINVAL);

	printf("testing unlink with no lower streams attached\n");
	unlink_test(mux_fd, -1, 0, 0);

	printf("doing simple link\n");
	if (link_test(mux_fd, echo_fd, 0, 0) != -1) {
		printf("attempting push onto lower stream\n");
		if (stream_ioctl(echo_fd, I_PUSH, "pass") == -1  &&  errno == EINVAL)
			printf("push failed as expected with errno %d\n", EINVAL);
		else
			printe(true, "push failed incorrectly");
		printf("attempting second link with same lower stream\n");
		link_test(mux_fd, echo_fd, -1, EINVAL);
	}

	stream_close(echo_fd);
	stream_close(mux_fd);
}

staticf void
test1 ()
{
	char	buf[256];
	int	mux_fd, echo_fd, muxid;

	printf("testing simple link\n");
	if ((mux_fd = stream_open("/dev/tmux", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux failed");
		return;
	}
	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		stream_close(mux_fd);
		printe(true, "stream_open of /dev/echo failed");
		return;
	}
	if ((muxid = link_test(mux_fd, echo_fd, 0, 0)) != -1) {
		printf("testing write/read thru mux\n");
		if (stream_write(mux_fd, buf, sizeof(buf)) == sizeof(buf)) {
			if (stream_read(mux_fd, buf, sizeof(buf)) == sizeof(buf))
				printf("write and read successful\n");
			else
				printe(true, "read failed");
		} else
			printe(true, "write failed");
		printf("testing simple valid unlink\n");
		unlink_test(mux_fd, muxid, 0, 0);
	}
	stream_close(echo_fd);
	stream_close(mux_fd);
}

staticf void
test2 ()
{
	char	buf[200];
	int	mux_fd, echo_fd, muxid;

	printf("testing simple link and closing lower stream fd\n");
	if ((mux_fd = stream_open("/dev/tmux", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux failed");
		return;
	}
	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		stream_close(mux_fd);
		printe(true, "stream_open of /dev/echo failed");
		return;
	}
	printf("pushing pass module on lower stream\n");
	if (stream_ioctl(echo_fd, I_PUSH, "pass") == -1) {
		printe(true, "push of 'pass' failed");
		goto bumr;
	}
	if ((muxid = link_test(mux_fd, echo_fd, 0, 0)) == -1) {
bumr:;
		stream_close(echo_fd);
		stream_close(mux_fd);
		return;
	}
	printf("closing lower stream\n");
	stream_close(echo_fd);
	printf("testing write/read thru mux\n");
	if (stream_write(mux_fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (stream_read(mux_fd, buf, sizeof(buf)) == sizeof(buf))
			printf("write and read successful\n");
		else
			printe(true, "read failed");
	} else
		printe(true, "write failed");

	printf("unlinking\n");
	unlink_test(mux_fd, muxid, 0, 0);
	stream_close(mux_fd);
}

staticf void
test3 ()
{
	char	buf[400];
	int	mux_fd, echo_fd, muxid;

	printf("testing lower stream after link and unlink\n");
	if ((mux_fd = stream_open("/dev/tmux", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux failed");
		return;
	}
	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		stream_close(mux_fd);
		printe(true, "stream_open of /dev/echo failed");
		return;
	}

	printf("linking mux and lower stream\n");
	if ((muxid = link_test(mux_fd, echo_fd, 0, 0)) == -1) {
bumr:;
		stream_close(echo_fd);
		stream_close(mux_fd);
		return;
	}
	printf("unlinking\n");
	if (!unlink_test(mux_fd, muxid, 0, 0))
		goto bumr;

	printf("testing write/read on lower stream\n");
	if (stream_write(echo_fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (stream_read(echo_fd, buf, sizeof(buf)) == sizeof(buf))
			printf("write and read successful\n");
		else
			printe(true, "read failed");
	} else
		printe(true, "write failed");

	printf("linking again\n");
	if ((muxid = link_test(mux_fd, echo_fd, 0, 0)) == -1)
		goto bumr;
	printf("closing mux\n");
	stream_close(mux_fd);
	printf("testing write/read on lower stream\n");
	if (stream_write(echo_fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (stream_read(echo_fd, buf, sizeof(buf)) == sizeof(buf))
			printf("write and read successful\n");
		else
			printe(true, "read failed");
	} else
		printe(true, "write failed");
	stream_close(echo_fd);
}

staticf void
test4 ()
{
	char	buf[128];
	int	mux_fd, echo_fd, muxid, fd;

	printf("testing second stream to mux\n");
	if ((mux_fd = stream_open("/dev/tmux", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux failed");
		return;
	}
	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		stream_close(mux_fd);
		printe(true, "stream_open of /dev/echo failed");
		return;
	}
	printf("linking mux and lower stream\n");
	if ((muxid = link_test(mux_fd, echo_fd, 0, 0)) == -1) {
bumr:;
		stream_close(echo_fd);
		stream_close(mux_fd);
		return;
	}
	printf("opening additional stream to mux\n");
	if ((fd = stream_open("/dev/tmux", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux failed");
		goto bumr;
	}

	printf("testing write/read on new stream\n");
	if (stream_write(fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (stream_read(fd, buf, sizeof(buf)) == sizeof(buf))
			printf("write and read successful\n");
		else
			printe(true, "read failed");
	} else
		printe(true, "write failed");
	stream_close(fd);
	stream_close(mux_fd);
	stream_close(echo_fd);
}

int
link_test (fd, arg_fd, ret_val, expected_errno)
	int	fd;
	int	arg_fd;
	int	ret_val;
	int	expected_errno;
{
	int	muxid;

	if ((muxid = stream_ioctl(fd, I_LINK, arg_fd)) == -1) {
		if (ret_val != -1) {
			printe(true, "link failed");
			return -1;
		}
		if (errno != expected_errno) {
			printe(true, "link failed incorrectly, expecting errno %d", expected_errno);
			return -1;
		}
		printf("link failed as expected with errno %d\n", errno);
		return -1;
	}
	if (ret_val == -1)
		printe(false, "link successful");
	else
		printf("link successful\n");
	return muxid;
}

boolean
unlink_test (fd, muxid, ret_val, expected_errno)
	int	fd;
	int	muxid;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_UNLINK, muxid)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "unlink failed");
		if (errno != expected_errno)
			return printe(true, "unlink failed incorrectly, expecting errno %d", expected_errno);
		return printok("unlink failed as expected with errno %d", errno);
	case 0:
		if (ret_val != 0)
			return printe(false, "unlink successful");
		return printok("unlink successful");
	default:
		return printe(false, "unlink returned %d when expecting %d", i1, ret_val);
	}
}
