static char sccsid[] = "@(#)67	1.1  src/bos/usr/sbin/pse/utils/sthd/plinkt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:48";
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
 ** plinkt.c 2.1, last change 11/14/90
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
staticf	int	plink_test(   int fd, int arg_fd, int ret_val, int expected_errno   );
staticf	boolean	punlink_test(   int fd, int muxid, int ret_val, int expected_errno   );

void
plinkt (state)
	int	state;
{
	if (state & (ERROR | HANGUP)) {
		printok("Cannot test persistent links in ERROR or HANGUP state");
		return;
	}

	printf("Testing persistent links\n");

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
	int	echo_fd;
	int	mux_fd;
	int	muxid;

	printf("testing plink and punlink with various bogus parameters\n");
	if ((mux_fd = stream_open("/dev/tmux0", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux0 failed");
		return;
	}

	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		printe(true, "stream_open of /dev/echo failed");
		return;
	}

	printf("testing plink with bogus fd arg\n");
	plink_test(mux_fd, -1, -1, EBADF) != -1;

	printf("testing plink with non-multiplexor driver\n");
	plink_test(echo_fd, mux_fd, -1, EINVAL);

	printf("testing punlink with bogus muxid\n");
	punlink_test(mux_fd, 0, -1, EINVAL);

	printf("testing punlink with no lower streams attached\n");
	punlink_test(mux_fd, -1, 0, 0);

	printf("doing simple plink\n");
	if ((muxid = plink_test(mux_fd, echo_fd, 0, 0)) != -1) {
		printf("attempting push onto lower stream\n");
		if (stream_ioctl(echo_fd, I_PUSH, "pass") == -1  &&  errno == EINVAL)
			printf("push failed as expected with errno %d\n", EINVAL);
		else
			printe(true, "push failed incorrectly");

		printf("attempting second plink with same lower stream\n");
		plink_test(mux_fd, echo_fd, -1, EINVAL);

		printf("attempting I_UNLINK on persistent link\n");
		unlink_test(mux_fd, muxid, -1, EINVAL);

		printf("doing punlink\n");
		punlink_test(mux_fd, muxid, 0, 0);

		printf("doing simple link\n");
		if ((muxid = link_test(mux_fd, echo_fd, 0, 0)) != -1) {
			printf("attempting I_PUNLINK on regular link\n");
			punlink_test(mux_fd, muxid, -1, EINVAL);
			printf("doing unlink\n");
			unlink_test(mux_fd, muxid, 0, 0);
		}
	}

	stream_close(echo_fd);
	stream_close(mux_fd);
}

staticf void
test1 ()
{
	char	buf[128];
	int	mux_fd, echo_fd, muxid;

	printf("testing simple plink\n");
	if ((mux_fd = stream_open("/dev/tmux0", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux0 failed");
		return;
	}
	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		stream_close(mux_fd);
		printe(true, "stream_open of /dev/echo failed");
		return;
	}
	if ((muxid = plink_test(mux_fd, echo_fd, 0, 0)) != -1) {
		printf("testing write/read thru mux\n");
		if (stream_write(mux_fd, buf, sizeof(buf)) == sizeof(buf)) {
			if (stream_read(mux_fd, buf, sizeof(buf)) == sizeof(buf))
				printf("write and read successful\n");
			else
				printe(true, "read failed");
		} else
			printe(true, "write failed");
		printf("testing simple valid punlink\n");
		punlink_test(mux_fd, muxid, 0, 0);
	}
	stream_close(echo_fd);
	stream_close(mux_fd);
}

staticf void
test2 ()
{
	char	buf[256];
	int	mux_fd, echo_fd, muxid;

	printf("testing simple plink and closing lower stream fd\n");
	if ((mux_fd = stream_open("/dev/tmux0", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux0 failed");
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
	if ((muxid = plink_test(mux_fd, echo_fd, 0, 0)) == -1) {
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

	printf("punlinking\n");
	punlink_test(mux_fd, muxid, 0, 0);
	stream_close(mux_fd);
}

staticf void
test3 ()
{
	char	buf[50];
	int	mux_fd, echo_fd, muxid;

	printf("testing lower stream after plink and punlink\n");
	if ((mux_fd = stream_open("/dev/tmux0", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux0 failed");
		return;
	}
	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		stream_close(mux_fd);
		printe(true, "stream_open of /dev/echo failed");
		return;
	}

	printf("plinking mux and lower stream\n");
	if ((muxid = plink_test(mux_fd, echo_fd, 0, 0)) == -1) {
bumr:;
		stream_close(echo_fd);
		stream_close(mux_fd);
		return;
	}
	printf("doing punlink\n");
	if (!punlink_test(mux_fd, muxid, 0, 0))
		goto bumr;

	printf("testing write/read on detached lower stream\n");
	if (stream_write(echo_fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (stream_read(echo_fd, buf, sizeof(buf)) == sizeof(buf))
			printf("write and read successful\n");
		else
			printe(true, "read failed");
	} else
		printe(true, "write failed");

	printf("doing plink again\n");
	if ((muxid = plink_test(mux_fd, echo_fd, 0, 0)) == -1)
		goto bumr;
	printf("closing mux\n");
	stream_close(mux_fd);

	printf("testing write/read on lower stream\n");
	if (stream_write(echo_fd, buf, sizeof(buf)) == -1)
		printf("write failed as expected\n");
	else
		printe(false, "write successful");
	printf("reopening /dev/tmux0 in order to do punlink\n");
	if ((mux_fd = stream_open("/dev/tmux0", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux0 failed");
		return;
	}
	printf("doing punlink with reopened mux stream\n");
	punlink_test(mux_fd, muxid, 0, 0);

	printf("testing write/read on detached lower stream\n");
	if (stream_write(echo_fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (stream_read(echo_fd, buf, sizeof(buf)) == sizeof(buf))
			printf("write and read successful\n");
		else
			printe(true, "read failed");
	} else
		printe(true, "write failed");

	stream_close(mux_fd);
	stream_close(echo_fd);
}

staticf void
test4 ()
{
	char	buf[128];
	int	mux_fd, echo_fd, muxid, fd;

	printf("testing second stream to mux\n");
	if ((mux_fd = stream_open("/dev/tmux0", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux0 failed");
		return;
	}
	if ((echo_fd = stream_open("/dev/echo", 2)) == -1) {
		stream_close(mux_fd);
		printe(true, "stream_open of /dev/echo failed");
		return;
	}
	printf("plinking mux and lower stream\n");
	if ((muxid = plink_test(mux_fd, echo_fd, 0, 0)) == -1) {
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

	printf("closing original mux stream\n");
	stream_close(mux_fd);

	printf("testing write/read on new stream\n");
	if (stream_write(fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (stream_read(fd, buf, sizeof(buf)) == sizeof(buf))
			printf("write and read successful\n");
		else
			printe(true, "read failed");
	} else
		printe(true, "write failed");

	printf("attempting punlink on second mux stream\n");
	punlink_test(fd, muxid, -1, EINVAL);
	stream_close(fd);

	printf("reopening mux stream to do punlink\n");
	if ((mux_fd = stream_open("/dev/tmux0", 2)) == -1) {
		printe(true, "stream_open of /dev/tmux0 failed");
		return;
	}
	printf("doing punlink\n");
	punlink_test(mux_fd, muxid, 0, 0);

	stream_close(mux_fd);
	stream_close(echo_fd);
}

staticf int
plink_test (fd, arg_fd, ret_val, expected_errno)
	int	fd;
	int	arg_fd;
	int	ret_val;
	int	expected_errno;
{
	int	muxid;

#ifdef NETWARE
	if (arg_fd != -1) {
		arg_fd = stream_ioctl(arg_fd, I_GETSTREAMID, 0);
		if (arg_fd == -1) {
			printe(true, "couldn't get NetWare stream handle");
			return -1;
		}
	}
#endif
	if ((muxid = stream_ioctl(fd, I_PLINK, arg_fd)) == -1) {
		if (ret_val != -1) {
			printe(true, "plink failed");
			return -1;
		}
		if (errno != expected_errno) {
			printe(true, "plink failed incorrectly, expecting errno %d", expected_errno);
			return -1;
		}
		printf("plink failed as expected with errno %d\n", errno);
		return -1;
	}
	if (ret_val == -1)
		printe(false, "plink successful");
	else
		printf("plink successful\n");
	return muxid;
}

staticf boolean
punlink_test (fd, muxid, ret_val, expected_errno)
	int	fd;
	int	muxid;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_PUNLINK, muxid)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "punlink failed");
		if (errno != expected_errno)
			return printe(true, "punlink failed incorrectly, expecting errno %d", expected_errno);
		return printok("punlink failed as expected with errno %d", errno);
	case 0:
		if (ret_val != 0)
			return printe(false, "punlink successful");
		return printok("punlink successful");
	default:
		return printe(false, "punlink returned %d when expecting %d", i1, ret_val);
	}
}
